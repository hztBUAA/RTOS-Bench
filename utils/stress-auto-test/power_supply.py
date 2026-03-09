import serial
import time


class UDP6720:
    def __init__(self, port, baudrate=9600, timeout=1.0):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.ser = None

    def connect(self):
        self.ser = serial.Serial()
        self.ser.port = self.port
        self.ser.baudrate = self.baudrate
        self.ser.timeout = self.timeout

        self.ser.rtscts = False
        self.ser.dsrdtr = False

        try:
            self.ser.open()

            self.ser.setDTR(False)
            self.ser.setRTS(False)

            print(f"[Power] ⚡ 已成功连接 UDP-6720 ({self.port})")
            self.send_scpi("SYSTem:REMote")
        except Exception as e:
            raise ConnectionError(f"电源连接失败: {e}")

    def disconnect(self):
        if self.ser and self.ser.is_open:
            try:
                self.send_scpi("OUTPut OFF")
                self.send_scpi("SYSTem:LOCal")
                self.ser.flush()
                time.sleep(0.2)
                self.ser.reset_input_buffer()
                self.ser.reset_output_buffer()
            except Exception:
                pass
            finally:
                self.ser.close()
                print("[Power] 🛑 已断开电源连接")

    def send_scpi(self, cmd):
        if self.ser and self.ser.is_open:
            self.ser.write((cmd + '\r\n').encode('utf-8'))
            time.sleep(0.05)

    def query_scpi(self, cmd):
        if self.ser and self.ser.is_open:
            self.ser.write((cmd + '\r\n').encode('utf-8'))
            return self.ser.readline().decode('utf-8', errors='ignore').strip()
        return ""

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
        time.sleep(1)

    def turn_off(self):
        self.send_scpi("OUTPut OFF")

    def measure_all(self):
        resp = self.query_scpi("MEASure:ALL?")
        if not resp:
            return 0.0, 0.0, 0.0
        try:
            v, i, p = map(float, resp.split(','))
            return v, i, p
        except Exception:
            return 0.0, 0.0, 0.0
