# RTOS-Bench 分数计算集成方案

> 本文档整理当前框架的结果输出架构，并给出四个模块（test-realtime / test-schedule / test-stress / typical-workload）分数计算的最小侵入性集成方案，供团队对齐后集成。

---

## 一、当前架构概览

### 1.1 结果数据流

```
[模块内部测试] ─→ [模块静态结果变量] ─→ collect_*_result() ─→ [全局 g_result] ─→ JSON 序列化 ─→ 文件
```

关键文件：

| 文件 | 职责 |
|------|------|
| `generator/result_export.h` | 所有结果数据结构定义、API 声明 |
| `generator/result_export.c` | JSON 序列化、文件写入、全局 `g_result` |
| `generator/rtthread_entry.c` | `test-all` 编排、`collect_*_result()` 转换函数 |
| `generator/test_schedule.c` | 唯一已实现分数计算的模块 |

### 1.2 全局结果结构 (`struct rtbench_result`)

```c
// result_export.h:252
struct rtbench_result {
    char framework_version[16];       // "1.0.0"
    char test_timestamp[32];          // ISO 8601
    double total_duration_sec;
    struct rtbench_env_info env;
    struct rtbench_realtime_result realtime;
    struct rtbench_schedule_result schedule;
    struct rtbench_stress_result stress;
    struct rtbench_cmd_module_result cmd;
    struct rtbench_workload_module_result workload;
};
```

### 1.3 `test-all` 执行流程

```
rtbench_result_init()
rtbench_result_set_env(...)
rtbench_result_start()
  ├─ test_realtime_run()    → collect_realtime_result()
  ├─ test_schedule_run()    → collect_schedule_result()
  ├─ test_stress_run_stressor() → collect_stress_result()
  ├─ test_cmd_run()         → collect_cmd_result()
  └─ collect_workload_results()
rtbench_result_end()
rtbench_result_export_json(path)   → 写入 JSON 文件
rtbench_result_cleanup()
```

### 1.4 JSON 输出文件

- 默认路径: `/rtbench_result.json`（可通过 `-o` 指定）
- 自动导出: `rtbench_result_set_auto_export("/results/")` 会在 `result_end()` 时自动保存带时间戳的文件
- 128KB 缓冲，手写 `snprintf` JSON 序列化（无外部依赖）

---

## 二、各模块当前输出详情

### 2.1 test-realtime

**结果结构** (`result_export.h:82`):

```c
struct rtbench_realtime_result {
    int valid;
    double duration_sec;

    // 单核指标
    double context_switch_avg_us;               // 上下文切换 平均延迟 (μs)
    double interrupt_min_us/max_us/avg_us;      // 中断延迟 三值 (μs)
    double syscall_min_us/max_us/avg_us;        // 系统调用延迟 三值 (μs)
    struct rtbench_service_cost service_cost[16]; // IPC 服务开销 (8个操作 × 4种场景)
    int service_cost_count;

    // 多核指标
    int multicore_valid;
    struct rtbench_mem_bw_entry mem_bw[8];       // 内存带宽 (8种类型 × 4并发度, GB/s)
    double ipc_bw_c1/c2/c4/c8;                  // IPC 带宽 (GB/s)
    double task_lat_c1/c2/c4/c8;                // 任务创建/删除延迟 (μs)
    double core_comm_intra/inter;               // 核间通信带宽 (GB/s)
};
```

**Service cost 子项**:
```c
struct rtbench_service_cost {
    char operation[64];     // "sem_take", "sem_release", "mq_send", ...
    double immediate_us;    // -1 表示 N/A
    double suspend_us;
    double low_prio_us;
    double high_prio_us;
};
```

**数据来源**: `bench_init.c` 中的静态数组 → 通过 getter 函数 → `collect_realtime_result()` 进行 ns→μs 转换

