# RTOS-Bench 质量评估数据 Schema

> 本文档定义 RTOS-Bench 输出的标准化评估数据格式，供下游质量数据平台消费。
> 每次测试运行产生一个 JSON Array，包含 **536 条**（标准配置）扁平记录。

---

## 1. 记录结构

每条记录的完整 JSON 结构：

```jsonc
{
  "flow_job_history_id": "uuid-v4",

  "sw_info": {
    "sdk_type":       "RTOS",
    "sdk_version":    "5.0.0",
    "kernel_version": "5.0.0"
  },

  "hw_info": {
    "platform_type": "qemu",
    "platform_info": {
      "platform_name": "QEMU-virt-aarch64"
    },
    "cpu_type": "cortex-a53",
    "soc_info": {
      "cpu_total_core_num": "4",
      "bit_freq":           "1000",
      "bit_freq_unit":      "MHz"
    }
  },

  "test_case_info": {
    "test_suite":       "rtbench",
    "test_dir":         "realtime",
    "test_case":        "single_core_context_switch_avg_us",
    "test_data_source": "RTOS-Bench"
  },

  "test_config_info": {
    "test_mcpu":           "cortex-a53",
    "test_cpu_core_num":   "4",
    "test_option_alias":   "default",
    "test_option_detail":  ""
  },

  "test_result_info": {
    "test_result": "1.489",
    "test_result_static_info": {
      "test_unit":             "us",
      "test_run_times":        "1",
      "test_optimal_type":     "min",
      "test_result_rawdata":   "1.489",
      "test_result_calculate": "direct"
    },
    "test_result_valid": "valid",
    "test_result_type":  "daily"
  }
}
```

---

## 2. 字段说明

### 2.1 标识与环境

| 字段 | 类型 | 说明 |
|------|------|------|
| `flow_job_history_id` | string (UUID v4) | 每条记录的唯一标识 |
| `sw_info.sdk_type` | string | 固定 `"RTOS"` |
| `sw_info.sdk_version` | string | RTOS 版本号 |
| `sw_info.kernel_version` | string | 内核版本号（同 sdk_version） |
| `hw_info.platform_type` | string | `"qemu"` 或 `"evb"` |
| `hw_info.platform_info.platform_name` | string | 平台名称 |
| `hw_info.cpu_type` | string | CPU 型号 |
| `hw_info.soc_info.cpu_total_core_num` | string | CPU 核心数 |
| `hw_info.soc_info.bit_freq` | string | CPU 频率数值 |
| `hw_info.soc_info.bit_freq_unit` | string | 固定 `"MHz"` |

### 2.2 测试用例标识

| 字段 | 类型 | 说明 |
|------|------|------|
| `test_case_info.test_suite` | string | 固定 `"rtbench"` |
| `test_case_info.test_dir` | string | 测试模块：`realtime` / `schedule` / `stress` / `test-cmd` / `workload` |
| `test_case_info.test_case` | string | 指标名称（模块内唯一，完整枚举见第 4 节） |
| `test_case_info.test_data_source` | string | 固定 `"RTOS-Bench"` |

### 2.3 测试配置

| 字段 | 类型 | 说明 |
|------|------|------|
| `test_config_info.test_mcpu` | string | 测试 CPU 型号 |
| `test_config_info.test_cpu_core_num` | string | 测试核心数 |
| `test_config_info.test_option_alias` | string | 固定 `"default"` |
| `test_config_info.test_option_detail` | string | 固定空串 |

### 2.4 测试结果

| 字段 | 类型 | 说明 |
|------|------|------|
| `test_result_info.test_result` | string | 指标值（数值的字符串表示） |
| `test_result_static_info.test_unit` | string | 单位（见第 3 节） |
| `test_result_static_info.test_optimal_type` | string | 优化方向：`"min"`=越小越好，`"max"`=越大越好 |
| `test_result_static_info.test_run_times` | string | 固定 `"1"` |
| `test_result_static_info.test_result_rawdata` | string | 原始值（同 test_result） |
| `test_result_static_info.test_result_calculate` | string | 固定 `"direct"` |
| `test_result_info.test_result_valid` | string | 固定 `"valid"` |
| `test_result_info.test_result_type` | string | 固定 `"daily"` |

