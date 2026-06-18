# Stress Auto Test 工具使用指南

## 概述

Stress Auto Test 是一个自动化功耗测试工具，用于在 RTOS 平台上执行压力测试，并同步采集开发板功耗数据。当前项目集成了外部程控电源控制、DUT 连接、压力任务启动、功耗采样、Excel 导出和 JSON 结果合并功能。

当前版本的核心设计是：

- 板卡电压、电流参数统一放在 `PLATFORM_PROFILES` 的板卡层级。
- DUT 连接参数放在 `PLATFORM_PROFILES` 的板卡 + 平台组合层级。
- DUT 连接参数分为 `TELNET` 和 `COM` 两组。
- `CURRENT_TASK` 只负责选择当前板卡、平台和任务队列。
- 支持按顺序执行多个任务，例如 `standby`、`cpu`、`memory`、`file`。
- 当前项目支持 `TELNET` 和 `COM` 两种 DUT 连接方式；`SERIAL` 在解析时会被兼容映射为 `COM`。
- 当前项目不再使用 `SSH`、`BOARD_PROFILES`、`OS_PROFILES`。

## 目录结构

```text
utils/stress-auto-test/
├── __init__.py
├── config.py                 # 全局配置、板卡配置、平台配置、任务配置
├── main.py                   # 自动化测试主入口
├── test.py                   # 单项连接测试入口
├── profile_resolver.py       # 配置解析与校验
├── dut_connection.py         # DUT TELNET / COM 连接实现
├── power_supply.py           # UDP6720 程控电源控制
├── recorder.py               # 功耗采样与能量积分
├── excel_exporter.py         # Excel 导出与 JSON 合并
├── patch_pyserial_win32.py   # Windows pySerial WinError 31 补丁工具
└── requirements.txt          # Python 依赖
```

## 快速开始

### 1. 安装依赖

建议先进入工具目录再执行命令，避免相对路径输出到错误位置。

```bash
cd utils/stress-auto-test
pip install -r requirements.txt
```

当前 `requirements.txt` 内容为：

```text
pyserial
pandas
openpyxl
```

### 2. 配置当前测试任务

在 `config.py` 中修改 `CURRENT_TASK`。

```python
CURRENT_TASK = {
    "board": "Phytium-Pi",
    "os": "oneos",
    "jobs": ["standby", "cpu", "memory", "file"]
}
```

字段说明：

| 字段 | 类型 | 说明 |
|---|---:|---|
| `board` | `str` | 当前测试板卡名称，必须存在于 `PLATFORM_PROFILES` |
| `os` | `str` | 当前平台/系统名称，必须存在于该板卡的 `platforms` |
| `jobs` | `list[str]` | 任务队列，按顺序执行 |

当前项目仍兼容旧的单任务字段 `job`，但推荐统一使用 `jobs`。

示例：

```python
CURRENT_TASK = {
    "board": "LS2K1000LA",
    "os": "SylixOS",
    "jobs": ["cpu"]
}
```

```python
CURRENT_TASK = {
    "board": "Orange-Pi5",
    "os": "rede",
    "jobs": ["standby", "cpu", "memory", "file"]
}
```

## 配置结构

### POWER_SUPPLY_CONFIG

外部 UDP6720 程控电源配置单独放在 `POWER_SUPPLY_CONFIG` 中。

```python
POWER_SUPPLY_CONFIG = {
    "port": "COM3",
    "baudrate": 9600,
    "timeout": 1.0,
    "write_timeout": 1.0,
    "open_retries": 3,
    "retry_delay": 1.0,
    "close_delay": 1.0
}
```

字段说明：

| 字段 | 说明 |
|---|---|
| `port` | 程控电源串口，例如 Windows 下 `COM3` |
| `baudrate` | 电源串口波特率，UDP6720 通常为 `9600` |
| `timeout` | 串口读超时 |
| `write_timeout` | 串口写超时 |
| `open_retries` | 打开串口失败后的重试次数 |
| `retry_delay` | 打开串口失败后的重试间隔 |
| `close_delay` | 关闭串口后等待驱动释放的时间 |

### PLATFORM_PROFILES

`PLATFORM_PROFILES` 是当前项目的核心配置。它同时管理：