**JSON 输出 schema**:
```json
"test-realtime": {
    "status": "passed",
    "duration_sec": 45.000,
    "single_core": {
        "context_switch": { "avg_us": 1.234 },
        "interrupt": { "min_us": 0.5, "max_us": 10.0, "avg_us": 2.3 },
        "syscall": { "min_us": 0.1, "max_us": 5.0, "avg_us": 1.2 },
        "service_cost": [
            { "operation": "sem_take", "immediate_us": 0.5, "suspend_us": 1.0,
              "low_prio_us": null, "high_prio_us": null },
            ...
        ]
    },
    "multi_core": {
        "memory_bandwidth": [ { "type": "rd", "unit": "GB/s", "c1":..., "c2":..., "c4":..., "c8":... }, ... ],
        "ipc_bandwidth": { "c1":..., "c2":..., "c4":..., "c8":..., "unit": "GB/s" },
        "task_latency": { "c1":..., "c2":..., "c4":..., "c8":..., "unit": "us" },
        "core_comm": { "intra_core":..., "inter_core":..., "unit": "GB/s" }
    }
}
```

**当前分数**: 无。纯原始指标输出。

---

### 2.2 test-schedule

**模块内部结构** (`test_schedule.h`):
```c
struct test_schedule_result {
    int num_gradients;
    struct schedule_gradient_result gradients[8];  // 30%~100%, step 10%
    double average_miss_rate;
    double final_score;         // ← 已有分数
};
```

**导出结构** (`result_export.h:149`):
```c
struct rtbench_schedule_result {
    int valid;
    double duration_sec;
    int cycles, util_start, util_end, util_step;
    struct rtbench_wcet_entry wcet[32];
    int wcet_count;
    struct rtbench_gradient_result gradients[16];
    int gradient_count;
    double average_miss_rate;
    double final_score;         // ← 已有分数槽位
};
```

**已有分数公式** (`test_schedule.c:527-528`):
```
Final Score = 100 × (1 - Average_MR)
其中 Average_MR = sum(每个梯度的 miss_rate) / 梯度数
其中 miss_rate = total_deadline_misses / total_jobs
```

**JSON 输出 schema**:
```json
"test-schedule": {
    "status": "passed",
    "duration_sec": 300.000,
    "config": { "cycles": 10000, "util_start": 30, "util_end": 100, "util_step": 10 },
    "wcet_measurements": [ { "workload": "fast", "wcet_ms": 1.234 }, ... ],
    "gradients": [
        {
            "utilization_percent": 30,
            "actual_utilization": 0.3000,
            "total_jobs": 50000,
            "deadline_misses": 100,
            "miss_rate": 0.002000,
            "task_stats": [ { "name": "fast", "utilization": 0.15, "period_ms": 8.227,
                              "jobs": 10000, "misses": 20, "max_response_ms": 9.5 }, ... ]
        }, ...
    ],
    "summary": { "average_miss_rate": 0.123456, "final_score": 87.65 }
}
```

**当前分数**: 已实现，输出在 `summary.final_score`。

---

### 2.3 test-stress

**导出结构** (`result_export.h:189`):
```c
struct rtbench_stress_result {
    int valid;
    double duration_sec;
    struct rtbench_stressor_result stressors[16];
    int stressor_count;
};

struct rtbench_stressor_result {
    char name[64];        // "cpu", "matrix", "vm", ...
    char type[16];        // "cpu" / "memory" / "file"
    uint64_t bogo_ops;
    double duration_sec;
    double metric_value;  // bogo_ops_per_sec
    char metric_unit[16]; // "ops/s"
};
```

**数据来源**: `stress-ng.c` → `g_last_bogo.current_ops` → `collect_stress_result()` 计算 `bogo_per_sec`

**JSON 输出 schema**:
```json
"test-stress": {
    "status": "passed",
    "duration_sec": 10.000,
    "stressors": [
        { "name": "cpu", "type": "cpu", "bogo_ops": 12345,
          "duration_sec": 10.000, "metric_value": 1234.500, "metric_unit": "ops/s" }
    ]
}
```

**当前分数**: 无。仅有 `bogo_ops` 和 `metric_value` 原始指标。

