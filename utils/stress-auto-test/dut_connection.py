import re
import socket
import sys
import time

import serial


IAC = 255
DONT = 254
DO = 253
WONT = 252
WILL = 251
SB = 250
SE = 240


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

            if char == "\n":
                self._at_line_start = True

        sys.stdout.flush()

    def _log_input(self, command):
        if not self._at_line_start:
            sys.stdout.write("\n")

        print(f"[dut] >>> send: {command.strip()}")
        self._at_line_start = True

    @staticmethod
    def _normalize_commands(commands):
        if commands is None:
            return []
        if isinstance(commands, str):
            return [commands]
        return list(commands)

    def send_cmd(self, commands):
        raise NotImplementedError

    def wait_for_regex(self, pattern, timeout=None):
        raise NotImplementedError

    def read_for(self, duration):
        raise NotImplementedError

    def disconnect(self):
        raise NotImplementedError


class SerialConnection(BaseConnection):
    def __init__(
        self,
        port,
        baudrate=115200,
        timeout=0.1,
        boot_wait=0.0,
        command_delay=0.5,
        line_ending="\n",
        **kwargs
    ):
        super().__init__()
        self.command_delay = float(command_delay)
        self.line_ending = str(line_ending)
        self.ser = serial.Serial(
            port=port,
            baudrate=baudrate,
            timeout=timeout
        )

        if boot_wait:
            time.sleep(float(boot_wait))

        self.read_for(0.2)

    def _read_text(self):
        if not self.ser.in_waiting:
            return ""

        return self.ser.read(self.ser.in_waiting).decode(
            "utf-8",
            errors="ignore"
        )

    def send_cmd(self, commands):
        for command in self._normalize_commands(commands):
            command = str(command)
            self._log_input(command)

            if not command.endswith(("\n", "\r")):
                command += self.line_ending

            self.ser.write(command.encode("utf-8"))
            self.ser.flush()
            time.sleep(self.command_delay)

    def wait_for_regex(self, pattern, timeout=None):
        regex = re.compile(pattern)
        buffer = ""
        deadline = None if timeout is None else time.monotonic() + float(timeout)

        while True:
            if deadline is not None and time.monotonic() >= deadline:
                raise TimeoutError(
                    f"Timed out waiting for serial pattern: {pattern}"
                )

            data = self._read_text()
            if data:
                buffer += data
                self._log_output(data)

                if regex.search(buffer):
                    return buffer
            else:
                time.sleep(0.01)

    def read_for(self, duration):
        deadline = time.monotonic() + max(0.0, float(duration))
        buffer = ""

        while time.monotonic() < deadline:
            data = self._read_text()
            if data:
                buffer += data
                self._log_output(data)
            else:
                time.sleep(0.01)

        return buffer

    def disconnect(self):
        if self.ser and self.ser.is_open:
            self.ser.close()