- 板卡供电参数；
- 板卡 + 系统组合；
- 每个组合的 DUT 连接方式；
- 每个组合的 TELNET / COM 参数；
- 每个组合的压力测试启动命令；
- 每个组合的关机命令。

基本结构：

```python
PLATFORM_PROFILES = {
    "BoardName": {
        "power": {
            "voltage": 12.0,
            "current": 2.0,
            "protect_voltage": 13.2,
            "protect_current": 2.2
        },
        "platforms": {
            "OSName": {
                "dut_conn_type": "TELNET",
                "dut_conn_params": {
                    "TELNET": {...},
                    "COM": {...}
                },
                "start_cmd": {...},
                "shutdown_cmd": [...]
            }
        }
    }
}
```

### 板卡电源配置

每个板卡的电压、电流配置放在板卡层级的 `power` 中。

```python
"Phytium-Pi": {
    "power": {
        "voltage": 12.0,
        "current": 3.0,
        "protect_voltage": 13.2,
        "protect_current": 3.3
    },
    "platforms": {
        ...
    }
}
```

字段说明：

| 字段 | 说明 |
|---|---|
| `voltage` | 正常供电电压 |
| `current` | 正常限流值 |
| `protect_voltage` | 过压保护阈值 |
| `protect_current` | 过流保护阈值 |

### DUT 连接配置

每个板卡 + 系统组合中都有：

```python
"dut_conn_type": "TELNET",
"dut_conn_params": {
    "TELNET": {...},
    "COM": {...}
}
```

`dut_conn_type` 决定主程序实际使用哪一组参数。

切换为 TELNET：

```python
"dut_conn_type": "TELNET"
```

切换为串口：

```python
"dut_conn_type": "COM"
```

### TELNET 参数

`config.py` 中通过 `_telnet()` 生成 TELNET 配置。

```python
def _telnet(ip, login=False, username="", password=""):
    return {
        "ip": ip,
        "port": 23,
        "login": login,
        "username": username,
        "password": password,
        "timeout": 10,
        "connect_wait": 2.0,
        "login_prompt": "login:",
        "password_prompt": "password:"
    }
```

字段说明：

| 字段 | 说明 |
|---|---|
| `ip` | DUT 内网 IP |
| `port` | TELNET 端口，默认 `23` |
| `login` | 是否启用登录流程 |
| `username` | TELNET 用户名 |
| `password` | TELNET 密码 |
| `timeout` | 单次 TELNET TCP 连接或登录等待超时 |
| `connect_wait` | 无登录模式下，连接建立后等待 DUT 输出的时间 |
| `login_prompt` | 登录用户名提示符 |
| `password_prompt` | 登录密码提示符 |

无登录 TELNET：

```python
"TELNET": {
    "ip": "192.168.31.205",
    "port": 23,
    "login": False,
    "username": "",
    "password": "",
    "timeout": 10,
    "connect_wait": 2.0,
    "login_prompt": "login:",
    "password_prompt": "password:"
}
```

需要账号密码的 TELNET：

```python
"TELNET": {
    "ip": "192.168.31.205",
    "port": 23,
    "login": True,
    "username": "root",
    "password": "root",
    "timeout": 10,
    "connect_wait": 2.0,
    "login_prompt": "login:",
    "password_prompt": "password:"
}
```

注意：当前上传项目中的 `main.py` 会在 TELNET 连接前先 `ping` DUT。`wait_for_ping()` 当前没有总超时，DUT 如果一直 ping 不通，主程序会持续等待。

### COM 参数

`config.py` 中通过 `_com()` 生成 COM 串口配置。

```python
def _com(port="COM6", baudrate=115200):
    return {
        "port": port,
        "baudrate": baudrate,
        "timeout": 0.1,
        "boot_wait": 2.0
    }
```

字段说明：

| 字段 | 说明 |
|---|---|
| `port` | DUT 串口，例如 `COM6` |
| `baudrate` | DUT 串口波特率 |
| `timeout` | 串口读超时 |
| `boot_wait` | 打开串口后等待 DUT 输出的时间 |

示例：

```python
"COM": {
    "port": "COM6",
    "baudrate": 115200,
    "timeout": 0.1,
    "boot_wait": 2.0
}
```

## 当前板卡与网络配置