---

### 2.4 typical-workload

**导出结构** (`result_export.h:238`):
```c
struct rtbench_workload_module_result {
    int valid;
    double duration_sec;
    struct rtbench_workload_result workloads[32];
    int workload_count;
};

struct rtbench_workload_result {
    char name[64];
    char category[64];     // "vision", "estimation", "control", "network", "detection"
    int success;
    int rounds;            // 固定 100 轮
    double exec_time_ms;   // 总执行时间
    double avg_time_ms;    // 平均单轮时间
};
```

**数据来源**: `collect_workload_results()` 中逐个 workload 跑 100 轮，用 `rt_tick_get()` 测时

**JSON 输出 schema**:
```json
"typical-workload": {
    "status": "passed",
    "duration_sec": 60.000,
    "workloads": [
        { "name": "ekf", "category": "estimation", "success": true,
          "rounds": 100, "exec_time_ms": 5678.901, "avg_time_ms": 56.789 },
        ...
    ]
}
```

**当前分数**: 无。仅有执行时间原始指标。

---

## 三、分数计算集成方案

### 3.1 设计原则

1. **最小侵入性**: 只增加字段和计算函数，不改动已有数据流
2. **统一插槽**: 每个模块在导出结构中增加 `double score` 字段
3. **分层计算**: 模块内部计算子分数 → 框架层汇总总分
4. **可扩展**: 子分数计算函数独立，后续可替换公式
5. **向后兼容**: JSON 中新增字段，不破坏旧 schema

### 3.2 结构体变更

#### result_export.h 中增加分数字段

```c
// ① test-realtime: 增加分数
struct rtbench_realtime_result {
    // ... 已有字段不变 ...
    double score;              // [新增] 实时性能综合分数 (0-100)
};

// ② test-schedule: 已有 final_score, 无需改动

// ③ test-stress: 增加分数
struct rtbench_stress_result {
    // ... 已有字段不变 ...
    double score;              // [新增] 压力测试综合分数 (0-100)
};

// ④ typical-workload: 增加分数
struct rtbench_workload_result {
    // ... 已有字段不变 ...
    double score;              // [新增] 单 workload 分数 (0-100)
};
struct rtbench_workload_module_result {
    // ... 已有字段不变 ...
    double score;              // [新增] 典型负载综合分数 (0-100)
};

// ⑤ 总分: rtbench_result 增加
struct rtbench_result {
    // ... 已有字段不变 ...
    double total_score;        // [新增] 综合总分 (0-100)
};
```

### 3.3 分数计算函数（新增文件: `generator/score_calc.h` + `score_calc.c`）

独立文件，避免污染已有模块逻辑。

```c
// score_calc.h
#ifndef SCORE_CALC_H
#define SCORE_CALC_H

#include "result_export.h"

// 计算各模块分数（就地修改 result 中的 score 字段）
void score_calc_realtime(struct rtbench_realtime_result *r);
void score_calc_stress(struct rtbench_stress_result *r);
void score_calc_workload(struct rtbench_workload_module_result *r);
// test-schedule 已有 final_score, 不需要额外计算

// 计算综合总分（依赖各模块 score 已填入）
void score_calc_total(struct rtbench_result *r);

#endif
```

### 3.4 各模块评分方案参考

> 以下公式为**参考设计**，具体权重和阈值需团队确认。标记 `[待定]` 的参数需要团队讨论决定。

#### ① test-realtime 评分

**思路**: 对各延迟指标基于阈值打分，加权求和。

```
子分数 = f(指标值, 阈值_优秀, 阈值_及格)
       = 100            若 指标值 ≤ 阈值_优秀
       = 线性插值        若 阈值_优秀 < 指标值 < 阈值_及格
       = 0              若 指标值 ≥ 阈值_及格

实时分数 = w1 × S(context_switch_avg)
         + w2 × S(interrupt_avg)
         + w3 × S(syscall_avg)
         + w4 × S(service_cost_avg)   // 所有 IPC 操作的均值
```

