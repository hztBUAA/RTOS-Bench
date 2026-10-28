# RTBench 评测评分系统文档

## 1. 总体评分层级

```
子项原始值(raw_value)
    ↓ calc_R()
子项基准比 R_value
    ↓ normalize_items_for_board()  【跨系统 min-max】
子项得分 item_score
    ↓ 算术平均
大指标得分（如 scheduling_latency_score）
    ↓ 加权
维度得分（F/RT/T/P Score）
    ↓ 加权
单板总分 Total_Score
```

---

## 2. 总分公式

$$
\text{Total} = 0.2 \times F + 0.3 \times RT + 0.4 \times T + 0.1 \times P
$$

| 维度 | 权重 | 当前状态 |
|------|------|----------|
| 基础功能 F | 20% | 占位填充，不计算 |
| 实时性能 RT | 30% | 从 JSON 计算 |
| 典型负载 T | 40% | 从 JSON 计算 |
| 功耗 P | 10% | 从 Excel 计算 |

---

## 3. 通用得分公式

### 3.1 基准比 R

```
延迟类指标：  R = baseline_latency / measured_latency
吞吐/带宽类：R = measured_bandwidth / baseline_bandwidth
功耗类：     R = baseline_power / measured_power
```

> 当前脚本使用固定的 **SylixOS + LS2K1000LA** 测量值作为绝对基准。

### 3.2 min-max 归一化（T_Score / RT_Score 子项）

$$
\text{Score} = $R - R_{\min}$ / ($R_{\max} - R_{\min}$) \times 100
$$

- $R_{\min}$、$R_{\max}$ 取**同一板卡、同一子项**下所有已测系统的 R 值极值
- 结果裁剪至 $[0, 100]$
- 若 $R_{\min} = R_{\max}$（所有系统表现相同），则 `item_score = null`，`norm_pending = true`
- **每新增一个系统，同板卡所有系统的子项得分都会被重新归一化**
- 多核测试值缺失时不生成该子项记录，也不参与 min-max；其他有效多核子项仍按其实际 R 值正常计算
- 若完全没有有效的多核子项，`multicore_overhead = null`，从 `Base_RT` 中剔除，  并对 `0.10/0.10/0.40` 三个剩余权重重新归一化
- **单核板卡**：目前仅 RISC-V 哪吒板卡忽略输入中的 `multi_core` 数据，同样按剩余权重计算 `Base_RT`

### 3.3 调度性能（不走 min-max）

$$
\text{Sched\_Score} = 100 \times (1 - \overline{MR})
$$

其中 $\overline{MR}$ 为多个利用率梯度（30%～100%，步长10%）的平均截止时间错失率。

---

## 4. 实时性能 RT_Score

$$
RT = 0.5 \times \text{Base\_RT} + 0.5 \times \text{Sched}
$$

### 4.1 基础实时性能 Base_RT

$$
\text{Base\_RT} = 0.10 \times S_{\text{ctx}} + 0.10 \times S_{\text{intr}} + 0.40 \times S_{\text{sched}} + 0.40 \times S_{\text{mc}}
$$

| 大指标 | 权重 | 说明 | 子项来源（JSON） |
|--------|------|------|--------------|
| `context_switch_latency` | 10% | 上下文切换平均延迟 | `single_core.context_switch.avg_us` |
| `interrupt_latency` | 10% | 中断平均延迟 | `single_core.interrupt.avg_us` |
| `scheduling_latency` | 40% | 服务调用各场景延迟 | `single_core.service_cost[*]` |
| `multicore_overhead` | 40% | 多核并发开销 | `single_core` 同级的 `multi_core` 对象 |

每个大指标得分 = 其**所有子项 item_score 的算术平均**。

**`scheduling_latency` 子项展开：**

每个 `service_cost` 条目（`sem_take`, `sem_release`, `mq_send`, `mq_recv`, `mutex_take`, `mutex_release`, `mempool_alloc`, `mempool_free`）的以下非零字段各为一个独立子项：