当前 `config.py` 中已经配置了以下板卡、系统和 IP。

| 板卡名称 | 系统名称 | IP |
|---|---|---|
| `LS2K1000LA` | `SylixOS` | `192.168.31.200` |
| `Orange-Pi5` | `SylixOS` | `192.168.31.201` |
| `DH-1` | `SylixOS` | `192.168.31.202` |
| `MIC-7700` | `SylixOS` | `192.168.31.203` |
| `Phytium-Pi` | `SylixOS` | `192.168.31.204` |
| `Phytium-Pi` | `oneos` | `192.168.31.205` |
| `MIC-7700` | `intewell` | `192.168.31.206` |
| `Orange-Pi5` | `intewell` | `192.168.31.207` |
| `Orange-Pi5` | `oneos` | `192.168.31.208` |
| `LS2K1000LA` | `rede` | `192.168.31.209` |
| `Phytium-Pi` | `rede` | `192.168.31.210` |
| `DH-1` | `oneos` | `192.168.31.211` |
| `Orange-Pi5` | `rede` | `192.168.31.212` |
| `LS2K1000LA` | `oneos` | `192.168.31.213` |

名称对应关系：

| 中文名称     | 配置名称 |
|----------|---|
| 龙芯 / 龙芯派 | `LS2K1000LA` |
| 香橙派      | `Orange-Pi5` |
| 哪吒 / 哪吒派 | `DH-1` |
| 工控机      | `MIC-7700` |
| 飞腾派      | `Phytium-Pi` |
| 翼辉       | `SylixOS` |
| 东土       | `intewell` |
| 锐华       | `rede` |
| 中移OneOS  | `oneos` |

## 平台命令配置

### start_cmd

`start_cmd` 支持按任务配置命令。

```python
ONEOS_START_CMD = {
    "standby": [],
    "cpu": ["cd /user/", "ld xx.out", "rtbench --job cpu"],
    "memory": ["cd /user/", "ld xx.out", "rtbench --job memory"],
    "file": ["cd /user/", "ld xx.out", "rtbench --job file"],
    "default": ["cd /user/", "ld xx.out", "rtbench"]
}
```

主程序会根据当前任务名选择对应命令。例如任务为 `cpu` 时，发送：

```text
cd /user/
ld xx.out
rtbench --job cpu
```

`standby` 默认命令为空：

```python
"standby": []
```

表示只上电并采集功耗，不向 DUT 发送压力测试命令。

### shutdown_cmd

每个平台都有自己的关机命令。

```python
ONEOS_SHUTDOWN_CMD = ["sync", "shutdown"]
SYLIXOS_SHUTDOWN_CMD = ["sync", "shutdown"]
INTEWELL_SHUTDOWN_CMD = ["reboot"]
REDE_SHUTDOWN_CMD = ["sync", "shutdown"]
```

主程序在任务结束或异常清理阶段，如果 DUT 连接已经建立，会尝试发送 `shutdown_cmd`。

## 任务配置

### JOB_PROFILES

任务默认行为配置在 `JOB_PROFILES` 中。

```python
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
```

任务模式说明：

| 模式 | 说明 |
|---|---|
| `duration` | 固定时间采样，常用于待机功耗 |
| `regex` | 等待 DUT 输出开始正则，开始采样；等待结束正则，停止采样 |

`standby` 默认采样 600 秒。

```python
"standby": {
    "mode": "duration",
    "duration": 600,
    "start_cmd": None
}
```

`cpu`、`memory`、`file` 默认使用正则匹配模式。

```python
"cpu": {
    "mode": "regex",
    "start_cmd": None
}
```

### 单个任务覆盖配置

`jobs` 中也可以使用字典覆盖默认配置。

```python
CURRENT_TASK = {
    "board": "Phytium-Pi",
    "os": "oneos",
    "jobs": [
        {
            "name": "standby",
            "duration": 300
        },
        {
            "name": "cpu",
            "start_cmd": ["cd /user/", "rtbench --job cpu"]
        }
    ]
}
```

字段说明：

| 字段 | 说明 |
|---|---|
| `name` | 任务名 |
| `mode` | 可覆盖任务模式 |
| `duration` | duration 模式采样时间 |
| `start_cmd` | 可覆盖平台默认启动命令 |
| `start_regex` | 可覆盖开始正则 |
| `end_regex` | 可覆盖结束正则 |

