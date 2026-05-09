#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Run entry-unification checks through the RTOS-Bench jump host."""

import argparse
import os
import re
import sys
import time

from deploy_via_jumphost import JUMPHOST, connect_jumphost, read_until, send_line


PROMPT_RE = r"(\[root@sylixos:[^\]]+\]#|sh\s+/\w+>|msh\s*/?>|vm\d+\s+\[\s+[^\]]+\s+\]#)\s*$"


BOARDS = {
    "loongson": ("SylixOS", "10.134.151.45", 3008, "root", ["root", "rtbench"], "/apps/hzt/loongson-rtos-bench", "/apps/hzt"),
    "orangepi-sylixos": ("SylixOS", "10.134.151.45", 3011, "root", ["root", "rtbench"], "/apps/hzt/orangepi-rtos-bench", "/apps/hzt"),
    "nezha-sylixos": ("SylixOS", "10.134.151.45", 3014, "root", ["root", "rtbench"], "/apps/hzt/nezha-rtos-bench", "/apps/hzt"),
    "gongkong-sylixos": ("SylixOS", "10.134.151.45", 3017, "root", ["root", "rtbench"], "/apps/hzt/gongkong-rtos-bench", "/apps/hzt"),
    "feiteng-sylixos": ("SylixOS", "10.134.151.45", 3020, "root", ["root", "rtbench"], "/apps/hzt/feiteng-rtos-bench", "/apps/hzt"),
    "oneos-feiteng": ("OneOS", "10.134.151.45", 3023, "root", ["root", "rtbench", ""], "rtbench", "/tmp"),
    "dongtu-orangepi": ("Dongtu", "192.168.31.207", 23, "", [""], "rtbench", "/nfsd"),
}

TELNET_FALLBACKS = {
    "loongson": [("10.134.151.45", 3008), ("192.168.31.200", 23)],
    "nezha-sylixos": [("10.134.151.45", 3014), ("192.168.31.202", 23)],
    "gongkong-sylixos": [("10.134.151.45", 3017), ("192.168.31.203", 23)],
}


def board_dict(name):
    os_name, ip, port, username, passwords, remote, writable = BOARDS[name]
    return {
        "name": name,
        "os": os_name,
        "ip": ip,
        "port": port,
        "username": username,
        "passwords": passwords,
        "remote": remote,
        "writable": writable,
        "telnet_fallbacks": TELNET_FALLBACKS.get(name, [(ip, port)]),
    }


def open_telnet(ssh, board, timeout=25):
    last_error = None
    for host, port in board.get("telnet_fallbacks", [(board["ip"], board["port"])]):
        try:
            channel = ssh.get_transport().open_session()
            channel.get_pty(width=180, height=60)
            channel.exec_command(f"telnet {host} {port}")
            board["ip"] = host
            board["port"] = port
            return finish_telnet_login(channel, board, timeout)
        except Exception as exc:
            last_error = exc
            try:
                channel.close()
            except Exception:
                pass
    raise RuntimeError(f"telnet refused all endpoints: {last_error}")


def finish_telnet_login(channel, board, timeout):
    output, matched = read_until(
        channel,
        [r"login:", r"username:", r"password:", PROMPT_RE,
         r"server is full", r"Connection closed", r"Unable to connect"],
        timeout=timeout,
    )
    if any(s in output for s in ("server is full", "Connection closed", "Unable to connect")):
        channel.close()
        raise RuntimeError("telnet refused connection")
    if re.search(r"(login|username):", output, re.I) and board["username"]:
        send_line(channel, board["username"])
        output, matched = read_until(channel, [r"password:", PROMPT_RE], timeout=8)
    if re.search(r"password:", output, re.I):
        for password in board["passwords"]:
            send_line(channel, password)
            output, matched = read_until(channel, [PROMPT_RE, r"login fail", r"password:"], timeout=10)
            if re.search(PROMPT_RE, output, re.I | re.M):
                return channel
        channel.close()
        raise RuntimeError("telnet login failed")
    if not matched and not re.search(PROMPT_RE, output, re.I | re.M):
        channel.close()
        raise RuntimeError("telnet did not reach shell prompt")
    return channel


def build_commands(board, batch, mode):
    remote = board["remote"]
    writable = board["writable"].rstrip("/") or "/"
    prefix = f"{writable}/entry_unify_{board['name']}_{batch}".replace("//", "/")
    smoke_timeout = 14400 if board["os"] == "Dongtu" else 7200
    schedule_timeout = 14400 if board["os"] == "Dongtu" else 7200
    commands = [
        ("help", f"{remote} --help", 45),
        ("export", f"{remote} export-result -o {prefix}_probe.json", 45),
        ("bad-command", f"{remote} bad-command", 45),
    ]
    if mode in ("smoke", "full"):
        commands.insert(
            2,
            ("test-all-smoke", f"{remote} test-all --quick --schedule-cycles 1 --util-start 30 --util-end 30 --util-step 30 --stress-job all-quick -o {prefix}_quick.json", smoke_timeout),
        )
    if mode == "full":
        commands.append(("schedule-acceptance", f"{remote} test-schedule --cycles 3", schedule_timeout))
    return prefix, commands


