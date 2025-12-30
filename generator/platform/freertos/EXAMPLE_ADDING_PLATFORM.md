# 添加新平台示例：FreeRTOS

本文档展示如何为 rt-bench 添加 FreeRTOS 平台支持，作为添加新平台的完整示例。

## 步骤概览

1. ✅ 修改 `platform_abstraction.h` 添加平台检测
2. ✅ 创建 `platform/freertos/` 目录
3. ✅ 实现所有接口函数
4. ✅ 更新 Makefile
5. ✅ 测试验证

## 步骤 1: 修改平台检测

在 `generator/platform_abstraction.h` 中添加：

```c
/* Platform detection */
#if defined(RT_THREAD_PLATFORM)
    #include <rtthread.h>
    #define RTBENCH_PLATFORM_RTTHREAD
#elif defined(FREERTOS_PLATFORM)          // ← 新增
    #include "FreeRTOS.h"
    #include "timers.h"
    #include "semphr.h"
    #define RTBENCH_PLATFORM_FREERTOS      // ← 新增
#elif defined(__linux__)
    #define RTBENCH_PLATFORM_LINUX
#else
    #error "Unsupported platform. Please define RT_THREAD_PLATFORM, FREERTOS_PLATFORM or use Linux."
#endif
```

## 步骤 2: 创建目录结构

```bash
mkdir -p generator/platform/freertos
```

## 步骤 3: 实现接口函数

### 3.1 timer.c - 定时器实现

```c
#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_FREERTOS

#include "FreeRTOS.h"
#include "timers.h"
#include <stdlib.h>

struct rtbench_timer_internal {
    TimerHandle_t timer;
    rtbench_timer_callback_t callback;
    void *user_data;
    rtbench_timer_type_t type;
};

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
    const char *name = (timer_type == RTBENCH_TIMER_PERIOD) ? 
                       "rtbench_period" : "rtbench_deadline";
    
    timer = pvPortMalloc(sizeof(struct rtbench_timer_internal));
    if (!timer) return NULL;
    
    timer->callback = callback;
    timer->user_data = user_data;
    timer->type = timer_type;
    
    timer->timer = xTimerCreate(
        name,
        pdMS_TO_TICKS(1000),  // 默认值，会被 settime 覆盖
        (timer_type == RTBENCH_TIMER_PERIOD) ? pdTRUE : pdFALSE,
        timer,
        freertos_timer_callback);
    
    if (!timer->timer) {
        vPortFree(timer);
        return NULL;
    }
    
    return (rtbench_timer_t)timer;
}

int rtbench_timer_settime(rtbench_timer_t timer, long sec, long nsec)
{
    struct rtbench_timer_internal *t = (struct rtbench_timer_internal *)timer;
    TickType_t ticks;
    
    if (!t || !t->timer) return -1;
    
    ticks = pdMS_TO_TICKS((sec * 1000) + (nsec / 1000000));
    if (ticks == 0) {
        xTimerStop(t->timer, 0);
        return 0;
    }
    
    xTimerChangePeriod(t->timer, ticks, 0);
    xTimerStart(t->timer, 0);
    return 0;
}

int rtbench_timer_delete(rtbench_timer_t timer)
{
    struct rtbench_timer_internal *t = (struct rtbench_timer_internal *)timer;
    if (!t) return -1;
    if (t->timer) xTimerDelete(t->timer, 0);
    vPortFree(t);
    return 0;
}

#endif /* RTBENCH_PLATFORM_FREERTOS */
```

### 3.2 sync.c - 信号量实现

