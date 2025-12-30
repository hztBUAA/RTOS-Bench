# RT-Bench 在 RT-Thread 上的移植指南

## 概述

本文档说明如何将 rt-bench 移植到 RT-Thread 操作系统，以便在不同 RTOS 上进行基准测试。

## RT-Bench 测试逻辑

### 核心工作流程

rt-bench 的核心测试逻辑在 `generator/periodic_benchmark.c` 中实现：

1. **初始化阶段**：
   - 设置定时器（period timer 和 deadline timer）
   - 设置信号处理器（SIGRTMIN 用于 deadline，SIGRTMIN+1 用于 period）
   - 初始化性能计数器（如果支持）
   - 调用 `benchmark_init()` 初始化负载

2. **周期性执行**：
   - 等待 period 信号（通过信号量 `period_sem`）
   - 记录周期开始时间戳
   - 调用 `benchmark_execution()` 执行负载
   - 记录周期结束时间戳和性能数据
   - 输出 CSV 格式的测试结果

3. **清理阶段**：
   - 调用 `benchmark_teardown()` 清理资源
   - 关闭定时器和文件

### 关键接口

每个负载需要实现三个函数：

```c
// 初始化（只执行一次）
int benchmark_init(int parameters_num, void **parameters);

// 周期性执行（每个周期调用一次）
void benchmark_execution(int parameters_num, void **parameters);

// 清理（退出时调用）
void benchmark_teardown(int parameters_num, void **parameters);
```

## 需要适配的 Linux 特定功能

### 1. 定时器系统

**Linux 实现**：
- 使用 POSIX 定时器：`timer_create()`, `timer_settime()`, `timer_delete()`
- 定时器到期时发送实时信号（SIGRTMIN）

**RT-Thread 适配**：
- 使用 RT-Thread 软件定时器：`rt_timer_create()`, `rt_timer_start()`, `rt_timer_delete()`
- 定时器回调函数中发送事件或信号量

**需要修改的文件**：
- `generator/periodic_benchmark.c` 中的 `setup_timer()` 函数

### 2. 信号和信号处理

**Linux 实现**：
- 使用 POSIX 实时信号：`SIGRTMIN`, `SIGRTMIN+1`
- 使用 `sigaction()` 注册信号处理器
- 使用信号量 `sem_t` 进行同步

**RT-Thread 适配**：
- 使用 RT-Thread 事件集：`rt_event_create()`, `rt_event_send()`, `rt_event_recv()`
- 或者使用信号量：`rt_sem_create()`, `rt_sem_release()`, `rt_sem_take()`

**需要修改的文件**：
- `generator/periodic_benchmark.c` 中的 `setup_signal()` 函数
- `period_handler()` 和 `deadline_handler()` 改为事件处理函数

### 3. 调度策略

**Linux 实现**：
- 使用 `sched_setattr()` 设置 SCHED_FIFO 或 SCHED_DEADLINE
- 使用 `sched_setaffinity()` 设置 CPU 亲和性

**RT-Thread 适配**：
- 使用 `rt_thread_control()` 设置线程优先级
- 使用 `rt_thread_control()` 绑定线程到特定 CPU（如果支持多核）

**需要修改的文件**：
- `generator/main.c` 中的 `set_sched_fifo_prio()` 和 `set_sched_deadline()` 函数
- `generator/sched_attr.h` 需要重新实现

### 4. 时间戳获取

**Linux 实现**：
- `get_rdtsc()`: 使用 RDTSC 指令（x86）或 CNTVCT（ARM）
- `get_timestamp()`: 使用 `clock_gettime(CLOCK_MONOTONIC)`

**RT-Thread 适配**：
- `get_rdtsc()`: 使用 RT-Thread 的硬件定时器或系统时钟
- `get_timestamp()`: 使用 `rt_tick_get()` 或 `rt_hw_tick_get()`

**需要修改的文件**：
- `generator/get_cpu_timestamp.c`

### 5. 内存监控

**Linux 实现**：
- 使用 `malloc()` 和 `mmap()` 的 wrapper（通过 `-Wl,--wrap=malloc`）

**RT-Thread 适配**：
- 使用 RT-Thread 的内存管理 API：`rt_malloc()`, `rt_free()`
- 需要实现类似的 wrapper 机制

**需要修改的文件**：
- `generator/memory_watcher.c`

### 6. 性能计数器

**Linux 实现**：
- 使用 Linux Perf 子系统读取硬件性能计数器

**RT-Thread 适配**：
- 如果硬件支持，可以通过寄存器直接读取
- 或者暂时禁用此功能

**需要修改的文件**：
- `generator/performance_counters.c`

## 移植步骤

### 步骤 1: 创建平台抽象层

建议在 `generator/` 目录下创建平台抽象层：

```
generator/
  platform/
    rt-thread/
      timer.c          # RT-Thread 定时器封装
      event.c          # RT-Thread 事件封装
      scheduler.c      # RT-Thread 调度封装
      timestamp.c      # RT-Thread 时间戳封装
    linux/
      timer.c          # Linux 定时器实现（现有代码）
      ...
```

### 步骤 2: 修改编译系统

在 `generator/Makefile` 中添加平台选择：

```makefile
PLATFORM ?= linux
ifeq ($(PLATFORM),rt-thread)
  CFLAGS += -DRT_THREAD_PLATFORM -I$(RT_THREAD_ROOT)/include
  # RT-Thread 特定的编译选项
else
  CFLAGS += -DLINUX_PLATFORM
  # Linux 特定的编译选项
endif
```

### 步骤 3: 实现核心适配

#### 3.1 定时器适配

