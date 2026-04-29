#!/usr/bin/env python3
"""Run one command in the Dongtu RTOS VM shell over telnet.

This script is intended to run from the rtbench jump host, which can reach the
RTOS VM network directly. It waits for the shell prompt after the command so
long RTOS-Bench tests do not lose their tail output.
"""

import argparse
import re
import sys
import warnings

warnings.filterwarnings("ignore", category=DeprecationWarning)
import telnetlib
import time


PROMPT_RE = re.compile(rb"\r?\nvm\d+\s+\[\s+[^\]]+\s+\]#\s*$")


def read_until_prompt_or_idle(tn, timeout, idle_timeout, stream=True):
    start = time.monotonic()
    last_data = start
    data = bytearray()
    status = "idle"

    while True:
        now = time.monotonic()
        if now - start > timeout:
            status = "timeout"
            break

        try:
            chunk = tn.read_very_eager()
        except EOFError:
            status = "eof"
            break

        if chunk:
            data.extend(chunk)
            if stream:
                sys.stdout.write(chunk.decode("utf-8", errors="replace"))
                sys.stdout.flush()
            last_data = now
            if PROMPT_RE.search(bytes(data)):
                status = "prompt"
                break
        elif now - last_data > idle_timeout:
            break

        time.sleep(0.1)

    return bytes(data), status, time.monotonic() - start


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("command", help="Command to execute in the RTOS shell")
    parser.add_argument("--host", default="192.168.31.207")
    parser.add_argument("--port", type=int, default=23)
    parser.add_argument("--connect-timeout", type=float, default=8.0)
    parser.add_argument("--timeout", type=float, default=600.0)
    parser.add_argument("--idle-timeout", type=float, default=5.0)
    parser.add_argument("--no-stream", action="store_true",
                        help="Buffer command output until completion")
    args = parser.parse_args()

    started = time.monotonic()
    tn = telnetlib.Telnet(args.host, args.port, timeout=args.connect_timeout)
    try:
        tn.write(b"\r\n")
        read_until_prompt_or_idle(tn, timeout=5.0, idle_timeout=1.0,
                                  stream=False)

        tn.write(args.command.encode("utf-8") + b"\r\n")
        output, status, duration = read_until_prompt_or_idle(
            tn, timeout=args.timeout, idle_timeout=args.idle_timeout,
            stream=not args.no_stream
        )
    finally:
        tn.close()

    if args.no_stream:
        sys.stdout.write(output.decode("utf-8", errors="replace"))
    sys.stdout.write("\n")
    sys.stdout.write("__RTBENCH_TELNET_STATUS=%s\n" % status)
    sys.stdout.write("__RTBENCH_TELNET_COMMAND=%s\n" % args.command)
    sys.stdout.write("__RTBENCH_TELNET_DURATION_SEC=%.3f\n" % duration)
    sys.stdout.write("__RTBENCH_TELNET_TOTAL_SEC=%.3f\n" % (time.monotonic() - started))
    return 0 if status == "prompt" else 2


if __name__ == "__main__":
    sys.exit(main())
