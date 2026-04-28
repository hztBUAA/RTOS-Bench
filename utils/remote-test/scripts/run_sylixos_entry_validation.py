#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Run SylixOS entry-level validation through the jump host.

The script validates that the freshly deployed binaries do not accidentally
fall through to workload execution on help/unknown arguments, then runs a
bounded end-to-end test-all export and a schedule acceptance run.
"""

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


def check_output(name: str, output: str, must_have: list[str], must_not_have: list[str]) -> dict:
    missing = [pattern for pattern in must_have if pattern not in output]
    forbidden = [pattern for pattern in must_not_have if pattern in output]
    return {
        "name": name,
        "ok": not missing and not forbidden,
        "missing": missing,
        "forbidden": forbidden,
    }


def fetch_ftp_file(ssh, board: dict, remote_path: str, local_path: Path) -> bool:
    local_path.parent.mkdir(parents=True, exist_ok=True)
    jump_tmp = f"/tmp/{local_path.name}"
    url = f"ftp://{board['ip']}{remote_path}"
    cmd = (
        f"curl {shlex.quote(url)} --user root:root --connect-timeout 10 "
        f"--max-time 180 --fail --show-error --output {shlex.quote(jump_tmp)}"
    )
    stdin, stdout, stderr = ssh.exec_command(cmd, timeout=240)
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


def validate_json_file(local_path: Path) -> dict:
    try:
        with local_path.open("r", encoding="utf-8-sig") as handle:
            json.load(handle)
        return {"name": f"json_parse:{local_path.name}", "ok": True, "missing": [], "forbidden": []}
    except Exception as exc:
        return {
            "name": f"json_parse:{local_path.name}",
            "ok": False,
            "missing": [f"valid JSON ({exc})"],
            "forbidden": [],
        }


def validate_board(board_name: str, batch: str, log_dir: Path, schedule_cycles: int,
                   schedule_timeout_sec: int) -> dict:
    board = deploy.BOARDS[board_name]
    binary = board["remote_dir"].rstrip("/") + "/" + board["remote_name"]
    board_log = log_dir / f"entry_validation_{board_name}_{batch}.log"
    result = {
        "board": board_name,
        "ok": False,
        "checks": [],
        "fetched": [],
        "log": str(board_log),
    }

    ssh = None
    channel = None
    try:
        with board_log.open("w", encoding="utf-8") as log_handle:
            write_line(log_handle, f"[BOARD] {board_name}")
            write_line(log_handle, f"[BINARY] {binary}")
            ssh = deploy.connect_jumphost()
            channel = deploy.open_board_telnet(ssh, board, timeout=30)

            parser_checks = [
                ("top_help", f"{binary} --help", ["USAGE:"], ["[Phase 1]", "Final Score"]),
                ("test_all_help", f"{binary} test-all --help", ["Usage: rtbench test-all"], ["Comprehensive Test Suite"]),
                ("bad_split_schedule", f"{binary} test -schedule --help", ["ERROR: Unknown command 'test'"], ["[Phase 1]", "Final Score"]),
                ("bad_test_all_option", f"{binary} test-all --bad-option", ["[test-all] Unknown option"], ["Comprehensive Test Suite"]),
                ("bad_export_option", f"{binary} export-result --bad-option", ["[export-result] Unknown option"], []),
            ]
            for name, command, must_have, must_not_have in parser_checks:
                output = run_command(channel, command, log_handle, timeout=45)
                check = check_output(name, output, must_have, must_not_have)
                result["checks"].append(check)
                write_line(log_handle, f"[CHECK] {name}: {'PASS' if check['ok'] else 'FAIL'}")

            export_path = f"/apps/hzt/{board_name}_export_{batch}.json"
            output = run_command(
                channel,
                f"{binary} export-result -o {export_path}",
                log_handle,
                timeout=60,
            )
            check = check_output("export_result_command", output, ["Results exported"], ["Unknown option"])
            result["checks"].append(check)
            write_line(log_handle, f"[CHECK] export_result_command: {'PASS' if check['ok'] else 'FAIL'}")
            local_export = log_dir / f"{board_name}_export_{batch}.json"
            if fetch_ftp_file(ssh, board, export_path, local_export):
                result["fetched"].append(str(local_export))
                write_line(log_handle, f"[FETCH] {export_path} -> {local_export}")
                json_check = validate_json_file(local_export)
                result["checks"].append(json_check)
                write_line(log_handle, f"[CHECK] {json_check['name']}: {'PASS' if json_check['ok'] else 'FAIL'}")
            else:
                result["checks"].append({"name": "fetch_export_result", "ok": False})

            testall_path = f"/apps/hzt/{board_name}_testall_{batch}.json"
            testall_cmd = (
                f"{binary} test-all --quick --schedule-cycles 1 "
                f"--util-start 30 --util-end 30 --util-step 30 "
                f"--stress-job all-quick -o {testall_path}"
            )
            output = run_command(channel, testall_cmd, log_handle, timeout=3600)
            check = check_output(
                "test_all_quick_export",
                output,
                ["Comprehensive Test Suite", "Results saved to:"],
                ["Unknown option", "Failed to create test-all worker thread", "timer_create failed"],
            )
            result["checks"].append(check)
            write_line(log_handle, f"[CHECK] test_all_quick_export: {'PASS' if check['ok'] else 'FAIL'}")
            local_testall = log_dir / f"{board_name}_testall_{batch}.json"
            if fetch_ftp_file(ssh, board, testall_path, local_testall):
                result["fetched"].append(str(local_testall))
                write_line(log_handle, f"[FETCH] {testall_path} -> {local_testall}")
                json_check = validate_json_file(local_testall)
                result["checks"].append(json_check)
                write_line(log_handle, f"[CHECK] {json_check['name']}: {'PASS' if json_check['ok'] else 'FAIL'}")
            else:
                result["checks"].append({"name": "fetch_test_all_result", "ok": False})

            schedule_cmd = f"{binary} test-schedule --cycles {schedule_cycles}"
            output = run_command(channel, schedule_cmd, log_handle, timeout=schedule_timeout_sec)
            check = check_output(
                f"test_schedule_cycles_{schedule_cycles}",
                output,
                ["Final Score"],
                ["timer_create failed", "skipped for quick schedule", "Can not find dependent library"],
            )
            result["checks"].append(check)
            write_line(log_handle, f"[CHECK] test_schedule_cycles_{schedule_cycles}: {'PASS' if check['ok'] else 'FAIL'}")

            result["ok"] = all(check.get("ok") for check in result["checks"])
            write_line(log_handle, f"[BOARD_RESULT] {board_name}: {'PASS' if result['ok'] else 'FAIL'}")
    except Exception as exc:
        result["error"] = repr(exc)
        print(f"[BOARD_RESULT] {board_name}: ERROR {exc!r}", flush=True)
    finally:
        if channel:
            try:
                deploy.send_line(channel, "exit")
            except Exception:
                pass
            channel.close()
        if ssh:
            ssh.close()
    return result


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--boards", default="orangepi,gongkong,loongson")
    parser.add_argument("--log-dir", required=True)
    parser.add_argument("--schedule-cycles", type=int, default=3)
    parser.add_argument("--schedule-timeout-sec", type=int, default=7200)
    args = parser.parse_args()

    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    if hasattr(sys.stderr, "reconfigure"):
        sys.stderr.reconfigure(encoding="utf-8", errors="replace")

    batch = now_id()
    log_dir = Path(args.log_dir)
    log_dir.mkdir(parents=True, exist_ok=True)
    boards = [item.strip() for item in args.boards.split(",") if item.strip()]

    print(
        f"[START] batch={batch} boards={','.join(boards)} "
        f"cycles={args.schedule_cycles} schedule_timeout={args.schedule_timeout_sec}s",
        flush=True,
    )

    results = {}
    lock = threading.Lock()

    def worker(board_name: str) -> None:
        board_result = validate_board(
            board_name,
            batch,
            log_dir,
            args.schedule_cycles,
            args.schedule_timeout_sec,
        )
        with lock:
            results[board_name] = board_result
            summary_path = log_dir / f"entry_validation_summary_{batch}.json"
            summary_path.write_text(json.dumps(results, indent=2, ensure_ascii=False), encoding="utf-8")

    threads = [threading.Thread(target=worker, args=(board,), daemon=False) for board in boards]
    for thread in threads:
        thread.start()
    for thread in threads:
        thread.join()

    ok = all(results.get(board, {}).get("ok") for board in boards)
    summary_path = log_dir / f"entry_validation_summary_{batch}.json"
    summary_path.write_text(json.dumps(results, indent=2, ensure_ascii=False), encoding="utf-8")
    print(f"[SUMMARY] {summary_path}", flush=True)
    print(f"[RESULT] {'PASS' if ok else 'FAIL'}", flush=True)
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
