# 平台抽象层工作原理详解

## 概述

平台抽象层（Platform Abstraction Layer）是一种设计模式，允许 rt-bench 在不同操作系统上运行，而核心代码不需要修改。它通过**统一接口 + 条件编译 + 平台特定实现**来实现。

## 工作原理

### 1. 设计模式：策略模式（Strategy Pattern）

```
┌─────────────────────────────────────┐
│   rt-bench 核心代码                 │
│   (periodic_benchmark.c, main.c)   │
│                                     │
│   使用统一接口：                    │
│   - rtbench_timer_create()          │
│   - rtbench_sem_wait()              │
│   - rtbench_get_timestamp()         │
└──────────────┬──────────────────────┘
               │
               │ 调用
               ▼
┌─────────────────────────────────────┐
│   platform_abstraction.h           │
│   (统一接口定义)                    │
└──────────────┬──────────────────────┘
               │
               │ 实现
               ▼
    ┌──────────┴──────────┐
    │                     │
    ▼                     ▼
┌──────────┐      ┌──────────────┐
│ Linux    │      │ RT-Thread    │
│ 实现     │      │ 实现         │
│          │      │              │
│ timer.c  │      │ timer.c      │
│ sync.c   │      │ sync.c       │
│ ...      │      │ ...          │
└──────────┘      └──────────────┘
```

### 2. 条件编译机制

#### 2.1 平台检测（在 `platform_abstraction.h` 中）

```c
/* Platform detection */
#if defined(RT_THREAD_PLATFORM)
    #include <rtthread.h>
    #define RTBENCH_PLATFORM_RTTHREAD
#elif defined(__linux__)
    #define RTBENCH_PLATFORM_LINUX
#else
    #error "Unsupported platform. Please define RT_THREAD_PLATFORM or use Linux."
#endif
```

**工作原理**：
- 编译时通过 `-DRT_THREAD_PLATFORM` 或自动检测 `__linux__` 宏
- 定义平台特定的宏（如 `RTBENCH_PLATFORM_RTTHREAD`）
- 每个平台的实现文件使用 `#ifdef` 来包含/排除代码

#### 2.2 平台特定实现（在 `platform/rt-thread/timer.c` 中）

```c
#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_RTTHREAD
    // RT-Thread 特定代码
    #include <rtthread.h>
    
    rtbench_timer_t rtbench_timer_create(...) {
        // RT-Thread 实现
        rt_timer_t timer = rt_timer_create(...);
        ...
    }
#endif
```

**关键点**：
- 所有平台实现文件都包含 `platform_abstraction.h`
- 使用 `#ifdef RTBENCH_PLATFORM_XXX` 来条件编译
- 只有当前平台的代码会被编译

### 3. 编译系统集成

#### 3.1 Makefile 中的平台选择

```makefile
# Platform selection
PLATFORM ?= linux

ifeq ($(PLATFORM),rt-thread)
  CFLAGS += -DRT_THREAD_PLATFORM
  CFLAGS += -I$(RT_THREAD_ROOT)/include
  PLATFORM_DIR = platform/rt-thread
else ifeq ($(PLATFORM),linux)
  CFLAGS += -DLINUX_PLATFORM
  PLATFORM_DIR = platform/linux
else ifeq ($(PLATFORM),freertos)
  CFLAGS += -DFREERTOS_PLATFORM
  CFLAGS += -I$(FREERTOS_ROOT)/include
  PLATFORM_DIR = platform/freertos
endif

# 只编译当前平台的源文件
PLATFORM_SRC = $(PLATFORM_DIR)/timer.c \
               $(PLATFORM_DIR)/sync.c \
               $(PLATFORM_DIR)/timestamp.c \
               $(PLATFORM_DIR)/scheduler.c

OBJS += $(PLATFORM_SRC:.c=.o)
```

**工作原理**：
1. 通过 `PLATFORM` 变量选择平台
2. 设置平台特定的编译选项（`-D` 宏定义、头文件路径等）
3. 只编译当前平台的源文件（其他平台的 `.c` 文件不会被编译）

### 4. 统一接口设计

#### 4.1 接口定义（在 `platform_abstraction.h` 中）

```c
/* 不透明的句柄类型 */
typedef void* rtbench_timer_t;
typedef void* rtbench_sem_t;

/* 统一的函数接口 */
rtbench_timer_t rtbench_timer_create(...);
int rtbench_timer_settime(rtbench_timer_t timer, ...);
int rtbench_sem_wait(rtbench_sem_t sem);
```

