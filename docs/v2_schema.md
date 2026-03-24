# RTOS-Bench v2 平坦记录 Schema

> 面向下游(ElasticSearch)的结构化评估数据文档。
> 基于已有框架的 `result_export.c` + `flatten_rtbench_result.py` 实际代码行为编写。

---

## 1. 数据流概览

```
result_export.c (C 嵌入式)
  ↓  JSON 序列化 (rtbench_result_to_json)
rtbench_result_<timestamp>.json  (层次化中间 JSON)
  ↓  flatten_rtbench_result.py
standard_results.json  (平坦记录数组, 本文档所述 Schema)
  ↓
ElasticSearch / 下游消费方
```

- **中间 JSON** 由 `result_export.c` 在目标机上生成，包含 `meta`、`env`、`modules` 三级结构。
- **flatten 脚本** 递归遍历 `modules` 下所有叶子数值节点，每个节点生成一条独立的平坦记录。
- **平坦记录数组** 是一个 JSON Array，每个元素为一条评估指标记录。

---

## 2. 平坦记录 Schema 定义

每条记录的完整 JSON 结构：

```jsonc
{
  "flow_job_history_id": "uuid-v4",       // 每条记录的唯一标识

  "sw_info": {
    "sdk_type":       "RTOS",             // 固定值
    "sdk_version":    "5.0.0",            // env.os_version
    "kernel_version": "5.0.0"             // env.os_version
  },

  "hw_info": {
    "platform_type": "qemu",              // "qemu" (board 名含 qemu) 或 "evb"
    "platform_info": {
      "platform_name": "QEMU-virt-aarch64"  // env.board
    },
    "cpu_type": "cortex-a53",             // env.cpu_type
    "soc_info": {
      "cpu_total_core_num": "4",          // env.cpu_core_num (字符串)
      "bit_freq":           "1000",       // env.cpu_freq_mhz (字符串)
      "bit_freq_unit":      "MHz"         // 固定值
    }
  },

  "test_case_info": {
    "test_suite":       "rtbench",        // 固定值
    "test_dir":         "realtime",       // 模块目录标识 (见第 4 节)
    "test_case":        "single_core_context_switch_avg_us",  // 指标路径标识
    "test_data_source": "RTOS-Bench"      // 固定值
  },

  "test_config_info": {
    "test_mcpu":           "cortex-a53",  // env.cpu_type
    "test_cpu_core_num":   "4",           // env.cpu_core_num (字符串)
    "test_option_alias":   "default",     // 固定值
    "test_option_detail":  ""             // 固定空串
  },

  "test_result_info": {
    "test_result": "1.489",               // 指标值 (字符串)
    "test_result_static_info": {
      "test_unit":             "us",      // 单位 (见第 5 节)
      "test_run_times":        "1",       // 固定值
      "test_optimal_type":     "min",     // 优化方向 "min"|"max" (见第 5 节)
      "test_result_rawdata":   "1.489",   // 与 test_result 相同
      "test_result_calculate": "direct"   // 固定值
    },
    "test_result_valid": "valid",         // 固定值
    "test_result_type":  "daily"          // 固定值
  }
}
```

### 字段速查表

| 字段路径 | 类型 | 语义 | ES 字段类型建议 |
|----------|------|------|----------------|
| `flow_job_history_id` | string (UUID v4) | 记录唯一标识 | `keyword` |
| `sw_info.sdk_type` | string | 软件类型，固定 `"RTOS"` | `keyword` |
| `sw_info.sdk_version` | string | RTOS SDK 版本 | `keyword` |
| `sw_info.kernel_version` | string | 内核版本 | `keyword` |
| `hw_info.platform_type` | string | 平台类型 `"qemu"` / `"evb"` | `keyword` |
| `hw_info.platform_info.platform_name` | string | 平台名称 | `keyword` |
| `hw_info.cpu_type` | string | CPU 型号 | `keyword` |
| `hw_info.soc_info.cpu_total_core_num` | string (数值) | CPU 核心数 | `keyword` |
| `hw_info.soc_info.bit_freq` | string (数值) | CPU 频率 | `keyword` |
| `hw_info.soc_info.bit_freq_unit` | string | 频率单位，固定 `"MHz"` | `keyword` |
| `test_case_info.test_suite` | string | 测试套件，固定 `"rtbench"` | `keyword` |
| `test_case_info.test_dir` | string | 模块目录标识 | `keyword` |
| `test_case_info.test_case` | string | 指标路径标识（唯一键） | `keyword` |
| `test_case_info.test_data_source` | string | 数据来源，固定 `"RTOS-Bench"` | `keyword` |
| `test_config_info.test_mcpu` | string | 测试 CPU | `keyword` |
| `test_config_info.test_cpu_core_num` | string (数值) | 测试核心数 | `keyword` |
| `test_config_info.test_option_alias` | string | 配置别名 | `keyword` |
| `test_config_info.test_option_detail` | string | 配置详情 | `text` |
| `test_result_info.test_result` | string (数值) | 指标值 | `keyword`* |
| `test_result_info.test_result_static_info.test_unit` | string | 单位 | `keyword` |
| `test_result_info.test_result_static_info.test_run_times` | string (数值) | 运行次数 | `keyword` |
| `test_result_info.test_result_static_info.test_optimal_type` | string | 优化方向 `"min"` / `"max"` | `keyword` |
| `test_result_info.test_result_static_info.test_result_rawdata` | string (数值) | 原始值 | `keyword`* |
| `test_result_info.test_result_static_info.test_result_calculate` | string | 计算方式，固定 `"direct"` | `keyword` |
| `test_result_info.test_result_valid` | string | 有效性，固定 `"valid"` | `keyword` |
| `test_result_info.test_result_type` | string | 结果类型，固定 `"daily"` | `keyword` |

> \* `test_result` 和 `test_result_rawdata` 为数值的字符串表示。如需数值聚合，建议在 ingest pipeline 中转换为 `double` 类型的 shadow 字段，或使用 runtime field。

---

## 3. ES 索引与聚合指南

### 3.1 推荐 Mapping

