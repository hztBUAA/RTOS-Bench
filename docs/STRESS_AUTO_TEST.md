# Stress Auto Test 工具使用指南

## 概述

Stress Auto Test 是一个自动化功耗测试工具，用于在 RT-Thread、SylixOS 等 RTOS 平台上执行压力测试并同时测量系统的功耗数据。该工具集成了电源管理、设备连接、数据采集和结果导出功能，支持自动化持续集成测试。

**核心能力**：
- 支持多种连接方式：SSH、Telnet、Serial
- 自动功耗数据采集与能量计算
- 支持多种测试任务：CPU、Memory、File I/O、Standby（待机）
- Excel 数据导出和 JSON 结果合并
- 灵活的硬件配置管理
- 完整的日志记录和异常处理

## 目录结构

```
utils/stress-auto-test/
├── main.py              # 主程序入口
├── config.py            # 配置文件
├── dut_connection.py    # DUT 连接管理（SSH/Telnet/Serial）
├── power_supply.py      # UDP6720 电源管理
├── recorder.py          # 功耗数据采集
├── excel_exporter.py    # 结果导出
├── requirements.txt     # 依赖包列表
└── __init__.py
```

## 快速开始

### 1. 环境准备

#### 安装依赖

```bash
cd utils/stress-auto-test
pip install -r requirements.txt
```

#### 必要硬件
- UDP6720 电源模块（通过 USB/Serial 连接）
- 目标设备（DUT）及其网络连接
- 支持 SSH、Telnet 或 Serial 的通信方式

### 2. 配置系统参数

编辑 `config.py` 文件，根据实际环境配置以下内容：

```python
CURRENT_TASK = {
    "board": "LS2K1000LA",        # 板卡型号
    "os": "SylixOS",              # 操作系统名称
    "job": "file",                # 测试任务：cpu/memory/file/standby
    "is_debug": False,            # 快速验证模式（可选）
    "power_port": "COM3",         # 电源串口（Windows）或设备路径（Linux）
    "dut_conn_type": "TELNET",    # 连接方式：SSH/TELNET/SERIAL
    "dut_conn_params": {
        "ip": "10.4.120.104",     # DUT IP 地址（SSH/TELNET 必需）
        "username": "root",       # 用户名
        "password": "root"        # 密码
    }
}
```

### 3. 配置硬件参数

在 `BOARD_PROFILES` 中添加或编辑目标板卡的电源参数：

```python
BOARD_PROFILES = {
    "LS2K1000LA": {
        "voltage": 12.0,          # 工作电压 (V)
        "current": 2.0,           # 工作电流 (A)
        "protect_voltage": 13.2,  # 过压保护电压 (V)
        "protect_current": 2.2    # 过流保护电流 (A)
    },
    # 添加其他板卡...
}
```

### 4. 配置操作系统命令

在 `OS_PROFILES` 中配置目标系统的启动和关闭命令：

```python
OS_PROFILES = {
    "SylixOS": {
        "start_cmd": [
            "cd /apps/stress-ng/",
            "./rtos_stress"
        ],
        "shutdown_cmd": ["sync", "shutdown"]
    },
    # 添加其他系统...
}
```

### 5. 配置结果导出

在 `EXCEL_EXPORTER` 中配置导出参数：

```python
EXCEL_EXPORTER = {
    "use_module": True,                      # 使用内置导出模块
    "module": "excel_exporter",              # 模块名称
    "output_path": "record",                 # Excel 输出路径（目录或完整文件名）
    "merge_result_json": "data/rtt/result.json",  # 合并到的 JSON 文件路径
    "power_unit": "J"                        # 功耗单位：J(焦耳) 或 Wh(瓦时)
}
```

## 运行测试

### 基本命令

```bash
# 在项目根目录运行
python utils/stress-auto-test/main.py
```

### 运行流程

1. **初始化电源**
   - 连接 UDP6720 电源模块
   - 设置保护参数（过压、过流）
   - 设置工作电压和电流

