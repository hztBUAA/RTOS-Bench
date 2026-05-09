#!/usr/bin/env python3
"""Run several SylixOS Feiteng commands in one telnet shell session."""

from __future__ import annotations

import argparse
import importlib.util
import os
import re
import sys
import time
from pathlib import Path


THIS_FILE = Path(__file__).resolve()
REPO_ROOT = THIS_FILE.parents[4]
DEPLOY_HELPER = REPO_ROOT / "utils" / "remote-test" / "deploy_via_jumphost.py"
PROMPT_RE = re.compile(r"(\[root@sylixos:.*\]#|sh\s+/\w+>)\s*$", re.I | re.M)


def load_deploy_helper():
	spec = importlib.util.spec_from_file_location("deploy_via_jumphost", DEPLOY_HELPER)
	if spec is None or spec.loader is None:
		raise RuntimeError(f"failed to load {DEPLOY_HELPER}")
	module = importlib.util.module_from_spec(spec)
	spec.loader.exec_module(module)
	return module


def log_line(handle, text: str) -> None:
	line = f"[{time.strftime('%Y-%m-%d %H:%M:%S')}] {text}"
	handle.write(line + "\n")
	handle.flush()
	print(line, flush=True)


def drain(channel, log, duration=0.2):
	time.sleep(duration)
	while channel.recv_ready():
		chunk = channel.recv(8192).decode("utf-8", errors="replace")
		log.write(chunk)
		log.flush()


def wait_prompt(channel, log, timeout_sec: int) -> int:
	output = ""
	deadline = time.time() + timeout_sec
	while time.time() < deadline:
		if channel.recv_ready():
			chunk = channel.recv(8192).decode("utf-8", errors="replace")
			output += chunk
			log.write(chunk)
			log.flush()
			if PROMPT_RE.search(output):
				return 0
		else:
			if channel.exit_status_ready():
				return 2
			time.sleep(0.1)
	return 124

def wait_command_done(channel, log, command: str, timeout_sec: int) -> int:
	output = ""
	echo_end = -1
	deadline = time.time() + timeout_sec
	while time.time() < deadline:
		if channel.recv_ready():
			chunk = channel.recv(8192).decode("utf-8", errors="replace")
			output += chunk
			log.write(chunk)
			log.flush()
			if echo_end < 0:
				echo_pos = output.find(command)
				if echo_pos >= 0:
					echo_end = echo_pos + len(command)
			if echo_end >= 0 and PROMPT_RE.search(output[echo_end:]):
				return 0
		else:
			if channel.exit_status_ready():
				return 2
			time.sleep(0.1)
	return 124


def run_sequence(deploy, commands: list[str], log_path: Path, timeout_sec: int) -> int:
	board = deploy.BOARDS["feiteng"]
	ssh = None
	channel = None
	log_path.parent.mkdir(parents=True, exist_ok=True)
	with log_path.open("w", encoding="utf-8", errors="replace") as log:
		log_line(log, f"CONNECT feiteng {board['ip']}:{board['telnet_port']}")
		for idx, command in enumerate(commands, 1):
			log_line(log, f"COMMAND[{idx}] {command}")
		log_line(log, f"TIMEOUT_PER_COMMAND {timeout_sec}s")
		try:
			last_open_error = None
			for attempt in range(1, 5):
				try:
					log_line(log, f"TELNET_OPEN_ATTEMPT {attempt}")
					ssh = deploy.connect_jumphost()
					channel = deploy.open_board_telnet(ssh, board, timeout=45)
					last_open_error = None
					break
				except Exception as exc:
					last_open_error = exc
					log_line(log, f"TELNET_OPEN_FAILED attempt={attempt} error={exc!r}")
					if channel is not None:
						channel.close()
						channel = None
					if ssh is not None:
						ssh.close()
						ssh = None
					time.sleep(3.0)
			if channel is None:
				raise RuntimeError(f"telnet open failed after retries: {last_open_error!r}")

			drain(channel, log)
			for idx, command in enumerate(commands, 1):
				log_line(log, f"RUN[{idx}] {command}")
				drain(channel, log, duration=0.05)
				deploy.send_line(channel, command)
				status = wait_command_done(channel, log, command, timeout_sec)
				log_line(log, f"STATUS[{idx}] {status}")
				if status != 0:
					try:
						channel.send("\x03")
						drain(channel, log, duration=1.0)
					except Exception as exc:
						log_line(log, f"CTRL_C_FAILED {exc!r}")
					return status
			return 0
		except Exception as exc:
			log_line(log, f"STATUS ERROR {exc!r}")
			return 1
		finally:
			if channel is not None:
				try:
					deploy.send_line(channel, "exit")
				except Exception:
					pass
				channel.close()
			if ssh is not None:
				ssh.close()


def main(argv: list[str]) -> int:
	parser = argparse.ArgumentParser()
	parser.add_argument("--command", action="append", required=True)
	parser.add_argument("--log", required=True)
	parser.add_argument("--timeout", type=int, required=True)
	args = parser.parse_args(argv)

	if not os.environ.get("RTBENCH_JUMPHOST_PASSWORD"):
		os.environ["RTBENCH_JUMPHOST_PASSWORD"] = "rtbench"

	deploy = load_deploy_helper()
	return run_sequence(deploy, args.command, Path(args.log), args.timeout)


if __name__ == "__main__":
	raise SystemExit(main(sys.argv[1:]))
