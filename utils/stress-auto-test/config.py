CURRENT_TASK = {
    "board": "LS2K1000LA",
    "os": "SylixOS",
    "job": "file",               #任务类型：cpu/memory/file/standby(待机)
    "is_debug": False,              #是否开启快速验证
    "power_port": "COM3",           #电源串口
    "dut_conn_type": "TELNET",      #通信方式
    "dut_conn_params": {
        "ip": "10.4.120.104",
        "username": "root",
        "password": "root"
    }
}

BOARD_PROFILES = {
    "LS2K1000LA": {"voltage": 12.0, "current": 2.0, "protect_voltage": 13.2, "protect_current": 2.2},
}

OS_PROFILES = {
    "SylixOS": {
        "start_cmd": ["cd /apps/stress-ng/", "./rtos_stress"],
        "shutdown_cmd": ["sync", "shutdown"]
    },
}

STRESS_CONFIG = {
    "start_regex": r"Initializing Job: stored_jobfile_(\w+)\.txt\.\.\.",
    "end_regex": r"Job: stored_jobfile_(\w+)\.txt \[Built-in\]"
}
