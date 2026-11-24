# 功耗测试完整操作手册（离线版）

更新时间：2026-11-25

适用测试大纲：
- 6.2.7 功耗指标评测
- 6.2.7.1 逻辑快速验证测试（TC-TOOL-014）
- 6.2.7.2 全量验证测试（TC-TOOL-015）
- 6.2.7.3 CPU 压力功耗
- 6.2.7.4 Memory 压力功耗
- 6.2.7.5 File 压力功耗
- 6.2.7.6 Standby 压力功耗

适用环境：
- 测试主机：Windows
- 功耗仪：UDP-6720 程控直流电源
- 目标板：OneOS 飞腾派、SylixOS 龙芯派（至少两块）

本手册可在没有 AI、没有网络的测试主机上直接照做。

---

## 1. 准备清单

### 1.1 硬件

| 物品 | 数量 | 用途 |
|---|---:|---|
| UDP-6720 程控电源 | 1 | 供电与功耗测量 |
| RS232 转 USB 线 | 1 | 电源接测试主机 |
| USB 转 TTL 串口线 | 1 | 板卡控制台接测试主机 |
| 四根硅胶线 | 4 | 四线开尔文接法 |
| 网线（可选） | 1 | 板卡走 Telnet |
| 目标板 | 2 | OneOS 飞腾、SylixOS 龙芯 |

### 1.2 软件

| 软件 | 说明 |
|---|---|
| Python 3.8 以上 | 测试主机安装 |
| pyserial、pandas、openpyxl | 通过 pip 安装 |
| 本仓库 `utils/stress-auto-test` | 测试工具 |

安装命令：

```powershell
cd <仓库目录>\utils\stress-auto-test
pip install pyserial pandas openpyxl
```

如果出现 `WinError 31`：

```powershell
python patch_pyserial_win32.py
```

并在 Windows 电源选项里关闭「USB 选择性暂停」。

### 1.3 文件

| 文件 | 用途 |
|---|---|
| `config-oneos-phytium.py` | OneOS 飞腾参考配置 |
| `config-sylix-loong.py` | SylixOS 龙芯参考配置 |
| `data/result.json` | 待合并功耗的结果 JSON |
| 样板 Excel | 展示格式参考 |

---

## 2. 接线

### 2.1 四线开尔文接板卡

1. 板卡电源正极接电源 `S+` 与 `+`。
2. 板卡电源负极接电源 `S-` 与 `-`。
3. 正极用两根线，负极用两根线。
4. 螺丝拧紧，避免接触电阻。

注意：
- 先确认板卡额定电压，再设置电源。
- 正负极不得接反。
- 上电前设置过压、过流保护。

### 2.2 电源接测试主机

1. 电源背后 RS232 口接 RS232 转 USB 线。
2. USB 插入测试主机。
3. 设备管理器查看新增 COM 号，例如 `COM3`。

### 2.3 板卡接测试主机

1. 板卡调试串口接 USB 转 TTL 线。
2. USB 插入测试主机。
3. 设备管理器查看新增 COM 号，例如 `COM6`。
4. 若走 Telnet，可不接板卡串口，改用网线。

### 2.4 记录端口

| 设备 | 示例 | 实际填写 |
|---|---|---|
| 功耗仪 | COM3 | |
| 板卡 | COM6 | |

---

## 3. 配置

复制对应参考配置为 `config.py`：

```powershell
copy configs\config-oneos-phytium.py config.py
```

### 3.1 功耗仪串口

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

`port` 改成功耗仪实际 COM。`baudrate` 必须是 9600。

### 3.2 板卡串口

```python
def _com(port="COM3", baudrate=115200):
    return {
        "port": port,
        "baudrate": baudrate,
        "timeout": 0.1,
        "boot_wait": 30.0
    }

def _platform(ip, start_cmd, shutdown_cmd,
              dut_conn_type="COM", com_port="COM6"):
    ...
```

`com_port` 改成板卡实际 COM。OneOS 的 `boot_wait` 保持 30 秒，SylixOS 可改 5。

### 3.3 当前任务

OneOS 飞腾：

```python
CURRENT_TASK = {
    "board": "Phytium-Pi",
    "os": "oneos",
    "jobs": ["cpu", "memory", "file"],
    "stop_on_job_failure": True
}
```

SylixOS 龙芯：

```python
CURRENT_TASK = {
    "board": "LS2K1000LA",
    "os": "SylixOS",
    "jobs": ["cpu", "memory", "file"],
    "stop_on_job_failure": True
}
```

### 3.4 供电参数

| 板卡 | 电压 | 限流 | 过压 | 过流 |
|---|---:|---:|---:|---:|
| Phytium-Pi | 12 V | 3 A | 13.2 V | 3.3 A |
| LS2K1000LA | 12 V | 2 A | 13.2 V | 2.2 A |

在 `PLATFORM_PROFILES[board]["power"]` 中确认。

### 3.5 启动命令

OneOS：

```python
"cpu": ["ld /user/phytium_pi_out.out", "cd /user/", "rtbench test-stress --job cpu"],
```

SylixOS 参考：

```python
"cpu": ["cd /apps", "./rtos_bench test-stress --job cpu"],
```

上板确认实际路径。若是 `/apps/rtos-bench`，改为：

```python
"cpu": ["/apps/rtos-bench test-stress --job cpu"],
```

快速测试把 `--job cpu` 换成 `--job cpu-quick`，memory、file 同理。

### 3.6 导出

```python
EXCEL_EXPORTER = {
    "use_module": True,
    "module": "excel_exporter",
    "output_path": "record",
    "merge_result_json": "data/result.json",
    "power_unit": "J"
}
```