**参考阈值与权重** `[待定]`:

| 指标 | 阈值_优秀 (μs) | 阈值_及格 (μs) | 权重 |
|------|---------------|---------------|------|
| context_switch_avg | 1.0 | 50.0 | 0.30 |
| interrupt_avg | 1.0 | 50.0 | 0.30 |
| syscall_avg | 1.0 | 50.0 | 0.20 |
| service_cost_avg | 1.0 | 50.0 | 0.20 |

**多核指标**: 若 `multicore_valid`，可作为加分项或独立子维度 `[待定]`。

**实现要点**:
- 在 `score_calc_realtime()` 中计算
- 写入 `r->score`
- 调用时机: `collect_realtime_result()` 末尾

#### ② test-schedule 评分

**已实现**, 无需修改:
```
final_score = 100 × (1 - average_miss_rate)
```

- 数据路径: `test_schedule.c:528` → `collect_schedule_result()` → `sched->final_score`
- JSON 路径: `modules.test-schedule.summary.final_score`

在 `score_calc_total()` 中直接读取 `r->schedule.final_score`。

#### ③ test-stress 评分

**思路**: 压力测试本质是度量"计算吞吐量"，单独看 bogo_ops 绝对值无意义（依赖硬件），需要基准对比。

**方案 A — 基准比值法** `[推荐]`:
```
stress_score = min(100, 100 × (bogo_ops_per_sec / baseline_ops_per_sec))
```
- `baseline_ops_per_sec`: 预设的参考值 `[待定]`，可根据 QEMU 或目标硬件的典型值设定
- 多 stressor 场景: 各 stressor 分数取加权平均或最小值

**方案 B — 归一化法**（无需外部基准）:
```
stress_score = 100   // 压力测试只要完成就给满分（通过/失败二值）
```
- 适用于"压力测试重在是否崩溃"的评估逻辑

**实现要点**:
- 在 `score_calc_stress()` 中计算
- 写入 `r->score`
- 调用时机: `collect_stress_result()` 末尾

#### ④ typical-workload 评分

**思路**: 衡量 RTOS 执行各类工业负载的效率。

**方案 — 基于 WCET 基准**:
```
单 workload 分数:
  wl_score = min(100, 100 × (baseline_ms / avg_time_ms))

模块综合分数:
  workload_score = 按 category 分组后均值，再跨 category 均值
```

**基准值来源** `[待定]`:
- 可从首次测试结果中自动生成 baseline 配置文件
- 或在 `score_calc.c` 中硬编码目标平台参考值

**实现要点**:
- 在 `score_calc_workload()` 中遍历 `r->workloads[]`
- 每个 workload 写入 `workloads[i].score`
- 模块总分写入 `r->score`
- 调用时机: `collect_workload_results()` 末尾

### 3.5 综合总分

```c
void score_calc_total(struct rtbench_result *r) {
    double scores[4];
    double weights[4];
    int n = 0;

    // 只对有效模块参与计算
    if (r->realtime.valid)  { scores[n] = r->realtime.score;         weights[n] = W_REALTIME;  n++; }
    if (r->schedule.valid)  { scores[n] = r->schedule.final_score;   weights[n] = W_SCHEDULE;  n++; }
    if (r->stress.valid)    { scores[n] = r->stress.score;           weights[n] = W_STRESS;    n++; }
    if (r->workload.valid)  { scores[n] = r->workload.score;         weights[n] = W_WORKLOAD;  n++; }

    // 加权平均（权重归一化到已运行模块的总和）
    double sum_w = 0, sum_ws = 0;
    for (int i = 0; i < n; i++) { sum_w += weights[i]; sum_ws += weights[i] * scores[i]; }
    r->total_score = (sum_w > 0) ? sum_ws / sum_w : 0;
}
```

**参考权重** `[待定]`:

| 模块 | 权重 | 理由 |
|------|------|------|
| test-realtime | 0.30 | 实时性是 RTOS 核心指标 |
| test-schedule | 0.30 | 可调度性是工业基准核心 |
| test-stress | 0.15 | 吞吐/功耗辅助指标 |
| typical-workload | 0.25 | 工业负载实际表现 |