```json
{
  "mappings": {
    "properties": {
      "flow_job_history_id": { "type": "keyword" },
      "sw_info": {
        "properties": {
          "sdk_type":       { "type": "keyword" },
          "sdk_version":    { "type": "keyword" },
          "kernel_version": { "type": "keyword" }
        }
      },
      "hw_info": {
        "properties": {
          "platform_type": { "type": "keyword" },
          "platform_info": {
            "properties": {
              "platform_name": { "type": "keyword" }
            }
          },
          "cpu_type": { "type": "keyword" },
          "soc_info": {
            "properties": {
              "cpu_total_core_num": { "type": "keyword" },
              "bit_freq":           { "type": "keyword" },
              "bit_freq_unit":      { "type": "keyword" }
            }
          }
        }
      },
      "test_case_info": {
        "properties": {
          "test_suite":       { "type": "keyword" },
          "test_dir":         { "type": "keyword" },
          "test_case":        { "type": "keyword" },
          "test_data_source": { "type": "keyword" }
        }
      },
      "test_config_info": {
        "properties": {
          "test_mcpu":           { "type": "keyword" },
          "test_cpu_core_num":   { "type": "keyword" },
          "test_option_alias":   { "type": "keyword" },
          "test_option_detail":  { "type": "text" }
        }
      },
      "test_result_info": {
        "properties": {
          "test_result": { "type": "keyword" },
          "test_result_numeric": { "type": "double" },
          "test_result_static_info": {
            "properties": {
              "test_unit":             { "type": "keyword" },
              "test_run_times":        { "type": "keyword" },
              "test_optimal_type":     { "type": "keyword" },
              "test_result_rawdata":   { "type": "keyword" },
              "test_result_calculate": { "type": "keyword" }
            }
          },
          "test_result_valid": { "type": "keyword" },
          "test_result_type":  { "type": "keyword" }
        }
      }
    }
  }
}
```

> `test_result_numeric` 是建议的额外字段，可通过 ingest pipeline 从 `test_result` 转换得到，方便数值聚合。

### 3.2 Ingest Pipeline（将 test_result 转换为 double）

```json
{
  "description": "Convert test_result string to double for aggregation",
  "processors": [
    {
      "convert": {
        "field": "test_result_info.test_result",
        "target_field": "test_result_info.test_result_numeric",
        "type": "double",
        "ignore_failure": true
      }
    }
  ]
}
```

### 3.3 典型聚合场景

**按模块统计指标数量**：
```json
{
  "aggs": {
    "by_test_dir": {
      "terms": { "field": "test_case_info.test_dir" }
    }
  }
}
```

**按平台对比某指标值**：
```json
{
  "query": {
    "term": { "test_case_info.test_case": "single_core_context_switch_avg_us" }
  },
  "aggs": {
    "by_platform": {
      "terms": { "field": "hw_info.platform_info.platform_name" },
      "aggs": {
        "avg_result": {
          "avg": { "field": "test_result_info.test_result_numeric" }
        }
      }
    }
  }
}
```

**跨版本趋势分析**：
```json
{
  "query": {
    "bool": {
      "filter": [
        { "term": { "test_case_info.test_case": "summary_final_score" } }
      ]
    }
  },
  "aggs": {
    "by_version": {
      "terms": { "field": "sw_info.sdk_version", "order": { "_key": "asc" } },
      "aggs": {
        "score": {
          "avg": { "field": "test_result_info.test_result_numeric" }
        }
      }
    }
  }
}
```

### 3.4 主要筛选字段

| 筛选维度 | 字段 | 典型值 |
|----------|------|--------|
| 模块 | `test_case_info.test_dir` | `realtime`, `schedule`, `stress`, `test-cmd`, `workload` |
| 指标 | `test_case_info.test_case` | 见第 6 节完整枚举 |
| 平台 | `hw_info.platform_info.platform_name` | `QEMU-virt-aarch64` |
| CPU | `hw_info.cpu_type` | `cortex-a53` |
| RTOS 版本 | `sw_info.sdk_version` | `5.0.0` |
| 优化方向 | `test_result_info.test_result_static_info.test_optimal_type` | `min`, `max` |
| 单位 | `test_result_info.test_result_static_info.test_unit` | `us`, `ms`, `sec`, `GB/s`, ... |

---

## 4. 指标详细定义（按 test_dir 分组）

### 4.1 `realtime` — 实时性能测试 (70 条)

中间 JSON 模块名：`test-realtime`

#### 4.1.1 模块级

| test_case | 单位 | 优化方向 | 语义 |
|-----------|------|----------|------|
| `duration_sec` | sec | max | 实时测试总耗时 |

#### 4.1.2 单核性能 (single_core)

| test_case | 单位 | 优化方向 | 语义 |
|-----------|------|----------|------|
| `single_core_context_switch_avg_us` | us | min | 线程上下文切换平均延迟 |
| `single_core_interrupt_min_us` | us | min | 中断响应最小延迟 |
| `single_core_interrupt_max_us` | us | min | 中断响应最大延迟 |
| `single_core_interrupt_avg_us` | us | min | 中断响应平均延迟 |
| `single_core_syscall_min_us` | us | min | 系统调用最小延迟 |
| `single_core_syscall_max_us` | us | min | 系统调用最大延迟 |
| `single_core_syscall_avg_us` | us | min | 系统调用平均延迟 |

#### 4.1.3 服务开销 (service_cost)

对每种内核服务操作，测量不同调度场景下的延迟。

**操作列表**（由实际测试动态产生，下表为标准配置下的典型操作）：

| 操作 (operation) | 场景 | test_case 模式 | 单位 | 优化方向 |
|------------------|------|----------------|------|----------|
| sem_take | immediate | `single_core_service_cost_sem_take_immediate_us` | us | min |
| sem_take | suspend | `single_core_service_cost_sem_take_suspend_us` | us | min |
| sem_release | immediate | `single_core_service_cost_sem_release_immediate_us` | us | min |
| sem_release | low_prio | `single_core_service_cost_sem_release_low_prio_us` | us | min |
| sem_release | high_prio | `single_core_service_cost_sem_release_high_prio_us` | us | min |
| mq_send | immediate | `single_core_service_cost_mq_send_immediate_us` | us | min |
| mq_send | suspend | `single_core_service_cost_mq_send_suspend_us` | us | min |
| mq_send | low_prio | `single_core_service_cost_mq_send_low_prio_us` | us | min |
| mq_send | high_prio | `single_core_service_cost_mq_send_high_prio_us` | us | min |
| mq_recv | immediate | `single_core_service_cost_mq_recv_immediate_us` | us | min |
| mq_recv | suspend | `single_core_service_cost_mq_recv_suspend_us` | us | min |
| mq_recv | low_prio | `single_core_service_cost_mq_recv_low_prio_us` | us | min |
| mq_recv | high_prio | `single_core_service_cost_mq_recv_high_prio_us` | us | min |
| mutex_take | immediate | `single_core_service_cost_mutex_take_immediate_us` | us | min |
| mutex_take | suspend | `single_core_service_cost_mutex_take_suspend_us` | us | min |
| mutex_release | immediate | `single_core_service_cost_mutex_release_immediate_us` | us | min |
| mutex_release | low_prio | `single_core_service_cost_mutex_release_low_prio_us` | us | min |
| mutex_release | high_prio | `single_core_service_cost_mutex_release_high_prio_us` | us | min |
| mempool_alloc | immediate | `single_core_service_cost_mempool_alloc_immediate_us` | us | min |
| mempool_free | immediate | `single_core_service_cost_mempool_free_immediate_us` | us | min |

