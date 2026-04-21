# 可调度性测试 (test-schedule) 使用指南

本文档说明 RTOS-Bench 中的可调度性验证测试功能 (`test-schedule`)，该功能基于工业操作系统通用基准检测指标体系指导书实现。

## 概述

可调度性验证基于工业场景任务模型，复用典型负载性能评估中的负载程序，将负载程序封装为周期任务。采用 UUniFast 算法生成利用率 30%-100% 的梯度任务集（步长 10%），每个任务集包含随机周期与最坏执行时间配置。

默认会纳入所有工业负载（仅排除 `stub`、`busywait` 等 utility 负载）。

为保证端到端可收敛出分数，`test-schedule` 采用以下鲁棒性策略：
- 每个利用率梯度最多尝试 3 次（中途失败会自动重试）。
- 单次尝试存在超时保护（常规默认 45s，quick 模式 20s）。
- 若该梯度多次尝试仍失败，不中断全流程；该梯度按 fallback 记分（`MR=1.0`），并在结果中标记 `fallback_failed`。
- 最终仍会输出总分；如存在 fallback，模块状态为 `passed_with_degradation`。

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
  --cycles <n>      每个任务集执行的周期数 (默认: 100)
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

# 完整测试（更高统计置信度）
msh /> rtbench test-schedule --cycles 1000
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
Running task set for 100 cycles (timeout: 45s, max attempts: 3)...
Gradient 30% complete: MR = 0.0000 (0/500)

... (重复每个梯度) ...

=============================================================
[Phase 3] Final Results
=============================================================
U= 30%: MR=0.0000 (0 misses / 500 jobs)
U= 40%: MR=0.0012 (1 misses / 500 jobs)
U= 50%: MR=0.0040 (2 misses / 500 jobs)
Gradient 70% failed after 3 attempts (timeout), fallback MR=1.0000
...

------------------------------------------------------
Average Miss Rate: 0.0420
Final Score: 95.80 / 100
Failed Gradients: 1 / 8
Retries Used: 2
Status: PASSED_WITH_DEGRADATION
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

最坏执行时间 (WCET) 通过“目标迭代数 + 时间预算”自适应测量：

```
WCET = max(execution_time[i]) for i in sampled iterations
```

默认目标迭代数为 50 次，但每个 workload 存在阶段预算（常规 20s，quick 8s），达到最小样本后若预算耗尽会提前收敛；若测量异常（如返回 0），会使用保守 WCET 兜底，保证后续调度阶段可继续执行。

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

---

## 附录 A: RT-Thread 构建系统名词解释

本节解释 RTOS-Bench 在 RT-Thread 平台上的构建相关术语和配置文件。

### A.1 构建系统概述

```
┌─────────────────────────────────────────────────────────────────┐
│                    RT-Thread 构建流程                            │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   .config ──────> rtconfig.h ──────> 源代码编译                  │
│   (Kconfig)       (C宏定义)          (GCC/G++)                   │
│       │               │                  │                      │
│       └───────────────┴──────────────────┘                      │
│                       │                                         │
│                   SCons 构建系统                                 │
│                       │                                         │
│   SConstruct ────> SConscript(s) ────> rtthread.elf             │
│   (顶层入口)       (各模块构建)        (最终固件)                 │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### A.2 配置文件说明

#### `.config` 文件

**位置**: `extern/rt-thread/bsp/<board>/.config`

**作用**: Kconfig 配置数据库，存储所有编译选项的当前值。

**格式示例**:
```bash
CONFIG_RT_USING_PTHREADS=y          # 启用 POSIX threads
CONFIG_PTHREAD_NUM_MAX=8            # 最大 pthread 数量
CONFIG_RT_USING_CPLUSPLUS=y         # 启用 C++ 支持
# CONFIG_RT_USING_LWIP is not set   # 禁用 lwIP 网络栈
CONFIG_RT_LWIP_TCP_SND_BUF=8196     # TCP 发送缓冲区大小
```

**修改方式**:
1. `menuconfig` 图形界面（推荐）
2. 直接编辑文件（高级用户）

**注意**: 修改 `.config` 后需要重新生成 `rtconfig.h`

#### `rtconfig.h` 文件

**位置**: `extern/rt-thread/bsp/<board>/rtconfig.h`

**作用**: 由 `.config` 生成的 C 头文件，包含所有编译宏定义。

**格式示例**:
```c
#define RT_USING_PTHREADS
#define PTHREAD_NUM_MAX 8
#define RT_USING_CPLUSPLUS
/* RT_USING_LWIP is not defined */
#define RT_LWIP_TCP_SND_BUF 8196
```

**重要**: 这是 C/C++ 源代码实际读取的配置文件。`.config` 和 `rtconfig.h` 必须保持同步。

#### `SConstruct` 文件

**位置**: `extern/rt-thread/bsp/<board>/SConstruct`

**作用**: SCons 构建系统的顶层入口脚本，相当于 Makefile 的顶层文件。

**主要功能**:
```python
# 设置工具链路径
rtconfig.EXEC_PATH = '/path/to/toolchain/bin'