| 子项 key 格式 | 含义 |
|---|---|
| `{op}_immediate_us` | 立即执行场景延迟 |
| `{op}_suspend_us` | 挂起等待场景延迟 |
| `{op}_low_prio_us` | 低优先级抢占场景延迟 |
| `{op}_high_prio_us` | 高优先级抢占场景延迟 |

**syscall 指标（不参与 Base_RT）：**

`single_core.syscall.{min_us,max_us,avg_us}` 仍会解析、保存和导出，用于数据质量
诊断，但不参与 `scheduling_latency` 的算术平均、Base_RT 或 RT_Score 计算。
记录使用 `category=realtime`、`sub_category=syscall_latency`。

**`multicore_overhead` 子项展开：**

多核对象包含以下 42 个子项，均归入 `multicore_overhead` 大指标：

| JSON 字段 | 子项数量 | 单位 | 指标类型 | 展开 key |
|---|---:|---|---|---|
| `multi_core.memory_bandwidth[*].{c1,c2,c4,c8}` | 32 | GB/s | bandwidth | `multi_core_memory_bandwidth_{type}_{cN}` |
| `multi_core.ipc_bandwidth.{c1,c2,c4,c8}` | 4 | GB/s | bandwidth | `multi_core_ipc_bandwidth_{cN}` |
| `multi_core.task_latency.{c1,c2,c4,c8}` | 4 | us | latency | `multi_core_task_latency_{cN}` |
| `multi_core.core_comm.{intra_core,inter_core}` | 2 | GB/s | bandwidth | `multi_core_core_comm_{direction}` |

其中内存带宽 `type` 为 `rd/wr/cp/frd/fwr/fcp/memset/memcpy`，并发度为
`1/2/4/8`。带宽类按 `R = measured / baseline`，任务延迟按
`R = baseline / measured`。

**单核板卡：**

- 当前唯一单核板卡为 RISC-V 架构的哪吒板卡，由
  `is_single_core_board(board_model, arch, source)` 判定。判断不依赖
  `env.cpu_core_num`，因为部分多核 x86 数据会报告 0 或 1。历史哪吒结果若缺少
  `cpu_type` 或错误使用 `OneOS-Board / ARM` 等通用环境字段，以板名或结果
  文件名“哪吒/Nezha”识别；其他板卡即使 `cpu_core_num=1` 仍按多核板处理。
- 单核板卡不生成多核子项；即使输入 JSON 含 `multi_core`，也不参与评分。

### 4.2 调度性能 Sched_Score

见 §3.3，来源于 JSON 中 `test-schedule.summary.average_miss_rate`。

---

## 5. 典型负载 T_Score

$$
T = 0.25 \times C_{\text{ctrl}} + 0.25 \times C_{\text{perc}} + 0.25 \times C_{\text{comm}} + 0.25 \times C_{\text{mon}}
$$

每类得分 = 该类所有任务 item_score 的算术平均（min-max 归一化后）。

**任务分类映射：**

| 任务名 | 大类 | metric_type |
|--------|------|-------------|
| `pid` | `realtime_control` | latency |
| `ekf` | `perception` | latency |
| `icp` | `perception` | latency |
| `fast` | `perception` | latency |
| `epnp` | `perception` | latency |
| `modbus` | `communication` | latency |
| `mqtt` | `communication` | latency |
| `cusum` | `monitoring` | latency |
| `ewma` | `monitoring` | latency |

**缺失处理（partial 模式）：** 若某类缺失，按已有类的权重重新归一化，并标记 `incomplete=true`。

---

## 6. 功耗 P_Score

$$
\text{PowerRatio} = \frac{1}{4}\left(\frac{P_{\text{base,static}}}{P_{\text{meas,static}}} + \frac{P_{\text{base,cpu}}}{P_{\text{meas,cpu}}} + \frac{P_{\text{base,mem}}}{P_{\text{meas,mem}}} + \frac{P_{\text{base,file}}}{P_{\text{meas,file}}}\right)
$$

$$
P = \frac{\text{PowerRatio} - \text{Ratio}_{\min}}{\text{Ratio}_{\max} - \text{Ratio}_{\min}} \times 100
$$

**Excel 任务映射：**

