#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Run a command in the Dongtu/Intewell RTOS telnet shell and log output."""

from __future__ import annotations

import argparse
import re
import socket
import sys
import time
from pathlib import Path


IAC = 255
DONT = 254
DO = 253
WONT = 252
WILL = 251
SB = 250
SE = 240

PROMPT_PATTERN = r"(?:vm\d+\s+\[\s*.*?\s*\]#|sh\s+/\w+>|[A-Za-z0-9_.-]+#)\s*$"
PROMPT_RE = re.compile(PROMPT_PATTERN, re.M)


def decode_telnet(data: bytes) -> tuple[str, bytes]:
    """Strip telnet negotiation and answer all options with DONT/WONT."""
    out = bytearray()
    reply = bytearray()
    index = 0
    while index < len(data):
        byte = data[index]
        if byte != IAC:
            out.append(byte)
            index += 1
            continue
        index += 1
        if index >= len(data):
            break
        command = data[index]
        index += 1
        if command == IAC:
            out.append(IAC)
            continue
        if command in (DO, DONT, WILL, WONT):
            if index < len(data):
                option = data[index]
                index += 1
                reply.extend([IAC, WONT if command in (DO, DONT) else DONT, option])
            continue
        if command == SB:
            while index + 1 < len(data):
                if data[index] == IAC and data[index + 1] == SE:
                    index += 2
                    break
                index += 1
            continue
    return out.decode("utf-8", errors="replace"), bytes(reply)


def write_text(handle, text: str) -> None:
    sys.stdout.write(text)
    sys.stdout.flush()
    handle.write(text)
    handle.flush()


def read_available(sock: socket.socket, log_handle, quiet: bool = False) -> str:
    chunks: list[str] = []
    while True:
        try:
            data = sock.recv(8192)
        except socket.timeout:
            break
        if not data:
            break
        text, reply = decode_telnet(data)
        if reply:
            sock.sendall(reply)
        if text:
            chunks.append(text)
            if not quiet:
                write_text(log_handle, text)
    return "".join(chunks)


def wait_for(sock: socket.socket, log_handle, pattern: re.Pattern[str], timeout: float) -> str:
    deadline = time.time() + timeout
    output = ""
    while time.time() < deadline:
        output += read_available(sock, log_handle)
        if pattern.search(output):
            return output
        time.sleep(0.1)
    raise TimeoutError(f"timeout waiting for pattern: {pattern.pattern}")


def run_command(sock: socket.socket, log_handle, command: str, timeout: float, idle_timeout: float) -> str:
    write_text(log_handle, f"\n>>> {command}\n")
    sock.sendall((command + "\r\n").encode("utf-8"))
    output = ""
    deadline = time.time() + timeout
    idle_deadline = time.time() + idle_timeout
    while time.time() < deadline:
        chunk = read_available(sock, log_handle)
        if chunk:
            output += chunk
            idle_deadline = time.time() + idle_timeout
            if PROMPT_RE.search(output):
                return output
        elif time.time() > idle_deadline:
            write_text(log_handle, f"\n[TIMEOUT] idle for {idle_timeout:.0f}s\n")
            return output
        time.sleep(0.2)
    write_text(log_handle, f"\n[TIMEOUT] command exceeded {timeout:.0f}s\n")
    return output


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="192.168.31.207")
    parser.add_argument("--port", type=int, default=23)
    parser.add_argument("--username", default="root")
    parser.add_argument("--password", default="root")
    parser.add_argument("--connect-timeout", type=float, default=10)
    parser.add_argument("--prompt-timeout", type=float, default=20)
    parser.add_argument("--timeout", type=float, default=7200)
    parser.add_argument("--idle-timeout", type=float, default=1200)
    parser.add_argument("--log", required=True)
    parser.add_argument("--send-exit", action="store_true", help="send 'exit' after commands finish")
    parser.add_argument("command", nargs="+")
    args = parser.parse_args()

    log_path = Path(args.log)
    log_path.parent.mkdir(parents=True, exist_ok=True)

    with log_path.open("w", encoding="utf-8", errors="replace") as log_handle:
        with socket.create_connection((args.host, args.port), timeout=args.connect_timeout) as sock:
            sock.settimeout(0.2)
            write_text(log_handle, f"[CONNECT] {args.host}:{args.port}\n")
            initial = ""
            try:
                initial = wait_for(sock, log_handle, re.compile(r"login:|password:|" + PROMPT_PATTERN, re.I | re.M), args.prompt_timeout)
            except TimeoutError:
                initial += read_available(sock, log_handle)

            if re.search(r"login:", initial, re.I):
                sock.sendall((args.username + "\r\n").encode("ascii"))
                initial += wait_for(sock, log_handle, re.compile(r"password:|" + PROMPT_PATTERN, re.I | re.M), args.prompt_timeout)
            if re.search(r"password:", initial, re.I):
                sock.sendall((args.password + "\r\n").encode("ascii"))
                wait_for(sock, log_handle, PROMPT_RE, args.prompt_timeout)
            elif not PROMPT_RE.search(initial):
                wait_for(sock, log_handle, PROMPT_RE, args.prompt_timeout)

            results = []
            for command in args.command:
                results.append(run_command(sock, log_handle, command, args.timeout, args.idle_timeout))

            if args.send_exit:
                try:
                    sock.sendall(b"exit\r\n")
                except OSError:
                    pass

    combined = "\n".join(results)
    if "[TIMEOUT]" in combined or "[FAIL]" in combined:
        return 2
    return 0 if PROMPT_RE.search(combined) else 1


if __name__ == "__main__":
    raise SystemExit(main())