## STRESS_CONFIG

`STRESS_CONFIG` 用于识别压力测试开始和结束。

```python
STRESS_CONFIG = {
    "start_regex": r"Initializing Job: stored_jobfile_(\w+)\.txt\.\.\.",
    "end_regex": r"Job: stored_jobfile_(\w+)\.txt \[Built-in\]"
}
```

`regex` 模式的流程是：

```text
发送 start_cmd
等待 start_regex
启动功耗采样
等待 end_regex
停止功耗采样
导出结果
```

如果 DUT 实际输出与默认正则不一致，需要修改 `STRESS_CONFIG` 或在单个任务里覆盖正则。

示例：

```python
CURRENT_TASK = {
    "board": "Phytium-Pi",
    "os": "oneos",
    "jobs": [
        {
            "name": "cpu",
            "start_regex": r"CPU test start",
            "end_regex": r"CPU test finished"
        }
    ]
}
```

## EXCEL_EXPORTER

导出配置在 `EXCEL_EXPORTER` 中。

```python
EXCEL_EXPORTER = {
    "use_module": True,
    "module": "excel_exporter",
    "output_path": "record",
    "merge_result_json": "data/rtt/result.json",
    "power_unit": "J"
}
```

字段说明：

| 字段 | 说明 |
|---|---|
| `use_module` | 是否启用内置导出 |
| `module` | 当前固定为 `excel_exporter` |
| `output_path` | Excel 输出目录或完整文件名 |
| `merge_result_json` | 要合并功耗结果的 JSON 文件路径 |
| `power_unit` | JSON 中记录的功耗单位，通常为 `J` |

如果 `output_path` 是目录，文件名自动生成：

```text
{os}_{board}_{JOB}_{timestamp}.xlsx
```

示例：

```text
oneos_Phythium-Pi_CPU_20260618_134500.xlsx
```

当前 Excel 工作表包括：

| 工作表 | 说明 |
|---|---|
| `测试汇总` | 板卡、系统、任务、总耗时、平均功率、总功耗等 |
| `详细采样数据` | 每次采样的时间、电压、电流、功率 |

### JSON 合并结构

当前 `excel_exporter.py` 会把功耗数据合并到：

```json
{
  "modules": {
    "test-stress": {
      "power": [
        {
          "name": "cpu",
          "power_data": 123.4567,
          "power_unit": "J"
        }
      ]
    }
  }
}
```

注意：如果 `merge_result_json` 指定的文件不存在，导出 Excel 成功后，JSON 合并会失败并记录错误。

## 运行主测试

在工具目录中运行：

```bash
python main.py
```

主流程：

```text
解析 CURRENT_TASK
解析板卡 power 配置
解析平台 DUT 连接配置
按 jobs 顺序执行任务
每个任务：
  连接 UDP6720
  设置保护电压/电流
  设置供电电压/电流
  打开电源
  TELNET 模式下等待 ping 通
  建立 TELNET 或 COM 连接
  发送 start_cmd
  根据任务模式采样功耗
  导出 Excel
  合并 JSON
  发送 shutdown_cmd
  关闭电源
  断开连接
```

当前上传项目中的 `main.py` 行为说明：

- 每个 job 都会重新上电、连接 DUT、执行、关机、断电。
- TELNET 模式下，`ping` 通后只尝试建立一次 TELNET 连接。
- 如果 TELNET 连接失败，当前 job 会记录失败，然后进入下一个 job。
- 当前 `wait_for_ping()` 没有总超时；如果 DUT 一直 ping 不通，会持续等待。
- 当前 `wait_for_regex()` 如果没有传入 timeout，会一直等待匹配输出。

如果需要 TELNET 连接失败后重试，需要在 `main.py` 中增加连接重试逻辑，并在 `config.py` 的 TELNET 参数中增加 `connect_retries`、`connect_retry_interval` 等字段。

## 单项测试命令

`test.py` 用于单独验证配置、电源和 DUT 连接。

### 查看解析后的配置

```bash
python test.py profile
```

输出内容包括：

```text
board
os
dut_conn_type
dut_conn_params
power_profile
power_supply_config
```

### 测试程控电源