| `测试任务` 字段值 | 内部字段 |
|---|---|
| `standby` | `static_power` |
| `cpu` | `cpu_power` |
| `memory` | `memory_power` |
| `file` | `file_power` |

功率值优先使用 `平均功率(W)`，缺失时回退为 `总功耗(J) / 测试总耗时(s)`。

---

## 7. 基础功能 F_Score

当前阶段无实测数据，直接取配置项：

```
F_Score = default_function_score  # 默认 60.0
```

输出中 `source` 标记为 `"placeholder"`，所有子项为 `null`。

待测试数据就绪后，按如下公式计算：

$$
F = \left(0.15 \times \frac{BW}{BW_{\text{base}}} + 0.10 \times \frac{L_{\text{base}}}{L} + 0.125 \times \frac{\text{IOPS}_s}{\text{IOPS}_{s,\text{base}}} + 0.125 \times \frac{\text{IOPS}_l}{\text{IOPS}_{l,\text{base}}} + 0.125 \times \frac{NB}{NB_{\text{base}}} + 0.125 \times \frac{NL_{\text{base}}}{NL} + 0.25 \times \frac{N_{\text{posix}}}{N_{\text{total}}}\right) \times 100
$$

---

## 8. JSON 输入格式

程序支持如下结构的测试结果 JSON（字段缺失时按最小粒度降级处理）：

```json
{
  "meta": {
    "framework_version": "string",
    "test_timestamp":    "ISO8601",
    "total_duration_sec": 0.0
  },
  "env": {
    "os_name":       "string",
    "os_version":    "string",
    "board":         "string",
    "cpu_type":      "string",
    "cpu_freq_mhz":  0,
    "cpu_core_num":  0
  },
  "modules": {
    "test-realtime": {
      "status": "passed",
      "single_core": {
        "context_switch": { "avg_us": 0.0 },
        "interrupt":      { "min_us": 0.0, "max_us": 0.0, "avg_us": 0.0 },
        "syscall":        { "min_us": 0.0, "max_us": 0.0, "avg_us": 0.0 },
        "service_cost": [
          {
            "operation":    "sem_take",
            "immediate_us": 0.0,
            "suspend_us":   0.0,
            "low_prio_us":  0.0,
            "high_prio_us": 0.0
          }
        ]
      },
      "multi_core": {
        "memory_bandwidth": [
          {
            "type": "rd",
            "unit": "GB/s",
            "c1": 0.0,
            "c2": 0.0,
            "c4": 0.0,
            "c8": 0.0
          }
        ],
        "ipc_bandwidth": {
          "c1": 0.0,
          "c2": 0.0,
          "c4": 0.0,
          "c8": 0.0,
          "unit": "GB/s"
        },
        "task_latency": {
          "c1": 0.0,
          "c2": 0.0,
          "c4": 0.0,
          "c8": 0.0,
          "unit": "us"
        },
        "core_comm": {
          "intra_core": 0.0,
          "inter_core": 0.0,
          "unit": "GB/s"
        }
      }
    },
    "test-schedule": {
      "status": "passed",
      "config": {
        "cycles":      10000,
        "util_start":  30,
        "util_end":    100,
        "util_step":   10
      },
      "gradients": [
        {
          "utilization_percent": 30,
          "actual_utilization":  0.3,
          "total_jobs":          0,
          "deadline_misses":     0,
          "miss_rate":           0.0,
          "task_stats": [
            {
              "name":            "string",
              "utilization":     0.0,
              "period_ms":       0.0,
              "jobs":            0,
              "misses":          0,
              "max_response_ms": 0.0
            }
          ]
        }
      ],
      "summary": {
        "average_miss_rate": 0.0,
        "final_score":       100.0
      }
    },
    "typical-workload": {
      "status": "passed",
      "workloads": [
        {
          "name":         "pid",
          "category":     "control",
          "success":      true,
          "rounds":       5,
          "exec_time_ms": 0.0,
          "avg_time_ms":  0.0
        }
      ]
    }
  }
}
```

**字段提取规则：**

