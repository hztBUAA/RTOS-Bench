# 补充或更新一块板的数据

本文说明测到原始数据后，如何算出分数并更新表格。

## 1. 需要准备什么

一份 `result.json`，必须包含以下字段。可直接复制同目录的 `template_result.json` 修改。

### env

| 字段 | 取值 |
|---|---|
| `os_name` | `SylixOS`、`Intewell`、`ReWorks`、`OneOS`、`RT-Thread`、`openEuler` |
| `board` | `MIC-7700`、`Orange-Pi5`、`DH-1`、`LS2K1000LA`、`Phytium-Pi` |
| `cpu_type` | `aarch64`、`loongarch64`、`riscv64`、`x86_64` |

### modules.test-realtime

```json
{
  "single_core": {
    "context_switch": { "avg_us": 0.967 },
    "interrupt": { "min_us": 0.120, "max_us": 0.260, "avg_us": 0.163 },
    "syscall": { "min_us": 0.020, "max_us": 0.040, "avg_us": 0.021 },
    "service_cost": [
      { "operation": "sem_take", "immediate_us": 0.300, "suspend_us": 1.800, "low_prio_us": 0.0, "high_prio_us": 0.0 }
    ]
  },
  "multi_core": {
    "memory_bandwidth": [
      { "type": "rd", "c1": 0.270, "c2": 0.536, "c4": 0.553, "c8": 0.562 }
    ],
    "ipc_bandwidth": { "c1": 0.246, "c2": 0.347, "c4": 0.412, "c8": 0.002 },
    "task_latency": { "c1": 15.953, "c2": 541.906, "c4": 266.750, "c8": 137.549 },
    "core_comm": { "intra_core": 0.246, "inter_core": 0.0 }
  }
}
```

- `service_cost` 支持 8 个操作：`sem_take`、`sem_release`、`mq_send`、`mq_recv`、`mutex_take`、`mutex_release`、`mempool_alloc`、`mempool_free`。
- `memory_bandwidth` 支持 `rd`、`wr`、`cp`、`frd`、`fwr`、`fcp`、`memset`、`memcpy`。
- 哪吒为单核，`multi_core` 可留空，权重会自动重归一化。
- `syscall` 只作诊断，不进入 Base_RT。

### modules.test-schedule

```json
{
  "gradients": [
    { "utilization_percent": 30, "total_jobs": 27, "deadline_misses": 0, "miss_rate": 0.0 }
  ]
}
```

### modules.typical-workload

```json
{
  "workloads": [
    { "name": "pid", "avg_time_ms": 23.779 }
  ]
}
```

### modules.test-stress.power

功耗是总分的来源，缺它就只能算 partial 总分。

```json
{
  "power": [
    { "name": "standby", "power_data": 15176.102 },
    { "name": "cpu",     "power_data": 7059.196 },
    { "name": "memory",  "power_data": 8639.869 },
    { "name": "file",    "power_data": 3925.092 }
  ]
}
```

`name` 只能是 `standby`、`cpu`、`memory`、`file`，`power_data` 单位是 J。

## 2. 放到 raw 目录

```
docs/examples/final/raw/<板卡>/result.json
```

一块板一个子目录，文件名固定 `result.json`。

## 3. 跑管线

```bash
python3 tools/pipeline.py --version 9load \
  --ssot   docs/examples/final/ssot/9load \
  --scored docs/examples/final/scored/9load \
  --out    docs/examples/final/outputs/9load
```

7 负载把 `9load` 换成 `7load`。

## 4. 产物

| 位置 | 内容 |
|---|---|
| `scored/<version>/*.scores.json` | 分数 |
| `outputs/<version>/scores.csv` | 汇总表 |
| `outputs/<version>/reports/*.md/*.docx` | 自然语言报告 |
| `outputs/<version>/display_<version>.xlsx` | 1103 样式 Excel |

## 5. 注意

- 缺功耗时，总分按 partial 权重算，需在备注里写明。
- openEuler 两块板缺功耗，总分按 partial 权重计算。
- 改了 Excel 的 sheet 结构（插行、改列），解析会错位；先按 `SOURCE_DATA.md` 核对块布局。
- 所有改动通过 PR 提交，便于追溯。