**数据类型说明**：
- 数值型指标：`"1.489"`, `"50000"`
- 布尔型指标（supported/success）：`"1"` 表示 true，`"0"` 表示 false
- null 指标不产生记录

---

## 3. 单位与优化方向

### 3.1 单位列表

| 单位 | 含义 | 使用场景 |
|------|------|----------|
| `us` | 微秒 | 延迟指标（上下文切换、中断、系统调用、服务开销、任务唤醒） |
| `ms` | 毫秒 | WCET、任务响应时间、负载执行时间 |
| `sec` | 秒 | 模块/stressor 执行耗时 |
| `GB/s` | 吉字节每秒 | 内存带宽、IPC 带宽、核间通信 |
| `ratio` | 比率 | 未命中率 |
| `score` | 分数 | 可调度性最终得分 |
| `bool` | 布尔 | 命令支持、负载成功 |
| (空) | 无量纲 | 利用率、作业数、未命中计数等 |

### 3.2 优化方向

| 方向 | 含义 | 典型指标 |
|------|------|----------|
| `min` | 越小越好 | 延迟、WCET、响应时间、未命中率、执行时间 |
| `max` | 越大越好 | 带宽、吞吐、得分、作业数、成功标志 |

> `duration_sec`（各模块耗时）的 `optimal_type` 为 `max`，但其本质是测试执行时间的元数据，不应作为性能排名指标。

---

## 4. 完整指标定义

### 4.1 `realtime` — 实时性能测试（70 条）

#### 单核性能

| test_case | 单位 | 方向 | 语义 |
|-----------|------|------|------|
| `duration_sec` | sec | max | 测试总耗时（元数据） |
| `single_core_context_switch_avg_us` | us | min | 线程上下文切换平均延迟 |
| `single_core_interrupt_min_us` | us | min | 中断响应最小延迟 |
| `single_core_interrupt_max_us` | us | min | 中断响应最大延迟 |
| `single_core_interrupt_avg_us` | us | min | 中断响应平均延迟 |
| `single_core_syscall_min_us` | us | min | 系统调用最小延迟 |
| `single_core_syscall_max_us` | us | min | 系统调用最大延迟 |
| `single_core_syscall_avg_us` | us | min | 系统调用平均延迟 |

#### 内核服务开销（20 条）

命名规则：`single_core_service_cost_{operation}_{scenario}_us`

| 操作 | 场景 | 语义 |
|------|------|------|
| `sem_take` | immediate, suspend | 信号量获取 |
| `sem_release` | immediate, low_prio, high_prio | 信号量释放 |
| `mq_send` | immediate, suspend, low_prio, high_prio | 消息队列发送 |
| `mq_recv` | immediate, suspend, low_prio, high_prio | 消息队列接收 |
| `mutex_take` | immediate, suspend | 互斥锁获取 |
| `mutex_release` | immediate, low_prio, high_prio | 互斥锁释放 |
| `mempool_alloc` | immediate | 内存池分配 |
| `mempool_free` | immediate | 内存池释放 |

全部 20 条，单位 `us`，方向 `min`。

#### 多核内存带宽（32 条）

命名规则：`multi_core_memory_bandwidth_{type}_c{N}`，N 取 1/2/4/8

| 访问模式 | 说明 |
|----------|------|
| `rd` | 顺序读 |
| `wr` | 顺序写 |
| `cp` | 内存拷贝 |
| `frd` | 函数式读 |
| `fwr` | 函数式写 |
| `fcp` | 函数式拷贝 |
| `memset` | memset 填充 |
| `memcpy` | memcpy 拷贝 |

8 种模式 × 4 并发度 = 32 条，单位 `GB/s`，方向 `max`。

#### 多核 IPC 带宽（4 条）

| test_case | 单位 | 方向 | 语义 |
|-----------|------|------|------|
| `multi_core_ipc_bandwidth_c{1,2,4,8}` | GB/s | max | IPC 带宽 |

#### 多核任务延迟（4 条）

