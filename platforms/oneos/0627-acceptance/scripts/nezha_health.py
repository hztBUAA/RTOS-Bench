#!/usr/bin/env python3
# 哪吒派复位后健康检查: ping + telnet shell 是否真的有回显
import socket, subprocess, time, sys
HOST="192.168.31.211"
IAC,DONT,DO,WONT,WILL,SB,SE=255,254,253,252,251,250,240
def clean(sock,d):
    o=bytearray(); i=0
    while i<len(d):
        if d[i]!=IAC: o.append(d[i]); i+=1; continue
        if i+1>=len(d): break
        c=d[i+1]
        if c==IAC: o.append(IAC); i+=2
        elif c in (DO,DONT,WILL,WONT) and i+2<len(d):
            opt=d[i+2]; sock.sendall(bytes([IAC, WONT if c==DO else DONT, opt])); i+=3
        elif c==SB:
            i+=2
            while i+1<len(d) and not(d[i]==IAC and d[i+1]==SE): i+=1
            i+=2
        else: i+=2
    return bytes(o)
# 1) ping
p=subprocess.run(["ping","-c","3","-W","2",HOST],capture_output=True,text=True)
ping_ok = "0% packet loss" in p.stdout
print(f"[1] ping {HOST}: {'OK' if ping_ok else 'FAIL'}")
# 2) telnet shell 回显
got=bytearray()
try:
    s=socket.create_connection((HOST,23),timeout=6); s.settimeout(0.4)
    time.sleep(0.5)
    for cmd in (b"\r\n", b"help\r\n", b"ifconfig\r\n"):
        s.sendall(cmd); t=time.time()
        while time.time()-t<3:
            try: d=s.recv(4096)
            except socket.timeout: d=b""
            if d: got+=clean(s,d)
    s.close()
except Exception as e:
    print(f"[2] telnet: connect/IO error: {e}")
txt=got.decode("utf-8","replace")
shell_ok = len(got)>20 and ("sh /" in txt or "e00" in txt or "ifconfig" in txt.lower() or "command" in txt.lower())
print(f"[2] telnet shell 回显: {len(got)} bytes -> {'OK(shell可用)' if shell_ok else 'FAIL(静默/仍卡死)'}")
if txt.strip():
    print("---- 板端回显片段 ----")
    print(txt[:400])
print("\n==> 结论:", "板子可用 ✅ 可以跑验收" if (ping_ok and shell_ok) else "板子未恢复 ❌ (需再复位/串口重启 telnetd)")
sys.exit(0 if (ping_ok and shell_ok) else 1)