```bash
python test.py power
```

流程：

```text
连接 UDP6720
设置保护参数
设置供电参数
打开输出
采样 3 次电压、电流、功率
断开电源
```

### 测试当前配置中的 DUT 连接

```bash
python test.py dut
```

### 临时指定连接方式

测试 TELNET：

```bash
python test.py dut --connection TELNET
```

测试 COM：

```bash
python test.py dut --connection COM
```

### 发送命令

```bash
python test.py dut --command "help"
```

发送多条命令：

```bash
python test.py dut \
  --command "cd /user/" \
  --command "rtbench --job cpu"
```

Windows PowerShell 示例：

```powershell
python test.py dut --command "cd /user/" --command "rtbench --job cpu"
```

### 等待指定输出

```bash
python test.py dut --command "help" --expect "usage|help" --expect-timeout 10
```

如果不使用 `--expect`，默认读取输出 2 秒：

```bash
python test.py dut --read-seconds 5
```

注意：当前上传项目中的 `test.py` 不支持通过命令行临时覆盖 TELNET IP 或端口。如果需要改 IP，需要修改 `config.py` 中对应平台的 `_platform(...)` 配置。

## Windows pySerial 兼容性补丁

当前项目包含：

```text
patch_pyserial_win32.py
```

用于处理 Windows + USB 虚拟串口 + pySerial 场景下可能出现的错误：

```text
Cannot configure port, something went wrong.
PermissionError(13, '连到系统上的设备没有发挥作用。', None, 31)
```

手动执行：

```bash
python patch_pyserial_win32.py
```

成功时会输出类似：

```text
Patch applied: ...\site-packages\serial\serialwin32.py; backup: ...serialwin32.py.rtos_bench_backup
```

如果已经打过补丁，会输出：

```text
Patch already applied: ...
```

注意：当前上传项目中，`patch_pyserial_win32.py` 是独立工具，并没有在 `power_supply.py` 中自动调用。需要时请手动执行。

同时建议在 Windows 电源选项中关闭：

```text
USB 选择性暂停
```

路径：

```text
控制面板
→ 系统和安全
→ 电源选项
→ 更改计划设置
→ 更改高级电源设置
→ USB 设置
→ USB 选择性暂停设置
→ 已禁用
```

## 日志说明

日志格式：

```text
2026-06-18 13:43:20 INFO [Test] Job queue started | board=Phytium-Pi | os=oneos | connection=TELNET | jobs=['cpu', 'memory', 'file']
2026-06-18 13:43:20 INFO [Test] Start job 1/3 | board=Phytium-Pi | os=oneos | job=cpu | mode=regex | connection=TELNET
2026-06-18 13:43:36 INFO [Test] DUT network is reachable.
2026-06-18 13:43:36 INFO [Test] Connecting to DUT...
```

常见日志含义：

| 日志 | 含义 |
|---|---|
| `Job queue started` | 任务队列启动 |
| `Start job x/y` | 开始第 x 个任务 |
| `DUT network is reachable` | TELNET 模式下 ping 已通 |
| `Connecting to DUT` | 开始连接 TELNET 或 COM |
| `Sending start_cmd exactly as configured` | 发送配置中的启动命令 |
| `Waiting for start pattern` | 等待压力测试开始正则 |
| `Waiting for end pattern` | 等待压力测试结束正则 |
| `Export completed` | Excel / JSON 导出完成 |
| `Job 'xxx' failed` | 当前任务失败 |
| `All jobs completed` | 主程序完成任务循环 |

## 常见问题

### Q1：`UDP6720.__init__()` 报 `unexpected keyword argument 'write_timeout'`

原因：`POWER_SUPPLY_CONFIG` 中配置了 `write_timeout` 等字段，但本地 `power_supply.py` 的 `UDP6720.__init__()` 仍是旧签名。

错误示例：

```text
TypeError: UDP6720.__init__() got an unexpected keyword argument 'write_timeout'
```

处理方式：

- 使用支持 `write_timeout/open_retries/retry_delay/close_delay` 的新版 `power_supply.py`；
- 或把 `POWER_SUPPLY_CONFIG` 临时精简成只包含 `port`、`baudrate`、`timeout`。

### Q2：TELNET 连接失败，但 ping 已经通