| test_case | 单位 | 方向 | 语义 |
|-----------|------|------|------|
| `multi_core_task_latency_c{1,2,4,8}` | us | min | 任务唤醒延迟 |

#### 核间通信（2 条）

| test_case | 单位 | 方向 | 语义 |
|-----------|------|------|------|
| `multi_core_core_comm_intra_core` | GB/s | max | 核内通信带宽 |
| `multi_core_core_comm_inter_core` | GB/s | max | 核间通信带宽 |

---

### 4.2 `schedule` — 可调度性测试（262 条）

#### 模块级 + 汇总（3 条）

| test_case | 单位 | 方向 | 语义 |
|-----------|------|------|------|
| `duration_sec` | sec | max | 测试总耗时（元数据） |
| `summary_average_miss_rate` | ratio | min | 所有梯度的平均未命中率 |
| `summary_final_score` | score | max | 可调度性最终得分 (0-100) |

#### WCET 测量（9 条）

| test_case | 单位 | 方向 | 语义 |
|-----------|------|------|------|
| `wcet_measurements_{0..8}_wcet_ms` | ms | min | 各 workload 的最坏执行时间 |

#### 梯度调度（250 条）

5 个利用率梯度（30%/50%/70%/90%/100%），索引 G 取 0..4。

**梯度级**（每梯度 5 条，共 25 条）：

| test_case | 单位 | 方向 | 语义 |
|-----------|------|------|------|
| `gradients_{G}_utilization_percent` | (空) | max | 目标利用率 |
| `gradients_{G}_actual_utilization` | (空) | max | 实际利用率 |
| `gradients_{G}_total_jobs` | (空) | max | 总作业数 |
| `gradients_{G}_deadline_misses` | (空) | min | 截止期限未命中数 |
| `gradients_{G}_miss_rate` | ratio | min | 未命中率 |

**任务级**（每梯度 9 个任务 × 5 字段 = 45 条，共 225 条）：

命名规则：`gradients_{G}_task_stats_{task}_{field}`

任务名：`pid`, `ekf`, `fast`, `epnp`, `icp`, `modbus`, `mqtt`, `cusum`, `ewma`

| field | 单位 | 方向 | 语义 |
|-------|------|------|------|
| `utilization` | (空) | max | 任务利用率 |
| `period_ms` | ms | min | 任务周期 |
| `jobs` | (空) | max | 任务作业数 |
| `misses` | (空) | min | 任务未命中数 |
| `max_response_ms` | ms | min | 任务最大响应时间 |

---

### 4.3 `stress` — 压力测试（146 条）

#### 模块级（1 条）

| test_case | 单位 | 方向 | 语义 |
|-----------|------|------|------|
| `duration_sec` | sec | max | 测试总耗时（元数据） |

#### 压力因子（145 条）

命名规则：`stressors_{name}_s{stage}_{field}`，stage 取 1..5

每个 stressor 在每个 stage 产生 `duration_sec`，部分 stressor 额外产生 `metric_value`。

| 类别 | stressor 名称 | 有 metric_value | 说明 |
|------|---------------|:---:|------|
| CPU | `cpu`, `matrix`, `qsort`, `atomic`, `bitops`, `bsearch`, `context`, `fp`, `prime`, `stack`, `str`, `trig`, `vecmath` | 否 | 13 种 |
| 内存 | `memcpy`, `vm`, `malloc`, `memthrash`, `ptr_chase` | 否 | 5 种 |
| 内存 | `stream` | 是 | 流式内存访问 |
| 文件 | `hdd`, `open`, `copy_file`, `unlink`, `fstat`, `dentry`, `rename` | 否 | 7 种 |
| 文件 | `pipe` | 是 | 管道通信 |

- `duration_sec`：27 种 × 5 stages = 135 条，单位 `sec`，方向 `max`（元数据）
- `metric_value`：2 种 × 5 stages = 10 条，单位 (空)，方向 `max`

---

### 4.4 `test-cmd` — Shell 命令支持测试（13 条）

