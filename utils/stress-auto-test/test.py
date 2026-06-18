import argparse
import sys
import time

import config
from dut_connection import SerialConnection, TelnetConnection
from power_supply import UDP6720
from profile_resolver import resolve_task


def _create_dut_connection(connection_type, connection_params):
    if connection_type == "TELNET":
        return TelnetConnection(**connection_params)

    if connection_type == "COM":
        return SerialConnection(**connection_params)

    raise ValueError(f"Unsupported DUT connection type: {connection_type}")


def test_power():
    runtime = resolve_task(config.CURRENT_TASK)
    board_name = runtime["board_name"]
    power_profile = runtime["power_profile"]
    power = UDP6720(**runtime["power_supply_config"])

    try:
        print(
            f"[Test] Power test started. board={board_name}, "
            f"port={runtime['power_supply_config']['port']}"
        )

        power.connect()
        power.set_protect_voltage(power_profile["protect_voltage"])
        power.set_protect_current(power_profile["protect_current"])
        power.set_voltage(power_profile["voltage"])
        power.set_current(power_profile["current"])
        power.turn_on()

        for index in range(3):
            time.sleep(5)
            voltage, current, power_value = power.measure_all()
            print(
                f"[Test] Sample {index + 1}: "
                f"V={voltage} V, I={current} A, P={power_value} W"
            )

    finally:
        power.disconnect()
        print("[Test] Power test finished.")


def test_dut(args):
    runtime = resolve_task(
        config.CURRENT_TASK,
        connection_type=args.connection
    )
    connection_type = runtime["dut_conn_type"]
    connection_params = runtime["dut_conn_params"]

    print(
        f"[Test] DUT connection test started. "
        f"board={runtime['board_name']}, os={runtime['os_name']}, "
        f"connection={connection_type}"
    )

    connection = None

    try:
        connection = _create_dut_connection(
            connection_type,
            connection_params
        )
        print("[Test] DUT connection established.")

        if args.command:
            print(f"[Test] Sending commands: {args.command}")
            connection.send_cmd(args.command)

        if args.expect:
            print(f"[Test] Waiting for pattern: {args.expect}")
            connection.wait_for_regex(args.expect, timeout=args.expect_timeout)
            print("[Test] Expected pattern matched.")
        else:
            print(
                f"[Test] Reading available output for "
                f"{args.read_seconds:g} seconds..."
            )
            connection.read_for(args.read_seconds)

        print("[Test] DUT connection test passed.")

    finally:
        if connection:
            connection.disconnect()
        print("[Test] DUT connection test finished.")


def show_profile():
    runtime = resolve_task(config.CURRENT_TASK)
    safe_connection_params = dict(runtime["dut_conn_params"])

    if safe_connection_params.get("password"):
        safe_connection_params["password"] = "******"

    print(f"board: {runtime['board_name']}")
    print(f"os: {runtime['os_name']}")
    print(f"dut_conn_type: {runtime['dut_conn_type']}")
    print(f"dut_conn_params: {safe_connection_params}")
    print(f"power_profile: {runtime['power_profile']}")
    print(f"power_supply_config: {runtime['power_supply_config']}")


def build_parser():
    parser = argparse.ArgumentParser(
        description="Hardware connection tests for stress-auto-test."
    )
    subparsers = parser.add_subparsers(dest="test_name")

    subparsers.add_parser(
        "power",
        help="Test the external programmable power supply."
    )

    dut_parser = subparsers.add_parser(
        "dut",
        help="Test the selected DUT TELNET or COM connection."
    )
    dut_parser.add_argument(
        "--connection",
        choices=["TELNET", "COM"],
        default=None,
        help="Override dut_conn_type for this test only."
    )
    dut_parser.add_argument(
        "--command",
        action="append",
        default=[],
        help="Command to send. This option can be specified multiple times."
    )
    dut_parser.add_argument(
        "--expect",
        default=None,
        help="Regular expression expected from DUT output."
    )
    dut_parser.add_argument(
        "--expect-timeout",
        type=float,
        default=10.0,
        help="Timeout used with --expect."
    )
    dut_parser.add_argument(
        "--read-seconds",
        type=float,
        default=2.0,
        help="Time used to read DUT output when --expect is omitted."
    )

    subparsers.add_parser(
        "profile",
        help="Print the resolved current board/platform configuration."
    )

    return parser


def main():
    parser = build_parser()
    args = parser.parse_args()

    try:
        if args.test_name in (None, "power"):
            test_power()
        elif args.test_name == "dut":
            test_dut(args)
        elif args.test_name == "profile":
            show_profile()
        else:
            parser.print_help()
            return 2

    except KeyboardInterrupt:
        print("\n[Test] Interrupted by user.")
        return 130
    except Exception as exc:
        print(f"[Test] Failed: {exc}")
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
