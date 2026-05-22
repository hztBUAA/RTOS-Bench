#!/usr/bin/env python3
"""Deploy and validate OneOS RTOS-Bench module through TFTP.

OneOS V1.5 uses a board-side TFTP pull instead of FTP upload:

    tftp_client 192.168.31.110 get phytium_pi_out.out /user/phytium_pi_out.out

The jump host is also the TFTP server (`192.168.31.110`), so this script first
copies the local module into `/tftp`, then drives the OneOS shell with a raw
socket. The raw socket is intentional: the system `telnet` client closes
immediately after the OneOS prompt in non-interactive pipelines.
"""

import argparse
import os
import socket
import sys
import time
from pathlib import Path

from deploy_via_jumphost import connect_jumphost


BOARD_IP = "192.168.31.205"
BOARD_TELNET_PORT = 23
TFTP_SERVER_IP = "192.168.31.110"
TFTP_DIR = "/tftp"
TFTP_NAME = "phytium_pi_out.out"
REMOTE_MODULE = "/user/phytium_pi_out.out"
LOCAL_MODULE = r"C:\OneOSStudio\workspace\phytium_pi_out\out\phytium_pi_out.out"
PROMPTS = [b"sh /user>", b"sh />"]


def strip_telnet_iac(data):
    out = bytearray()
    i = 0
    while i < len(data):
        if data[i] == 255 and i + 2 < len(data):
            i += 3
            continue
        out.append(data[i])
        i += 1
    return bytes(out).decode("utf-8", errors="replace")


def has_prompt(text):
    return "sh /user>" in text or "sh />" in text


def read_shell(sock, timeout=20, require_prompt=True, idle=0.8):
    deadline = time.time() + timeout
    last = time.time()
    chunks = []

    while time.time() < deadline:
        try:
            data = sock.recv(4096)
            if not data:
                break
            chunks.append(data)
            last = time.time()
            joined = b"".join(chunks)
            if any(prompt in joined for prompt in PROMPTS):
                if time.time() - last >= idle:
                    break
        except socket.timeout:
            if not require_prompt and chunks and time.time() - last >= idle:
                break
            joined = b"".join(chunks)
            if any(prompt in joined for prompt in PROMPTS):
                break

    return strip_telnet_iac(b"".join(chunks))


def run_shell_command(sock, log, name, command, timeout, required=()):
    print(f">>> {command}", flush=True)
    log.write(f"\n>>> {command}\n")
    log.flush()

    sock.sendall(command.encode("utf-8") + b"\r")
    output = read_shell(sock, timeout=timeout, require_prompt=True)
    print(output, end="" if output.endswith("\n") else "\n", flush=True)
    log.write(output)
    log.flush()

    lower = output.lower()
    ok = has_prompt(output)
    hard_failures = [
        "segmentation",
        "assert",
        "fatal",
        "exception",
        "load failed",
        "can't access",
        "command not found",
    ]
    if any(token in lower for token in hard_failures):
        ok = False
    if required and not any(token in output for token in required):
        ok = False

    log.write(f"\n### RESULT {name}: {'PASS' if ok else 'FAIL'}\n")
    log.flush()
    return name, command, ok, output[-2000:]


def upload_module_to_tftp(local_module):
    if not os.path.exists(local_module):
        raise FileNotFoundError(local_module)

    remote_path = f"{TFTP_DIR}/{TFTP_NAME}"
    ssh = connect_jumphost()
    try:
        sftp = ssh.open_sftp()
        sftp.put(local_module, remote_path)
        sftp.chmod(remote_path, 0o644)
        sftp.close()
        stdin, stdout, stderr = ssh.exec_command(
            f"ls -l {remote_path}; sha256sum {remote_path}", timeout=30
        )
        out = stdout.read().decode("utf-8", errors="replace")
        err = stderr.read().decode("utf-8", errors="replace")
        if out:
            print(out, end="" if out.endswith("\n") else "\n")
        if err:
            print(err, end="" if err.endswith("\n") else "\n")
    finally:
        ssh.close()


def open_oneos_socket(telnet_host, telnet_port, via_jumphost=True):
    ssh = None
    if via_jumphost:
        ssh = connect_jumphost()
        transport = ssh.get_transport()
        if transport is None:
            ssh.close()
            raise RuntimeError("jump host transport is not available")
        sock = transport.open_channel(
            "direct-tcpip",
            (telnet_host, telnet_port),
            ("127.0.0.1", 0),
        )
    else:
        sock = socket.socket()
        sock.settimeout(10)
        sock.connect((telnet_host, telnet_port))
    sock.settimeout(1)
    return ssh, sock