def classify(kind, output, reached_done):
    if not reached_done:
        return False, "completion marker not reached before timeout"
    if kind == "help":
        ok = "RTOS-Bench" in output or "USAGE:" in output or "Usage:" in output
        return ok, "help text found" if ok else "help text missing"
    if kind == "export":
        ok = "Write error" not in output and "Failed to save" not in output and "Failed to open" not in output
        return ok, "no export write error" if ok else "export failed"
    if kind.startswith("test-all"):
        ok = ("Results saved to:" in output and "Write error" not in output
              and "Failed to save" not in output and "Failed to open" not in output)
        return ok, "result json saved" if ok else "test-all result save missing or failed"
    if kind in ("schedule", "schedule-acceptance"):
        ok = "Final Score" in output
        return ok, "schedule output observed" if ok else "schedule output missing"
    if kind == "bad-command":
        ok = "Unknown command" in output or "Unknown option" in output or "USAGE:" in output
        return ok, "bad command rejected" if ok else "bad command was not rejected"
    return True, "not classified"


def command_done(kind, output):
    if kind == "help":
        return "EXAMPLES:" in output or "Show this help" in output
    if kind == "export":
        return "Write error" in output or "Failed to save" in output or "Failed to open" in output
    if kind.startswith("test-all"):
        return "Results saved to:" in output or "Failed to save" in output or "Failed to open" in output
    if kind in ("schedule", "schedule-acceptance"):
        return "Final Score" in output
    if kind == "bad-command":
        return "Unknown command" in output or "Unknown option" in output or "USAGE:" in output
    return False


def run_command(channel, log, kind, command, timeout):
    print(f"\n>>> {command}", flush=True)
    log.write(f"\n>>> {command}\n")
    log.flush()
    send_line(channel, command)
    output = ""
    if kind == "export":
        timeout = min(timeout, 5)
    deadline = time.time() + timeout
    while time.time() < deadline:
        if channel.recv_ready():
            chunk = channel.recv(4096).decode("utf-8", errors="replace")
            output += chunk
            print(chunk, end="", flush=True)
            log.write(chunk)
            log.flush()
            if command_done(kind, output):
                break
        else:
            time.sleep(0.1)
    reached_done = command_done(kind, output)
    ok, reason = classify(kind, output, reached_done)
    log.write(f"\n### RESULT {kind}: {'PASS' if ok else 'FAIL'} - {reason}\n")
    log.flush()
    return ok, reason


def run_board(board_name, log_root, mode, selected_cases):
    board = board_dict(board_name)
    batch = time.strftime("%Y%m%d_%H%M%S")
    out_dir = os.path.join(log_root, f"entry-unification-{board_name}-{batch}")
    os.makedirs(out_dir, exist_ok=True)
    raw_log = os.path.join(out_dir, "terminal.log")
    summary = os.path.join(out_dir, "SUMMARY.md")
    prefix, commands = build_commands(board, batch, mode)
    if selected_cases:
        wanted = set(selected_cases)
        commands = [item for item in commands if item[0] in wanted]
    results = []
    with open(raw_log, "w", encoding="utf-8") as log:
        log.write(f"# board={board_name} os={board['os']} ip={board['ip']} remote={board['remote']} mode={mode}\n")
        log.flush()
        ssh = connect_jumphost()
        channel = open_telnet(ssh, board)
        try:
            for kind, command, timeout in commands:
                ok, reason = run_command(channel, log, kind, command, timeout)
                results.append((kind, command, ok, reason))
        finally:
            try:
                send_line(channel, "exit")
            except Exception:
                pass
            channel.close()
            ssh.close()
    with open(summary, "w", encoding="utf-8") as fh:
        fh.write(f"# Entry Unification Validation: {board_name}\n\n")
        fh.write(f"- OS: `{board['os']}`\n")
        fh.write(f"- Board IP: `{board['ip']}:{board['port']}`\n")
        fh.write(f"- Remote executable/command: `{board['remote']}`\n")
        fh.write(f"- Writable prefix: `{prefix}`\n")
        fh.write(f"- Jump host: `{JUMPHOST['host']}:{JUMPHOST['port']}`\n")
        fh.write(f"- Mode: `{mode}`\n\n")
        for kind, command, ok, reason in results:
            fh.write(f"- {'PASS' if ok else 'FAIL'} {kind}: `{command}` ({reason})\n")
    return all(ok for _, _, ok, _ in results), out_dir


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("boards", nargs="+", choices=sorted(BOARDS.keys()))
    parser.add_argument("--mode", choices=["basic", "smoke", "full"], default="smoke")
    parser.add_argument("--case", action="append",
                        choices=["help", "export", "bad-command",
                                 "test-all-smoke", "schedule-acceptance"],
                        help="Run only the selected case; may be repeated")
    parser.add_argument("--log-root", default=os.path.join(os.path.dirname(__file__), "logs"))
    args = parser.parse_args()
    ok = True
    for board in args.boards:
        board_ok, out_dir = run_board(board, args.log_root, args.mode, args.case)
        print(f"\n[{board}] {'PASS' if board_ok else 'FAIL'} {out_dir}")
        ok = ok and board_ok
    return 0 if ok else 2


if __name__ == "__main__":
    sys.exit(main())
