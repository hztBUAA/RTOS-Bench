from time import sleep

from power_supply import UDP6720
import config
from dut_connection import SSHConnection, SerialConnection, TelnetConnection

def testPower():
    task = config.CURRENT_TASK
    board_name = task["board"]
    board_cfg = config.BOARD_PROFILES[board_name]

    power = UDP6720(port=task["power_port"], baudrate=9600)
    power.connect()

    power.set_protect_voltage(board_cfg["protect_voltage"])
    power.set_protect_current(board_cfg["protect_current"])
    power.set_voltage(board_cfg["voltage"])
    power.set_current(board_cfg["current"])
    power.turn_on()

    sleep(5)
    v,i,p = power.measure_all()
    print(f"V:{v} V,I:{i} A,P:{p} W")
    sleep(5)
    v, i, p = power.measure_all()
    print(f"V:{v} V,I:{i} A,P:{p} W")
    sleep(5)
    v, i, p = power.measure_all()
    print(f"V:{v} V,I:{i} A,P:{p} W")
    sleep(5)

    power.turn_off()

    power.disconnect()

if __name__ == "__main__":
    testPower()