| JSON 路径 | 用途 |
|-----------|------|
| `env.board` | → `board_model` |
| `env.os_name` | → `os_name` |
| `env.cpu_type` | → `arch` |
| `modules.test-realtime.single_core.context_switch.avg_us` | 上下文切换延迟 |
| `modules.test-realtime.single_core.interrupt.avg_us` | 中断延迟（= 0 时跳过）|
| `modules.test-realtime.single_core.syscall.*` | syscall 诊断数据，保存但不参与 Base_RT |
| `modules.test-realtime.single_core.service_cost[*]` | 各调度场景延迟子项 |
| `modules.test-realtime.multi_core.memory_bandwidth[*]` | 32 个内存带宽子项（bandwidth）|
| `modules.test-realtime.multi_core.ipc_bandwidth` | 4 个 IPC 带宽子项（bandwidth）|
| `modules.test-realtime.multi_core.task_latency` | 4 个任务延迟子项（latency）|
| `modules.test-realtime.multi_core.core_comm` | 2 个核间通信带宽子项（bandwidth）|
| `modules.test-schedule.summary.average_miss_rate` | 平均 MR → Sched_Score |
| `modules.test-schedule.gradients[*]` | 各梯度 MR 详情（只存储，不参与汇总） |
| `modules.typical-workload.workloads[*].avg_time_ms` | 各典型任务平均延迟 |

---

## 9. JSON 输出格式

```json
{
  "board_model": "QEMU-virt-aarch64",
  "arch":        "cortex-a53",
  "os_name":     "RT-Thread",
  "total_score": 78.4,
  "is_partial":  true,
  "scores": {
    "function": {
      "score":  60.0,
      "source": "placeholder",
      "details": {
        "memory_bandwidth":   null,
        "memory_latency":     null,
        "small_file_iops":    null,
        "large_file_iops":    null,
        "network_bandwidth":  null,
        "network_latency":    null,
        "posix_compatibility": null
      }
    },
    "realtime": {
      "score":      82.1,
      "incomplete": false,
      "details": {
        "base_realtime": {
          "score": 79.3,
          "sub_scores": {
            "context_switch_latency": 88.0,
            "interrupt_latency":      null,
            "scheduling_latency":     76.5,
            "multicore_overhead":     null
          },
          "items": {
            "context_switch_latency": {
              "avg_us": {
                "raw": 3.816, "unit": "us",
                "R": 1.0, "score": 50.0,
                "norm_pending": false, "status": "ok"
              }
            },
            "scheduling_latency": {
              "sem_take_immediate_us": {
                "raw": 1.120, "unit": "us",
                "R": 1.0, "score": 50.0,
                "norm_pending": false, "status": "ok"
              }
            }
          }
        },
        "schedulability": {
          "score":      100.0,
          "average_mr": 0.0,
          "items": {
            "average_mr": {
              "raw": 0.0, "unit": "ratio",
              "R": null, "score": 100.0,
              "norm_pending": false, "status": "ok"
            }
          }
        },
        "diagnostics": {
          "syscall_latency": {
            "included_in_base_rt": false,
            "items": {
              "avg_us": {
                "raw": 0.016, "unit": "us",
                "R": 1.0, "score": 50.0,
                "norm_pending": false, "status": "ok"
              }
            }
          }
        }
      }
    },
    "workload": {
      "score":    88.2,
      "incomplete": false,
      "missing_categories": [],
      "details": {
        "realtime_control": {
          "score": 90.0,
          "items": {
            "pid": {
              "raw": 322.0, "unit": "ms",
              "R": 1.0, "score": 50.0,
              "norm_pending": false, "status": "ok"
            }
          }
        },
        "perception": { "score": 85.0, "items": {} },
        "communication": { "score": 91.0, "items": {} },
        "monitoring": { "score": 87.0, "items": {} }
      }
    },
    "power": {
      "score": null,
      "normalization_pending": true,
      "power_ratio": null,
      "incomplete": true,
      "missing_tasks": ["static_power", "cpu_power", "memory_power", "file_power"],
      "details": {
        "static_power":  {},
        "cpu_power":     {},
        "memory_power":  {},
        "file_power":    {}
      }
    }
  },
  "completeness": {
    "missing_baselines": ["workload.perception.ekf"],
    "missing_scores":    ["power"]
  },
  "last_updated": "2025-01-01T00:00:00+00:00"
}
```

