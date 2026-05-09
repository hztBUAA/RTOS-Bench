#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Deploy and validate OneOS phytium_pi_out.out through the jump host."""

import os
import re
import sys
import time

from deploy_via_jumphost import JUMPHOST, connect_jumphost, read_until, send_line


BOARD_IP = "192.168.31.205"
TELNET_PORT = 23
REMOTE_MODULE = "/user/phytium_pi_out.out"
LOCAL_MODULE = r"C:\OneOSStudio\workspace\phytium_pi_out\out\phytium_pi_out.out"
PROMPT_RE = r"(sh\s+/user>|msh\s*/?>)\s*$"


def open_oneos_telnet(ssh):
    channel = ssh.get_transport().open_session()
    channel.get_pty(width=180, height=60)
    channel.exec_command(f"telnet {BOARD_IP} {TELNET_PORT}")
    output, matched = read_until(
        channel,
        [PROMPT_RE, r"login:", r"password:", r"Connection closed", r"Unable to connect"],
        timeout=25,
    )
    if "Connection closed" in output or "Unable to connect" in output:
        channel.close()
        raise RuntimeError("OneOS telnet connection refused")
    if re.search(r"login:", output, re.I):
        send_line(channel, "root")
        output, matched = read_until(channel, [r"password:", PROMPT_RE], timeout=8)
    if re.search(r"password:", output, re.I):
        send_line(channel, "root")
        output, matched = read_until(channel, [PROMPT_RE], timeout=10)
    if not matched and not re.search(PROMPT_RE, output, re.I | re.M):
        channel.close()
        raise RuntimeError("OneOS telnet did not reach shell prompt")
    return channel


def run_command(channel, log, command, done_regex, timeout):
    print(f"\n>>> {command}", flush=True)
    log.write(f"\n>>> {command}\n")
    log.flush()
    send_line(channel, command)
    output = ""
    deadline = time.time() + timeout
    pattern = re.compile(done_regex, re.I | re.M)
    while time.time() < deadline:
        if channel.recv_ready():
            chunk = channel.recv(4096).decode("utf-8", errors="replace")
            output += chunk
            print(chunk, end="", flush=True)
            log.write(chunk)
            log.flush()
            if pattern.search(output):
                return output, True
        else:
            time.sleep(0.1)
    return output, False


def main():
    if not os.path.exists(LOCAL_MODULE):
        raise FileNotFoundError(LOCAL_MODULE)

    batch = time.strftime("%Y%m%d_%H%M%S")
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
    out_dir = os.path.join(repo_root, "utils", "remote-test", "logs",
                           f"entry-unification-oneos-feiteng-{batch}")
    os.makedirs(out_dir, exist_ok=True)
    raw_log = os.path.join(out_dir, "terminal.log")
    summary = os.path.join(out_dir, "SUMMARY.md")
    jump_tmp = f"/tmp/phytium_pi_out_{batch}.out"

    ssh = connect_jumphost()
    try:
        print(f"[1/3] upload local module to jump host: {jump_tmp}")
        sftp = ssh.open_sftp()
        sftp.put(LOCAL_MODULE, jump_tmp)
        sftp.chmod(jump_tmp, 0o644)
        sftp.close()

        print("[2/3] upload module to OneOS FTP")
        ftp_cmd = (
            f"curl -T {jump_tmp} 'ftp://{BOARD_IP}{REMOTE_MODULE}' "
            "--connect-timeout 10 --max-time 240 --fail --show-error"
        )
        stdin, stdout, stderr = ssh.exec_command(ftp_cmd, timeout=300)
        ftp_out = stdout.read().decode("utf-8", errors="replace")
        ftp_err = stderr.read().decode("utf-8", errors="replace")
        ftp_code = stdout.channel.recv_exit_status()
        if ftp_out:
            print(ftp_out)
        if ftp_err:
            print(ftp_err)
        if ftp_code != 0:
            raise RuntimeError(f"OneOS FTP upload failed: {ftp_code}")

        print("[3/3] telnet validation")
        channel = open_oneos_telnet(ssh)
        results = []
        try:
            with open(raw_log, "w", encoding="utf-8") as log:
                log.write(f"# OneOS validation batch={batch} module={REMOTE_MODULE}\n")
                commands = [
                    ("list-before", "list_lmodule", r"sh\s+/user>", 20, True),
                    ("unload", f"unld {REMOTE_MODULE}", r"sh\s+/user>", 20, True),
                    ("load", f"ld {REMOTE_MODULE}", r"sh\s+/user>", 60, True),
                    ("help", "rtbench --help", r"RTOS-Bench|USAGE:|Usage:", 45, True),
                    ("schedule", "rtbench test-schedule --cycles 1 --util-start 30 --util-end 30 --util-step 30", r"Final Score:", 1800, True),
                    ("bad-command", "rtbench bad-command", r"Unknown command|Unknown option|USAGE:", 45, True),
                ]
                for name, cmd, done, timeout, required in commands:
                    output, ok = run_command(channel, log, cmd, done, timeout)
                    if name == "load":
                        ok = ok and "Error" not in output and "fail" not in output.lower()
                    results.append((name, cmd, ok, required))
        finally:
            try:
                send_line(channel, "exit")
            except Exception:
                pass
            channel.close()
    finally:
        ssh.exec_command(f"rm -f {jump_tmp}")
        ssh.close()

    all_required_ok = all(ok for _, _, ok, required in results if required)
    with open(summary, "w", encoding="utf-8") as fh:
        fh.write("# Entry Unification Validation: oneos-feiteng\n\n")
        fh.write(f"- Board IP: `{BOARD_IP}:{TELNET_PORT}`\n")
        fh.write(f"- Local module: `{LOCAL_MODULE}`\n")
        fh.write(f"- Remote module: `{REMOTE_MODULE}`\n")
        fh.write(f"- Jump host: `{JUMPHOST['host']}:{JUMPHOST['port']}`\n\n")
        for name, cmd, ok, _ in results:
            fh.write(f"- {'PASS' if ok else 'FAIL'} {name}: `{cmd}`\n")
    print(f"SUMMARY={summary}")
    return 0 if all_required_ok else 2


if __name__ == "__main__":
    sys.exit(main())