**设计原则**：
- **不透明句柄**：使用 `void*` 隐藏平台特定的数据结构
- **统一函数签名**：所有平台使用相同的函数名和参数
- **返回值约定**：0 表示成功，负数表示失败

#### 4.2 核心代码使用（在 `periodic_benchmark.c` 中）

```c
#include "platform_abstraction.h"

int periodic_benchmark(...) {
    // 使用统一接口，不关心底层实现
    rtbench_timer_t period_timer = rtbench_timer_create(
        RTBENCH_TIMER_PERIOD, period_handler, NULL);
    
    rtbench_sem_t period_sem = rtbench_sem_create(0);
    
    while (...) {
        rtbench_sem_wait(period_sem);  // 等待周期
        benchmark_execution(...);
    }
}
```

**关键点**：
- 核心代码只包含 `platform_abstraction.h`
- 不包含任何平台特定的头文件
- 所有平台相关调用都通过抽象层

## 如何添加新平台

### 步骤 1: 在 `platform_abstraction.h` 中添加平台检测

```c
/* Platform detection */
#if defined(RT_THREAD_PLATFORM)
    #include <rtthread.h>
    #define RTBENCH_PLATFORM_RTTHREAD
#elif defined(FREERTOS_PLATFORM)
    #include "FreeRTOS.h"
    #include "timers.h"
    #define RTBENCH_PLATFORM_FREERTOS
#elif defined(__linux__)
    #define RTBENCH_PLATFORM_LINUX
#else
    #error "Unsupported platform."
#endif
```

### 步骤 2: 创建平台目录和实现文件

```bash
mkdir -p generator/platform/freertos
```

创建以下文件：
- `generator/platform/freertos/timer.c`
- `generator/platform/freertos/sync.c`
- `generator/platform/freertos/timestamp.c`
- `generator/platform/freertos/scheduler.c`

### 步骤 3: 实现平台特定函数

以 `timer.c` 为例：

```c
/**
 * @file timer.c
 * @brief FreeRTOS timer implementation for rt-bench
 */

#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_FREERTOS

#include "FreeRTOS.h"
#include "timers.h"
#include <stdlib.h>

/* FreeRTOS 定时器内部结构 */
struct rtbench_timer_internal {
    TimerHandle_t timer;
    rtbench_timer_callback_t callback;
    void *user_data;
    rtbench_timer_type_t type;
};

/* FreeRTOS 定时器回调包装器 */
static void freertos_timer_callback(TimerHandle_t xTimer)
{
    struct rtbench_timer_internal *t = 
        (struct rtbench_timer_internal *)pvTimerGetTimerID(xTimer);
    
    if (t && t->callback) {
        t->callback(t->user_data);
    }
}

rtbench_timer_t rtbench_timer_create(rtbench_timer_type_t timer_type,
                                      rtbench_timer_callback_t callback,
                                      void *user_data)
{
    struct rtbench_timer_internal *timer;
    const char *timer_name;
    
    if (!callback) {
        return NULL;
    }
    
    timer = (struct rtbench_timer_internal *)pvPortMalloc(
        sizeof(struct rtbench_timer_internal));
    if (!timer) {
        return NULL;
    }
    
    timer->callback = callback;
    timer->user_data = user_data;
    timer->type = timer_type;
    
    /* FreeRTOS 定时器名称 */
    timer_name = (timer_type == RTBENCH_TIMER_PERIOD) ? 
                 "rtbench_period" : "rtbench_deadline";
    
    /* 创建 FreeRTOS 软件定时器 */
    timer->timer = xTimerCreate(
        timer_name,
        pdMS_TO_TICKS(1000),  /* 默认 1 秒 */
        (timer_type == RTBENCH_TIMER_PERIOD) ? pdTRUE : pdFALSE,  /* 周期/单次 */
        timer,  /* 定时器 ID */
        freertos_timer_callback);
    
    if (!timer->timer) {
        vPortFree(timer);
        return NULL;
    }
    
    return (rtbench_timer_t)timer;
}

int rtbench_timer_settime(rtbench_timer_t timer, long sec, long nsec)
{
    struct rtbench_timer_internal *t = 
        (struct rtbench_timer_internal *)timer;
    TickType_t timeout_ticks;
    
    if (!t || !t->timer) {
        return -1;
    }
    
    /* 转换秒和纳秒到 FreeRTOS ticks */
    timeout_ticks = pdMS_TO_TICKS((sec * 1000) + (nsec / 1000000));
    
    if (timeout_ticks == 0) {
        xTimerStop(t->timer, 0);
        return 0;
    }
    
    /* 设置定时器周期并启动 */
    xTimerChangePeriod(t->timer, timeout_ticks, 0);
    xTimerStart(t->timer, 0);
    
    return 0;
}

int rtbench_timer_delete(rtbench_timer_t timer)
{
    struct rtbench_timer_internal *t = 
        (struct rtbench_timer_internal *)timer;
    
    if (!t) {
        return -1;
    }
    
    if (t->timer) {
        xTimerDelete(t->timer, 0);
        t->timer = NULL;
    }
    
    vPortFree(t);
    
    return 0;
}

#endif /* RTBENCH_PLATFORM_FREERTOS */
```

