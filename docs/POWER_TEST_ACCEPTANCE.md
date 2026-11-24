# 功耗测试验收手册（TC-TOOL-014 / TC-TOOL-015）

更新时间：2026-11-24 23:58

对应测试大纲：
- TC-TOOL-014 功耗逻辑快速验证测试
- TC-TOOL-015 功耗全量验证测试

工具目录：`utils/stress-auto-test/`

---

## 1. 测试目的

用程控直流电源 UDP-6720 给板卡供电，测量不同压力负载下的功耗。

覆盖：
- CPU 压力
- 内存压力
- 文件 I/O 压力
- 待机

---

## 2. 物理连线（四线开尔文）

1. 按板卡电源接头制作线材。以龙芯派 DC 母头为例，准备四根硅胶线，两根接 +，两根接 -。
2. 板卡 + 接电源背后 `S+` 与 `+`，板卡 - 接 `S-` 与 `-`。
3. 电源背后 RS232 口经 RS232-USB 线接测试主机，确认串口号。
4. 板卡网线接测试主机；串口也接测试主机。

参考：四线开尔文连接说明 https://zhuanlan.zhihu.com/p/1931003870315811543

---

## 3. 配置

进入工具目录：

```bash
cd utils/stress-auto-test
pip install pyserial pandas openpyxl
```

### 3.1 参考配置

| 文件 | 用途 |
|---|---|
| `configs/config-oneos-phytium.py` | OneOS 飞腾 |
| `configs/config-sylix-loong.py` | SylixOS 龙芯 |

使用：

```bash
cp configs/config-oneos-phytium.py config.py
```

### 3.2 关键字段

| 配置 | 值 | 说明 |
|---|---|---|
| `POWER_SUPPLY_CONFIG.port` | `/dev/ttyUSB0` | 电源串口 |
| `POWER_SUPPLY_CONFIG.baudrate` | `9600` | 与电源一致 |
| `_com().port` | `/dev/ttyUSB1` | DUT 串口 |
| `_com().baudrate` | `115200` | 与板卡一致 |
| `_com().boot_wait` | `30.0` | OneOS 固化系统启动慢 |
| `EXCEL_EXPORTER.merge_result_json` | `data/result.json` | 相对 config.py |
| `CURRENT_TASK.jobs` | `["cpu","memory","file"]` | 按顺序执行 |

---

## 4. 快速验证（TC-TOOL-014）

`start_cmd` 使用 quick 任务：

```python
SYLIXOS_START_CMD = {
    "standby": [],
    "cpu": ["cd /apps/stress-ng/", "./rtos_bench test-stress --job cpu-quick"],
    "memory": ["cd /apps/stress-ng/", "./rtos_bench test-stress --job memory-quick"],
    "file": ["cd /apps/stress-ng/", "./rtos_bench test-stress --job file-quick"],
    "default": ["cd /apps/stress-ng/", "./rtos_bench test-stress --job all-quick"]
}
```

运行：

```bash
python main.py
```

预期：

| Job | 完成计数 | 结果表 |
|---|---|---|
| cpu-quick | 13/13 Finished | Stage=1 |
| memory-quick | 6/6 Finished | Stage=1 |
| file-quick | 8/8 Finished | Stage=1 |

指标检查：

- memory：`stream bandwidth(MB/Sec)`、`compute(Mflop/Sec)` 有输出。
- file：`pipe throughput(KB/Sec)` 有输出；不支持 pipe 的 OS 允许 ops=0。

---

## 5. 全量验证（TC-TOOL-015）

`start_cmd` 使用全量任务：

```python
SYLIXOS_START_CMD = {
    "standby": [],
    "cpu": ["cd /apps/stress-ng/", "./rtos_bench test-stress --job cpu"],
    "memory": ["cd /apps/stress-ng/", "./rtos_bench test-stress --job memory"],
    "file": ["cd /apps/stress-ng/", "./rtos_bench test-stress --job file"],
    "default": ["cd /apps/stress-ng/", "./rtos_bench test-stress --job all"]
}
```

预期：

| Job | 完成计数 | 结果表 |
|---|---|---|
| cpu | 65/65 Finished | Stage=1-5 |
| memory | 30/30 Finished | Stage=1-5 |
| file | 40/40 Finished | Stage=1-5 |

---

## 6. 输出物

每个 Job 生成一个 Excel：

```text
{os}_{board}_{JOB}_{YYYYmmdd_HHMMSS}.xlsx
```

| Sheet | 内容 |
|---|---|
| 测试汇总 | 板卡、系统、任务、电压、限流、耗时、平均功率、总功耗(J)、总功耗(Wh) |
| 详细采样数据 | Time(s)、Voltage(V)、Current(A)、Power(W) |

同时写回 JSON：

```json
"modules": {
  "test-stress": {
    "power": [
      { "name": "cpu", "power_data": 2001.626, "power_unit": "J" }
    ]
  }
}
```

展平后得到 `power_cpu` / `power_memory` / `power_file` / `power_standby`。

---

## 7. 单片验证顺序

```bash
python test.py profile
python test.py power
python test.py dut --connection COM --read-seconds 5
python main.py
```

`test.py power` 会连接电源、设保护、开输出、采样 3 次。

---

## 8. 注意事项

1. 每个 Job 都会重新上电、等启动、连 DUT，耗时包含冷启动。
2. 龙芯需要人工按电源键；代码只打印提示。
3. 龙芯 PERE 命令包含 `mmc read`、`go`、`mount dosfs /c`、`ld`，必须按板卡实际路径确认。
4. SylixOS 全量命令中的目录与二进制名要与板上一致；本仓库实机已有 `/apps/rtos-bench`，若用 `/apps/stress-ng/rtos_bench` 需确认该目录存在。
5. 电源与 DUT 串口不要混淆。
6. 采样间隔 10 ms；串口查询较慢时会自动拉长实际间隔。
7. standby 默认 600 秒；演示可覆盖 `duration`。

---

## 9. 快速/全量耗时参考

| 板卡 | 全量 stress 时长 | 主要耗时项 |
|---|---:|---|
| OneOS 飞腾 | 约 40 分钟 | ptr-chase 五阶段 |
| ReWorks 龙芯 | 约 41 分钟 | memcpy 五阶段 |
| Intewell x86 | 约 10 分钟 | stream 五阶段 |

快速模式只跑 Stage=1，耗时显著降低。
