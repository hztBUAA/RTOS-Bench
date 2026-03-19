"""
增强版 DUT 连接模块 — 支持日志文件捕获、超时等待、自定义端口

基于 utils/stress-auto-test/dut_connection.py 增强：
- log_file: 终端全量输出 tee 到文件
- wait_for_regex timeout: 超时抛 TimeoutError
- SSH port 参数: 支持 FRP 场景非标端口
"""

import socket
import re
import sys
import time
from typing import Optional

import paramiko

try:
    import serial
except ImportError:
    serial = None

try:
    import telnetlib
except ImportError:
    telnetlib = None


class BaseConnection:
    def __init__(self, log_file: Optional[str] = None):
        self._at_line_start = True
        self._log_fh = None
        self._buffer = ""
        if log_file:
            self._log_fh = open(log_file, "w", encoding="utf-8")

    def _log_output(self, data: str):
        if not data:
            return

        for char in data:
            if self._at_line_start:
                sys.stdout.write("[dut] ")
                self._at_line_start = False

            sys.stdout.write(char)

            if char == '\n':
                self._at_line_start = True

        sys.stdout.flush()

        if self._log_fh:
            self._log_fh.write(data)
            self._log_fh.flush()

    def _log_input(self, cmd: str):
        if not self._at_line_start:
            sys.stdout.write("\n")

        line = f"[dut] >>> {cmd.strip()}\n"
        sys.stdout.write(line)
        sys.stdout.flush()
        self._at_line_start = True

        if self._log_fh:
            self._log_fh.write(f">>> {cmd.strip()}\n")
            self._log_fh.flush()

    def close_log(self):
        if self._log_fh:
            self._log_fh.close()
            self._log_fh = None

    @property
    def buffer(self) -> str:
        return self._buffer

    def send_cmd(self, cmd):
        raise NotImplementedError

    def wait_for_regex(self, pattern: str, timeout: Optional[float] = None) -> str:
        raise NotImplementedError

    def disconnect(self):
        self.close_log()


class SSHConnection(BaseConnection):
    def __init__(self, ip: str, username: str, password: str,
                 port: int = 22, log_file: Optional[str] = None):
        super().__init__(log_file=log_file)
        self.client = paramiko.SSHClient()
        self.client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        self.client.connect(ip, port=port, username=username, password=password)
        self.channel = self.client.invoke_shell()

    def send_cmd(self, cmd):
        if isinstance(cmd, str):
            cmd = [cmd]
        for c in cmd:
            self._log_input(c)
            if not c.endswith('\n'):
                c += '\n'
            self.channel.send(c)
            time.sleep(0.5)

    def wait_for_regex(self, pattern: str, timeout: Optional[float] = None) -> str:
        deadline = time.monotonic() + timeout if timeout else None
        while True:
            if deadline and time.monotonic() > deadline:
                raise TimeoutError(
                    f"等待正则 '{pattern}' 超时 ({timeout}s)")
            if self.channel.recv_ready():
                data = self.channel.recv(4096).decode('utf-8', errors='ignore')
                self._buffer += data
                self._log_output(data)
                if re.search(pattern, self._buffer):
                    return self._buffer
            time.sleep(0.01)

    def disconnect(self):
        super().disconnect()
        self.client.close()


class SerialConnection(BaseConnection):
    def __init__(self, port: str, baudrate: int = 115200,
                 log_file: Optional[str] = None):
        super().__init__(log_file=log_file)
        if serial is None:
            raise ImportError("pyserial is required for SerialConnection")
        self.ser = serial.Serial(port, baudrate, timeout=0.1)

    def send_cmd(self, cmd):
        if isinstance(cmd, str):
            cmd = [cmd]
        for c in cmd:
            self._log_input(c)
            if not c.endswith('\n'):
                c += '\n'
            self.ser.write(c.encode('utf-8'))
            time.sleep(0.5)

    def wait_for_regex(self, pattern: str, timeout: Optional[float] = None) -> str:
        deadline = time.monotonic() + timeout if timeout else None
        while True:
            if deadline and time.monotonic() > deadline:
                raise TimeoutError(
                    f"等待正则 '{pattern}' 超时 ({timeout}s)")
            if self.ser.in_waiting:
                data = self.ser.read(self.ser.in_waiting).decode('utf-8', errors='ignore')
                self._buffer += data
                self._log_output(data)
                if re.search(pattern, self._buffer):
                    return self._buffer
            time.sleep(0.01)

    def disconnect(self):
        super().disconnect()
        self.ser.close()


class TelnetConnection(BaseConnection):
    def __init__(self, ip: str, username: str, password: str,
                 port: int = 23, timeout: int = 5,
                 log_file: Optional[str] = None):
        super().__init__(log_file=log_file)
        if telnetlib is None:
            raise ImportError("telnetlib is required for TelnetConnection")
        self.tn = telnetlib.Telnet(ip, port, timeout)

        data = self.tn.read_until(b"login: ").decode('utf-8', errors='ignore')
        self._buffer += data
        self._log_output(data)

        self._log_input(username)
        self.tn.write(username.encode('ascii') + b"\n")

        if password:
            data = self.tn.read_until(b"password: ").decode('utf-8', errors='ignore')
            self._buffer += data
            self._log_output(data)
            self._log_input(password)
            self.tn.write(password.encode('ascii') + b"\n")

        time.sleep(5)
        welcome_data = self.tn.read_very_eager().decode('utf-8', errors='ignore')
        if welcome_data:
            self._buffer += welcome_data
            self._log_output(welcome_data)

    def send_cmd(self, cmd):
        if isinstance(cmd, str):
            cmd = [cmd]
        for c in cmd:
            self._log_input(c)
            if not c.endswith('\n'):
                c += '\n'
            self.tn.write(c.encode('utf-8'))
            time.sleep(3)

    def wait_for_regex(self, pattern: str, timeout: Optional[float] = None) -> str:
        deadline = time.monotonic() + timeout if timeout else None
        while True:
            if deadline and time.monotonic() > deadline:
                raise TimeoutError(
                    f"等待正则 '{pattern}' 超时 ({timeout}s)")
            try:
                data = self.tn.read_some().decode('utf-8', errors='ignore')
                if data:
                    self._buffer += data
                    self._log_output(data)
                    if re.search(pattern, self._buffer):
                        return self._buffer
            except (socket.timeout, TimeoutError):
                pass
            except EOFError:
                break
            time.sleep(0.01)
        return self._buffer

    def disconnect(self):
        super().disconnect()
        self.tn.close()