2. **启动 DUT**
   - 打开电源
   - 等待网络连通（ping 检测）
   - 建立 SSH/Telnet/Serial 连接

3. **执行测试**
   - 发送启动命令到 DUT
   - 等待测试开始信号（正则表达式匹配）
   - 同时启动功耗数据采集
   - 等待测试完成信号

4. **数据处理**
   - 停止功耗采集
   - 计算总功耗和平均功率
   - 导出 Excel 报告
   - 合并结果到 JSON 文件

5. **清理资源**
   - 发送系统关闭命令
   - 关闭电源
   - 断开所有连接

## 日志说明

工具采用标准 Python logging 格式输出日志：

```
2026-03-27 10:30:45 INFO [Test]启动测试 | 板卡: LS2K1000LA | 系统: SylixOS | 任务: file
2026-03-27 10:30:46 INFO [Test]发送测试启动指令...
2026-03-27 10:30:47 INFO [Test]等待测试开始信号...
2026-03-27 10:31:15 INFO [Test]监听到结束信号，停止录制。
2026-03-27 10:31:20 INFO [Test] 导出完成: record/SylixOS_LS2K1000LA_FILE_20260327_103120.xlsx
```

## 配置详解

### CURRENT_TASK 参数

| 参数 | 类型 | 说明 | 示例 |
|------|------|------|------|
| board | str | 板卡型号 | "LS2K1000LA" |
| os | str | 操作系统名称 | "SylixOS", "RTThread" |
| job | str | 测试任务类型 | "cpu", "memory", "file", "standby" |
| is_debug | bool | 快速验证模式（可选） | False |
| power_port | str | 电源模块串口 | "COM3" (Windows), "/dev/ttyUSB0" (Linux) |
| dut_conn_type | str | 连接方式 | "SSH", "TELNET", "SERIAL" |
| dut_conn_params | dict | 连接参数 | 见下表 |

### dut_conn_params 参数

#### SSH 连接
```python
"dut_conn_params": {
    "ip": "10.4.120.104",
    "port": 22,              # 可选，默认 22
    "username": "root",
    "password": "root"       # 或使用 key 文件
}
```

#### Telnet 连接
```python
"dut_conn_params": {
    "ip": "10.4.120.104",
    "port": 23,              # 可选，默认 23
    "username": "root",
    "password": "root"
}
```

#### Serial 连接
```python
"dut_conn_params": {
    "port": "COM1",          # Windows: COM1/COM2 等
                             # Linux: /dev/ttyS0 等
    "baudrate": 115200,      # 波特率
    "timeout": 5             # 超时时间（秒）
}
```

### STRESS_CONFIG 参数

正则表达式用于识别测试的开始和结束信号：

```python
STRESS_CONFIG = {
    "start_regex": r"Initializing Job: stored_jobfile_(\w+)\.txt\.\.\.",
    "end_regex": r"Job: stored_jobfile_(\w+)\.txt \[Built-in\]"
}
```

这些表达式需要与目标系统的实际输出相匹配。修改时请参考 rtos_stress 的实际输出。

### EXCEL_EXPORTER 参数

| 参数 | 类型 | 说明 |
|------|------|------|
| use_module | bool | 是否使用内置导出模块 |
| module | str | Python 模块名称（与脚本同目录） |
| output_path | str | Excel 输出路径。如果是目录，则自动生成文件名；如果指定完整路径，则使用该文件名 |
| merge_result_json | str | JSON 结果文件路径。若指定，则自动将功耗数据合并到该 JSON 文件中 |
| power_unit | str | 功耗单位，支持 "J"（焦耳）或 "Wh"（瓦时） |

## 输出文件

### Excel 报告

文件名格式：`{OS}_{BOARD}_{JOB}_{TIMESTAMP}.xlsx`

示例：`SylixOS_LS2K1000LA_FILE_20260327_103120.xlsx`

