CURRENT_TASK = {
    "board": "Orange-Pi5", #LS2K1000LA MIC-7700 DH-1 Phytium-Pi Orange-Pi5
    "os": "rede",
    "jobs": ["cpu", "memory", "file"],
    # Stop the queue when one job fails after all retry attempts.
    "stop_on_job_failure": True
}

POWER_SUPPLY_CONFIG = {
    "port": "COM4",
    "baudrate": 9600,
    "timeout": 1.0,
    "write_timeout": 1.0,
    "open_retries": 3,
    "retry_delay": 1.0,
    "close_delay": 1.0
}


def _telnet(ip, login=False, username="", password=""):
    return {
        "ip": ip,
        "port": 23,
        "login": login,
        "username": username,
        "password": password,

        # Timeout for one TCP connection/login attempt.
        "timeout": 10,

        # Wait after a TCP connection is established when login is disabled.
        "connect_wait": 2.0,
        "login_prompt": "login:",
        "password_prompt": "password:",

        # Maximum time to wait for ICMP ping after powering on the board.
        "ping_timeout": 120,

        # Interval between ping attempts.
        "ping_interval": 3,

        # Timeout for one ping command.
        "ping_attempt_timeout": 1,

        # Number of TELNET connection attempts after ping succeeds.
        "connect_retries": 10,

        # Delay between TELNET connection attempts.
        "connect_retry_interval": 3
    }



def _com(port="COM3", baudrate=1500000):
    return {
        "port": port,
        "baudrate": baudrate,
        "timeout": 0.1,
        "boot_wait": 2.0
    }


def _dut_params(ip, com_port="COM3"):
    return {
        "TELNET": _telnet(ip),
        "COM": _com(com_port)
    }


def _platform(ip, start_cmd, shutdown_cmd, dut_conn_type="COM", com_port="COM3"):
    return {
        "dut_conn_type": dut_conn_type,
        "dut_conn_params": _dut_params(ip, com_port=com_port),
        "start_cmd": start_cmd,
        "shutdown_cmd": shutdown_cmd
    }


SYLIXOS_START_CMD = {
    "standby": [],
    "cpu": ["cd /apps/stress-ng/", "./rtos_bench test-stress --job cpu"],
    "memory": ["cd /apps/stress-ng/", "./rtos_bench test-stress --job memory"],
    "file": ["cd /apps/stress-ng/", "./rtos_bench test-stress --job file"],
    "default": ["cd /apps/stress-ng/", "./rtos_bench test-stress"]
}

ONEOS_START_CMD = {
    "standby": [],
    "cpu": ["ld /user/orange_pi_out.out", "cd /user/", "rtbench test-stress --job cpu"],
    "memory": ["ld /user/orange_pi_out.out", "cd /user/", "rtbench test-stress --job memory"],
    "file": ["ld /user/orange_pi_out.out", "cd /user/", "rtbench test-stress --job file"],
    "default": ["ld /user/orange_pi_out.out", "cd /user/", "rtbench test-stress"]
}

INTEWELL_START_CMD = {
    "standby": [],
    "cpu": ["rtbench test-stress --job cpu"],
    "memory": ["rtbench test-stress --job memory"],
    "file": ["rtbench test-stress --job file"],
    "default": ["rtbench test-stress"]
}

REDE_PERE_CMD = ["mmc dev 0;mmc read 0x9400000 0x600000 0x8000;go 0x9400000", "mount(\"dosfs\",\"/dev/mmc0p2\",\"/c\")", "cd /c", "ld \"rtosbench-new.out\""]

REDE_START_CMD = {
    "standby": REDE_PERE_CMD,
    "cpu": REDE_PERE_CMD + ["rtbench_test_stress_cpu"],
    "memory": REDE_PERE_CMD + ["rtbench_test_stress_memory"],
    "file": REDE_PERE_CMD + ["rtbench_test_stress_file"],
    "default": REDE_PERE_CMD + ["rtbench_test_stress_all"]
}

SYLIXOS_SHUTDOWN_CMD = ["sync", "shutdown"]
ONEOS_SHUTDOWN_CMD = []
INTEWELL_SHUTDOWN_CMD = []
REDE_SHUTDOWN_CMD = []

