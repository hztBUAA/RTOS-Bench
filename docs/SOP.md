# RTOS-Bench 板级测试 SOP

端到端测试流程：从编译固件到获得评分报告。

## 流程总览

```
  宿主机 (Host)                        被测板 (Target / QEMU)
 ┌──────────────────────┐             ┌───────────────────────────┐
 │ deploy.py -c board.yaml            │                           │
 │                      │   SFTP      │                           │
 │  1. 上传固件          │ ──────────→ │  固件部署到 /tmp/          │
 │  2. 建立连接          │  SSH/       │                           │
 │     (SSH/Telnet/串口)  │  Telnet/   │                           │
 │  3. 发送 test-all 命令│  Serial    │  3'. rtbench test-all     │
 │  4. 等待完成信号      │ ←─ 终端流 ──│      执行 5 个测试模块     │
 │  5. 断开连接          │             │      生成 JSON 结果文件    │
 │  6. 取回 JSON 结果    │ ←── SFTP ──│  /tmp/rtbench_result.json │
 │  7. 保存终端原始日志   │             │                           │
 │  8. 展平 → 标准格式    │             └───────────────────────────┘
 │  9. 评分 → score报告   │
 └──────────────────────┘
```

## 前置准备

### 依赖安装

```bash
pip install paramiko pyyaml pandas openpyxl
# 串口连接还需:
pip install pyserial
```

### 编译固件

```bash
# RT-Thread (QEMU aarch64)
./run-rtthread.sh -b

# SylixOS / 其他平台
# 使用对应 IDE 或 Makefile 交叉编译，产物为 rtbench 可执行文件或固件 bin
```

## 配置文件

复制 `utils/remote-test/board_config_example.yaml` 为 `board.yaml`，按实际环境填写。

### 配置结构

```yaml
# board — 板卡基本信息（用于输出目录命名和评分标识）
board:
  name: "LS2K1000LA"          # 板卡型号
  os: "SylixOS"               # 被测 RTOS
  arch: "mips64"              # 架构

# connection — 如何连接到被测板
connection:
  type: "ssh"                 # ssh | telnet | serial
  ip: "192.168.1.100"
  port: 22                    # FRP 场景可用非标端口
  username: "root"
  password: "root"

# deploy — 固件上传（可选，--skip-deploy 跳过）
deploy:
  local_binary: "./rtbench"   # 宿主机上的编译产物
  remote_dir: "/tmp/"         # 板上目标目录

# test — 测试执行
test:
  command: "cd /tmp && ./rtbench test-all -o /tmp/rtbench_result.json"
  end_regex: "\\[RTOS-Bench\\] Results saved to:"   # 匹配 C 代码实际输出
  timeout_sec: 600            # 超时秒数

# result — 结果收集与后处理
result:
  remote_path: "/tmp/rtbench_result.json"
  local_output_dir: "./results/"
  retrieval_method: "sftp"    # sftp | terminal_capture
  run_flatten: true           # 展平为标准格式
  run_score: false            # 评分（需 ≥2 个 OS 数据才有效）
```

### 三种连接方式

| 类型 | 必填字段 | 典型场景 |
|------|----------|----------|
| `ssh` | `ip`, `port`, `username`, `password` | 有 SSH 的开发板、FRP 远程 |
| `telnet` | `ip`, `port`, `username`, `password` | SylixOS 等 Telnet Shell |
| `serial` | `device`, `baudrate` | 串口直连 (`/dev/ttyUSB0`) |

Serial 示例：
```yaml
connection:
  type: "serial"
  device: "/dev/ttyUSB0"
  baudrate: 115200
```

### 两种结果取回方式

| 方式 | 适用场景 | 原理 |
|------|----------|------|
| `sftp` | SSH 连接的板子 | 通过 SFTP 下载 JSON 文件 |
| `terminal_capture` | 串口/Telnet 无文件传输能力 | 从终端输出缓冲区中提取 JSON |

> 注意: `terminal_capture` 当前对深层嵌套 JSON 有兼容性问题，优先使用 `sftp`。

## 执行测试

### 一键执行

```bash
python utils/remote-test/deploy.py -c board.yaml
```

### 常用选项

```bash
# 跳过固件上传（板上已有可执行文件）
python utils/remote-test/deploy.py -c board.yaml --skip-deploy

# 指定输出目录
python utils/remote-test/deploy.py -c board.yaml -o ./my_results/

# 跳过结果下载（仅执行测试和日志采集）
python utils/remote-test/deploy.py -c board.yaml --skip-retrieve
```

