#!/usr/bin/env python3
"""Run non-quick SylixOS test-all on real boards and fetch exported JSON."""

from __future__ import annotations

import argparse
import importlib.util
import json
import os
import re
import shlex
import sys
import threading
import time
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[3]
REMOTE_TEST_DIR = REPO_ROOT / "utils" / "remote-test"
DEPLOY_HELPER = REMOTE_TEST_DIR / "deploy_via_jumphost.py"
PROMPT_RE = re.compile(r"(\[root@sylixos:.*\]#|sh\s+/\w+>)\s*$", re.I | re.M)


def load_deploy_helper():
    spec = importlib.util.spec_from_file_location("deploy_via_jumphost", DEPLOY_HELPER)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"failed to load {DEPLOY_HELPER}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


deploy = load_deploy_helper()


def now_id() -> str:
    return time.strftime("%Y%m%d_%H%M%S")


def write_line(handle, text: str) -> None:
    handle.write(text + "\n")
    handle.flush()
    print(text, flush=True)


def run_command(channel, command: str, log_handle, timeout: int) -> str:
    write_line(log_handle, f"\n>>> {command}")
    deploy.send_line(channel, command)
    output = ""
    deadline = time.time() + timeout
    while time.time() < deadline:
        if channel.recv_ready():
            chunk = channel.recv(8192).decode("utf-8", errors="replace")
            output += chunk
            log_handle.write(chunk)
            log_handle.flush()
            if PROMPT_RE.search(output):
                return output
        else:
            time.sleep(0.1)
    write_line(log_handle, f"[TIMEOUT] command exceeded {timeout}s")
    return output


def fetch_ftp_file(ssh, board: dict, remote_path: str, local_path: Path) -> bool:
    local_path.parent.mkdir(parents=True, exist_ok=True)
    jump_tmp = f"/tmp/{local_path.name}"
    url = f"ftp://{board['ip']}{remote_path}"
    cmd = (
        f"curl {shlex.quote(url)} --user root:root --connect-timeout 10 "
        f"--max-time 180 --fail --show-error --output {shlex.quote(jump_tmp)}"
    )
    _, stdout, stderr = ssh.exec_command(cmd, timeout=240)
    exit_status = stdout.channel.recv_exit_status()
    if exit_status != 0:
        err = stderr.read().decode("utf-8", errors="replace")
        print(f"[FETCH] failed {remote_path}: {err}", flush=True)
        return False
    sftp = ssh.open_sftp()
    try:
        sftp.get(jump_tmp, str(local_path))
    finally:
        sftp.close()
        ssh.exec_command(f"rm -f {shlex.quote(jump_tmp)}")
    return local_path.exists() and local_path.stat().st_size > 0


def validate_json(local_path: Path) -> tuple[bool, str | None]:
    try:
        with local_path.open("r", encoding="utf-8-sig") as handle:
            json.load(handle)
        return True, None
    except Exception as exc:
        return False, str(exc)


def run_board(board_name: str, batch: str, log_dir: Path, timeout_sec: int) -> dict:
    board = deploy.BOARDS[board_name]
    binary = board["remote_dir"].rstrip("/") + "/" + board["remote_name"]
    output_remote = f"/apps/hzt/{board_name}_testall_full_{batch}.json"
    board_log = log_dir / f"full_testall_{board_name}_{batch}.log"
    local_json = log_dir / f"{board_name}_testall_full_{batch}.json"
    result = {
        "board": board_name,
        "ok": False,
        "command": f"{binary} test-all -o {output_remote}",
        "log": str(board_log),
        "json": str(local_json),
        "checks": {},
    }

    ssh = None
    channel = None
    with board_log.open("w", encoding="utf-8") as log_handle:
        try:
            ssh = deploy.connect_jumphost()
            channel = deploy.open_board_telnet(ssh, board, timeout=30)
            output = run_command(channel, result["command"], log_handle, timeout=timeout_sec)

            checks = {
                "comprehensive_suite": "[RTOS-Bench] Comprehensive Test Suite" in output,
                "not_quick": "Mode: QUICK" not in output,
                "schedule_final_score": "Final Score:" in output,
                "results_saved": "[RTOS-Bench] Results saved to:" in output,
                "no_unknown_option": "Unknown option" not in output,
                "no_timer_create_failed": "timer_create failed" not in output,
                "no_missing_library": "Can not find dependent library" not in output,
            }

            fetched = fetch_ftp_file(ssh, board, output_remote, local_json)
            checks["fetch_json"] = fetched
            json_ok, json_err = validate_json(local_json) if fetched else (False, "fetch failed")
            checks["json_parse"] = json_ok
            if json_err:
                result["json_error"] = json_err
            result["checks"] = checks
            result["ok"] = all(checks.values())
            write_line(log_handle, f"[BOARD_RESULT] {board_name}: {'PASS' if result['ok'] else 'FAIL'}")
            return result
        except Exception as exc:
            result["error"] = repr(exc)
            write_line(log_handle, f"[BOARD_RESULT] {board_name}: ERROR {exc!r}")
            return result
        finally:
            if channel:
                try:
                    deploy.send_line(channel, "exit")
                except Exception:
                    pass
                channel.close()
            if ssh:
                ssh.close()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--boards", required=True)
    parser.add_argument("--log-dir", required=True)
    parser.add_argument("--timeout-sec", type=int, default=18000)
    args = parser.parse_args()

    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")

    batch = now_id()
    log_dir = Path(args.log_dir)
    log_dir.mkdir(parents=True, exist_ok=True)
    boards = [item.strip() for item in args.boards.split(",") if item.strip()]
    print(f"[START] full test-all batch={batch} boards={','.join(boards)} timeout={args.timeout_sec}s", flush=True)

    results = {}
    lock = threading.Lock()

    def worker(board_name: str) -> None:
        board_result = run_board(board_name, batch, log_dir, args.timeout_sec)
        with lock:
            results[board_name] = board_result
            summary_path = log_dir / f"full_testall_summary_{batch}.json"
            summary_path.write_text(json.dumps(results, indent=2, ensure_ascii=False), encoding="utf-8")
            print(f"[BOARD_RESULT] {board_name}: {'PASS' if board_result.get('ok') else 'FAIL'}", flush=True)

    threads = [threading.Thread(target=worker, args=(board,), daemon=False) for board in boards]
    for thread in threads:
        thread.start()
    for thread in threads:
        thread.join()

    ok = all(results.get(board, {}).get("ok") for board in boards)
    print(f"[RESULT] {'PASS' if ok else 'FAIL'}", flush=True)
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
