import importlib.util
import os
import re
import sys
import time


SCRIPT_PATH = r"C:\Users\hzt\yihui-workspace\deploy_via_jumphost.py"
spec = importlib.util.spec_from_file_location("deploy_via_jumphost", SCRIPT_PATH)
d = importlib.util.module_from_spec(spec)
spec.loader.exec_module(d)


def fetch_ftp_file(ssh, board, remote_path, local_path):
    cmd = (
        f"curl 'ftp://{board['ip']}{remote_path}' --user root:root "
        "--connect-timeout 10 --max-time 120 --fail --show-error"
    )
    stdin, stdout, stderr = ssh.exec_command(cmd, timeout=150)
    data = stdout.read()
    err = stderr.read().decode("utf-8", errors="replace")
    status = stdout.channel.recv_exit_status()
    if status != 0:
        raise RuntimeError(f"fetch failed exit={status}: {err}")
    with open(local_path, "wb") as out:
        out.write(data)


def main():
    board = d.BOARDS["nezha"]
    log_dir = sys.argv[1]
    batch = sys.argv[2]
    os.makedirs(log_dir, exist_ok=True)

    remote_bin = f"{board['remote_dir']}{board['remote_name']}"
    remote_json = f"{board['remote_dir']}rtbench_nezha_testall_{batch}.json"
    local_log = os.path.join(log_dir, f"test_nezha_testall_{batch}.log")
    local_json = os.path.join(log_dir, f"rtbench_nezha_testall_{batch}.json")
    command = f"{remote_bin} test-all --quick -o {remote_json}"

    ssh = None
    channel = None
    exit_code = 1
    with open(local_log, "w", encoding="utf-8", newline="\n") as log:
        log.write(f"BOARD=nezha\n")
        log.write(f"REMOTE_BIN={remote_bin}\n")
        log.write(f"REMOTE_JSON={remote_json}\n")
        log.write(f"COMMAND={command}\n")
        log.write(f"BATCH={batch}\n\n")
        try:
            ssh = d.connect_jumphost()
            channel = d.open_board_telnet(ssh, board, timeout=20)

            prechecks = [
                ("chmod", f"chmod 755 {remote_bin}", None),
                ("refresh", "dlconfig refresh", None),
                ("help-top", f"{remote_bin} --help", "test-all"),
                ("help-test-all", f"{remote_bin} test-all --help", "Usage: rtbench test-all"),
                ("bad-test-all", f"{remote_bin} test-all --bad-option", "Unknown option"),
            ]
            for label, cmd, expected in prechecks:
                log.write(f"\n>>> [{label}] {cmd}\n")
                log.flush()
                d.send_line(channel, cmd)
                output, matched = d.read_until(
                    channel,
                    [d.PROMPT_RE],
                    timeout=45,
                    echo=True,
                    log_handle=log,
                )
                if not matched:
                    raise TimeoutError(f"precheck timeout: {label}")
                if expected and expected not in output:
                    raise RuntimeError(f"precheck missing marker {expected!r}: {label}")

            log.write(f"\n>>> [test-all] {command}\n")
            log.flush()
            d.send_line(channel, command)

            output = ""
            saved_seen = False
            deadline = time.time() + 3600
            while time.time() < deadline:
                if channel.recv_ready():
                    chunk = channel.recv(4096).decode("utf-8", errors="replace")
                    output += chunk
                    print(chunk, end="", flush=True)
                    log.write(chunk)
                    log.flush()
                    if "[RTOS-Bench] Results saved to:" in output:
                        saved_seen = True
                    if saved_seen and re.search(d.PROMPT_RE, output, re.I | re.M):
                        break
                else:
                    time.sleep(0.1)

            if not saved_seen:
                log.write("\n[FAIL] Results saved marker not seen\n")
                raise RuntimeError("test-all did not complete")

            fetch_ftp_file(ssh, board, remote_json, local_json)
            log.write(f"\n[PASS] test-all completed; JSON fetched to {local_json}\n")
            exit_code = 0
        except Exception as exc:
            log.write(f"\n[BLOCKED] {type(exc).__name__}: {exc}\n")
            print(f"[BLOCKED] {type(exc).__name__}: {exc}")
            exit_code = 2
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

    print(f"TEST_ALL_LOG={local_log}")
    if os.path.exists(local_json):
        print(f"TEST_ALL_JSON={local_json}")
    return exit_code


if __name__ == "__main__":
    sys.exit(main())