### 3.6 JSON 输出变更

在已有 JSON 结构中追加分数字段，不改动已有字段：

```json
{
  "meta": { ... },
  "env": { ... },
  "modules": {
    "test-realtime": {
      "status": "passed",
      "score": 85.50,                          // ← 新增
      "single_core": { ... },
      "multi_core": { ... }
    },
    "test-schedule": {
      ...
      "summary": { "average_miss_rate": 0.12, "final_score": 87.65 }
      // score 即 final_score, 无需新增
    },
    "test-stress": {
      "status": "passed",
      "score": 92.30,                          // ← 新增
      "stressors": [ ... ]
    },
    "typical-workload": {
      "status": "passed",
      "score": 78.90,                          // ← 新增
      "workloads": [
        { "name": "ekf", ..., "score": 82.5 }, // ← 新增
        ...
      ]
    }
  },
  "total_score": 85.20                         // ← 新增（顶层）
}
```

### 3.7 代码变更清单

| 文件 | 变更 | 侵入性 |
|------|------|--------|
| `generator/result_export.h` | 结构体增加 `score` / `total_score` 字段 | 低 — 仅追加字段 |
| `generator/result_export.c` | JSON 序列化增加 `score` / `total_score` 输出 | 低 — 追加 JSON_APPEND |
| `generator/score_calc.h` | **新增** — 分数计算函数声明 | 无侵入（新文件） |
| `generator/score_calc.c` | **新增** — 分数计算实现 | 无侵入（新文件） |
| `generator/rtthread_entry.c` | 在 `collect_*_result()` 后调用 `score_calc_*()`；在 `result_end()` 前调用 `score_calc_total()` | 低 — 每处加一行调用 |
| 构建系统 (SConscript 等) | 增加 `score_calc.c` 编译 | 低 |

### 3.8 集成步骤

```
1. 在 result_export.h 中追加 score 字段
2. 创建 score_calc.h / score_calc.c
3. 在 result_export.c 的 JSON 序列化中追加 score 输出
4. 在 rtthread_entry.c 的 test_all_thread_entry() 中:
   - collect_realtime_result() 后加 score_calc_realtime(&r->realtime)
   - collect_schedule_result() 后无需操作（已有 final_score）
   - collect_stress_result()  后加 score_calc_stress(&r->stress)
   - collect_workload_results() 后加 score_calc_workload(&r->workload)
   - rtbench_result_end() 前加 score_calc_total(r)
5. 更新构建脚本
6. 编译验证
```

---

## 四、总结对照表

| 维度 | test-realtime | test-schedule | test-stress | typical-workload |
|------|---------------|---------------|-------------|------------------|
| **中间数据** | 静态数组 (bench_init.c) | 静态 g_result (test_schedule.c) | g_last_bogo (stress-ng.c) | rt_tick_get() 现场测量 |
| **collect 函数** | `collect_realtime_result()` | `collect_schedule_result()` | `collect_stress_result()` | `collect_workload_results()` |
| **导出结构** | `rtbench_realtime_result` | `rtbench_schedule_result` | `rtbench_stress_result` | `rtbench_workload_module_result` |
| **JSON 键** | `modules.test-realtime` | `modules.test-schedule` | `modules.test-stress` | `modules.typical-workload` |
| **已有分数** | 无 | `final_score` (100分制) | 无 | 无 |
| **分数公式** | 阈值加权 `[待定]` | 100×(1-avg_MR) | 基准比值 `[待定]` | WCET 基准比值 `[待定]` |
| **新增结构字段** | `double score` | 无需 | `double score` | `double score` (模块+单项) |
| **侵入文件** | result_export.h/c + rtthread_entry.c | 无 | 同 realtime | 同 realtime |

标记 `[待定]` 的参数（阈值、权重、基准值）需团队确认后填入 `score_calc.c`。