# 配置编译器选项
env = Environment(tools=['gcc', 'g++', 'ar'])

# 准备构建环境，扫描所有 SConscript
objs = PrepareBuilding(env, RTT_ROOT)

# 执行构建
DoBuilding(TARGET, objs)
```

#### `SConscript` 文件

**位置**: 各模块目录下（如 `generator/SConscript`, `workloads/SConscript`）

**作用**: 描述单个模块的源文件、依赖和编译选项。

**格式示例**:
```python
from building import *

cwd = GetCurrentDir()
src = Glob('*.c')                    # 收集所有 .c 文件
CPPPATH = [cwd]                      # 头文件搜索路径

# 定义构建组
# depend 参数指定依赖的 Kconfig 选项
group = DefineGroup('module_name', src,
                    depend=['RT_USING_PTHREADS'],  # 依赖条件
                    CPPPATH=CPPPATH)

Return('group')
```

**关键函数**:
- `Glob('*.c')`: 匹配文件
- `GetDepend(['CONFIG'])`: 检查配置是否启用
- `DefineGroup()`: 定义编译目标组
- `depend=['']`: 空依赖表示始终编译；`depend=['CONFIG']` 表示仅当 CONFIG 启用时编译

### A.3 工具链 (Toolchain)

**定义**: 工具链是交叉编译所需的一组工具，用于在一个平台（如 x86 Linux）上编译另一个平台（如 ARM64）的代码。

**RTOS-Bench 使用的工具链**:
```
extern/toolchains/xpack-aarch64-none-elf-gcc-14.2.1-1.1/
├── bin/
│   ├── aarch64-none-elf-gcc      # C 编译器
│   ├── aarch64-none-elf-g++      # C++ 编译器
│   ├── aarch64-none-elf-ld       # 链接器
│   ├── aarch64-none-elf-ar       # 静态库工具
│   ├── aarch64-none-elf-objcopy  # 二进制转换
│   └── aarch64-none-elf-nm       # 符号表查看
├── lib/
│   └── gcc/aarch64-none-elf/     # 编译器运行时库
└── aarch64-none-elf/
    ├── include/                   # 标准头文件 (newlib)
    └── lib/                       # C 标准库 (newlib)
```

**工具链命名规则**: `<arch>-<vendor>-<os>-<abi>`
- `aarch64`: 目标架构 (ARM 64-bit)
- `none`: 无特定厂商
- `elf`: 输出格式 (ELF 可执行文件)

**配置方式** (在 `rtconfig.py` 或 `SConstruct` 中):
```python
EXEC_PATH = '/path/to/toolchain/bin'
PREFIX = 'aarch64-none-elf-'
CC = PREFIX + 'gcc'
CXX = PREFIX + 'g++'
```

### A.4 SCons vs Make

| 特性 | SCons | Make |
|------|-------|------|
| 语言 | Python | Makefile DSL |
| 依赖检测 | 自动 MD5 校验 | 时间戳 |
| 跨平台 | 原生支持 | 需要额外工具 |
| 学习曲线 | 需要 Python | 独特语法 |
| RT-Thread | 官方支持 | 部分 BSP 支持 |

---

## 附录 B: 实现过程记录

### B.1 开发时间线

#### 2026-02-12: feat/test-schedule 分支创建

**基础提交**: `f82cb31 feat: Add test-schedule command for schedulability verification`

初始实现包含:
- `generator/uunifast.h/c`: UUniFast 算法
- `generator/test_schedule.h/c`: test-schedule 命令框架
- 修改 `generator/rtthread_entry.c`: 添加命令入口

### B.2 遇到的问题与解决方案

#### 问题 1: RT-Thread 5.x SMP 调度器 API 变更

**现象**: 编译错误 `'rt_thread' has no member named 'current_priority'`

**原因**: RT-Thread 5.x SMP 版本将调度器数据抽象到 `rtsched.h`，不再直接暴露 `current_priority` 成员。

**解决方案**:
```c
// 旧代码 (RT-Thread 4.x)
self->current_priority

// 新代码 (RT-Thread 5.x SMP)
#include <rtsched.h>
RT_SCHED_PRIV(self).current_priority
```

**提交**: `ae386eb fix: Update RT-Thread scheduler API for SMP builds`

#### 问题 2: rtbench_workloads.cpp 未被编译

**现象**: 链接错误 `undefined reference to 'rtosbench_register_rtos_workloads'`

**原因**: `workloads/SConscript` 只扫描子目录，没有编译顶层的 `rtbench_workloads.cpp`。

**解决方案**: 修改 `workloads/SConscript`:
```python
# 添加顶层 cpp 文件编译
src = Glob('rtbench_workloads.cpp')
group = DefineGroup('rtbench_workloads', src, depend=[''], CPPPATH=CPPPATH)
objs.append(group)

