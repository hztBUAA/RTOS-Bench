import socket,sys,time
from datetime import datetime
IAC,DONT,DO,WONT,WILL,SB,SE=255,254,253,252,251,250,240
HOST='192.168.31.211'; TFTP='192.168.31.110'
class T:
    def __init__(s,h,p=23,t=6.0):
        s.s=socket.create_connection((h,p),timeout=t); s.s.settimeout(0.4)
    def _tn(s,d):
        o=bytearray();i=0
        while i<len(d):
            b=d[i]
            if b!=IAC:o.append(b);i+=1;continue
            if i+1>=len(d):break
            c=d[i+1]
            if c==IAC:o.append(IAC);i+=2
            elif c in(DO,DONT,WILL,WONT):
                if i+2>=len(d):break
                opt=d[i+2]
                if c==DO:s.s.sendall(bytes([IAC,WONT,opt]))
                elif c==WILL:s.s.sendall(bytes([IAC,DONT,opt]))
                i+=3
            elif c==SB:
                i+=2
                while i+1<len(d) and not(d[i]==IAC and d[i+1]==SE):i+=1
                i+=2 if i+1<len(d) else 0
            else:i+=2
        return bytes(o)
    def rd(s):
        try:d=s.s.recv(4096)
        except socket.timeout:return ''
        if not d:raise ConnectionError('closed')
        t=s._tn(d).decode('utf-8','replace')
        if t:print(t,end='',flush=True)
        return t
    def wait(s,timeout,idle=2.0,until=None,prompt='sh /'):
        end=time.time()+timeout;last=time.time();buf=''
        while time.time()<end:
            ch=s.rd()
            if ch:buf+=ch;last=time.time()
            if until and until in buf:return buf
            if until is None and prompt in buf and time.time()-last>=idle:return buf
            time.sleep(0.05)
        return buf
    def cmd(s,c,timeout,idle=2.0,until=None):
        print(f"\n===== CMD: {c} =====",flush=True)
        s.s.sendall((c+'\r\n').encode());return s.wait(timeout,idle,until)
print(f"[schf] start={datetime.now().isoformat()}")
tn=T(HOST)
try:
    tn.wait(5,0.8)
    for c,to in [('default_netif e00',8),('telnetd start',8),(f'ping {TFTP}',15)]: tn.cmd(c,to)
    tn.cmd('unld /user/schf.out',8,0.8); tn.cmd('rm /user/schf.out',8,0.8)
    o=tn.cmd(f'tftp_client {TFTP} get schf.out /user/schf.out',120)
    if 'err=0' not in o: print('[schf] FAIL tftp'); sys.exit(10)
    tn.cmd('ld /user/schf.out',60,2.0)
    # runner task runs ~3s later; read until DONE / Final Score
    out=tn.wait(700, until='test-schedule DONE')
    print(f"\n[schf] FinalScore={'Final Score' in out} end={datetime.now().isoformat()}")
finally:
    tn.close()