ping 通只说明 DUT 的 ICMP 已响应，不代表 TELNET 服务已经启动。

常见错误：

```text
[WinError 10061] 由于目标计算机积极拒绝，无法连接。
```

含义：目标 IP 可达，但 23 端口没有服务监听，或服务拒绝连接。

```text
[WinError 10054] 远程主机强迫关闭了一个现有的连接。
```

含义：TCP 连接建立后，远端主动断开。通常是 TELNET 服务异常、协议不匹配、登录流程不符合预期或端口不是交互式 TELNET shell。

排查顺序：

```bash
ping 192.168.31.205
telnet 192.168.31.205 23
python test.py profile
python test.py dut --connection TELNET --read-seconds 5
```

如果手动 telnet 都失败，主程序无法通过代码修复，需要先修 DUT 端 TELNET 服务，或切换为 COM。

### Q3：如何把某个平台切换为 COM

在 `config.py` 中找到对应板卡和系统，例如：

```python
"Phytium-Pi": {
    "platforms": {
        "oneos": _platform(
            "192.168.31.205",
            ONEOS_START_CMD,
            ONEOS_SHUTDOWN_CMD
        )
    }
}
```

改成：

```python
"oneos": _platform(
    "192.168.31.205",
    ONEOS_START_CMD,
    ONEOS_SHUTDOWN_CMD,
    dut_conn_type="COM",
    com_port="COM6"
)
```

然后测试：

```bash
python test.py dut --connection COM
```

### Q4：程序卡在 ping 阶段

当前 `main.py` 的 `wait_for_ping()` 没有总超时。TELNET 模式下，如果 DUT 一直 ping 不通，程序会一直等待。

排查：

```bash
ping 192.168.31.205
python test.py profile
```

确认：

- `CURRENT_TASK["board"]` 是否正确；
- `CURRENT_TASK["os"]` 是否正确；
- 解析出来的 IP 是否正确；
- PC 和 DUT 是否在同一网段；
- DUT 是否真的完成启动；
- 网线、交换机、防火墙是否正常。

### Q5：测试一直等待 start_regex 或 end_regex

`cpu`、`memory`、`file` 默认是 `regex` 模式。主程序需要从 DUT 输出中匹配：

```python
STRESS_CONFIG = {
    "start_regex": r"Initializing Job: stored_jobfile_(\w+)\.txt\.\.\.",
    "end_regex": r"Job: stored_jobfile_(\w+)\.txt \[Built-in\]"
}
```

如果 DUT 输出不包含这些字符串，程序会一直等待。

排查：

```bash
python test.py dut --command "rtbench --job cpu" --read-seconds 10
```

观察实际输出，然后修改 `STRESS_CONFIG`。

### Q6：Excel 成功导出，但 JSON 合并失败

检查：

```python
EXCEL_EXPORTER = {
    "merge_result_json": "data/rtt/result.json"
}
```

如果该文件不存在，JSON 合并会失败。

处理方式：

- 确认运行目录正确；
- 确认 `data/rtt/result.json` 已生成；
- 或临时关闭 JSON 合并：

```python
EXCEL_EXPORTER = {
    "use_module": True,
    "module": "excel_exporter",
    "output_path": "record",
    "merge_result_json": None,
    "power_unit": "J"
}
```

### Q7：Windows 下第二次连接电源串口失败

可能是 Windows USB 虚拟串口驱动与 pySerial 的兼容问题。

现象：

```text
Cannot configure port, something went wrong.
PermissionError(13, '连到系统上的设备没有发挥作用。', None, 31)
```

处理：

```bash
python patch_pyserial_win32.py
```

同时关闭 Windows USB 选择性暂停。

## 添加新板卡

在 `PLATFORM_PROFILES` 中新增板卡：

```python
"NewBoard": {
    "power": {
        "voltage": 5.0,
        "current": 2.0,
        "protect_voltage": 5.5,
        "protect_current": 2.2
    },
    "platforms": {
        "SylixOS": _platform(
            "192.168.31.220",
            SYLIXOS_START_CMD,
            SYLIXOS_SHUTDOWN_CMD
        )
    }
}
```

然后修改：

```python
CURRENT_TASK = {
    "board": "NewBoard",
    "os": "SylixOS",
    "jobs": ["cpu"]
}
```