| test_case | 单位 | 方向 | 语义 |
|-----------|------|------|------|
| `cmd_count` | (空) | max | 测试命令总数 |
| `pass_count` | (空) | max | 通过命令数 |
| `commands_date_supported` | bool | max | date 命令是否支持 |
| `commands_ps_supported` | bool | max | ps 命令是否支持 |
| `commands_mkdir_supported` | bool | max | mkdir 命令是否支持 |
| `commands_cd_supported` | bool | max | cd 命令是否支持 |
| `commands_pwd_supported` | bool | max | pwd 命令是否支持 |
| `commands_echo_supported` | bool | max | echo 命令是否支持 |
| `commands_cp_supported` | bool | max | cp 命令是否支持 |
| `commands_mv_supported` | bool | max | mv 命令是否支持 |
| `commands_ls_supported` | bool | max | ls 命令是否支持 |
| `commands_cat_supported` | bool | max | cat 命令是否支持 |
| `commands_rm_supported` | bool | max | rm 命令是否支持 |

布尔值以 `"1"`（支持）/ `"0"`（不支持）表示。

---

### 4.5 `workload` — 典型工业负载测试（45 条）

#### 模块级（1 条）

| test_case | 单位 | 方向 | 语义 |
|-----------|------|------|------|
| `duration_sec` | sec | max | 测试总耗时（元数据） |

#### 负载结果（44 条）

命名规则：`workloads_{name}_{field}`

| 负载名称 | 说明 |
|----------|------|
| `stub` | 空操作（校准用） |
| `busywait` | 忙等待（校准用） |
| `fast` | FAST 特征检测 |
| `epnp` | EPnP 位姿估计 |
| `ekf` | 扩展卡尔曼滤波 |
| `icp` | ICP 点云配准 |
| `modbus` | Modbus 协议解析 |
| `mqtt` | MQTT 协议处理 |
| `pid` | PID 控制器 |
| `cusum` | CUSUM 变点检测 |
| `ewma` | EWMA 指数加权移动平均 |

每个负载 4 条：

| field | 单位 | 方向 | 语义 |
|-------|------|------|------|
| `success` | bool | max | 是否执行成功（`"1"` / `"0"`） |
| `rounds` | (空) | max | 执行轮次 |
| `exec_time_ms` | ms | min | 总执行时间 |
| `avg_time_ms` | ms | min | 单轮平均执行时间 |

---

## 5. 质量维度分类

### 5.1 性能效率

| 模块 | 指标模式 | 子维度 |
|------|----------|--------|
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
| `stress` | `stressors_stream_*_metric_value` | 内存流吞吐 |
| `stress` | `stressors_pipe_*_metric_value` | 管道通信吞吐 |
| `workload` | `workloads_*_exec_time_ms` | 负载执行时间 |
| `workload` | `workloads_*_avg_time_ms` | 负载单轮平均时间 |

### 5.2 功能可靠性

| 模块 | 指标模式 | 子维度 |
|------|----------|--------|
| `schedule` | `gradients_*_deadline_misses` | 截止期限未命中计数 |
| `schedule` | `gradients_*_task_stats_*_misses` | 任务级未命中计数 |
| `schedule` | `summary_average_miss_rate` | 平均未命中率 |
| `workload` | `workloads_*_success` | 负载执行成功性 |
| `test-cmd` | `commands_*_supported` | 命令功能支持性 |

### 5.3 兼容性

| 模块 | 指标模式 | 子维度 |
|------|----------|--------|
| `test-cmd` | `cmd_count` / `pass_count` | Shell 命令覆盖度 |
| `test-cmd` | `commands_*_supported` | 各命令兼容情况 |

---

## 6. 记录数量汇总

| test_dir | 记录数 |
|----------|--------|
| `realtime` | 70 |
| `schedule` | 262 |
| `stress` | 146 |
| `test-cmd` | 13 |
| `workload` | 45 |
| **合计** | **536** |

536 条为标准配置下的参考值。实际记录数随测试配置（注册的内核服务操作数、stressor 类型、workload 列表等）动态变化。

**唯一键**：同一次测试中，`test_dir` + `test_case` 组合唯一标识一个指标。跨运行使用 `flow_job_history_id` 区分。