### 手动执行（不用 deploy.py）

在板上 Shell 中直接运行：

```bash
# RT-Thread msh
msh /> rtbench test-all -o /rtbench_result.json

# SylixOS / Linux
./rtbench test-all -o /tmp/rtbench_result.json
```

然后手动取回结果：
```bash
scp root@<板子IP>:/tmp/rtbench_result.json ./results/
```

## 输出目录结构

`deploy.py` 自动在 `local_output_dir` 下创建带时间戳的子目录：

```
results/
└── LS2K1000LA_SylixOS_20260324_153045/
    ├── rtbench_raw_20260324_153045_CST.log   # 终端原始日志（全量）
    ├── rtbench_result.json                    # 设备端结构化输出
    ├── standard_results.json                  # 展平后标准格式（run_flatten: true）
    └── score_report.json                      # 评分报告（run_score: true）
```

### 各文件说明

#### rtbench_raw_*.log — 终端原始日志

DUT 终端的全量输出，包括启动信息、测试进度、错误日志等。格式：

```
>>> rtbench test-all -o /tmp/rtbench_result.json
[RTOS-Bench] Running all tests...
[test-realtime] Context switch latency AVG: 1.863 us
...
[RTOS-Bench] Results saved to: /tmp/rtbench_result.json
```

#### rtbench_result.json — 结构化测试结果

设备端 `result_export.c` 生成，包含 5 个测试模块的完整数据：

```json
{
  "meta": {
    "framework_version": "1.0.0",
    "test_timestamp": "2026-03-24T15:30:00Z",
    "total_duration_sec": 356.7
  },
  "env": {
    "os_name": "RT-Thread",
    "os_version": "5.3.0",
    "board": "QEMU-virt-aarch64",
    "cpu_type": "cortex-a53",
    "cpu_freq_mhz": 1000,
    "cpu_core_num": 4
  },
  "modules": {
    "test-realtime": { "status": "passed", "single_core": {...}, "multi_core": {...} },
    "test-schedule": { "status": "passed", "gradients": [...], "summary": {...} },
    "test-stress":   { "status": "passed", "stressors": [...] },
    "test-cmd":      { "status": "passed", "commands": [...] },
    "typical-workload": { "status": "passed", "workloads": [...] }
  }
}
```

5 个模块覆盖的指标：

| 模块 | 关键字段 |
|------|----------|
| test-realtime | context_switch avg, interrupt latency, syscall latency, service_cost (8 ops x 4 scenarios), multicore memory_bandwidth / ipc / task_latency / core_comm |
| test-schedule | gradients[] (miss_rate per utilization level), summary.average_miss_rate, summary.final_score |
| test-stress | stressors[] (name, type, stage, bogo_ops, duration_sec, metric_value) — 27 种 stressor x 5 级并发 |
| test-cmd | commands[] (name, supported) — 11 条 Shell 命令可用性 |
| typical-workload | workloads[] (name, category, exec_time_ms, avg_time_ms, rounds) |

完整 Schema 参见 `docs/reference/rtbench_result_schema.json`，完整示例参见 `docs/reference/rtbench_result_example.json`。

#### standard_results.json — 展平标准格式

`flatten_rtbench_result.py` 将嵌套 JSON 展平为逐条记录的数组，面向外部数据平台（阿里云 schema）：

```json
[
  {
    "flow_job_history_id": "uuid",
    "sw_info": { "sdk_type": "RT-Thread", "sdk_version": "5.3.0" },
    "hw_info": { "platform_type": "QEMU-virt-aarch64", "cpu_type": "cortex-a53" },
    "test_case_info": {
      "test_suite": "rtbench",
      "test_dir": "realtime",
      "test_case": "single_core.context_switch.avg_us"
    },
    "test_result_info": {
      "test_result": "1.489",
      "test_result_static_info": {
        "test_unit": "us",
        "test_optimal_type": "min"
      }
    }
  },
  ...
]
```

展平脚本也可独立使用：
```bash
python utils/flatten_rtbench_result.py results/rtbench_result.json -o results/flat.json -p
```

#### score_report.json — 评分报告

`score_caculate.py` 基于《工业操作系统通用基准检测指标体系指导书 v1.6》计算综合得分：

