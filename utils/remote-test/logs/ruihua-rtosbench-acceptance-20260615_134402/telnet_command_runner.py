import argparse
import re
import socket
import sys
import time
from pathlib import Path

PROMPT = 'reworks>'

def recv_until_prompt(sock, timeout, idle=1.0):
    end = time.time() + timeout
    last = time.time()
    data = bytearray()
    sock.settimeout(0.2)
    while time.time() < end:
        got = False
        try:
            chunk = sock.recv(65536)
            if chunk:
                data.extend(chunk)
                got = True
                last = time.time()
        except Exception:
            pass
        text = data.decode('utf-8', 'replace')
        if PROMPT in text and time.time() - last >= idle:
            break
        if not got:
            time.sleep(0.05)
    return bytes(data)

def parse_rc(text):
    matches = re.findall(r'0x[0-9a-fA-F]+\s*\((-?\d+)\)', text)
    if matches:
        return int(matches[-1])
    return 0 if PROMPT in text else 125

parser = argparse.ArgumentParser()
parser.add_argument('--host', default='192.168.31.210')
parser.add_argument('--port', type=int, default=23)
parser.add_argument('--cmd', required=True)
parser.add_argument('--log', required=True)
parser.add_argument('--timeout', type=float, default=120)
args = parser.parse_args()

log = Path(args.log)
log.parent.mkdir(parents=True, exist_ok=True)
with log.open('wb') as f:
    f.write(f'=== COMMAND: {args.cmd}\n=== TIMEOUT: {args.timeout}\n'.encode())
    try:
        sock = socket.create_connection((args.host, args.port), timeout=8)
        f.write(b'=== INITIAL ===\n')
        f.write(recv_until_prompt(sock, 8, idle=0.8))
        f.write(b'\n=== RUN ===\n')
        sock.sendall(args.cmd.encode() + b'\r\n')
        f.write(recv_until_prompt(sock, args.timeout, idle=1.0))
        sock.close()
    except Exception as exc:
        f.write(f'\n[runner-error] {exc}\n'.encode())
        raise SystemExit(124)

text = log.read_bytes().decode('utf-8', 'replace')
if PROMPT not in text:
    raise SystemExit(124)
raise SystemExit(parse_rc(text))
