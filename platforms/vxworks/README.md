# VxWorks 移植参考实现

本目录是 RTOS-Bench 面向 VxWorks 的**移植脚手架**，供移植方「填空」后接入自己的
VxWorks 工程。仓库**不包含**可直接用于生产 VxWorks 的完整适配——平台层的 5 个文件
提供了基于通用 VxWorks API（taskLib / semLib / tickLib）的参考实现，需要按目标 SDK
版本与内核配置（SMP/单核、内核态模块/DKM、是否启用 POSIX 层）做校准。

> **本分支已剪裁 EKF 与 EPnP 两个负载**，随之移除了 vendored 的 Eigen / OpenGV 依赖。
> 因此本脚手架**不含**任何 `-DEIGEN_*` 或 `-include fix_opengv.h`，
> 剩余负载为 7 个：`fast`、`icp`、`modbus`、`mqtt`、`pid`、`cusum`、`ewma`。

---

## 1. 目录结构

```
platforms/vxworks/
├── README.md      本文件
├── Makefile       独立构建模板（仓库不 include，供复制到你的工程）
├── timer.c        定时器：rtbench_timer_create / settime / delete
├── sync.c         信号量：rtbench_sem_create / wait / post / destroy
├── scheduler.c    调度属性：rtbench_set_priority / deadline / affinity
├── timestamp.c    时间戳：rtbench_get_rdtsc / rtbench_get_timestamp
└── signal.c       事件注册：rtbench_signal_register（+ 内部 trigger）
```

这 5 个 `.c` 当前都是**空编译单元**：整份文件被 `#if defined(RTBENCH_PLATFORM_VXWORKS)`
包住，而该宏在 `generator/platform_abstraction.h` 中**故意没有定义**。
这样做的目的：即使这些文件被误 glob 进其他平台的构建，也不会产生重复符号，
`git grep RTBENCH_PLATFORM_VXWORKS` 可以验证「未接线」这一事实。

---

## 2. 需要实现的契约

契约定义在 `generator/platform_abstraction.h`，共 12 个函数、5 组：

```c
/* 定时器 —— 支持单次(DEADLINE)与周期(PERIOD)两种语义 */
rtbench_timer_t rtbench_timer_create(rtbench_timer_type_t type,
                                     rtbench_timer_callback_t cb, void *user_data);
int rtbench_timer_settime(rtbench_timer_t t, long sec, long nsec);
int rtbench_timer_delete(rtbench_timer_t t);

/* 信号量 —— 只有信号量，无互斥锁 */
rtbench_sem_t rtbench_sem_create(unsigned int initial_value);
int rtbench_sem_wait(rtbench_sem_t s);
int rtbench_sem_post(rtbench_sem_t s);
int rtbench_sem_destroy(rtbench_sem_t s);

/* 调度属性 */
int rtbench_set_priority(unsigned int priority);
int rtbench_set_deadline(uint64_t runtime, uint64_t deadline, uint64_t period);
int rtbench_set_affinity(uint32_t cpu_mask);

/* 时间戳 */
unsigned long long rtbench_get_rdtsc(void);
long double        rtbench_get_timestamp(void);

/* 事件 */
int rtbench_signal_register(rtbench_signal_t sig, rtbench_signal_handler_t handler);
```

**契约的两个逃生口**：`rtbench_set_deadline()` 与 `rtbench_signal_register()`
都允许返回 `-1` 表示「本平台不支持」。统计层仍会用 deadline 判定 miss，
因此 VxWorks 上只实现优先级调度即可出分，不必强求 deadline 抢占。

---

## 3. 激活步骤

平台层文件在你完成下面这一步之前不会生效。请在你的 VxWorks 工程里，
往 `generator/platform_abstraction.h` 的平台探测链中插入 VxWorks 分支：

