#!/usr/bin/env python3
# OneOS Nezha D1H - watchdog-candidate P0 (test-schedule) acceptance over raw telnet.
# Deploys a candidate main module as /user/ctest.out (no /tftp files overwritten),
# then loads schedrun.out runner which calls cmd_rtbench_stub -> test-schedule.
import socket, sys, time
from datetime import datetime

IAC=255; DONT=254; DO=253; WONT=252; WILL=251; SB=250; SE=240
FATAL=['Instruction page fault','Exception in task','unhandled exception','Data abort','Load access fault']

HOST='192.168.31.211'; TFTP='192.168.31.110'
CAND='ctw.out'   # SHORT tftp name (OneOS shell input buffer truncates cmd at 80 chars)
RUNNER='schedrun.out'                            # existing quick test-schedule runner

class T:
    def __init__(s,h,p=23,t=6.0):
        s.s=socket.create_connection((h,p),timeout=t); s.s.settimeout(0.2); s.buf=bytearray()
    def close(s):
        try: s.s.close()
        except: pass
    def _tn(s,d):
        o=bytearray(); i=0
        while i<len(d):
            b=d[i]
            if b!=IAC: o.append(b); i+=1; continue
            if i+1>=len(d): break
            c=d[i+1]
            if c==IAC: o.append(IAC); i+=2
            elif c in (DO,DONT,WILL,WONT):
                if i+2>=len(d): break
                opt=d[i+2]
                if c==DO: s.s.sendall(bytes([IAC,WONT,opt]))
                elif c==WILL: s.s.sendall(bytes([IAC,DONT,opt]))
                i+=3
            elif c==SB:
                i+=2
                while i+1<len(d) and not(d[i]==IAC and d[i+1]==SE): i+=1
                i+=2 if i+1<len(d) else 0
            else: i+=2
        return bytes(o)
    def rd(s):
        try: d=s.s.recv(4096)
        except socket.timeout: return ''
        if not d: raise ConnectionError('socket closed by peer')
        txt=s._tn(d).decode('utf-8','replace')
        if txt: s.buf.extend(txt.encode('utf-8','replace')); print(txt,end='',flush=True)
        return txt
    def wait(s,timeout=10,idle=1.0,prompt='sh /'):
        end=time.time()+timeout; last=time.time(); buf=''
        while time.time()<end:
            ch=s.rd()
            if ch: buf+=ch; last=time.time()
            if prompt in buf and time.time()-last>=idle: return buf
            time.sleep(0.05)
        return buf
    def cmd(s,c,timeout=10,idle=1.0):
        print(f"\n===== CMD: {c} =====",flush=True)
        s.s.sendall((c+'\r\n').encode('ascii'))
        return s.wait(timeout=timeout,idle=idle)

def fatal(txt):
    return [p for p in FATAL if p in txt]

def main():
    print(f"[wd] start={datetime.now().isoformat()} host={HOST} tftp={TFTP} module={CAND}")
    tn=T(HOST)
    try:
        tn.wait(timeout=5,idle=0.8)
        for c,to in [('ifconfig',8),('default_netif e00',8),('telnetd start',8),(f'ping {TFTP}',20)]:
            tn.cmd(c,timeout=to,idle=1.0)
        # deploy my module as /user/ctest.out
        tn.cmd('unld /user/ctest.out',timeout=8,idle=0.8)
        tn.cmd('rm /user/ctest.out',timeout=8,idle=0.8)
        out=tn.cmd(f'tftp_client {TFTP} get {CAND} /user/ctest.out',timeout=120,idle=1.2)
        if 'err=0' not in out:
            print('\n[wd] FAIL: tftp get ctest err!=0',flush=True); return 10
        out=tn.cmd('ld /user/ctest.out',timeout=120,idle=2.0)
        if fatal(out): print(f'\n[wd] FAIL ld ctest fatal: {fatal(out)}',flush=True); return 20
        # deploy + run schedule runner (this triggers test-schedule; allow watchdog budget + margin)
        tn.cmd(f'unld /user/{RUNNER}',timeout=8,idle=0.8)
        tn.cmd(f'rm /user/{RUNNER}',timeout=8,idle=0.8)
        out=tn.cmd(f'tftp_client {TFTP} get {RUNNER} /user/{RUNNER}',timeout=120,idle=1.2)
        if 'err=0' not in out:
            print('\n[wd] FAIL: tftp get runner err!=0',flush=True); return 11
        run=tn.cmd(f'ld /user/{RUNNER}',timeout=780,idle=3.0)
        if fatal(run): print(f'\n[wd] FAIL run fatal: {fatal(run)}',flush=True); return 21
        # verdicts
        nine = 'Found 9 industrial workloads' in run
        score = 'Final Score' in run
        retok = 'schedule ret=0' in run
        print(f"\n[wd] markers: 9workloads={nine} FinalScore={score} ret0={retok}",flush=True)
        if score or retok:
            print(f"[wd] COMPLETE rc=0 end={datetime.now().isoformat()}",flush=True); return 0
        print(f"[wd] INCOMPLETE: no Final Score / ret=0 marker",flush=True); return 22
    finally:
        tn.close()

if __name__=='__main__':
    try: sys.exit(main())
    except Exception as e:
        print(f"\n[wd] EXCEPTION: {type(e).__name__}: {e}",flush=True); sys.exit(99)
