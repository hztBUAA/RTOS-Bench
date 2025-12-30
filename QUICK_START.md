# RT-Bench RT-Thread 快速开始指南

## 概述

本指南帮助你快速开始将 rt-bench 移植到 RT-Thread 平台。

## 前置条件

1. **RT-Thread 环境**：
   - 已安装 RT-Thread 开发环境
   - 了解 RT-Thread 的基本 API（定时器、信号量等）

2. **rt-bench 源码**：
   - 已克隆 rt-bench 仓库
   - 了解 rt-bench 的基本结构

## 快速开始步骤

### 步骤 1: 了解当前状态

rt-bench 目前主要针对 Linux 平台，使用了以下 Linux 特定功能：

- **POSIX 定时器** (`timer_create`, `timer_settime`)
- **POSIX 信号** (`SIGRTMIN`, `sigaction`)
- **POSIX 信号量** (`sem_init`, `sem_wait`, `sem_post`)
- **Linux 调度系统调用** (`sched_setattr`, `sched_setaffinity`)
- **Linux 时间 API** (`clock_gettime`, `gettimeofday`)

### 步骤 2: 创建平台适配文件

1. **创建目录结构**：
```bash
cd generator
mkdir -p platform/rt-thread
```

2. **复制示例文件并重命名**：
```bash
cp platform/rt-thread/timer.c.example platform/rt-thread/timer.c
cp platform/rt-thread/sync.c.example platform/rt-thread/sync.c
cp platform/rt-thread/timestamp.c.example platform/rt-thread/timestamp.c
```

3. **根据你的 RT-Thread 版本调整代码**：
   - 检查 RT-Thread API 版本
   - 调整定时器精度和参数
   - 验证信号量 API 兼容性

### 步骤 3: 实现调度器适配

创建 `generator/platform/rt-thread/scheduler.c`：

```c
#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_RTTHREAD

#include <rtthread.h>

int rtbench_set_priority(unsigned int priority)
{
    rt_thread_t thread = rt_thread_self();
    if (!thread) {
        return -1;
    }
    
    /* RT-Thread priority: lower number = higher priority */
    /* You may need to adjust the mapping based on your needs */
    rt_uint8_t rt_priority = (rt_uint8_t)priority;
    
    rt_err_t result = rt_thread_control(thread, 
                                         RT_THREAD_CTRL_CHANGE_PRIORITY, 
                                         &rt_priority);
    return (result == RT_EOK) ? 0 : -1;
}

int rtbench_set_deadline(uint64_t runtime, uint64_t deadline, uint64_t period)
{
    /* RT-Thread may not support deadline scheduling natively */
    /* You can implement a software-based deadline scheduler if needed */
    /* For now, return success (no-op) */
    return 0;
}

int rtbench_set_affinity(uint32_t cpu_mask)
{
    /* RT-Thread may support CPU affinity on multi-core systems */
    /* Check your RT-Thread version for multi-core support */
    rt_thread_t thread = rt_thread_self();
    if (!thread) {
        return -1;
    }
    
    /* Example implementation (adjust based on your RT-Thread version) */
    /* rt_err_t result = rt_thread_control(thread, RT_THREAD_CTRL_BIND_CPU, &cpu_mask); */
    
    /* For now, return success (no-op) */
    return 0;
}

#endif
```

### 步骤 4: 修改编译系统

在 `generator/Makefile` 中添加平台支持：

```makefile
# Platform selection
PLATFORM ?= linux

ifeq ($(PLATFORM),rt-thread)
  CFLAGS += -DRT_THREAD_PLATFORM
  CFLAGS += -I$(RT_THREAD_ROOT)/include
  LDFLAGS += -L$(RT_THREAD_ROOT)/lib -lrtthread
  PLATFORM_DIR = platform/rt-thread
else
  CFLAGS += -DLINUX_PLATFORM
  PLATFORM_DIR = platform/linux
endif

# Platform abstraction source files
PLATFORM_SRC = $(PLATFORM_DIR)/timer.c \
               $(PLATFORM_DIR)/sync.c \
               $(PLATFORM_DIR)/timestamp.c \
               $(PLATFORM_DIR)/scheduler.c

OBJS += $(PLATFORM_SRC:.c=.o)
```

### 步骤 5: 修改核心文件

在 `generator/periodic_benchmark.c` 的开头添加：

```c
#include "platform_abstraction.h"

/* Replace timer functions */
#ifdef RTBENCH_PLATFORM_RTTHREAD
  /* Use platform abstraction */
#else
  /* Keep original Linux implementation */
#endif
```

### 步骤 6: 创建测试负载

创建 `test_simple.c`：

```c
#include "periodic_benchmark.h"
#include "logging.h"

int benchmark_init(int parameters_num, void **parameters)
{
    elogf(LOG_LEVEL_INFO, "Benchmark initialized\n");
    return 0;
}

void benchmark_execution(int parameters_num, void **parameters)
{
    /* Simple workload: busy loop */
    volatile int i, j;
    for (i = 0; i < 1000; i++) {
        for (j = 0; j < 1000; j++) {
            /* Empty loop */
        }
    }
}

void benchmark_teardown(int parameters_num, void **parameters)
{
    elogf(LOG_LEVEL_INFO, "Benchmark teardown\n");
}
```

### 步骤 7: 编译和测试

```bash
# 编译（Linux 平台，用于验证）
cd generator
make PLATFORM=linux test_simple

# 编译（RT-Thread 平台）
make PLATFORM=rt-thread test_simple

# 运行测试
./test_simple -p 1.0 -d 0.5 -t 5 -l 3
```

## 常见问题

### Q1: 定时器精度不够？

**A**: RT-Thread 的软件定时器精度取决于 `RT_TICK_PER_SECOND`。如果需要更高精度：
- 使用硬件定时器（如果硬件支持）
- 调整 `RT_TICK_PER_SECOND` 值（注意会影响系统性能）

### Q2: 如何获取高精度时间戳？

**A**: 取决于硬件：
- **ARM Cortex-M**: 使用 DWT (Data Watchpoint and Trace) 单元
- **ARM Cortex-A**: 使用系统计数器（CNTVCT_EL0）
- **其他**: 使用系统 tick 计数器（精度较低）

### Q3: RT-Thread 不支持 deadline 调度怎么办？

**A**: 可以：
1. 暂时禁用 deadline 调度功能
2. 使用优先级调度模拟
3. 实现软件层面的 deadline 监控

### Q4: 编译错误：找不到 RT-Thread 头文件

**A**: 确保：
1. 设置了 `RT_THREAD_ROOT` 环境变量
2. RT-Thread 头文件路径正确
3. 在 Makefile 中正确设置了 `-I` 选项

## 下一步

完成基本适配后，可以：

1. **验证功能**：运行测试负载，检查输出是否正确
2. **性能测试**：对比 Linux 和 RT-Thread 版本的性能差异
3. **负载转换**：将学长的负载转换为 rt-bench 格式
4. **优化**：根据测试结果优化实现

## 参考文档

- **RT-THREAD_PORTING_GUIDE.md** - 详细的移植指南
- **IMPLEMENTATION_PLAN.md** - 实施计划
- **platform_abstraction.h** - 平台抽象层接口定义
- RT-Thread 官方文档：https://www.rt-thread.org/document/site/

## 获取帮助

如果遇到问题：

1. 检查 RT-Thread 和 rt-bench 的文档
2. 查看示例代码中的注释
3. 与思为学长讨论具体实现细节
4. 参考现有 benchmark 的实现（如 `vision/benchmarks/disparity/`）