```c
#elif defined(VXWORKS_PLATFORM)
    /* VxWorks：POSIX 层可选；类型补充见下 */
    #define RTBENCH_PLATFORM_VXWORKS
    #ifndef _CLOCK_T_DECLARED
    typedef unsigned long clock_t;
    #define _CLOCK_T_DECLARED
    #endif
    #ifndef _CLOCKID_T_DECLARED
    typedef unsigned long clockid_t;
    #define _CLOCKID_T_DECLARED
    #endif
    #ifndef _TIMER_T_DECLARED
    typedef unsigned long timer_t;
    #define _TIMER_T_DECLARED
    #endif
    #ifndef _PID_T_DECLARED
    typedef int pid_t;
    #define _PID_T_DECLARED
    #endif
```

插入位置在 `#elif defined(RT_THREAD_PLATFORM)` 之前或之后均可，
但要保证它排在最后的 `#else #error "Unsupported platform..."` 之前
（否则会因为未识别平台直接编译失败）。

同时把构建宏 `-DVXWORKS_PLATFORM` 加进编译选项（`Makefile` 里的 `DEFS` 已包含）。

> 本仓库**故意不替你做这一步**，以保证其他平台（Linux/SylixOS/OneOS/Dongtu/Ruihua/RT-Thread）
> 的编译行为完全不变。这也是 `RTBENCH_PLATFORM_VXWORKS` 在仓库内应始终搜不到的原因。

---

## 4. 契约符号 ↔ VxWorks API 映射

| 契约 | 参考实现 | 备注 |
|---|---|---|
| `rtbench_timer_create/settime/delete` | `taskSpawn` + `taskDelay` + `taskDelete` | 见 §5.1；精度受 `sysClkRateGet()` 限制 |
| `rtbench_sem_*` | `semBCreate` / `semTake` / `semGive` / `semDelete` | `SEM_Q_FIFO`；防优先级反转可改 `SEM_Q_PRIORITY` |
| `rtbench_set_priority` | `taskPrioritySet(taskIdSelf(), prio)` | **与 POSIX 相反：数值越小优先级越高** |
| `rtbench_set_deadline` | 返回 `-1` | VxWorks 无原生 deadline 调度 |
| `rtbench_set_affinity` | `taskCpuAffinitySet`（`#ifdef VX_SMP`） | `cpuset_t` 构造方式随版本而异，需按 SDK 校准 |
| `rtbench_get_timestamp` | `tickGet64()/sysClkRateGet()`，或 `clock_gettime(CLOCK_MONOTONIC)` | 后者需定义 `RTBENCH_VXWORKS_USE_POSIX_CLOCK` |
| `rtbench_get_rdtsc` | 同 `rtbench_get_timestamp`，以纳秒表示 | 无硬件 TSC 时的近似 |
| `rtbench_signal_register` | 处理器表 + 内部 `rtbench_signal_trigger` | 需要 POSIX 信号时自行接 `sigaction` |

### 5.1 为什么定时器用 taskSpawn 而不是 watchdog

`wdCreate/wdStart` 的回调运行在**中断上下文**，而 rt-bench 的定时器回调会释放周期
信号量（可能唤醒任务）。在中断上下文里做阻塞型内核调用在多数 VxWorks 配置下是非法的，
会触发 `intLib`/`kernel` 层面的报错。因此参考实现选了 `taskSpawn` + `taskDelay` 轮询：
精度较低（tick 级），但回调上下文稳定。若你的 BSP 明确允许 ISR 上下文回调，
可以换成 watchdog 版本以提升精度。

---

## 5. 构建模板用法

`Makefile` 是一份独立模板，**不会被仓库任何构建引用**。复制到你的 VxWorks 工程后，
按 SDK 修改顶部的：

```make
VXWORKS_ROOT      ?= /opt/windriver
TOOLCHAIN_PREFIX  ?= wr-          # VxWorks 6.9 常见 arm-none-eabi-
CPU               ?= V7ARM
TOOL              ?= diab
```

然后 `make` 会产出 `build/` 下的目标文件。三个必须留意的点：

