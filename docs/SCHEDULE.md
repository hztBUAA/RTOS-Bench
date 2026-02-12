# 可调度性测试 (test-schedule) 使用指南

本文档说明 RTOS-Bench 中的可调度性验证测试功能 (`test-schedule`)，该功能基于工业操作系统通用基准检测指标体系指导书实现。

## 概述

可调度性验证基于工业场景任务模型，复用典型负载性能评估中的负载程序，将负载程序封装为周期任务。采用 UUniFast 算法生成利用率 30%-100% 的梯度任务集（步长 10%），每个任务集包含随机周期与最坏执行时间配置。

**核心指标**：
- 截止时间错失率 (Miss Rate, MR) = 错失任务数 / 总任务数
- 调度性能评分 = 100 × (1 - 平均MR)

## 命令用法

### 基本用法

```bash
# RT-Thread msh 中
msh /> rtbench test-schedule

# Linux 终端中
./rtbench test-schedule
```

### 可选参数

```bash
rtbench test-schedule [OPTIONS]

OPTIONS:
  --cycles <n>      每个任务集执行的周期数 (默认: 10000)
  --util-start <n>  起始利用率百分比 (默认: 30)
  --util-end <n>    结束利用率百分比 (默认: 100)
  --util-step <n>   利用率步长 (默认: 10)
  -q                安静模式，减少输出
```

### 示例

```bash
# 快速测试（减少周期数）
msh /> rtbench test-schedule --cycles 100

# 自定义利用率范围
msh /> rtbench test-schedule --util-start 50 --util-end 90 --util-step 5

# 完整测试
msh /> rtbench test-schedule --cycles 10000
```

## 输出格式

```
=============================================================
[Phase 1] Measuring WCET for 5 workloads...
=============================================================
  [fast]: WCET = 12.345 ms
  [pid]: WCET = 2.100 ms
  [ekf]: WCET = 150.000 ms
  [modbus]: WCET = 45.678 ms
  [cusum]: WCET = 5.432 ms

=============================================================
[Phase 2] Running Task Sets (U: 30% - 100%, step 10%)
=============================================================

>>> Utilization Gradient: 30% <<<
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
ekf          | 150.000    | 15.00    | 1000.0000
modbus       | 45.678     | 8.00     | 571.0000
fast         | 12.345     | 4.00     | 308.6250
...
Running task set for 10000 cycles...
Gradient 30% complete: MR = 0.0000 (0/50000)

... (重复每个梯度) ...

=============================================================
[Phase 3] Final Results
=============================================================
U= 30%: MR=0.0000 (0 misses / 50000 jobs)
U= 40%: MR=0.0012 (60 misses / 50000 jobs)
U= 50%: MR=0.0045 (225 misses / 50000 jobs)
U= 60%: MR=0.0123 (615 misses / 50000 jobs)
U= 70%: MR=0.0256 (1280 misses / 50000 jobs)
U= 80%: MR=0.0512 (2560 misses / 50000 jobs)
U= 90%: MR=0.0890 (4450 misses / 50000 jobs)
U=100%: MR=0.1523 (7615 misses / 50000 jobs)

------------------------------------------------------
Average Miss Rate: 0.0420
Final Score: 95.80 / 100
------------------------------------------------------
```

## 算法说明

### UUniFast 算法

UUniFast 算法用于生成均匀分布的任务利用率，确保生成的任务集具有无偏性。

**参考文献**: Bini & Buttazzo, "Measuring the Performance of Schedulability Tests", Real-Time Systems, 2005

**核心公式**:
- 对于 n 个任务和目标总利用率 U_total
- 每个任务的利用率 U_i 满足: Σ U_i = U_total
- 分布在 n 维单纯形上均匀

### WCET 测量

最坏执行时间 (WCET) 通过多次运行负载取最大值获得：

```
WCET = max(execution_time[i]) for i in 1..N iterations
```

默认运行 50 次迭代。

### 周期计算

根据隐式截止时间模型 (Implicit Deadline)：

```
Period = WCET / Utilization
Deadline = Period
```

### 截止时间判定

任务在以下情况视为截止时间错失：

```
Response_Time > Deadline
```

其中 Response_Time = Completion_Time - Activation_Time

## 架构设计

### 多任务并发执行

与单负载周期测试不同，`test-schedule` 同时运行多个负载作为独立的周期任务：