class TelnetConnection(BaseConnection):
    def __init__(
        self,
        ip,
        port=23,
        username=None,
        password=None,
        login=None,
        timeout=10,
        connect_wait=1.0,
        login_prompt="login:",
        password_prompt="password:",
        command_delay=0.5,
        line_ending="\n",
        **kwargs
    ):
        super().__init__()
        self.timeout = float(timeout)
        self.command_delay = float(command_delay)
        self.line_ending = str(line_ending)
        self.sock = None

        self._iac_state = "data"
        self._iac_command = None

        try:
            self.sock = socket.create_connection(
                (ip, int(port)),
                timeout=self.timeout
            )
            self.sock.settimeout(0.1)

            if login is None:
                login = bool(username)

            if login:
                self._login(
                    username,
                    password,
                    login_prompt,
                    password_prompt
                )
            else:
                time.sleep(float(connect_wait))
                self.read_for(1.0)
        except Exception:
            self.disconnect()
            raise

    def _send_bytes(self, data):
        self.sock.sendall(data)

    def _send_telnet_response(self, command, option):
        if command == DO:
            self._send_bytes(bytes([IAC, WONT, option]))
        elif command == WILL:
            self._send_bytes(bytes([IAC, DONT, option]))

    def _filter_telnet_bytes(self, raw):
        output = bytearray()

        for byte in raw:
            if self._iac_state == "data":
                if byte == IAC:
                    self._iac_state = "iac"
                else:
                    output.append(byte)

            elif self._iac_state == "iac":
                if byte == IAC:
                    output.append(IAC)
                    self._iac_state = "data"
                elif byte in (DO, DONT, WILL, WONT):
                    self._iac_command = byte
                    self._iac_state = "negotiation"
                elif byte == SB:
                    self._iac_state = "subnegotiation"
                else:
                    self._iac_state = "data"

            elif self._iac_state == "negotiation":
                self._send_telnet_response(self._iac_command, byte)
                self._iac_command = None
                self._iac_state = "data"

            elif self._iac_state == "subnegotiation":
                if byte == IAC:
                    self._iac_state = "subnegotiation_iac"

            elif self._iac_state == "subnegotiation_iac":
                if byte == SE:
                    self._iac_state = "data"
                else:
                    self._iac_state = "subnegotiation"

        return bytes(output)

    def _recv_text(self):
        try:
            raw = self.sock.recv(4096)
            if raw == b"":
                raise ConnectionError("TELNET connection closed by remote host")

            filtered = self._filter_telnet_bytes(raw)
            return filtered.decode("utf-8", errors="ignore")

        except socket.timeout:
            return ""
        except ConnectionResetError as exc:
            raise ConnectionError(
                "TELNET connection was reset by the remote host"
            ) from exc
        except OSError as exc:
            raise ConnectionError(f"TELNET socket error: {exc}") from exc

    def _read_until(self, marker, timeout=None):
        timeout = self.timeout if timeout is None else float(timeout)
        marker = str(marker)
        marker_lower = marker.lower()
        deadline = time.monotonic() + timeout
        buffer = ""

        while time.monotonic() < deadline:
            data = self._recv_text()
            if data:
                buffer += data
                self._log_output(data)

                if marker_lower in buffer.lower():
                    return buffer
            else:
                time.sleep(0.01)

        raise TimeoutError(f"Timed out waiting for TELNET marker: {marker}")

    def _login(self, username, password, login_prompt, password_prompt):
        if not username:
            raise ValueError("TELNET login is enabled, but username is empty")

        self._read_until(login_prompt)
        self._write_line(username)

        if password is not None and password != "":
            self._read_until(password_prompt)
            self._write_line(password, log_text="******")

        time.sleep(1.0)
        self.read_for(1.0)

    def _write_line(self, text, log_text=None):
        text = str(text)
        self._log_input(log_text if log_text is not None else text)

        if not text.endswith(("\n", "\r")):
            text += self.line_ending

        self._send_bytes(text.encode("utf-8"))

    def send_cmd(self, commands):
        for command in self._normalize_commands(commands):
            self._write_line(command)
            time.sleep(self.command_delay)

    def wait_for_regex(self, pattern, timeout=None):
        regex = re.compile(pattern)
        buffer = ""
        deadline = None if timeout is None else time.monotonic() + float(timeout)

        while True:
            if deadline is not None and time.monotonic() >= deadline:
                raise TimeoutError(
                    f"Timed out waiting for TELNET pattern: {pattern}"
                )

            data = self._recv_text()
            if data:
                buffer += data
                self._log_output(data)

                if regex.search(buffer):
                    return buffer
            else:
                time.sleep(0.01)

    def read_for(self, duration):
        deadline = time.monotonic() + max(0.0, float(duration))
        buffer = ""

        while time.monotonic() < deadline:
            data = self._recv_text()
            if data:
                buffer += data
                self._log_output(data)
            else:
                time.sleep(0.01)

        return buffer

    def disconnect(self):
        if not self.sock:
            return

        try:
            self.sock.shutdown(socket.SHUT_RDWR)
        except Exception:
            pass

        try:
            self.sock.close()
        except Exception:
            pass