```json
{
  "board_model": "LS2K1000LA",
  "os_name": "SylixOS",
  "arch": "mips64",
  "total_score": 72.5,
  "is_partial": true,
  "scores": {
    "function":  { "score": 60.0,  "source": "placeholder" },
    "realtime":  { "score": 78.3,  "details": { "base_realtime": {...}, "schedulability": {...} } },
    "workload":  { "score": 85.2,  "details": { "control": {...}, "estimation": {...} } },
    "power":     { "score": null }
  }
}
```

评分公式: `Total = 0.2×F + 0.3×RT + 0.4×T + 0.1×P`

| 维度 | 权重 | 数据来源 | 说明 |
|------|------|----------|------|
| F_Score (功能) | 20% | test-cmd | 当前占位 60 分 |
| RT_Score (实时) | 30% | test-realtime + test-schedule | 50% base_realtime + 50% schedulability |
| T_Score (负载) | 40% | typical-workload | 4 个负载分类等权平均 |
| P_Score (功耗) | 10% | 外部功耗仪 | 当前无数据，需外部导入 |

> **重要**: 评分使用跨 OS min-max 归一化。数据库中只有 1 个 OS 时所有 item_score 为 null。需至少 2 个不同 OS 在同一板子上的结果才能产生有效分数。

评分脚本也可独立使用：
```bash
# 导入 JSON 并评分
python utils/score_caculate.py import-json "results/*.json"

# 查看数据库中已有结果
python utils/score_caculate.py list

# 重新计算分数
python utils/score_caculate.py recalc "LS2K1000LA" "SylixOS" "mips64"
```

## deploy.py 12 步流水线

| 步骤 | 操作 | 可跳过 |
|------|------|--------|
| 1 | 加载 YAML 配置 | - |
| 2 | 创建带时间戳的输出目录 | - |
| 3 | 生成日志文件名 | - |
| 4 | SFTP 上传固件到 DUT | `--skip-deploy` |
| 5 | 建立 DUT 连接 (SSH/Telnet/Serial) | - |
| 6 | 发送测试命令 | - |
| 7 | 等待 `end_regex` 匹配（超时可配） | - |
| 8 | 断开连接，终端日志自动落盘 | - |
| 9 | 取回 JSON 结果 (SFTP / terminal_capture) | `--skip-retrieve` |
| 10 | 展平为标准格式 | `run_flatten: false` |
| 11 | 计算评分 | `run_score: false` |
| 12 | 打印摘要 | - |

## 数据流管线

展平和评分是**两条并行管线**，互不依赖：

```
                          rtbench_result.json
                                 │
                    ┌────────────┼────────────┐
                    ↓            │            ↓
          flatten_rtbench_result.py       score_caculate.py
                    ↓            │            ↓
          standard_results.json  │    score_report.json
                    ↓            │         (需 ≥2 OS)
            外部数据平台          │
           (阿里云 schema)       │
                                 │
                          (原始 JSON 可直接归档)
```

- **展平** 面向外部平台数据录入，将嵌套 JSON 拆成逐条记录
- **评分** 直接读原始 JSON 中的指标值，写入 SQLite 后做归一化和加权计算
- 两者共享同一份 `rtbench_result.json` 输入，输出各自独立

## 文件索引

| 文件 | 用途 |
|------|------|
| `utils/remote-test/deploy.py` | 编排主脚本 |
| `utils/remote-test/config_schema.py` | YAML 配置加载与默认值填充 |
| `utils/remote-test/dut_connection.py` | SSH/Serial/Telnet 连接（含日志 tee） |
| `utils/remote-test/file_transfer.py` | SFTP 上传/下载 + 终端 JSON 提取 |
| `utils/remote-test/log_utils.py` | 北京时间戳生成 |
| `utils/remote-test/board_config_example.yaml` | 配置示例 |
| `utils/flatten_rtbench_result.py` | JSON → 标准格式展平 |
| `utils/score_caculate.py` | 评分引擎 |
| `generator/result_export.c/.h` | 设备端 JSON 序列化 |
| `generator/rtthread_entry.c` | RT-Thread test-all 入口 |
| `docs/reference/rtbench_result_schema.json` | JSON Schema |
| `docs/reference/rtbench_result_example.json` | 完整输出示例 |
| `docs/SCORE-CALCULATE.md` | 评分算法详细说明 |