PLATFORM_PROFILES = {
    "LS2K1000LA": {
        "power": {
            "voltage": 12.0,
            "current": 2.0,
            "protect_voltage": 13.2,
            "protect_current": 2.2
        },
        "platforms": {
            "SylixOS": _platform(
                "192.168.137.200",
                SYLIXOS_START_CMD,
                SYLIXOS_SHUTDOWN_CMD
            ),
            "rede": _platform(
                "192.168.137.218",
                REDE_START_CMD,
                REDE_SHUTDOWN_CMD
            ),
            "oneos": _platform(
                "192.168.137.213",
                ONEOS_START_CMD,
                ONEOS_SHUTDOWN_CMD
            )
        }
    },

    "Orange-Pi5": {
        "power": {
            "voltage": 5.0,
            "current": 4.0,
            "protect_voltage": 5.5,
            "protect_current": 4.4
        },
        "platforms": {
            "SylixOS": _platform(
                "192.168.137.201",
                SYLIXOS_START_CMD,
                SYLIXOS_SHUTDOWN_CMD
            ),
            "intewell": _platform(
                "192.168.137.216",
                INTEWELL_START_CMD,
                INTEWELL_SHUTDOWN_CMD
            ),
            "oneos": _platform(
                "192.168.137.217",
                ONEOS_START_CMD,
                ONEOS_SHUTDOWN_CMD
            ),
            "rede": _platform(
                "192.168.137.212",
                REDE_START_CMD,
                REDE_SHUTDOWN_CMD
            )
        }
    },

    "DH-1": {
        "power": {
            "voltage": 5.0,
            "current": 2.0,
            "protect_voltage": 5.5,
            "protect_current": 2.2
        },
        "platforms": {
            "SylixOS": _platform(
                "192.168.137.202",
                SYLIXOS_START_CMD,
                SYLIXOS_SHUTDOWN_CMD
            ),
            "oneos": _platform(
                "192.168.137.211",
                ONEOS_START_CMD,
                ONEOS_SHUTDOWN_CMD
            )
        }
    },

    "MIC-7700": {
        "power": {
            "voltage": 20.0,
            "current": 7.0,
            "protect_voltage": 22.4,
            "protect_current": 7.7
        },
        "platforms": {
            "SylixOS": _platform(
                "192.168.137.203",
                SYLIXOS_START_CMD,
                SYLIXOS_SHUTDOWN_CMD
            ),
            "intewell": _platform(
                "192.168.137.215",
                INTEWELL_START_CMD,
                INTEWELL_SHUTDOWN_CMD
            )
        }
    },

    "Phytium-Pi": {
        "power": {
            "voltage": 12.0,
            "current": 3.0,
            "protect_voltage": 13.2,
            "protect_current": 3.3
        },
        "platforms": {
            "SylixOS": _platform(
                "192.168.137.204",
                SYLIXOS_START_CMD,
                SYLIXOS_SHUTDOWN_CMD
            ),
            "oneos": _platform(
                "192.168.31.211",
                ONEOS_START_CMD,
                ONEOS_SHUTDOWN_CMD
            ),
            "rede": _platform(
                "192.168.31.210",
                REDE_START_CMD,
                REDE_SHUTDOWN_CMD
            )
        }
    }
}

JOB_PROFILES = {
    "standby": {
        "mode": "duration",
        "duration": 600,
        "start_cmd": None
    },
    "cpu": {
        "mode": "regex",
        "start_cmd": None
    },
    "memory": {
        "mode": "regex",
        "start_cmd": None
    },
    "file": {
        "mode": "regex",
        "start_cmd": None
    }
}

STRESS_CONFIG = {
    "start_regex": r"Initializing Job: stored_jobfile_(\w+)\.txt\.\.\.",
    "end_regex": r"Job: stored_jobfile_(\w+)\.txt \[Built-in\]"
}

EXCEL_EXPORTER = {
    "use_module": True,
    "module": "excel_exporter",
    "output_path": "record",
    "merge_result_json": "data/rtt/result.json",
    "power_unit": "J"
}