---

## 10. 数据库 Schema

### `raw_records` 表（最小粒度子项）

| 字段 | 类型 | 说明 |
|------|------|------|
| `board_model` | TEXT | 板卡型号（主键之一）|
| `os_name` | TEXT | 操作系统（主键之一）|
| `arch` | TEXT | 架构（主键之一）|
| `category` | TEXT | `realtime` / `workload` / `power` / `function` |
| `sub_category` | TEXT | 如 `scheduling_latency` / `syscall_latency` / `perception` / `cpu_power` |
| `item_key` | TEXT | 最细粒度 key，如 `sem_take_immediate_us` |
| `raw_value` | REAL | 原始测量值 |
| `raw_unit` | TEXT | 单位，如 `us` / `ms` / `W` |
| `metric_type` | TEXT | `latency` / `bandwidth` / `power` / `miss_rate` |
| `baseline_value` | REAL | 绝对基准系统（SylixOS + LS2K1000LA）的原始值 |
| `baseline_source` | TEXT | `db`（从数据库查找）/ `json`（JSON 自带）|
| `R_value` | REAL | 计算得到的基准比 |
| `item_score` | REAL | 归一化后子项得分（0-100），`norm_pending=1` 时为 null |
| `norm_pending` | INT | `1`=待归一化，`0`=已归一化 |
| `status` | TEXT | `ok` / `null_value` / `baseline_missing` / `error` |
| `source_file` | TEXT | 来源文件名 |
| `last_updated` | TEXT | ISO8601 时间戳 |

### `score_cache` 表（各级汇总得分）

| 字段 | 类型 | 说明 |
|------|------|------|
| `board_model` | TEXT | 主键 |
| `os_name` | TEXT | 主键 |
| `arch` | TEXT | 主键 |
| `score_level` | TEXT | `total` / `realtime` / `workload` / `power` |
| `score_key` | TEXT | 如 `total_score` / `rt_score` / `base_realtime` / `schedulability` |
| `score_value` | REAL | 得分值（null 表示待计算）|
| `is_partial` | INT | `1`=部分数据，结果为估算 |
| `extra_json` | TEXT | 附加信息（子分、缺失项等）|
| `last_updated` | TEXT | ISO8601 时间戳 |

---

## 11. CLI 命令

```bash
# 导入 JSON 测试结果（先导入 RT-Thread 作为基准）
python rtos_scorer.py import-json "./data/rt-thread_qemu.json"

# 导入其他系统（自动从 DB 查找 RT-Thread 基准并归一化）
python rtos_scorer.py import-json "./data/sylixos_qemu.json"

# 批量导入
python rtos_scorer.py import-json "./data/*.json"

# 导入功耗 Excel
python rtos_scorer.py import-excel "./data/power/*.xlsx"

# 导出某系统结果
python rtos_scorer.py export \
  --board "QEMU-virt-aarch64" \
  --os "RT-Thread" \
  --arch "cortex-a53" \
  --output result.json

# 查看所有已入库系统
python rtos_scorer.py list
```

---

## 12. 关键注意事项

| 事项 | 说明 |
|------|------|
| **基准系统必须先导入** | `SylixOS + LS2K1000LA` 的 JSON 必须先于其他系统导入，否则所有子项标记 `baseline_missing` |
| **min-max 跨系统触发** | 每次新系统入库后，同板卡**所有系统**的 item_score 都会被重算 |
| **Sched_Score 不归一化** | 调度性能直接用 `100*(1-MR)`，不参与 min-max |
| **功耗需多系统才能归一化** | 单个系统时 P_Score 为 null，标记 `normalization_pending=true` |
| **partial 模式** | 大类缺失时按已有类重新归一化权重，标记 `is_partial=true` |
| **总分 partial** | 某维度为 null 时，按已有维度重新归一化权重估算总分 |
| **多核缺失** | 缺失子项不进入 min-max；无有效多核项时从 Base_RT 剔除并重归一化剩余权重 |
| **syscall 仅诊断** | syscall 单独存于 `syscall_latency`，明确不参与 Base_RT/RT_Score |
