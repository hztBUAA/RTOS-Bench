#!/usr/bin/env python3
"""Minimal read-only TFTP server for OneOS board bring-up.

This server is intentionally small: it only supports RRQ in octet mode and
serves files from a fixed root directory. It is for direct Windows-to-board
debugging when the shared rtbench TFTP host is unavailable.
"""

from __future__ import annotations

import argparse
import os
import socket
import struct
from pathlib import Path


OP_RRQ = 1
OP_DATA = 3
OP_ACK = 4
OP_ERROR = 5
BLOCK_SIZE = 512


ALIASES = {
    "oneos-nezha-d1h-current.out": "ctest.out",
    "oneos-nezha-d1h-schedule-current.out": "schedrun.out",
    "oneos-nezha-d1h-realtime-current.out": "rtrt.out",
    "oneos-nezha-d1h-workloads-current.out": "wlrun.out",
    "oneos-nezha-d1h-stress-current.out": "strun.out",
    "oneos-nezha-d1h-testall-current.out": "allrun.out",
}


def error_packet(code: int, message: str) -> bytes:
    return struct.pack("!HH", OP_ERROR, code) + message.encode("ascii", "replace") + b"\0"


def data_packet(block: int, payload: bytes) -> bytes:
    return struct.pack("!HH", OP_DATA, block) + payload


def parse_rrq(packet: bytes) -> tuple[str, str] | None:
    if len(packet) < 4:
        return None
    op = struct.unpack("!H", packet[:2])[0]
    if op != OP_RRQ:
        return None
    parts = packet[2:].split(b"\0")
    if len(parts) < 2:
        return None
    filename = parts[0].decode("utf-8", "replace").replace("\\", "/").lstrip("/")
    mode = parts[1].decode("ascii", "replace").lower()
    return filename, mode


def safe_resolve(root: Path, filename: str) -> Path | None:
    mapped = ALIASES.get(filename, filename)
    candidate = (root / mapped).resolve()
    try:
        candidate.relative_to(root)
    except ValueError:
        return None
    return candidate


def serve_file(bind_addr: str, root: Path, client: tuple[str, int], filename: str) -> None:
    path = safe_resolve(root, filename)
    if path is None or not path.is_file():
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as error_sock:
            error_sock.bind((bind_addr, 0))
            error_sock.sendto(error_packet(1, "file not found"), client)
        print(f"MISS {client[0]}:{client[1]} {filename}", flush=True)
        return

    print(f"SEND {client[0]}:{client[1]} {filename} -> {path.name}", flush=True)
    block = 1
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as data_sock, path.open("rb") as f:
        data_sock.bind((bind_addr, 0))
        while True:
            payload = f.read(BLOCK_SIZE)
            packet = data_packet(block, payload)
            for attempt in range(5):
                data_sock.sendto(packet, client)
                data_sock.settimeout(2.0)
                try:
                    ack, ack_client = data_sock.recvfrom(1024)
                except socket.timeout:
                    continue
                except ConnectionResetError:
                    # Windows reports ICMP port-unreachable as WSAECONNRESET on
                    # UDP sockets. Treat it like a lost ACK and retry.
                    continue
                if ack_client != client or len(ack) < 4:
                    continue
                op, ack_block = struct.unpack("!HH", ack[:4])
                if op == OP_ACK and ack_block == block:
                    break
            else:
                print(f"TIMEOUT {client[0]}:{client[1]} block={block}", flush=True)
                return

            if len(payload) < BLOCK_SIZE:
                print(f"DONE {client[0]}:{client[1]} {filename}", flush=True)
                return
            block = (block + 1) & 0xFFFF
            if block == 0:
                block = 1


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", required=True, help="Directory containing accepted .out files")
    parser.add_argument("--bind", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=69)
    args = parser.parse_args()

    root = Path(args.root).resolve()
    if not root.is_dir():
        raise SystemExit(f"root directory does not exist: {root}")

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((args.bind, args.port))
    print(f"TFTP root={root} bind={args.bind}:{args.port}", flush=True)
    print("Aliases:", flush=True)
    for src, dst in sorted(ALIASES.items()):
        print(f"  {src} -> {dst}", flush=True)

    while True:
        sock.settimeout(None)
        packet, client = sock.recvfrom(2048)
        rrq = parse_rrq(packet)
        if rrq is None:
            sock.sendto(error_packet(4, "unsupported request"), client)
            continue
        filename, mode = rrq
        if mode not in ("octet", "netascii"):
            sock.sendto(error_packet(4, "unsupported mode"), client)
            continue
        serve_file(args.bind, root, client, filename)


if __name__ == "__main__":
    raise SystemExit(main())