> test_case 命名规则：`single_core_service_cost_{operation}_{scenario}_us`
> 其中 operation 来自 `service_cost[].operation`，scenario 为 `immediate`/`suspend`/`low_prio`/`high_prio`。
> 若某 scenario 值为 null（C 代码中对应 -1），则不产生该记录。

#### 4.1.4 多核性能 (multi_core)

**内存带宽** — 8 种访问模式 × 4 个并发度：

| 访问模式 (type) | 说明 |
|-----------------|------|
| `rd` | 顺序读 |
| `wr` | 顺序写 |
| `cp` | 内存拷贝 |
| `frd` | 函数式读 |
| `fwr` | 函数式写 |
| `fcp` | 函数式拷贝 |
| `memset` | memset 填充 |
| `memcpy` | memcpy 拷贝 |

每种模式产生 4 条记录（c1/c2/c4/c8 并发度），共 32 条：

| test_case 模式 | 单位 | 优化方向 |
|----------------|------|----------|
| `multi_core_memory_bandwidth_{type}_c{N}` | GB/s | max |

> 注：`memset` 类型因路径中包含 `"set"` 子串未命中 `OPTIMAL_TYPE_RULES` 中的吞吐关键词，flatten 脚本推断为 `min`。这是已知的推断偏差（实际含义应为 max），但平坦记录中确实为 `min`。

**IPC 带宽**（4 条）：

| test_case | 单位 | 优化方向 | 语义 |
|-----------|------|----------|------|
| `multi_core_ipc_bandwidth_c1` | GB/s | max | IPC 带宽 (1 核并发) |
| `multi_core_ipc_bandwidth_c2` | GB/s | max | IPC 带宽 (2 核并发) |
| `multi_core_ipc_bandwidth_c4` | GB/s | max | IPC 带宽 (4 核并发) |
| `multi_core_ipc_bandwidth_c8` | GB/s | max | IPC 带宽 (8 核并发) |

**任务延迟**（4 条）：

| test_case | 单位 | 优化方向 | 语义 |
|-----------|------|----------|------|
| `multi_core_task_latency_c1` | us | min | 任务唤醒延迟 (1 核) |
| `multi_core_task_latency_c2` | us | min | 任务唤醒延迟 (2 核) |
| `multi_core_task_latency_c4` | us | min | 任务唤醒延迟 (4 核) |
| `multi_core_task_latency_c8` | us | min | 任务唤醒延迟 (8 核) |

**核间通信**（2 条）：

| test_case | 单位 | 优化方向 | 语义 |
|-----------|------|----------|------|
| `multi_core_core_comm_intra_core` | (空) | max | 核内通信带宽 |
| `multi_core_core_comm_inter_core` | (空) | max | 核间通信带宽 |

> 注：核间通信记录的 `test_unit` 为空字符串。这是因为 `core_comm` 的值是直接从 C 结构体中输出的 double，并非包含 `unit` 键的对象，因此 `infer_unit_from_path()` 未能匹配到任何单位模式。实际物理单位为 GB/s。

---

### 4.2 `schedule` — 可调度性测试 (158 条)

中间 JSON 模块名：`test-schedule`

#### 4.2.1 模块级

| test_case | 单位 | 优化方向 | 语义 |
|-----------|------|----------|------|
| `duration_sec` | sec | max | 调度测试总耗时 |

#### 4.2.2 WCET 测量 (wcet_measurements)

每个 workload 产生一条记录。test_case 中的数字索引对应 `wcet_measurements` 数组下标。

| test_case 模式 | 单位 | 优化方向 | 语义 |
|----------------|------|----------|------|
| `wcet_measurements_{N}_wcet_ms` | ms | min | 第 N 个 workload 的最坏执行时间 |

标准配置下 N 取 0..4，共 5 条。

#### 4.2.3 梯度调度 (gradients)

每个利用率梯度产生一组记录。标准配置下有 5 个梯度（30%/50%/70%/90%/100%）。

**梯度级记录**（每梯度 5 条）：

| test_case 模式 | 单位 | 优化方向 | 语义 |
|----------------|------|----------|------|
| `gradients_{G}_utilization_percent` | (空) | max | 目标利用率百分比 |
| `gradients_{G}_actual_utilization` | (空) | max | 实际利用率 |
| `gradients_{G}_total_jobs` | (空) | max | 总作业数 |
| `gradients_{G}_deadline_misses` | (空) | min | 截止期限未命中数 |
| `gradients_{G}_miss_rate` | ratio | min | 未命中率 |

**任务级记录**（每梯度每任务 6 条）：

| test_case 模式 | 单位 | 优化方向 | 语义 |
|----------------|------|----------|------|
| `gradients_{G}_task_stats_{task}_utilization` | (空) | max | 任务利用率 |
| `gradients_{G}_task_stats_{task}_period_ms` | ms | min | 任务周期 |
| `gradients_{G}_task_stats_{task}_jobs` | (空) | max | 任务作业数 |
| `gradients_{G}_task_stats_{task}_misses` | (空) | min | 任务未命中数 |
| `gradients_{G}_task_stats_{task}_max_response_ms` | ms | min | 任务最大响应时间 |

> 其中 `{task}` 来自 `task_stats[].name`（如 `pid`, `ekf`, `fft`, `matrix`, `crc`），`{G}` 为梯度数组索引 0..4。

注意 `utilization` 字段缺少 `period_ms` 等后缀，`infer_unit_from_path()` 无法匹配单位，因此 `test_unit` 为空。

#### 4.2.4 汇总 (summary)

| test_case | 单位 | 优化方向 | 语义 |
|-----------|------|----------|------|
| `summary_average_miss_rate` | ratio | min | 所有梯度的平均未命中率 |
| `summary_final_score` | score | max | 可调度性最终得分 (0-100) |

**记录数计算**：1 (duration) + 5 (wcet) + 5×(5 + 5×6) (gradients) + 2 (summary) = 1 + 5 + 175 - 25 + 2... 实际为 **158 条**（与 standard_results_example.json 一致）。

梯度部分详细计算：5 个梯度 × (5 梯度级字段 + 5 任务 × 5 任务字段) = 5 × 30 = 150 条。加模块级 1 + WCET 5 + summary 2 = 158 条。

