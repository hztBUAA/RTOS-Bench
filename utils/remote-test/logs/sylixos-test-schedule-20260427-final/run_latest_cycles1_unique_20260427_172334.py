import importlib.util
import os
import re
import sys
import time

script_path = r"C:\Users\hzt\yihui-workspace\deploy_via_jumphost.py"
spec = importlib.util.spec_from_file_location("deploy_via_jumphost", script_path)
d = importlib.util.module_from_spec(spec)
spec.loader.exec_module(d)

board_name = sys.argv[1]
log_dir = sys.argv[2]
batch = sys.argv[3]
os.makedirs(log_dir, exist_ok=True)
board = d.BOARDS[board_name]
remote_path = f"{board['remote_dir']}{board['remote_name']}"
unique_path = f"{board['remote_dir']}{board['remote_name']}-latest-{batch}"
board_log = os.path.join(log_dir, f"test_latest_cycles1_{board_name}_{batch}.log")

bad_markers = [
    "timer_create failed",
    "skipped for quick schedule",
    "Can not find dependent library",
    "OS-version",
]
prechecks = [
    ("copy-latest", f"cp {remote_path} {unique_path}", None),
    ("chmod", f"chmod 755 {unique_path}", None),
    ("refresh", "dlconfig refresh", None),
    ("help-top", f"{unique_path} --help", "Usage:"),
    ("help-schedule", f"{unique_path} test-schedule --help", "Usage: rtbench test-schedule"),
    ("bad-command", f"{unique_path} test -schedule --help", "Unknown option or command"),
    ("bad-cycles", f"{unique_path} test-schedule --cycles abc", "Invalid cycles"),
]

ssh = None
channel = None
exit_code = 1
try:
    ssh = d.connect_jumphost()
    channel = d.open_board_telnet(ssh, board, timeout=20)
    with open(board_log, "w", encoding="utf-8", newline="\n") as log_handle:
        log_handle.write(f"BOARD={board_name}\n")
        log_handle.write(f"REMOTE_SOURCE={remote_path}\n")
        log_handle.write(f"REMOTE_UNIQUE={unique_path}\n")
        log_handle.write(f"BATCH={batch}\n\n")
        for label, command, expected in prechecks:
            log_handle.write(f"\n>>> [{label}] {command}\n")
            log_handle.flush()
            d.send_line(channel, command)
            output, matched = d.read_until(channel, [d.PROMPT_RE], timeout=30, echo=True, log_handle=log_handle)
            if not matched:
                log_handle.write(f"\n[FAIL] precheck timeout: {label}\n")
                raise TimeoutError(f"precheck timeout: {label}")
            if expected and expected not in output:
                log_handle.write(f"\n[FAIL] precheck missing marker {expected!r}: {label}\n")
                raise RuntimeError(f"precheck missing marker {expected!r}: {label}")

        command = f"{unique_path} test-schedule --cycles 1"
        log_handle.write(f"\n>>> [cycles1] {command}\n")
        log_handle.flush()
        d.send_line(channel, command)

        output = ""
        final_seen = False
        deadline = time.time() + 2400
        while time.time() < deadline:
            if channel.recv_ready():
                chunk = channel.recv(4096).decode("utf-8", errors="replace")
                output += chunk
                print(chunk, end="", flush=True)
                log_handle.write(chunk)
                log_handle.flush()
                if "Final Score" in output:
                    final_seen = True
                if final_seen and re.search(d.PROMPT_RE, output, re.I | re.M):
                    break
            else:
                time.sleep(0.1)

        marker_hits = [marker for marker in bad_markers if marker in output]
        if not final_seen:
            log_handle.write("\n[FAIL] Final Score not seen\n")
        if marker_hits:
            log_handle.write("\n[FAIL] Bad markers: " + ", ".join(marker_hits) + "\n")
        if final_seen and not marker_hits:
            log_handle.write("\n[PASS] cycles=1 completed without known bad markers\n")
            exit_code = 0
finally:
    if channel is not None:
        try:
            d.send_line(channel, "exit")
        except Exception:
            pass
        try:
            channel.close()
        except Exception:
            pass
    if ssh is not None:
        ssh.close()

print(f"BOARD_LOG={board_log}")
print(f"REMOTE_UNIQUE={unique_path}")
sys.exit(exit_code)