def main():
    parser = argparse.ArgumentParser(description="OneOS TFTP runtime validation")
    parser.add_argument("--local-module", default=LOCAL_MODULE)
    parser.add_argument("--case", choices=["smoke", "schedule", "schedule-default"], default="smoke")
    parser.add_argument("--skip-upload", action="store_true")
    parser.add_argument("--telnet-host", default=BOARD_IP)
    parser.add_argument("--telnet-port", type=int, default=BOARD_TELNET_PORT)
    parser.add_argument("--direct-telnet", action="store_true")
    args = parser.parse_args()

    batch = time.strftime("%Y%m%d_%H%M%S")
    repo_root = Path(__file__).resolve().parents[2]
    out_dir = repo_root / "utils" / "remote-test" / "logs" / f"oneos-tftp-runtime-{batch}"
    out_dir.mkdir(parents=True, exist_ok=True)
    raw_log = out_dir / "terminal.log"
    summary = out_dir / "SUMMARY.md"

    if not args.skip_upload:
        upload_module_to_tftp(args.local_module)

    commands = [
        ("pwd", "pwd", 10, ()),
        ("list-before", "list_lmodule", 30, ()),
        ("unload", f"unld {REMOTE_MODULE}", 30, ()),
        (
            "tftp-get",
            f"tftp_client {TFTP_SERVER_IP} get {TFTP_NAME} {REMOTE_MODULE}",
            300,
            ("err=0", TFTP_NAME),
        ),
        ("ls-user", "ls /user", 20, (TFTP_NAME,)),
        ("load", f"ld {REMOTE_MODULE}", 120, ("loaded", REMOTE_MODULE)),
        ("list-after", "list_lmodule", 30, (TFTP_NAME,)),
        ("help", "rtbench --help", 45, ("RTOS-Bench", "USAGE:", "Usage:")),
        ("list-workloads", "rtbench -L", 60, ("Available workloads", "stub", "busywait")),
        ("workload-stub", "rtbench -b stub -t 1", 180, ()),
        ("workload-busywait", "rtbench -b busywait -t 1", 180, ()),
    ]
    if args.case in ("smoke", "schedule"):
        commands.append(
            (
                "schedule-quick",
                "rtbench test-schedule --cycles 1 --util-start 30 --util-end 30 --util-step 30",
                1800,
                ("Final Score:",),
            )
        )
    elif args.case == "schedule-default":
        commands.append(
            (
                "schedule-default",
                "rtbench test-schedule",
                3600,
                ("Final Score:",),
            )
        )

    results = []
    ssh, sock = open_oneos_socket(
        args.telnet_host,
        args.telnet_port,
        via_jumphost=not args.direct_telnet,
    )
    try:
        with raw_log.open("w", encoding="utf-8") as log:
            banner = read_shell(sock, timeout=10, require_prompt=True)
            if not has_prompt(banner):
                sock.sendall(b"\r")
                banner += read_shell(sock, timeout=10, require_prompt=True)
            print(banner, end="" if banner.endswith("\n") else "\n")
            log.write(banner)
            if not has_prompt(banner):
                raise RuntimeError("OneOS telnet did not reach shell prompt")
            for item in commands:
                result = run_shell_command(sock, log, *item)
                results.append(result)
                if not result[2]:
                    break
    finally:
        sock.close()
        if ssh is not None:
            ssh.close()

    with summary.open("w", encoding="utf-8") as fh:
        fh.write("# OneOS TFTP Runtime Validation\n\n")
        fh.write(f"- Batch: `{batch}`\n")
        fh.write(f"- Board: `{args.telnet_host}:{args.telnet_port}`\n")
        fh.write(f"- TFTP source: `{TFTP_SERVER_IP}:{TFTP_DIR}/{TFTP_NAME}`\n")
        fh.write(f"- Remote module: `{REMOTE_MODULE}`\n")
        fh.write(f"- Local module: `{args.local_module}`\n")
        fh.write(f"- Log: `{raw_log}`\n\n")
        for name, command, ok, tail in results:
            fh.write(f"- {'PASS' if ok else 'FAIL'} {name}: `{command}`\n")
            if not ok:
                fh.write("\n```text\n")
                fh.write(tail.replace("```", "` ` `"))
                fh.write("\n```\n")

    print(f"SUMMARY={summary}")
    return 0 if all(item[2] for item in results) else 2


if __name__ == "__main__":
    sys.exit(main())