创建 `generator/platform/rt-thread/timer.c`：

```c
#include <rtthread.h>

// RT-Thread 定时器结构
struct rtbench_timer {
    rt_timer_t timer;
    rt_sem_t sem;
    int signal_type;  // DEADLINE 或 PERIOD
};

// 创建定时器
int rtbench_timer_create(struct rtbench_timer **timer, 
                         int signal_type,
                         void (*handler)(void *)) {
    // 实现 RT-Thread 定时器创建
}

// 设置定时器
int rtbench_timer_settime(struct rtbench_timer *timer,
                          long sec, long nsec) {
    // 实现 RT-Thread 定时器设置
}
```

#### 3.2 事件/信号量适配

修改 `periodic_benchmark.c` 中的信号量使用：

```c
#ifdef RT_THREAD_PLATFORM
#include <rtthread.h>
#define rtbench_sem_t rt_sem_t
#define rtbench_sem_init(sem, pshared, value) \
    rt_sem_init(sem, "period_sem", value, RT_IPC_FLAG_FIFO)
#define rtbench_sem_wait(sem) rt_sem_take(sem, RT_WAITING_FOREVER)
#define rtbench_sem_post(sem) rt_sem_release(sem)
#else
// Linux 实现
#endif
```

### 步骤 4: 修改主循环

`periodic_benchmark.c` 中的主循环需要适配：

```c
#ifdef RT_THREAD_PLATFORM
    // RT-Thread 版本：使用事件或信号量
    while (tasks_launched < exec_opts->tasks_to_launch ||
           exec_opts->tasks_to_launch == 0) {
        rt_sem_take(&period_sem, RT_WAITING_FOREVER);
        // 执行 benchmark_execution
    }
#else
    // Linux 版本：使用 POSIX 信号量
    while (...) {
        sem_wait(&period_sem);
        // 执行 benchmark_execution
    }
#endif
```

### 步骤 5: 测试验证

1. **最小化测试**：先实现一个简单的负载（如空循环），验证框架能正常运行
2. **功能测试**：逐步添加功能（定时器、事件、时间戳等）
3. **性能测试**：运行完整的负载，验证结果输出

## 负载转换指南

将现有负载转换为 rt-bench 风格：

### 原始负载结构

```c
int main(int argc, char **argv) {
    // 初始化代码
    init_data();
    
    // 执行代码
    run_workload();
    
    // 清理代码
    cleanup();
    return 0;
}
```

### RT-Bench 风格转换

```c
// 全局变量，用于在函数间共享状态
static void *workload_data = NULL;

int benchmark_init(int parameters_num, void **parameters) {
    // 从原始 main() 的初始化部分提取
    init_data();
    return 0;
}

void benchmark_execution(int parameters_num, void **parameters) {
    // 从原始 main() 的执行部分提取
    // 注意：需要确保每次执行后状态可重置
    reset_state();  // 重置状态
    run_workload();
}

void benchmark_teardown(int parameters_num, void **parameters) {
    // 从原始 main() 的清理部分提取
    cleanup();
}
```

## 当前状态和下一步

### 已完成
- [x] 和思为学长对齐后续的工作

### 进行中
- [ ] **【配置环境】rt-bench 在 rt-thread 上跑通**
  - 创建平台抽象层结构
  - 实现定时器适配
  - 实现事件/信号量适配
  - 实现时间戳适配
  - 修改主循环逻辑

### 待完成
- [ ] **【调研】rt-bench在不同rtos上抽象层适配**
  - 分析其他 RTOS（FreeRTOS, Zephyr 等）的适配需求
  - 设计通用的抽象层接口
  - 实现多平台支持

- [ ] **【代码】把学长在application上的几个负载进行rt-bench style的转换**
  - 分析现有负载代码结构
  - 实现 `benchmark_init/execution/teardown` 接口
  - 验证负载功能正确性

- [ ] **【测试】验证 rt-thread 上的测试逻辑**
  - 运行测试并收集结果
  - 验证 CSV 输出格式
  - 对比 Linux 和 RT-Thread 的性能差异

## 参考资源

1. **RT-Thread 文档**：
   - [RT-Thread 编程指南](https://www.rt-thread.org/document/site/)
   - [RT-Thread API 参考](https://www.rt-thread.org/document/site/programming-manual/api/api/)

2. **RT-Bench 文档**：
   - 在线文档：https://rt-bench.gitlab.io/rt-bench/
   - 源码文档：`docs/source/`

3. **相关文件**：
   - `generator/periodic_benchmark.c` - 核心测试逻辑
   - `generator/main.c` - 主入口和参数解析
   - `docs/source/3-Extending_rt-bench.markdown` - 扩展指南

## 常见问题

### Q: 是否需要在 RT-Thread 上完全实现所有功能？

A: 不一定。可以先实现核心功能（定时器、事件、时间戳），性能计数器和内存监控可以后续添加或暂时禁用。

### Q: 如何验证移植是否正确？

A: 建议分阶段验证：
1. 先验证定时器能正常触发
2. 再验证事件/信号量同步正常
3. 最后验证完整测试流程和结果输出

### Q: 负载转换时需要注意什么？

A: 关键点：
- `benchmark_execution()` 必须可重复执行（状态可重置）
- 避免在 `benchmark_execution()` 中分配大量内存（应在 `benchmark_init()` 中分配）
- 确保线程安全（如果使用多线程）

## 联系方式

如有问题，建议：
1. 查阅 RT-Bench 和 RT-Thread 官方文档
2. 参考现有 benchmark 实现（如 `vision/benchmarks/disparity/`）
3. 与思为学长讨论具体实现细节