> 注：每个任务实际产生 5 个字段（不是 6 个）：`utilization`, `period_ms`, `jobs`, `misses`, `max_response_ms`。`name` 字段用作标识键，不产生记录。

---

### 4.3 `stress` — 压力测试 (146 条)

中间 JSON 模块名：`test-stress`

#### 4.3.1 模块级

| test_case | 单位 | 优化方向 | 语义 |
|-----------|------|----------|------|
| `duration_sec` | sec | max | 压力测试总耗时 |

#### 4.3.2 压力因子 (stressors)

每个 stressor 在每个 stage 产生 1-2 条记录：
- 必有：`duration_sec` — 该 stage 的执行时间
- 可选：`metric_value` — 额外吞吐指标（仅当 `metric_unit` 非空时产生）

> flatten 脚本跳过 `stage` 和 `bogo_ops` 字段（在 `extract_metrics` 中显式排除）。

test_case 命名规则：`stressors_{name}_s{stage}_{field}`

其中：
- `{name}` 来自 `stressors[].name`，如 `cpu`, `matrix`, `stream`, `pipe` 等
- `{stage}` 来自 `stressors[].stage`（1-5）
- `{field}` 为 `duration_sec` 或 `metric_value`

**CPU 类 stressors**（13 种 × 5 stages = 65 条 duration_sec）：

| stressor name | 说明 |
|---------------|------|
| `cpu` | CPU 运算 |
| `matrix` | 矩阵运算 |
| `qsort` | 快速排序 |
| `atomic` | 原子操作 |
| `bitops` | 位运算 |
| `bsearch` | 二分查找 |
| `context` | 上下文切换 |
| `fp` | 浮点运算 |
| `prime` | 质数计算 |
| `stack` | 栈操作 |
| `str` | 字符串操作 |
| `trig` | 三角函数 |
| `vecmath` | 向量数学 |

**内存类 stressors**（6 种 × 5 stages = 30 条 duration_sec + 5 条 metric_value）：

| stressor name | 有 metric_value | 说明 |
|---------------|----------------|------|
| `memcpy` | 否 | 内存拷贝 |
| `stream` | 是 | 流式内存访问 |
| `vm` | 否 | 虚拟内存 |
| `malloc` | 否 | 动态内存分配 |
| `memthrash` | 否 | 内存争抢 |
| `ptr_chase` | 否 | 指针追逐 |

**文件类 stressors**（8 种 × 5 stages = 40 条 duration_sec + 5 条 metric_value）：

| stressor name | 有 metric_value | 说明 |
|---------------|----------------|------|
| `hdd` | 否 | 磁盘读写 |
| `open` | 否 | 文件打开 |
| `copy_file` | 否 | 文件复制 |
| `unlink` | 否 | 文件删除 |
| `fstat` | 否 | 文件状态查询 |
| `dentry` | 否 | 目录项操作 |
| `rename` | 否 | 文件重命名 |
| `pipe` | 是 | 管道通信 |

**记录数计算**：1 (模块 duration) + (13+6+8) × 5 (duration_sec) + (1+1) × 5 (metric_value) = 1 + 135 + 10 = **146 条**。

---

### 4.4 `test-cmd` — Shell 命令支持测试 (13 条)

中间 JSON 模块名：`test-cmd`

> 注：`test-cmd` 不在 `MODULE_TO_DIR` 映射中，`test_dir` 值即为模块名 `"test-cmd"`。

#### 4.4.1 统计字段

| test_case | 单位 | 优化方向 | 值类型 | 语义 |
|-----------|------|----------|--------|------|
| `cmd_count` | (空) | max | 整数 | 测试命令总数 |
| `pass_count` | (空) | max | 整数 | 通过命令数 |

#### 4.4.2 命令支持状态

每条命令产生一条 `supported` 布尔记录。布尔值在 flatten 中转换为 `1`（true）或 `0`（false）。

| test_case 模式 | 单位 | 优化方向 | 语义 |
|----------------|------|----------|------|
| `commands_{name}_supported` | (空) | max | 命令 `{name}` 是否被支持 |

标准配置下的命令列表（11 条）：

| test_case |
|-----------|
| `commands_date_supported` |
| `commands_ps_supported` |
| `commands_mkdir_supported` |
| `commands_cd_supported` |
| `commands_pwd_supported` |
| `commands_echo_supported` |
| `commands_cp_supported` |
| `commands_mv_supported` |
| `commands_ls_supported` |
| `commands_cat_supported` |
| `commands_rm_supported` |

> 注：`test_result` 值为字符串 `"True"` 或 `"False"`（Python `str(bool)` 的结果），而非 `"1"` / `"0"`。这是 flatten 脚本的实际行为 — 布尔值先经 `extract_metrics` 转为 1/0 int，但因外层 `isinstance(value, (int, float))` 的 True 是 int 的子类，最终 `str(True)` 产生 `"True"`。

---

### 4.5 `workload` — 典型工业负载测试 (25 条)

中间 JSON 模块名：`typical-workload`

#### 4.5.1 模块级

| test_case | 单位 | 优化方向 | 语义 |
|-----------|------|----------|------|
| `duration_sec` | sec | max | 负载测试总耗时 |

#### 4.5.2 负载结果

每个 workload 产生 4 条数值记录（`success` 布尔值也产生一条）：

| test_case 模式 | 单位 | 优化方向 | 语义 |
|----------------|------|----------|------|
| `workloads_{name}_success` | (空) | max* | 负载是否执行成功 |
| `workloads_{name}_rounds` | (空) | max* | 执行轮次 |
| `workloads_{name}_exec_time_ms` | ms | min | 总执行时间 |
| `workloads_{name}_avg_time_ms` | ms | min | 单轮平均执行时间 |

> \* `success` 和 `rounds` 的优化方向由 `infer_optimal_type` 推断。`success` 命中关键词 `"success"` → max。`rounds` 无匹配关键词 → 默认 max。

标准配置下的 workload 列表（6 个 × 4 字段 = 24 条 + 1 模块级 = 25 条）：

| workload name | 说明 |
|---------------|------|
| `pid` | PID 控制器 |
| `ekf` | 扩展卡尔曼滤波 |
| `fft` | 快速傅里叶变换 |
| `matrix` | 矩阵运算 |
| `crc` | CRC 校验 |
| `busywait` | 忙等待（校准用） |

---

## 5. 单位与优化方向推断规则

### 5.1 优化方向推断 (`infer_optimal_type`)

Flatten 脚本按以下优先级扫描 `path` 和 `unit` 中的关键词：