### 步骤 4: 更新 Makefile

```makefile
# Platform selection
PLATFORM ?= linux

ifeq ($(PLATFORM),rt-thread)
  CFLAGS += -DRT_THREAD_PLATFORM
  CFLAGS += -I$(RT_THREAD_ROOT)/include
  PLATFORM_DIR = platform/rt-thread
else ifeq ($(PLATFORM),freertos)
  CFLAGS += -DFREERTOS_PLATFORM
  CFLAGS += -I$(FREERTOS_ROOT)/include
  CFLAGS += -I$(FREERTOS_ROOT)/portable/GCC/ARM_CM4F
  PLATFORM_DIR = platform/freertos
else ifeq ($(PLATFORM),linux)
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

### 步骤 5: 测试新平台

```bash
# 编译 FreeRTOS 版本
make PLATFORM=freertos FREERTOS_ROOT=/path/to/FreeRTOS test_benchmark

# 运行测试
./test_benchmark -p 1.0 -d 0.5 -t 5
```

## 完整示例：添加 FreeRTOS 支持

### 目录结构

```
generator/
├── platform_abstraction.h      # 统一接口定义
├── platform/
│   ├── linux/
│   │   ├── timer.c
│   │   ├── sync.c
│   │   └── ...
│   ├── rt-thread/
│   │   ├── timer.c
│   │   ├── sync.c
│   │   └── ...
│   └── freertos/               # 新平台
│       ├── timer.c
│       ├── sync.c
│       ├── timestamp.c
│       └── scheduler.c
└── periodic_benchmark.c        # 核心代码（不需要修改）
```

### 关键设计要点

1. **条件编译隔离**：
   - 每个平台的实现文件都用 `#ifdef RTBENCH_PLATFORM_XXX` 包裹
   - 确保只有当前平台的代码被编译

2. **不透明句柄**：
   - 使用 `void*` 隐藏平台特定的数据结构
   - 内部结构在实现文件中定义

3. **统一接口**：
   - 所有平台实现相同的函数签名
   - 返回值约定一致（0=成功，负数=失败）

4. **编译时选择**：
   - 通过 Makefile 变量选择平台
   - 只编译当前平台的源文件

## 优势

1. **代码复用**：核心代码不需要修改
2. **易于扩展**：添加新平台只需实现接口
3. **编译隔离**：不同平台的代码不会互相干扰
4. **类型安全**：通过不透明句柄避免类型错误

## 注意事项

1. **API 差异**：不同 RTOS 的 API 可能差异很大，需要仔细适配
2. **精度问题**：时间精度可能不同，需要验证
3. **线程模型**：某些 RTOS 可能不支持多线程，需要适配
4. **内存管理**：不同平台的内存管理方式可能不同

## 总结

平台抽象层通过以下机制工作：

1. **统一接口**：`platform_abstraction.h` 定义所有平台必须实现的接口
2. **条件编译**：通过 `#ifdef` 在编译时选择平台
3. **平台实现**：每个平台在独立目录中实现接口
4. **编译系统**：Makefile 根据 `PLATFORM` 变量选择编译哪些文件

添加新平台只需：
1. 在 `platform_abstraction.h` 中添加平台检测
2. 创建平台目录和实现文件
3. 实现所有接口函数
4. 更新 Makefile

这样，rt-bench 就可以轻松支持多个 RTOS 平台了！