运行前确认 `data\result.json` 存在。

---

## 4. 板卡侧准备

### 4.1 OneOS 飞腾

1. 模块已在 `/user/phytium_pi_out.out`。
2. 未加载时执行 `ld /user/phytium_pi_out.out`。
3. 用 `list_lmodule` 确认 `stat[1]`。

### 4.2 SylixOS 龙芯

1. 确认测试程序路径，例如 `/apps/rtos-bench` 或 `/apps/stress-ng/rtos_bench`。
2. 龙芯启动可能需要**人工按电源键**，代码只打印提示。
3. 若使用 PERE 流程，确认 `mmc read`、`go`、`mount dosfs /c`、`ld` 命令与板卡一致。

---

## 5. 上电前验证

按顺序执行，三项都通过再跑主流程。

```powershell
python test.py profile
```

检查解析出的板卡、系统、连接参数、供电参数。

```powershell
python test.py power
```

预期：连接电源、设置保护、开输出、采样 3 次。

```powershell
python test.py dut --connection COM --read-seconds 5
```

预期：看到板卡启动或 Shell 输出。

---

## 6. 快速验证（6.2.7.1）

配置 `start_cmd` 使用 quick：

```python
"cpu": ["cd /apps", "./rtos_bench test-stress --job cpu-quick"],
"memory": ["cd /apps", "./rtos_bench test-stress --job memory-quick"],
"file": ["cd /apps", "./rtos_bench test-stress --job file-quick"],
```

运行：

```powershell
python main.py
```

预期：

| Job | 完成计数 | 结果表 |
|---|---|---|
| cpu-quick | 13/13 Finished | Stage=1 |
| memory-quick | 6/6 Finished | Stage=1 |
| file-quick | 8/8 Finished | Stage=1 |

指标：

- memory 有 `stream bandwidth(MB/Sec)` 与 `compute(Mflop/Sec)`。
- file 有 `pipe throughput(KB/Sec)`，不支持 pipe 时 ops=0。

---

## 7. 全量验证（6.2.7.2）

配置 `start_cmd` 去掉 `-quick`。

预期：

| Job | 完成计数 | 结果表 |
|---|---|---|
| cpu | 65/65 Finished | Stage=1-5 |
| memory | 30/30 Finished | Stage=1-5 |
| file | 40/40 Finished | Stage=1-5 |

参考耗时：

| 板卡 | 全量 stress 时长 |
|---|---:|
| OneOS 飞腾 | 约 40 分钟 |
| ReWorks 龙芯 | 约 41 分钟 |
| Intewell x86 | 约 10 分钟 |

每个 Job 都会重新上电，实际总时长还要加上启动时间。

---

## 8. 输出物

每个 Job 一个 Excel：

```text
{os}_{board}_{JOB}_{YYYYmmdd_HHMMSS}.xlsx
```

| Sheet | 内容 |
|---|---|
| 测试汇总 | 板卡、系统、任务、电压、限流、耗时、平均功率、总功耗(J)、总功耗(Wh) |
| 详细采样数据 | Time(s)、Voltage(V)、Current(A)、Power(W) |

JSON 写回：

```json
"modules": {
  "test-stress": {
    "power": [
      { "name": "cpu", "power_data": 2001.626, "power_unit": "J" }
    ]
  }
}
```

展平后得到：

- `power_cpu`
- `power_memory`
- `power_file`
- `power_standby`

---

## 9. 现场演示步骤

1. 展示接线：四线开尔文、电源 RS232、板卡串口。
2. 运行 `python test.py power`，展示实时电压、电流、功率。
3. 运行快速测试 `python main.py`。
4. 控制台展示 13/13、6/6、8/8 与 Stage=1。
5. 打开 Excel 两个 Sheet。
6. 打开 JSON，指出 4 条 power。
7. 打开展平结果，指出 4 条 power_*。
8. 如需全量，再跑 6.2.7.2，展示 65/65、30/30、40/40 与 Stage=1-5。

---

## 10. 故障排查

| 现象 | 原因 | 处理 |
|---|---|---|
| 打不开电源串口 | COM 号错或被占用 | 设备管理器确认，关闭占用程序 |
| `WinError 31` | pySerial 与 USB 串口兼容 | 运行 patch，关闭 USB 选择性暂停 |
| DUT 无输出 | COM 号错、波特率错、TX/RX 接反 | 交换 TX/RX，确认 115200 |
| 一直等 ping | IP 错或板卡没启动 | 改 COM 模式或修网络 |
| 一直等 start_regex | 命令没执行或输出不匹配 | 用 `test.py dut --command` 看实际输出 |
| Excel 成功但 JSON 失败 | `data/result.json` 不存在 | 放入结果 JSON 或关闭合并 |
| 龙芯不启动 | 需要人工按键 | 按电源键 |
| 电源输出保护 | 电压电流设置不当 | 检查板卡额定值 |

---

## 11. 收尾

1. 等待 Excel 与 JSON 写完。
2. 记录每个 Job 的文件名。
3. 关闭电源输出。
4. 断开串口与电源。
5. 保存测试主机上的 `record` 目录与 JSON。

---

## 12. 演示前检查表

- [ ] 电源与板卡接线正确
- [ ] 电源串口与板卡串口已确认
- [ ] `config.py` 的板卡、系统、COM、供电参数正确
- [ ] `data/result.json` 存在
- [ ] `python test.py profile` 通过
- [ ] `python test.py power` 通过
- [ ] `python test.py dut` 通过
- [ ] 快速测试拿到 13/13、6/6、8/8
- [ ] Excel 两个 Sheet 正常
- [ ] JSON 有 4 条 power
