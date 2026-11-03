# 补充或更新一块板的数据

本文说明对方测到原始数据后，如何自己算出分数并更新表格，不需要经过振庭。

## 1. 需要准备什么

一份 `result.json`，必须包含以下字段。

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

## 2. 算分

必须先有基准记录：SylixOS @ LS2K1000LA。

```bash
python3 utils/score_caculate.py --db results.db import-json "<json目录>/*.json"
python3 utils/score_caculate.py --db results.db recompute
python3 utils/score_caculate.py --db results.db export \
    --board Phytium-Pi --os openEuler --arch aarch64 -o out.json
```

`out.json` 里的 `scores` 就是基础实时、RT、T、P、总分。

## 3. 填表

把 `out.json` 的 `scores` 填到对应行：

| 列 | 字段 |
|---|---|
| 基础实时 | `scores.realtime.details.base_realtime.score` |
| 调度 | `scores.realtime.details.schedulability.score` |
| RT 得分 | `scores.realtime.score` |
| 四类负载 | `scores.workload.details.<category>.score` |
| T 得分 | `scores.workload.score` |
| P 得分 | `scores.power.score` |
| F 得分 | `scores.function.score`（当前固定 60） |
| 总分 | `total_score` |

或者直接更新 `docs/examples/final/` 下的 CSV 与 SSOT，再提交 PR。

## 4. 注意

- 缺功耗时，总分按 partial 权重算，会偏高，需在备注里写明。
- 欧拉两块板目前缺原始数据，补 `result.json` 后按上面流程算，再把行填入表。
- 改了 Excel 的 sheet 结构（插行、改列），解析会错位；先按 `SOURCE_DATA.md` 核对块布局。
- 所有改动通过 PR 提交，便于追溯。

## 5. 只想更新 Excel

如果对方只维护 Excel，不想跑脚本：

1. 按上面的 JSON 字段准备数据。
2. 把 JSON 交给能跑脚本的人，或直接在本机跑一次。
3. 用输出的 `scores` 填 Excel。
4. 不要只改 Excel 的最终表而不改原始数据，否则下次重算会变回去。