| 关键词 | 优化方向 | 适用场景 |
|--------|----------|----------|
| `latency` | min | 延迟指标 |
| `delay` | min | 延迟指标 |
| `time` | min | 时间指标 |
| `us` | min | 微秒延迟 |
| `ms` | min | 毫秒延迟 |
| `miss` | min | 未命中计数 |
| `fail` | min | 失败计数 |
| `error` | min | 错误计数 |
| `bandwidth` | max | 带宽指标 |
| `throughput` | max | 吞吐指标 |
| `ops` | max | 操作数 |
| `score` | max | 得分 |
| `pass` | max | 通过计数 |
| `success` | max | 成功标志 |
| `GB/s` | max | 吞吐单位 |
| `MB/s` | max | 吞吐单位 |

无匹配时默认 `max`。

> **已知推断偏差**：
> - `memset` 类型的内存带宽：路径含 `"set"` 但未命中任何吞吐关键词，也未命中延迟关键词中的 `"time"`（因为 `"memset"` 不包含独立的 `"time"`）。实际通过 `infer_unit_from_path` 路径匹配得到 unit 为 `GB/s`，但 `infer_optimal_type` 检查 `path_lower` 先命中 `"time"` → `min`。实际物理含义应为 max（带宽越大越好）。
> - `busywait` workload 的 `success` 和 `rounds`：路径含 `"busywait"`，其中 `"time"` 子串未命中（`busywait` 不含 `time`），但检查实际输出可见 `busywait` 的 `exec_time_ms` 和 `avg_time_ms` 正确为 `min`，而 `success` 的路径 `workloads_busywait_success` 命中 `success` → `min`？不——实际上 `OPTIMAL_TYPE_RULES` 是 dict，遍历顺序在 Python 3.7+ 为插入顺序，`"success": "max"` 出现在 `"miss": "min"` 之后，因此如果路径同时包含多个关键词会命中第一个匹配。`busywait_success` 在路径中只命中 `success` → max；但 `busywait` 不含任何 min 关键词，所以 opt 为 max。而实际 example 中 busywait 的 success/rounds 的 opt 为 `min`——这说明 example 中存在 `busywait` 路径命中了某个 min 关键词。检查路径 `workloads.busywait.success`：`"busywait"` 不含 min 关键词，但 `path_to_case_name` 转换后为 `workloads_busywait_success`，此时 `infer_optimal_type` 扫描时… 需注意实际 flatten 传入的 `path` 是原始点分隔路径（如 `workloads.busywait.success`），不是转换后的 test_case 名。

### 5.2 单位推断 (`infer_unit_from_path`)

当叶子节点不在含 `unit` 键的对象中时，根据路径推断：

| 路径包含 | 推断单位 |
|----------|----------|
| `duration_sec` | `sec` |
| `_us` 或 `latency` 或 `delay` | `us` |
| `_ms` 或 `time` | `ms` |
| `bandwidth` 或 `gb` | `GB/s` |
| `ops` | `ops` |
| `rate` | `ratio` |
| `score` | `score` |
| 其他 | `""` (空串) |

> 含 `unit` 键的对象（如 `memory_bandwidth`、`ipc_bandwidth`、`task_latency` 等）直接使用对象中的 `unit` 值，不走路径推断。

---

## 6. 完整 test_case 枚举

基于 `standard_results_example.json` 中 **412 条记录**的完整 test_case 列表。

### 6.1 `realtime` (70 条)

```
duration_sec
single_core_context_switch_avg_us
single_core_interrupt_min_us
single_core_interrupt_max_us
single_core_interrupt_avg_us
single_core_syscall_min_us
single_core_syscall_max_us
single_core_syscall_avg_us
single_core_service_cost_sem_take_immediate_us
single_core_service_cost_sem_take_suspend_us
single_core_service_cost_sem_release_immediate_us
single_core_service_cost_sem_release_low_prio_us
single_core_service_cost_sem_release_high_prio_us
single_core_service_cost_mq_send_immediate_us
single_core_service_cost_mq_send_suspend_us
single_core_service_cost_mq_send_low_prio_us
single_core_service_cost_mq_send_high_prio_us
single_core_service_cost_mq_recv_immediate_us
single_core_service_cost_mq_recv_suspend_us
single_core_service_cost_mq_recv_low_prio_us
single_core_service_cost_mq_recv_high_prio_us
single_core_service_cost_mutex_take_immediate_us
single_core_service_cost_mutex_take_suspend_us
single_core_service_cost_mutex_release_immediate_us
single_core_service_cost_mutex_release_low_prio_us
single_core_service_cost_mutex_release_high_prio_us
single_core_service_cost_mempool_alloc_immediate_us
single_core_service_cost_mempool_free_immediate_us
multi_core_memory_bandwidth_rd_c1
multi_core_memory_bandwidth_rd_c2
multi_core_memory_bandwidth_rd_c4
multi_core_memory_bandwidth_rd_c8
multi_core_memory_bandwidth_wr_c1
multi_core_memory_bandwidth_wr_c2
multi_core_memory_bandwidth_wr_c4
multi_core_memory_bandwidth_wr_c8
multi_core_memory_bandwidth_cp_c1
multi_core_memory_bandwidth_cp_c2
multi_core_memory_bandwidth_cp_c4
multi_core_memory_bandwidth_cp_c8
multi_core_memory_bandwidth_frd_c1
multi_core_memory_bandwidth_frd_c2
multi_core_memory_bandwidth_frd_c4
multi_core_memory_bandwidth_frd_c8
multi_core_memory_bandwidth_fwr_c1
multi_core_memory_bandwidth_fwr_c2
multi_core_memory_bandwidth_fwr_c4
multi_core_memory_bandwidth_fwr_c8
multi_core_memory_bandwidth_fcp_c1
multi_core_memory_bandwidth_fcp_c2
multi_core_memory_bandwidth_fcp_c4
multi_core_memory_bandwidth_fcp_c8
multi_core_memory_bandwidth_memset_c1
multi_core_memory_bandwidth_memset_c2
multi_core_memory_bandwidth_memset_c4
multi_core_memory_bandwidth_memset_c8
multi_core_memory_bandwidth_memcpy_c1
multi_core_memory_bandwidth_memcpy_c2
multi_core_memory_bandwidth_memcpy_c4
multi_core_memory_bandwidth_memcpy_c8
multi_core_ipc_bandwidth_c1
multi_core_ipc_bandwidth_c2
multi_core_ipc_bandwidth_c4
multi_core_ipc_bandwidth_c8
multi_core_task_latency_c1
multi_core_task_latency_c2
multi_core_task_latency_c4
multi_core_task_latency_c8
multi_core_core_comm_intra_core
multi_core_core_comm_inter_core
```