包含以下工作表：
- **Summary** - 汇总信息（板卡、系统、测试任务、总功耗等）
- **Raw Data** - 原始功耗数据（时间、电压、电流、功率）
- **Statistics** - 统计数据（平均值、最大值、最小值等）

### JSON 合并结果

当 `merge_result_json` 指定时，功耗数据会按以下格式合并到对应的测试结果中：

```json
{
  "test-stress": {
    "test_cases": [ ... ],
    "power": [
      {
        "name": "cpu",
        "power_data": 45.123,
        "power_unit": "J"
      },
      {
        "name": "file",
        "power_data": 67.456,
        "power_unit": "J"
      }
    ]
  }
}
```

## 常见问题

### Q1: 无法连接 DUT

**检查清单**：
1. 确认 DUT 网络配置正确（IP、网关）
2. 验证 PC 与 DUT 同一网络段
3. 检查防火墙设置
4. 尝试手动 ping/ssh/telnet 验证连通性

### Q2: 电源无法通信

**检查清单**：
1. 确认 USB 串口驱动已安装
2. 验证 `power_port` 参数正确
3. 检查电源模块的串口参数（通常 9600 波特率）
4. 尝试使用其他 USB 端口

### Q3: 测试无法启动

**检查清单**：
1. 确认 rtos_stress 已安装到 DUT
2. 验证 `start_cmd` 路径和命令正确
3. 检查 `start_regex` 与实际输出相匹配
4. 查看 DUT 端的实际输出信息

### Q4: 功耗数据异常

**检查清单**：
1. 确认电源供电电压和电流设置正确
2. 检查功耗采样间隔（默认 10ms）
3. 验证数据采集线程是否正常运行
4. 检查 Excel 导出的原始数据是否完整

## 扩展和定制

### 添加新的板卡型号

在 `config.py` 的 `BOARD_PROFILES` 中添加：

```python
BOARD_PROFILES = {
    # ...
    "NewBoard": {
        "voltage": 5.0,
        "current": 1.0,
        "protect_voltage": 5.5,
        "protect_current": 1.2
    }
}
```

### 添加新的操作系统

在 `config.py` 的 `OS_PROFILES` 中添加：

```python
OS_PROFILES = {
    # ...
    "NewOS": {
        "start_cmd": ["command1", "command2"],
        "shutdown_cmd": ["shutdown_cmd"]
    }
}
```

### 自定义连接方式

修改 `main.py` 中的连接部分：

```python
if task["dut_conn_type"] == "CUSTOM":
    from custom_connection import CustomConnection
    dut = CustomConnection(**task["dut_conn_params"])
```

## 性能优化

### 数据采样优化

修改 `recorder.py` 中的采样间隔（默认 0.01 秒）：

```python
def _record_loop(self):
    # ...
    time.sleep(0.01)  # 采样间隔
```

较小的间隔获得更精细的数据，但会增加内存占用和 CPU 负载。

### 导出优化

对于大型测试，可以调整 Excel 导出的数据点采样：

```python
# 在 excel_exporter.py 中修改数据采样
# 例如每 N 个数据点输出一个
```

## 故障排查

### 启用调试日志

修改 `main.py` 中的日志级别：

```python
logging.basicConfig(
    level=logging.DEBUG,  # 改为 DEBUG
    format="%(asctime)s %(levelname)s %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S"
)
```

### 手动测试连接

```bash
# 测试 SSH
ssh -u root 10.4.120.104

# 测试 Telnet
telnet 10.4.120.104 23

# 测试电源串口
# Windows: 使用 PuTTY 或 minicom
# Linux: minicom -D /dev/ttyUSB0
```

## 相关文档

- [STRESS.md](STRESS.md) - 压力测试命令说明
- [STRESS_GAP_ANALYSIS.md](STRESS_GAP_ANALYSIS.md) - 压力测试分析
- [SOP.md](SOP.md) - 标准操作流程

## 许可证

MIT License - 见 LICENSES/MIT.txt


