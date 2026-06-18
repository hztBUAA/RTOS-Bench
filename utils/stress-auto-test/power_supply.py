import threading
import time

import serial


class UDP6720:
    def __init__(
        self,
        port,
        baudrate=9600,
        timeout=1.0,
        write_timeout=1.0,
        open_retries=3,
        retry_delay=1.0,
        close_delay=1.0,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        xonxoff=False,
        rtscts=False,
        dsrdtr=False,
        dtr=False,
        rts=False,
        remote_on_connect=True,
        local_on_disconnect=True,
        output_off_on_disconnect=True,
        **kwargs
    ):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.write_timeout = write_timeout
        self.open_retries = max(1, int(open_retries))
        self.retry_delay = max(0.0, float(retry_delay))
        self.close_delay = max(0.0, float(close_delay))

        self.bytesize = bytesize
        self.parity = parity
        self.stopbits = stopbits
        self.xonxoff = xonxoff
        self.rtscts = rtscts
        self.dsrdtr = dsrdtr
        self.dtr = bool(dtr)
        self.rts = bool(rts)

        self.remote_on_connect = bool(remote_on_connect)
        self.local_on_disconnect = bool(local_on_disconnect)
        self.output_off_on_disconnect = bool(output_off_on_disconnect)

        self.ser = None
        self._io_lock = threading.Lock()

    @property
    def is_connected(self):
        return self.ser is not None and self.ser.is_open

    def _create_serial(self):
        ser = serial.Serial(
            port=None,
            baudrate=self.baudrate,
            bytesize=self.bytesize,
            parity=self.parity,
            stopbits=self.stopbits,
            timeout=self.timeout,
            write_timeout=self.write_timeout,
            xonxoff=self.xonxoff,
            rtscts=self.rtscts,
            dsrdtr=self.dsrdtr
        )

        ser.dtr = self.dtr
        ser.rts = self.rts
        ser.port = self.port
        return ser

    def connect(self):
        if self.is_connected:
            return

        last_error = None

        for attempt in range(1, self.open_retries + 1):
            candidate = self._create_serial()

            try:
                candidate.open()
                self.ser = candidate
                time.sleep(0.2)

                if self.remote_on_connect:
                    self.send_scpi("SYSTem:REMote")

                print(f"[Power] Connected to UDP-6720 ({self.port})")
                return

            except Exception as exc:
                last_error = exc

                try:
                    if candidate.is_open:
                        candidate.close()
                except Exception:
                    pass

                self.ser = None

                if attempt < self.open_retries:
                    print(
                        f"[Power] Failed to open {self.port}, "
                        f"retrying {attempt}/{self.open_retries}: {exc}"
                    )
                    time.sleep(self.retry_delay)

        raise ConnectionError(
            f"Failed to connect to UDP-6720 on {self.port} "
            f"after {self.open_retries} attempts: {last_error}"
        )

    def disconnect(self):
        ser = self.ser

        if ser is None:
            return

        try:
            if ser.is_open:
                try:
                    if self.output_off_on_disconnect:
                        self.send_scpi("OUTPut OFF")
                    if self.local_on_disconnect:
                        self.send_scpi("SYSTem:LOCal")
                except Exception:
                    pass

                try:
                    ser.flush()
                except Exception:
                    pass

                time.sleep(0.2)

        finally:
            try:
                if ser.is_open:
                    ser.close()
            except Exception:
                pass

            self.ser = None

            if self.close_delay:
                time.sleep(self.close_delay)

            print("[Power] Disconnected from UDP-6720")

    def send_scpi(self, command):
        if not self.is_connected:
            raise ConnectionError("The power supply is not connected")

        data = str(command).rstrip("\r\n") + "\r\n"

        with self._io_lock:
            self.ser.write(data.encode("ascii", errors="ignore"))
            self.ser.flush()

        time.sleep(0.05)

    def query_scpi(self, command):
        if not self.is_connected:
            raise ConnectionError("The power supply is not connected")

        data = str(command).rstrip("\r\n") + "\r\n"

        with self._io_lock:
            self.ser.write(data.encode("ascii", errors="ignore"))
            self.ser.flush()
            response = self.ser.readline()

        return response.decode("ascii", errors="ignore").strip()

    def set_voltage(self, voltage):
        self.send_scpi(f"VOLTage {voltage}")

    def set_current(self, current):
        self.send_scpi(f"CURRent {current}")

    def set_protect_voltage(self, voltage):
        self.send_scpi(f"VOLTage:PROTection {voltage}")
        self.send_scpi("VOLTage:PROTection:STATe ON")

    def set_protect_current(self, current):
        self.send_scpi(f"CURRent:PROTection {current}")
        self.send_scpi("CURRent:PROTection:STATe ON")

    def turn_on(self):
        self.send_scpi("OUTPut ON")
        time.sleep(1.0)

    def turn_off(self):
        if self.is_connected:
            self.send_scpi("OUTPut OFF")

    def measure_all(self):
        response = self.query_scpi("MEASure:ALL?")

        if not response:
            return 0.0, 0.0, 0.0

        try:
            voltage, current, power = map(float, response.split(","))
            return voltage, current, power
        except (TypeError, ValueError):
            return 0.0, 0.0, 0.0

    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.disconnect()
        return False