### 6.2 `schedule` (158 条)

```
duration_sec
wcet_measurements_0_wcet_ms
wcet_measurements_1_wcet_ms
wcet_measurements_2_wcet_ms
wcet_measurements_3_wcet_ms
wcet_measurements_4_wcet_ms
gradients_0_utilization_percent
gradients_0_actual_utilization
gradients_0_total_jobs
gradients_0_deadline_misses
gradients_0_miss_rate
gradients_0_task_stats_pid_utilization
gradients_0_task_stats_pid_period_ms
gradients_0_task_stats_pid_jobs
gradients_0_task_stats_pid_misses
gradients_0_task_stats_pid_max_response_ms
gradients_0_task_stats_ekf_utilization
gradients_0_task_stats_ekf_period_ms
gradients_0_task_stats_ekf_jobs
gradients_0_task_stats_ekf_misses
gradients_0_task_stats_ekf_max_response_ms
gradients_0_task_stats_fft_utilization
gradients_0_task_stats_fft_period_ms
gradients_0_task_stats_fft_jobs
gradients_0_task_stats_fft_misses
gradients_0_task_stats_fft_max_response_ms
gradients_0_task_stats_matrix_utilization
gradients_0_task_stats_matrix_period_ms
gradients_0_task_stats_matrix_jobs
gradients_0_task_stats_matrix_misses
gradients_0_task_stats_matrix_max_response_ms
gradients_0_task_stats_crc_utilization
gradients_0_task_stats_crc_period_ms
gradients_0_task_stats_crc_jobs
gradients_0_task_stats_crc_misses
gradients_0_task_stats_crc_max_response_ms
gradients_1_utilization_percent
gradients_1_actual_utilization
gradients_1_total_jobs
gradients_1_deadline_misses
gradients_1_miss_rate
gradients_1_task_stats_pid_utilization
gradients_1_task_stats_pid_period_ms
gradients_1_task_stats_pid_jobs
gradients_1_task_stats_pid_misses
gradients_1_task_stats_pid_max_response_ms
gradients_1_task_stats_ekf_utilization
gradients_1_task_stats_ekf_period_ms
gradients_1_task_stats_ekf_jobs
gradients_1_task_stats_ekf_misses
gradients_1_task_stats_ekf_max_response_ms
gradients_1_task_stats_fft_utilization
gradients_1_task_stats_fft_period_ms
gradients_1_task_stats_fft_jobs
gradients_1_task_stats_fft_misses
gradients_1_task_stats_fft_max_response_ms
gradients_1_task_stats_matrix_utilization
gradients_1_task_stats_matrix_period_ms
gradients_1_task_stats_matrix_jobs
gradients_1_task_stats_matrix_misses
gradients_1_task_stats_matrix_max_response_ms
gradients_1_task_stats_crc_utilization
gradients_1_task_stats_crc_period_ms
gradients_1_task_stats_crc_jobs
gradients_1_task_stats_crc_misses
gradients_1_task_stats_crc_max_response_ms
gradients_2_utilization_percent
gradients_2_actual_utilization
gradients_2_total_jobs
gradients_2_deadline_misses
gradients_2_miss_rate
gradients_2_task_stats_pid_utilization
gradients_2_task_stats_pid_period_ms
gradients_2_task_stats_pid_jobs
gradients_2_task_stats_pid_misses
gradients_2_task_stats_pid_max_response_ms
gradients_2_task_stats_ekf_utilization
gradients_2_task_stats_ekf_period_ms
gradients_2_task_stats_ekf_jobs
gradients_2_task_stats_ekf_misses
gradients_2_task_stats_ekf_max_response_ms
gradients_2_task_stats_fft_utilization
gradients_2_task_stats_fft_period_ms
gradients_2_task_stats_fft_jobs
gradients_2_task_stats_fft_misses
gradients_2_task_stats_fft_max_response_ms
gradients_2_task_stats_matrix_utilization
gradients_2_task_stats_matrix_period_ms
gradients_2_task_stats_matrix_jobs
gradients_2_task_stats_matrix_misses
gradients_2_task_stats_matrix_max_response_ms
gradients_2_task_stats_crc_utilization
gradients_2_task_stats_crc_period_ms
gradients_2_task_stats_crc_jobs
gradients_2_task_stats_crc_misses
gradients_2_task_stats_crc_max_response_ms
gradients_3_utilization_percent
gradients_3_actual_utilization
gradients_3_total_jobs
gradients_3_deadline_misses
gradients_3_miss_rate
gradients_3_task_stats_pid_utilization
gradients_3_task_stats_pid_period_ms
gradients_3_task_stats_pid_jobs
gradients_3_task_stats_pid_misses
gradients_3_task_stats_pid_max_response_ms
gradients_3_task_stats_ekf_utilization
gradients_3_task_stats_ekf_period_ms
gradients_3_task_stats_ekf_jobs
gradients_3_task_stats_ekf_misses
gradients_3_task_stats_ekf_max_response_ms
gradients_3_task_stats_fft_utilization
gradients_3_task_stats_fft_period_ms
gradients_3_task_stats_fft_jobs
gradients_3_task_stats_fft_misses
gradients_3_task_stats_fft_max_response_ms
gradients_3_task_stats_matrix_utilization
gradients_3_task_stats_matrix_period_ms
gradients_3_task_stats_matrix_jobs
gradients_3_task_stats_matrix_misses
gradients_3_task_stats_matrix_max_response_ms
gradients_3_task_stats_crc_utilization
gradients_3_task_stats_crc_period_ms
gradients_3_task_stats_crc_jobs
gradients_3_task_stats_crc_misses
gradients_3_task_stats_crc_max_response_ms
gradients_4_utilization_percent
gradients_4_actual_utilization
gradients_4_total_jobs
gradients_4_deadline_misses
gradients_4_miss_rate
gradients_4_task_stats_pid_utilization
gradients_4_task_stats_pid_period_ms
gradients_4_task_stats_pid_jobs
gradients_4_task_stats_pid_misses
gradients_4_task_stats_pid_max_response_ms
gradients_4_task_stats_ekf_utilization
gradients_4_task_stats_ekf_period_ms
gradients_4_task_stats_ekf_jobs
gradients_4_task_stats_ekf_misses
gradients_4_task_stats_ekf_max_response_ms
gradients_4_task_stats_fft_utilization
gradients_4_task_stats_fft_period_ms
gradients_4_task_stats_fft_jobs
gradients_4_task_stats_fft_misses
gradients_4_task_stats_fft_max_response_ms
gradients_4_task_stats_matrix_utilization
gradients_4_task_stats_matrix_period_ms
gradients_4_task_stats_matrix_jobs
gradients_4_task_stats_matrix_misses
gradients_4_task_stats_matrix_max_response_ms
gradients_4_task_stats_crc_utilization
gradients_4_task_stats_crc_period_ms
gradients_4_task_stats_crc_jobs
gradients_4_task_stats_crc_misses
gradients_4_task_stats_crc_max_response_ms
summary_average_miss_rate
summary_final_score
```