1. **test-schedule 的 4 个包装器源文件不能漏**（`SCHED_SRCS`）。这是仓库所有构建清单
   的共同要求，漏掉会报 `sched_*` 符号未定义。
2. C++ 负载（icp/pid）需要 libstdc++（链接阶段加 `-lstdc++`），并按需开启 `VX_FP_TASK`
   任务选项与浮点寄存器支持。
3. 网络类负载（modbus/mqtt）需要 socket 支持；没有网络时它们会自动走离线降级路径
   （modbus 仿真、mqtt pack-only），不影响其余测试。

---

## 6. 入口选择

- **首次打通**：用 `generator/posixlite_entry.c`。它不依赖 argp/性能计数器，
  提供 `-b/-p/-t/-q/-L` 等基本选项，最容易在 VxWorks 上先跑起来。
- **需要 shell 子命令**（`rtbench test-realtime` / `test-schedule` / `test-stress` /
  `test-cmd`）：改用 `generator/rtbench_command.c` + 仿照 `generator/oneos_entry.c`
  或 `generator/rtthread_entry.c` 写一个 `vxworks_entry.c`，实现
  `rtbench_platform_default_output_path()` / `rtbench_platform_get_env()` /
  `rtbench_platform_run_test_all()` 三个钩子。注意此时还需把
  `test_realtime.c`、`test_stress.c`、`test_cmd.c` 以及 `stress_orig/`、
  `realtime_orig/` 的源文件一并加入，具体清单参考 `generator/Makefile` 的 `RUIHUA_*` 分支。

`generator/test_schedule.c` 内部直接使用 `pthread_create/pthread_join`。
VxWorks 若启用 POSIX pthread 层可直接复用；若为内核态任务模型，
需要把 `create_task_thread()` / `pthread_entry_wrapper()` 两处换成 `taskSpawn/taskDelete`。

---

## 7. 验证清单

跑通后按顺序确认：

1. `rtbench -L` 应恰好列出 **9 项**：`stub, busywait, fast, icp, modbus, mqtt, pid, cusum, ewma`
   —— **不应出现 `ekf`、`epnp`**（本分支已剪裁）。
2. `rtbench -b fast -p 1 -t 1 -q` 能跑完一轮并打印 WCET 统计。
3. `rtbench test-schedule --quick` 的 Phase 1 应报 **7 个 workload** 的 WCET。
4. 检查结果 JSON 里 `task_stats` 的 workload 名单与上面一致。

---

## 8. 常见问题

| 现象 | 原因 / 处理 |
|---|---|
| `undefined reference to rtbench_timer_create` 等 | `RTBENCH_PLATFORM_VXWORKS` 未激活 —— 见 §3，`platform_abstraction.h` 还没加 VxWorks 分支 |
| `#error "Unsupported platform..."` | 同上；或 `-DVXWORKS_PLATFORM` 没加进 CFLAGS |
| `clock_gettime` / `CLOCK_MONOTONIC` 未定义 | VxWorks 6.9 内核态可能没有 POSIX 时钟：不要定义 `RTBENCH_VXWORKS_USE_POSIX_CLOCK`，走 `tickGet64()` 兜底 |
| `taskCpuAffinitySet` / `cpuset_t` 未定义 | 非 SMP 内核或版本 API 不同；单核下直接返回 0 即可（参考实现已在 `#else` 分支处理） |
| 链接报缺 `sched_*` 符号 | 构建清单漏了 `generator/test_schedule/` 下的 4 个包装器源文件 |
| 链接报 `undefined reference to __cxa_*` / `std::` | 缺 libstdc++，链接加 `-lstdc++`；或 C++ 负载未启用浮点任务选项 |
| 定时精度不够、周期抖动大 | `taskDelay` 是 tick 级；提高 `sysClkRateGet()`、改用 timebase 寄存器换算，或按 §5.1 换 watchdog 方案 |
