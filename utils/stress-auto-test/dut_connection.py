import socket
import paramiko
import serial
import re
import time
import sys
import telnetlib


class BaseConnection:
    def __init__(self):
        self._at_line_start = True

    def _log_output(self, data):
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

    def _log_input(self, cmd):
        if not self._at_line_start:
            sys.stdout.write("\n")

        print(f"[dut] >>> 发送指令: {cmd.strip()}")
        self._at_line_start = True

    def send_cmd(self, cmd):
        raise NotImplementedError

    def wait_for_regex(self, pattern):
        raise NotImplementedError

    def disconnect(self):
        raise NotImplementedError


class SSHConnection(BaseConnection):
    def __init__(self, ip, username, password):
        super().__init__()
        self.client = paramiko.SSHClient()
        self.client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        self.client.connect(ip, username=username, password=password)
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

    def wait_for_regex(self, pattern):
        buffer = ""
        while True:
            if self.channel.recv_ready():
                data = self.channel.recv(1024).decode('utf-8', errors='ignore')
                buffer += data
                self._log_output(data)
                if re.search(pattern, buffer):
                    return
            time.sleep(0.01)

    def disconnect(self):
        self.client.close()


class SerialConnection(BaseConnection):
    def __init__(self, port, baudrate):
        super().__init__()
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

    def wait_for_regex(self, pattern):
        buffer = ""
        while True:
            if self.ser.in_waiting:
                data = self.ser.read(self.ser.in_waiting).decode('utf-8', errors='ignore')
                buffer += data
                self._log_output(data)
                if re.search(pattern, buffer):
                    return
            time.sleep(0.01)

    def disconnect(self):
        self.ser.close()


class TelnetConnection(BaseConnection):
    def __init__(self, ip, username, password, port=23, timeout=5):
        super().__init__()
        self.tn = telnetlib.Telnet(ip, port, timeout)

        data = self.tn.read_until(b"login: ").decode('utf-8', errors='ignore')
        self._log_output(data)

        self._log_input(username)
        self.tn.write(username.encode('ascii') + b"\n")

        if password:
            data = self.tn.read_until(b"password: ").decode('utf-8', errors='ignore')
            self._log_output(data)
            self._log_input("root")
            self.tn.write(password.encode('ascii') + b"\n")
        time.sleep(5)
        welcome_data = self.tn.read_very_eager().decode('utf-8', errors='ignore')
        if welcome_data:
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

    def wait_for_regex(self, pattern):
        buffer = ""
        while True:
            try:
                data = self.tn.read_some().decode('utf-8', errors='ignore')
                if data:
                    buffer += data
                    self._log_output(data)
                    if re.search(pattern, buffer):
                        return
            except (socket.timeout, TimeoutError):
                pass
            except EOFError:
                break
            time.sleep(0.01)

    def disconnect(self):
        self.tn.close()