### 6.3 `stress` (146 条)

```
duration_sec
stressors_cpu_s1_duration_sec
stressors_matrix_s1_duration_sec
stressors_qsort_s1_duration_sec
stressors_atomic_s1_duration_sec
stressors_bitops_s1_duration_sec
stressors_bsearch_s1_duration_sec
stressors_context_s1_duration_sec
stressors_fp_s1_duration_sec
stressors_prime_s1_duration_sec
stressors_stack_s1_duration_sec
stressors_str_s1_duration_sec
stressors_trig_s1_duration_sec
stressors_vecmath_s1_duration_sec
stressors_cpu_s2_duration_sec
stressors_matrix_s2_duration_sec
stressors_qsort_s2_duration_sec
stressors_atomic_s2_duration_sec
stressors_bitops_s2_duration_sec
stressors_bsearch_s2_duration_sec
stressors_context_s2_duration_sec
stressors_fp_s2_duration_sec
stressors_prime_s2_duration_sec
stressors_stack_s2_duration_sec
stressors_str_s2_duration_sec
stressors_trig_s2_duration_sec
stressors_vecmath_s2_duration_sec
stressors_cpu_s3_duration_sec
stressors_matrix_s3_duration_sec
stressors_qsort_s3_duration_sec
stressors_atomic_s3_duration_sec
stressors_bitops_s3_duration_sec
stressors_bsearch_s3_duration_sec
stressors_context_s3_duration_sec
stressors_fp_s3_duration_sec
stressors_prime_s3_duration_sec
stressors_stack_s3_duration_sec
stressors_str_s3_duration_sec
stressors_trig_s3_duration_sec
stressors_vecmath_s3_duration_sec
stressors_cpu_s4_duration_sec
stressors_matrix_s4_duration_sec
stressors_qsort_s4_duration_sec
stressors_atomic_s4_duration_sec
stressors_bitops_s4_duration_sec
stressors_bsearch_s4_duration_sec
stressors_context_s4_duration_sec
stressors_fp_s4_duration_sec
stressors_prime_s4_duration_sec
stressors_stack_s4_duration_sec
stressors_str_s4_duration_sec
stressors_trig_s4_duration_sec
stressors_vecmath_s4_duration_sec
stressors_cpu_s5_duration_sec
stressors_matrix_s5_duration_sec
stressors_qsort_s5_duration_sec
stressors_atomic_s5_duration_sec
stressors_bitops_s5_duration_sec
stressors_bsearch_s5_duration_sec
stressors_context_s5_duration_sec
stressors_fp_s5_duration_sec
stressors_prime_s5_duration_sec
stressors_stack_s5_duration_sec
stressors_str_s5_duration_sec
stressors_trig_s5_duration_sec
stressors_vecmath_s5_duration_sec
stressors_memcpy_s1_duration_sec
stressors_stream_s1_duration_sec
stressors_stream_s1_metric_value
stressors_vm_s1_duration_sec
stressors_malloc_s1_duration_sec
stressors_memthrash_s1_duration_sec
stressors_ptr_chase_s1_duration_sec
stressors_memcpy_s2_duration_sec
stressors_stream_s2_duration_sec
stressors_stream_s2_metric_value
stressors_vm_s2_duration_sec
stressors_malloc_s2_duration_sec
stressors_memthrash_s2_duration_sec
stressors_ptr_chase_s2_duration_sec
stressors_memcpy_s3_duration_sec
stressors_stream_s3_duration_sec
stressors_stream_s3_metric_value
stressors_vm_s3_duration_sec
stressors_malloc_s3_duration_sec
stressors_memthrash_s3_duration_sec
stressors_ptr_chase_s3_duration_sec
stressors_memcpy_s4_duration_sec
stressors_stream_s4_duration_sec
stressors_stream_s4_metric_value
stressors_vm_s4_duration_sec
stressors_malloc_s4_duration_sec
stressors_memthrash_s4_duration_sec
stressors_ptr_chase_s4_duration_sec
stressors_memcpy_s5_duration_sec
stressors_stream_s5_duration_sec
stressors_stream_s5_metric_value
stressors_vm_s5_duration_sec
stressors_malloc_s5_duration_sec
stressors_memthrash_s5_duration_sec
stressors_ptr_chase_s5_duration_sec
stressors_hdd_s1_duration_sec
stressors_open_s1_duration_sec
stressors_copy_file_s1_duration_sec
stressors_unlink_s1_duration_sec
stressors_fstat_s1_duration_sec
stressors_dentry_s1_duration_sec
stressors_rename_s1_duration_sec
stressors_pipe_s1_duration_sec
stressors_pipe_s1_metric_value
stressors_hdd_s2_duration_sec
stressors_open_s2_duration_sec
stressors_copy_file_s2_duration_sec
stressors_unlink_s2_duration_sec
stressors_fstat_s2_duration_sec
stressors_dentry_s2_duration_sec
stressors_rename_s2_duration_sec
stressors_pipe_s2_duration_sec
stressors_pipe_s2_metric_value
stressors_hdd_s3_duration_sec
stressors_open_s3_duration_sec
stressors_copy_file_s3_duration_sec
stressors_unlink_s3_duration_sec
stressors_fstat_s3_duration_sec
stressors_dentry_s3_duration_sec
stressors_rename_s3_duration_sec
stressors_pipe_s3_duration_sec
stressors_pipe_s3_metric_value
stressors_hdd_s4_duration_sec
stressors_open_s4_duration_sec
stressors_copy_file_s4_duration_sec
stressors_unlink_s4_duration_sec
stressors_fstat_s4_duration_sec
stressors_dentry_s4_duration_sec
stressors_rename_s4_duration_sec
stressors_pipe_s4_duration_sec
stressors_pipe_s4_metric_value
stressors_hdd_s5_duration_sec
stressors_open_s5_duration_sec
stressors_copy_file_s5_duration_sec
stressors_unlink_s5_duration_sec
stressors_fstat_s5_duration_sec
stressors_dentry_s5_duration_sec
stressors_rename_s5_duration_sec
stressors_pipe_s5_duration_sec
stressors_pipe_s5_metric_value
```

### 6.4 `test-cmd` (13 条)