```c
#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_FREERTOS

#include "FreeRTOS.h"
#include "semphr.h"
#include <stdlib.h>

struct rtbench_sem_internal {
    SemaphoreHandle_t sem;
};

rtbench_sem_t rtbench_sem_create(unsigned int initial_value)
{
    struct rtbench_sem_internal *sem;
    
    sem = pvPortMalloc(sizeof(struct rtbench_sem_internal));
    if (!sem) return NULL;
    
    sem->sem = xSemaphoreCreateCounting(UINT_MAX, initial_value);
    if (!sem->sem) {
        vPortFree(sem);
        return NULL;
    }
    
    return (rtbench_sem_t)sem;
}

int rtbench_sem_wait(rtbench_sem_t sem)
{
    struct rtbench_sem_internal *s = (struct rtbench_sem_internal *)sem;
    if (!s || !s->sem) return -1;
    return (xSemaphoreTake(s->sem, portMAX_DELAY) == pdTRUE) ? 0 : -1;
}

int rtbench_sem_post(rtbench_sem_t sem)
{
    struct rtbench_sem_internal *s = (struct rtbench_sem_internal *)sem;
    if (!s || !s->sem) return -1;
    return (xSemaphoreGive(s->sem) == pdTRUE) ? 0 : -1;
}

int rtbench_sem_destroy(rtbench_sem_t sem)
{
    struct rtbench_sem_internal *s = (struct rtbench_sem_internal *)sem;
    if (!s) return -1;
    if (s->sem) vSemaphoreDelete(s->sem);
    vPortFree(s);
    return 0;
}

#endif /* RTBENCH_PLATFORM_FREERTOS */
```

### 3.3 timestamp.c - 时间戳实现

```c
#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_FREERTOS

#include "FreeRTOS.h"
#include "task.h"

unsigned long long rtbench_get_rdtsc(void)
{
    // FreeRTOS 使用 tick 计数器
    // 转换为近似周期数（需要根据 CPU 频率调整）
    TickType_t tick = xTaskGetTickCount();
    // 假设 CPU 频率 100MHz，RTOS tick 1ms
    return (unsigned long long)tick * 100000;  // 需要根据实际情况调整
}

long double rtbench_get_timestamp(void)
{
    TickType_t tick = xTaskGetTickCount();
    // 假设 configTICK_RATE_HZ = 1000 (1ms per tick)
    return (long double)tick / (long double)configTICK_RATE_HZ;
}

#endif /* RTBENCH_PLATFORM_FREERTOS */
```

### 3.4 scheduler.c - 调度器实现

```c
#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_FREERTOS

#include "FreeRTOS.h"
#include "task.h"

int rtbench_set_priority(unsigned int priority)
{
    TaskHandle_t task = xTaskGetCurrentTaskHandle();
    if (!task) return -1;
    
    // FreeRTOS 优先级：数字越大优先级越高
    // 需要根据实际需求映射
    vTaskPrioritySet(task, (UBaseType_t)priority);
    return 0;
}

int rtbench_set_deadline(uint64_t runtime, uint64_t deadline, uint64_t period)
{
    // FreeRTOS 不原生支持 deadline 调度
    // 可以返回成功但不实现，或实现软件层面的监控
    return 0;
}

int rtbench_set_affinity(uint32_t cpu_mask)
{
    // FreeRTOS 在多核系统上可能支持 CPU 亲和性
    // 需要根据具体版本实现
    return 0;
}

#endif /* RTBENCH_PLATFORM_FREERTOS */
```

## 步骤 4: 更新 Makefile

在 `generator/Makefile` 中添加：

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

## 步骤 5: 编译和测试

```bash
# 设置 FreeRTOS 路径
export FREERTOS_ROOT=/path/to/FreeRTOS

# 编译
cd generator
make PLATFORM=freertos test_benchmark

# 运行（如果 FreeRTOS 支持 shell）
./test_benchmark -p 1.0 -d 0.5 -t 5
```

## 验证清单

- [ ] 所有接口函数都已实现
- [ ] 条件编译正确（`#ifdef RTBENCH_PLATFORM_FREERTOS`）
- [ ] Makefile 正确配置
- [ ] 编译无错误
- [ ] 基本功能测试通过

## 注意事项

1. **时间精度**：FreeRTOS 的 tick 精度可能较低，需要验证
2. **内存管理**：使用 FreeRTOS 的 `pvPortMalloc/vPortFree`
3. **API 差异**：FreeRTOS 的 API 与 Linux/RT-Thread 不同，需要仔细适配
4. **多核支持**：如果目标平台是多核，可能需要额外的 CPU 亲和性实现

## 总结

添加新平台的核心步骤：

1. **平台检测**：在 `platform_abstraction.h` 中添加 `#ifdef`
2. **创建目录**：`platform/<platform-name>/`
3. **实现接口**：实现所有 `rtbench_*` 函数
4. **更新构建**：在 Makefile 中添加平台选项
5. **测试验证**：编译并运行测试

每个平台的实现都是独立的，不会影响其他平台！