## 添加新系统平台

在已有板卡的 `platforms` 中新增系统：

```python
"NewOS": _platform(
    "192.168.31.221",
    {
        "standby": [],
        "cpu": ["cd /test/", "./stress --cpu"],
        "memory": ["cd /test/", "./stress --memory"],
        "file": ["cd /test/", "./stress --file"],
        "default": ["cd /test/", "./stress"]
    },
    ["sync", "shutdown"],
    dut_conn_type="TELNET"
)
```

如果该平台默认走串口：

```python
"NewOS": _platform(
    "192.168.31.221",
    NEW_OS_START_CMD,
    ["sync", "shutdown"],
    dut_conn_type="COM",
    com_port="COM7"
)
```

## 添加新任务

新增任务需要同时配置三处。

第一，在 `CURRENT_TASK` 中使用任务名：

```python
CURRENT_TASK = {
    "board": "Phytium-Pi",
    "os": "oneos",
    "jobs": ["network"]
}
```

第二，在 `JOB_PROFILES` 中添加任务模式：

```python
JOB_PROFILES = {
    "network": {
        "mode": "regex",
        "start_cmd": None
    }
}
```

第三，在平台 `start_cmd` 中添加命令：

```python
ONEOS_START_CMD = {
    "network": ["cd /user/", "rtbench --job network"]
}
```

如果输出格式不同，也需要配置正则：

```python
CURRENT_TASK = {
    "board": "Phytium-Pi",
    "os": "oneos",
    "jobs": [
        {
            "name": "network",
            "start_regex": r"network test start",
            "end_regex": r"network test end"
        }
    ]
}
```

## 功耗采样说明

`recorder.py` 中默认采样间隔为 0.01 秒：

```python
time.sleep(0.01)
```

每条记录格式：

```text
(time_s, voltage_v, current_a, power_w)
```

能量计算使用梯形积分：

```text
E = Σ 平均功率 × 时间间隔
```

导出汇总字段包括：

| 字段 | 说明 |
|---|---|
| `测试总耗时(s)` | 当前任务采样总时间 |
| `平均功率(W)` | 采样期间平均功率 |
| `总功耗(J)` | 焦耳单位总能量 |
| `总功耗(Wh)` | 瓦时单位总能量 |

## 推荐验证流程

首次配置或换板卡时，按以下顺序验证：

```bash
python test.py profile
```

确认解析出的板卡、系统、连接参数和电源参数正确。

```bash
python test.py power
```

确认 UDP6720 能连接、设置参数并采样。

```bash
python test.py dut --connection TELNET --read-seconds 5
```

或：

```bash
python test.py dut --connection COM --read-seconds 5
```

确认 DUT 连接可用。

最后运行完整测试：

```bash
python main.py
```

## 当前版本限制

当前上传项目代码中存在以下限制：

1. 不支持 SSH。
2. `main.py` 没有命令行参数，所有主流程配置都来自 `config.py`。
3. `test.py` 不能通过命令行临时覆盖 TELNET IP。
4. TELNET 连接前的 ping 等待没有总超时。
5. TELNET 连接失败后，当前上传版本不会自动重试。
6. `regex` 模式默认没有等待超时，正则不匹配会一直等待。
7. `patch_pyserial_win32.py` 需要手动运行，当前没有自动集成到 `power_supply.py`。
8. 当前上传版本的 `POWER_SUPPLY_CONFIG` 与旧版 `power_supply.py` 可能存在参数不匹配问题，需要同步修复。

## 相关文件职责

| 文件 | 职责 |
|---|---|
| `config.py` | 配置当前任务、板卡、平台、连接、命令、导出 |
| `profile_resolver.py` | 解析并校验当前任务配置 |
| `main.py` | 执行完整自动化功耗测试 |
| `test.py` | 执行单项电源、DUT、配置测试 |
| `dut_connection.py` | 实现 TELNET 和 COM 连接 |
| `power_supply.py` | 控制 UDP6720 电源 |
| `recorder.py` | 采样功耗并计算能量 |
| `excel_exporter.py` | 导出 Excel 并合并 JSON |
| `patch_pyserial_win32.py` | 修复 Windows pySerial 串口兼容问题 |
:::