# 然后再扫描子目录
for item in os.listdir(cwd):
    # ...
```

**提交**: `b070e1a fix: Include rtbench_workloads.cpp in workloads build`

#### 问题 3: 网络 workloads (MODBUS/MQTT) 编译依赖

**现象**: `modbus_bench_run` 和 `mqtt_bench_run` 符号未定义

**原因**:
1. `workloads/MODBUS/SConscript` 设置了 `depend=['SAL_USING_POSIX']`
2. RT-Thread 配置未启用 SAL 网络栈

**初始错误尝试**: 在 `rtbench_workloads.cpp` 中添加 `#ifdef SAL_USING_POSIX` 条件编译

**正确解决方案**:
1. 这些 workloads 有内置的 offline fallback 机制（当 `socket()` 失败时执行本地仿真）
2. 应该移除构建时依赖，让 workload 始终编译
3. 启用 RT-Thread SAL/lwIP 支持使网络功能可用

**相关提交**:
- `d6c3545 fix: Guard network workloads with SAL_USING_POSIX` (初始错误方案)
- `8acd50a fix: Remove SAL_USING_POSIX dependency from network workloads` (正确方案)
- `e15974b fix: Always include MODBUS/MQTT workloads in registration`

#### 问题 4: clock_gettime 未定义

**现象**: 编译错误 `implicit declaration of function 'clock_gettime'`

**原因**: RT-Thread `.config` 中 `RT_USING_CLOCK_TIME` 未启用

**解决方案**: 在 `.config` 中启用:
```bash
CONFIG_RT_USING_CLOCK_TIME=y
CONFIG_RT_USING_POSIX_DELAY=y
CONFIG_RT_USING_POSIX_CLOCK=y
```

并在 `rtconfig.h` 中添加对应宏定义。

#### 问题 5: sys/socket.h 找不到

**现象**: `fatal error: sys/socket.h: No such file or directory`

**原因**: 虽然 `.config` 启用了 SAL，但 `rtconfig.h` 没有对应更新

**解决方案**: 手动在 `rtconfig.h` 中添加网络相关宏:
```c
/* Network */
#define RT_USING_SAL
#define SAL_USING_POSIX
#define SAL_USING_LWIP
#define SAL_SOCKETS_NUM 16
#define SOCKET_TABLE_STEP_LEN 4
#define RT_USING_POSIX_SOCKET
// ... lwIP 配置 ...
```

**教训**: `.config` 和 `rtconfig.h` 必须保持同步。理想情况下应该使用 `scons --menuconfig` 自动生成。

#### 问题 6: SOCKET_TABLE_STEP_LEN 未定义

**现象**: 编译 sal_socket.c 时报错 `'SOCKET_TABLE_STEP_LEN' undeclared`

**原因**: SAL 模块需要此配置但 rtconfig.h 中遗漏

**解决方案**: 添加到 rtconfig.h:
```c
#define SAL_SOCKETS_NUM 16
#define SOCKET_TABLE_STEP_LEN 4
```

### B.3 最终提交历史

```
e15974b fix: Always include MODBUS/MQTT workloads in registration
8acd50a fix: Remove SAL_USING_POSIX dependency from network workloads
ae386eb fix: Update RT-Thread scheduler API for SMP builds
f009636 refactor: Improve generator SConscript platform detection
d6c3545 fix: Guard network workloads with SAL_USING_POSIX
b070e1a fix: Include rtbench_workloads.cpp in workloads build
f82cb31 feat: Add test-schedule command for schedulability verification
```

### B.4 编译验证结果

最终构建包含所有 9 个 workloads:

```
$ nm rtthread.elf | grep bench_run
00000000400d1ee8 T cusum_bench_run
00000000401a677c T ekf_bench_run
00000000401b9480 T epnp_bench_run
00000000400d21f8 T ewma_bench_run
00000000400e8e68 T fast_bench_run_once
00000000401c3248 T icp_bench_run
00000000400ea48c T modbus_bench_run
000000004010e044 T mqtt_bench_run
00000000401a878c T pid_bench_run
```

固件大小: 4.1MB (text) / 59MB (with debug symbols)

### B.5 关键经验总结

1. **配置同步**: `.config` 修改后必须同步更新 `rtconfig.h`，否则 C 代码看不到新配置
2. **SConscript depend**: `depend=['']` 表示始终编译，`depend=['CONFIG']` 仅在 CONFIG 启用时编译
3. **工具链版本**: 使用与 RT-Thread BSP 匹配的工具链版本避免 ABI 兼容问题
4. **API 变更**: RT-Thread 5.x SMP 版本有较大 API 变化，需要注意 rtsched.h 抽象层
5. **Workload fallback**: 网络 workloads 应该有 offline fallback，避免硬依赖网络栈