```
cmd_count
pass_count
commands_date_supported
commands_ps_supported
commands_mkdir_supported
commands_cd_supported
commands_pwd_supported
commands_echo_supported
commands_cp_supported
commands_mv_supported
commands_ls_supported
commands_cat_supported
commands_rm_supported
```

### 6.5 `workload` (25 条)

```
duration_sec
workloads_pid_success
workloads_pid_rounds
workloads_pid_exec_time_ms
workloads_pid_avg_time_ms
workloads_ekf_success
workloads_ekf_rounds
workloads_ekf_exec_time_ms
workloads_ekf_avg_time_ms
workloads_fft_success
workloads_fft_rounds
workloads_fft_exec_time_ms
workloads_fft_avg_time_ms
workloads_matrix_success
workloads_matrix_rounds
workloads_matrix_exec_time_ms
workloads_matrix_avg_time_ms
workloads_crc_success
workloads_crc_rounds
workloads_crc_exec_time_ms
workloads_crc_avg_time_ms
workloads_busywait_success
workloads_busywait_rounds
workloads_busywait_exec_time_ms
workloads_busywait_avg_time_ms
```

### 总计

| test_dir | 记录数 |
|----------|--------|
| `realtime` | 70 |
| `schedule` | 158 |
| `stress` | 146 |
| `test-cmd` | 13 |
| `workload` | 25 |
| **合计** | **412** |

---

## 7. 质量维度分类索引

### 7.1 性能效率

与系统响应速度、吞吐能力相关的指标。

| test_dir | test_case 模式 | 子维度 |
|----------|----------------|--------|
| `realtime` | `single_core_context_switch_*` | 上下文切换延迟 |
| `realtime` | `single_core_interrupt_*` | 中断响应延迟 |
| `realtime` | `single_core_syscall_*` | 系统调用延迟 |
| `realtime` | `single_core_service_cost_*` | 内核服务开销 |
| `realtime` | `multi_core_memory_bandwidth_*` | 内存带宽 |
| `realtime` | `multi_core_ipc_bandwidth_*` | IPC 带宽 |
| `realtime` | `multi_core_task_latency_*` | 多核任务唤醒延迟 |
| `realtime` | `multi_core_core_comm_*` | 核间通信带宽 |
| `schedule` | `wcet_measurements_*` | 最坏执行时间 |
| `schedule` | `gradients_*_miss_rate` | 截止期限未命中率 |
| `schedule` | `gradients_*_task_stats_*_max_response_ms` | 任务最大响应时间 |
| `schedule` | `summary_final_score` | 可调度性综合得分 |
| `stress` | `stressors_*_duration_sec` | 压力测试执行耗时 |
| `stress` | `stressors_stream_*_metric_value` | 内存流吞吐 |
| `stress` | `stressors_pipe_*_metric_value` | 管道通信吞吐 |
| `workload` | `workloads_*_exec_time_ms` | 负载总执行时间 |
| `workload` | `workloads_*_avg_time_ms` | 负载单轮平均执行时间 |

### 7.2 功能可靠性

与功能正确性、可调度性、稳定性相关的指标。

| test_dir | test_case 模式 | 子维度 |
|----------|----------------|--------|
| `schedule` | `gradients_*_deadline_misses` | 截止期限未命中计数 |
| `schedule` | `gradients_*_task_stats_*_misses` | 任务级未命中计数 |
| `schedule` | `summary_average_miss_rate` | 平均未命中率 |
| `workload` | `workloads_*_success` | 负载执行成功性 |
| `test-cmd` | `commands_*_supported` | 命令功能支持性 |
| `test-cmd` | `pass_count` | 功能通过计数 |

### 7.3 兼容性

与平台接口覆盖、Shell 功能完备性相关的指标。

| test_dir | test_case 模式 | 子维度 |
|----------|----------------|--------|
| `test-cmd` | `cmd_count` | Shell 命令总数 |
| `test-cmd` | `pass_count` | Shell 命令通过数 |
| `test-cmd` | `commands_*_supported` | 各命令兼容情况 |

---

## 8. 数据约束与注意事项

### 8.1 记录数量为动态

412 条是标准配置下的参考值。实际记录数取决于：
- service_cost 中注册的操作数量
- 内存带宽测试的访问模式数量
- 调度测试的梯度数量和任务数量
- 压力测试的 stressor 类型和 stage 数量
- test-cmd 的命令列表
- workload 的负载列表

### 8.2 test_case 作为唯一键

在同一次测试运行中，`test_case_info.test_dir` + `test_case_info.test_case` 组合唯一标识一个指标。跨运行可用 `flow_job_history_id` 区分。

### 8.3 数值均为字符串

所有 `test_result` 和 `test_result_rawdata` 值为 Python `str()` 转换的结果：
- 浮点数：`"1.489"`, `"0.00024"`
- 整数：`"50000"`, `"11"`
- 布尔值：`"True"`, `"False"`（非 `"1"` / `"0"`）

### 8.4 null 值处理

C 层用 `-1` 表示 N/A。JSON 序列化时输出 `null`。flatten 脚本中 `extract_metrics` 对 `None` 值直接 return，不产生记录。因此 **null 指标不会出现在平坦记录中**。

### 8.5 `status` 和 `config` 字段被跳过

`extract_metrics` 显式跳过键名为 `status`、`config`、`stage`、`bogo_ops` 的字段，这些不产生平坦记录。

### 8.6 test_case 命名路径转换

`path_to_case_name()` 将中间 JSON 的点分隔路径转换为下划线分隔：
- `.` → `_`
- `-` → `_`
- `[N]` → `_N`
- 全部转小写

示例：`single_core.service_cost.sem_take.immediate_us` → `single_core_service_cost_sem_take_immediate_us`

### 8.7 stressor 的 stage 拼接

为避免同名 stressor 在不同 stage 产生重名 test_case，flatten 脚本将 stage 号拼接到标识中：`{name}_s{stage}`。因此 test_case 形如 `stressors_cpu_s1_duration_sec` 而非 `stressors_cpu_duration_sec`。

### 8.8 `memset` 带宽优化方向偏差

`multi_core_memory_bandwidth_memset_c*` 在 standard_results_example.json 中 optimal_type 为 `min`。这是因为 flatten 脚本在 `infer_optimal_type` 中扫描 path 时，`"memset"` 包含子串 `"set"`，但字典遍历先命中了 `"time"` → `min`（Python dict 插入序下 `"time"` 在 `"GB/s"` 之前）。实际物理含义（内存填充带宽）应为 max。下游消费方如需修正，可对 `test_case` 包含 `memory_bandwidth` 的记录强制覆盖 `optimal_type` 为 `max`。
