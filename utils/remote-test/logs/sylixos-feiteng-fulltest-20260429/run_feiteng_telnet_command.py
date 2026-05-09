#!/usr/bin/env python3
"""Run one SylixOS Feiteng command through the rtbench jump host.

This is a per-run evidence helper kept beside the logs. It is intentionally
not part of the production remote-test tooling.
"""

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


def run_command(deploy, command: str, log_path: Path, timeout_sec: int) -> int:
	board = deploy.BOARDS["feiteng"]
	ssh = None
	channel = None
	output = ""
	log_path.parent.mkdir(parents=True, exist_ok=True)
	with log_path.open("w", encoding="utf-8", errors="replace") as log:
		log_line(log, f"CONNECT feiteng {board['ip']}:{board['telnet_port']}")
		log_line(log, f"COMMAND {command}")
		log_line(log, f"TIMEOUT {timeout_sec}s")
		try:
			last_open_error = None
			for attempt in range(1, 4):
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
						try:
							channel.close()
						except Exception:
							pass
						channel = None
					if ssh is not None:
						try:
							ssh.close()
						except Exception:
							pass
						ssh = None
					time.sleep(2.0)
			if channel is None:
				raise RuntimeError(f"telnet open failed after retries: {last_open_error!r}")
			# Drop login banner/prompt bytes that may still be buffered after login.
			time.sleep(0.2)
			while channel.recv_ready():
				channel.recv(8192)
			deploy.send_line(channel, command)
			deadline = time.time() + timeout_sec
			while time.time() < deadline:
				if channel.recv_ready():
					chunk = channel.recv(8192).decode("utf-8", errors="replace")
					output += chunk
					log.write(chunk)
					log.flush()
					if PROMPT_RE.search(output):
						log_line(log, "STATUS PROMPT_SEEN")
						return 0
				else:
					if channel.exit_status_ready():
						log_line(log, "STATUS CHANNEL_EXIT")
						return 2
					time.sleep(0.1)

			log_line(log, "STATUS TIMEOUT")
			try:
				channel.send("\x03")
				time.sleep(1.0)
				if channel.recv_ready():
					chunk = channel.recv(8192).decode("utf-8", errors="replace")
					log.write(chunk)
					log.flush()
			except Exception as exc:
				log_line(log, f"CTRL_C_FAILED {exc!r}")
			return 124
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
	parser.add_argument("--command", required=True)
	parser.add_argument("--log", required=True)
	parser.add_argument("--timeout", type=int, required=True)
	args = parser.parse_args(argv)

	if not os.environ.get("RTBENCH_JUMPHOST_PASSWORD"):
		os.environ["RTBENCH_JUMPHOST_PASSWORD"] = "rtbench"

	deploy = load_deploy_helper()
	return run_command(deploy, args.command, Path(args.log), args.timeout)


if __name__ == "__main__":
	raise SystemExit(main(sys.argv[1:]))