```
┌─────────────────────────────────────────────────────────┐
│                    Test Schedule                        │
├─────────────────────────────────────────────────────────┤
│  Phase 1: WCET Measurement                              │
│    - Run each workload 50 times                         │
│    - Record maximum execution time                      │
├─────────────────────────────────────────────────────────┤
│  Phase 2: Task Set Execution (per gradient)             │
│    ┌──────────┐  ┌──────────┐  ┌──────────┐            │
│    │ Task 1   │  │ Task 2   │  │ Task N   │            │
│    │ (fast)   │  │ (pid)    │  │ (ekf)    │  ...       │
│    │ T=100ms  │  │ T=50ms   │  │ T=500ms  │            │
│    └────┬─────┘  └────┬─────┘  └────┬─────┘            │
│         │             │             │                   │
│    [Timer] ─────> [Semaphore] ─────> [Execute]          │
│         │             │             │                   │
│    Check deadline, record miss if exceeded              │
├─────────────────────────────────────────────────────────┤
│  Phase 3: Statistics Aggregation                        │
│    - Calculate MR per gradient                          │
│    - Calculate average MR                               │
│    - Compute final score                                │
└─────────────────────────────────────────────────────────┘
```

### 数据结构

```c
/* 任务配置 */
struct schedule_task_config {
    const char *name;       /* 负载名称 */
    uint64_t wcet_ns;       /* WCET (纳秒) */
    double utilization;     /* 分配的利用率 */
    uint64_t period_ns;     /* 周期 = WCET / U */
    uint64_t deadline_ns;   /* 截止时间 = 周期 */
};

/* 任务统计 */
struct schedule_task_stats {
    uint64_t total_jobs;        /* 总任务激活数 */
    uint64_t deadline_misses;   /* 截止时间错失数 */
};

/* 最终结果 */
struct test_schedule_result {
    double average_miss_rate;   /* 平均 MR */
    double final_score;         /* 100 * (1 - 平均MR) */
};
```

## 关键源文件

| 文件 | 说明 |
|------|------|
| `generator/test_schedule.h` | 接口定义和数据结构 |
| `generator/test_schedule.c` | 主要实现 |
| `generator/uunifast.h` | UUniFast 算法接口 |
| `generator/uunifast.c` | UUniFast 算法实现 |
| `generator/rtthread_entry.c` | RT-Thread 命令入口 |

## 平台支持

| 平台 | 状态 | 说明 |
|------|------|------|
| RT-Thread | ✅ 支持 | 使用 RT-Thread 线程和软定时器 |
| Linux | ✅ 支持 | 使用 pthread 和 POSIX timer |
| SylixOS | 🔄 待验证 | 应该可用（POSIX 兼容） |
| OneOS | 🔄 待验证 | 需要测试线程支持 |

## 开发日志

### 2026-02-12: 初始实现

**实现内容**:
1. 从 `period_extracted/period.cpp` 移植 UUniFast 算法
2. 实现多线程任务执行框架
3. 集成到 `rtbench test-schedule` 命令
4. 支持自定义参数（周期数、利用率范围）

**设计决策**:
- 使用隐式截止时间模型 (D = T)
- 每个任务使用独立的定时器和信号量
- WCET 测量使用 50 次迭代取最大值
- 支持 RT-Thread 和 POSIX 两种线程模型

**待完成**:
- 完整的 RT-Thread QEMU 测试
- 性能优化（高利用率时的定时器精度）
- 结果输出到文件

### 已知限制

1. **定时器精度**: RT-Thread tick 级别（通常 1ms），高精度测试可能不准确
2. **线程数量**: 受限于系统最大线程数
3. **内存占用**: 每个任务需要独立的栈空间
4. **WCET 变化**: 实际 WCET 可能因缓存、中断等因素变化

## 故障排查

### 问题：所有任务都错失截止时间

**可能原因**:
1. 利用率设置过高（>100%）
2. WCET 测量不准确
3. 系统负载过重

**解决方案**:
- 减少并发任务数
- 增加 WCET 测量迭代次数
- 检查系统是否有其他高优先级任务

### 问题：WCET 为 0

**可能原因**:
1. 负载函数执行时间太短
2. 时间戳精度不足

**解决方案**:
- 检查负载是否正确注册
- 使用更高精度的时间源

### 问题：线程创建失败

**可能原因**:
1. 栈空间不足
2. 线程数量超限

**解决方案**:
- 增加系统线程栈配置
- 减少并发任务数

## 参考资料

1. 工业操作系统通用基准检测指标体系指导书 v1.5
2. Bini & Buttazzo, "Measuring the Performance of Schedulability Tests"
3. Liu & Layland, "Scheduling Algorithms for Multiprogramming in a Hard Real-Time Environment"
