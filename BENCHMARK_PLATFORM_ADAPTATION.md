# Benchmark 代码的平台适配指南

## 核心问题

**是的，benchmark 本身的代码（`benchmark_init/execution/teardown`）也需要考虑不同 RTOS 平台！**

虽然 rt-bench **框架**通过平台抽象层已经隔离了平台差异，但 **benchmark 代码**如果使用了平台特定的 API，仍然需要适配。

## 问题分析

### 1. 框架 vs Benchmark 的职责划分

```
┌─────────────────────────────────────────┐
│  rt-bench 框架 (generator/)            │
│  - 定时器、信号量、调度等              │
│  - ✅ 已通过平台抽象层隔离             │
└─────────────────────────────────────────┘
              │ 调用
              ▼
┌─────────────────────────────────────────┐
│  Benchmark 代码 (IsolBench/, vision/)  │
│  - benchmark_init()                     │
│  - benchmark_execution()                │
│  - benchmark_teardown()                 │
│  - ⚠️ 可能使用平台特定 API             │
└─────────────────────────────────────────┘
```

### 2. 现有 Benchmark 中的平台特定代码

从代码分析来看，现有 benchmark 使用了以下平台特定的 API：

#### 2.1 时间相关 API

**latency.c**:
```c
clock_gettime(CLOCK_REALTIME, &start);  // Linux 特定
clock_gettime(CLOCK_REALTIME, &end);
```

**bandwidth.c**:
```c
gettimeofday(&time, NULL);  // Linux/Unix 特定
```

#### 2.2 内存管理 API

```c
malloc()  // 标准 C，但某些 RTOS 可能需要使用自己的内存管理
free()
```

#### 2.3 其他可能的平台特定代码

```c
#include <sched.h>      // Linux 调度相关
#include <sys/mman.h>   // Linux 内存映射
#include <unistd.h>     // POSIX 标准，但某些 RTOS 可能不支持
```

## 解决方案

### 方案 1: 使用标准 C 库（推荐，如果可能）

尽量使用标准 C 库函数，这些函数在大多数平台上都有实现：

```c
// ✅ 标准 C，通常可用
malloc(), free()
memset(), memcpy()
printf(), sprintf()

// ⚠️ POSIX 标准，某些 RTOS 可能不支持
getopt()  // 参数解析
```

### 方案 2: 使用平台抽象层扩展

如果 benchmark 需要平台特定的功能，可以扩展平台抽象层：

#### 2.1 扩展 `platform_abstraction.h`

```c
/* ============================================================================
 * Benchmark Helper Functions
 * ============================================================================ */

/**
 * @brief Get high-resolution timestamp for benchmark measurements
 * @param[out] sec Seconds
 * @param[out] nsec Nanoseconds
 * @return 0 on success, negative on failure
 */
int rtbench_get_highres_time(long *sec, long *nsec);

/**
 * @brief Allocate memory (platform-aware)
 * @param size Size in bytes
 * @return Pointer to allocated memory, NULL on failure
 */
void* rtbench_malloc(size_t size);

/**
 * @brief Free memory (platform-aware)
 * @param ptr Pointer to memory
 */
void rtbench_free(void *ptr);
```

#### 2.2 实现平台特定版本

**Linux 实现** (`platform/linux/benchmark_helpers.c`):
```c
#ifdef RTBENCH_PLATFORM_LINUX
#include <time.h>
#include <stdlib.h>

int rtbench_get_highres_time(long *sec, long *nsec)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
        *sec = ts.tv_sec;
        *nsec = ts.tv_nsec;
        return 0;
    }
    return -1;
}

void* rtbench_malloc(size_t size)
{
    return malloc(size);
}

void rtbench_free(void *ptr)
{
    free(ptr);
}
#endif
```

**RT-Thread 实现** (`platform/rt-thread/benchmark_helpers.c`):
```c
#ifdef RTBENCH_PLATFORM_RTTHREAD
#include <rtthread.h>

int rtbench_get_highres_time(long *sec, long *nsec)
{
    rt_tick_t tick = rt_tick_get();
    rt_uint32_t tick_per_sec = RT_TICK_PER_SECOND;
    
    *sec = tick / tick_per_sec;
    *nsec = (tick % tick_per_sec) * (1000000000L / tick_per_sec);
    return 0;
}

void* rtbench_malloc(size_t size)
{
    return rt_malloc(size);
}

void rtbench_free(void *ptr)
{
    rt_free(ptr);
}
#endif
```

#### 2.3 Benchmark 使用抽象层

**修改 latency.c**:
```c
// 原来的代码
clock_gettime(CLOCK_REALTIME, &start);

// 改为使用抽象层
long start_sec, start_nsec;
rtbench_get_highres_time(&start_sec, &start_nsec);
```

### 方案 3: 条件编译（简单直接）

如果只有少数几个地方需要适配，可以使用条件编译：

```c
void benchmark_execution(int parameters_num, void **parameters)
{
    // 时间测量
#ifdef RT_THREAD_PLATFORM
    rt_tick_t start_tick = rt_tick_get();
    // ... 执行负载 ...
    rt_tick_t end_tick = rt_tick_get();
    uint64_t nsdiff = (end_tick - start_tick) * (1000000000L / RT_TICK_PER_SECOND);
#elif defined(__linux__)
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    // ... 执行负载 ...
    clock_gettime(CLOCK_REALTIME, &end);
    uint64_t nsdiff = get_elapsed(&start, &end);
#else
    #error "Unsupported platform"
#endif
    
    avglat = (double)nsdiff / workingset_size / repeat;
}
```

### 方案 4: 使用 rt-bench 框架提供的时间戳

如果 benchmark 只需要测量执行时间，可以使用框架已经提供的时间戳函数：

```c
#include "get_cpu_timestamp.h"  // 框架提供的时间戳

void benchmark_execution(int parameters_num, void **parameters)
{
    // 使用框架的时间戳（已经通过平台抽象层）
    long double start = get_timestamp();
    unsigned long long start_cycles = get_rdtsc();
    
    // ... 执行负载 ...
    
    long double end = get_timestamp();
    unsigned long long end_cycles = get_rdtsc();
    
    // 计算延迟
    long double elapsed = end - start;
    uint64_t elapsed_cycles = end_cycles - start_cycles;
}
```

## 实际案例：适配 latency.c

### 当前实现（Linux 特定）

```c
void benchmark_execution(int parameters_num, void **parameters)
{
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);  // Linux 特定
    
    // ... 执行内存访问 ...
    
    clock_gettime(CLOCK_REALTIME, &end);    // Linux 特定
    nsdiff = get_elapsed(&start, &end);
    avglat = (double)nsdiff / workingset_size / repeat;
}
```

### 适配后（跨平台）

**方案 A：使用框架时间戳**
```c
#include "get_cpu_timestamp.h"

void benchmark_execution(int parameters_num, void **parameters)
{
    long double start = get_timestamp();  // 框架提供，已跨平台
    unsigned long long start_cycles = get_rdtsc();
    
    // ... 执行内存访问 ...
    
    long double end = get_timestamp();
    unsigned long long end_cycles = get_rdtsc();
    
    uint64_t nsdiff = (uint64_t)((end - start) * 1000000000.0);
    // 或者使用 cycles（更精确）
    // uint64_t nsdiff = (end_cycles - start_cycles) * (1000000000.0 / CPU_FREQ);
    
    avglat = (double)nsdiff / workingset_size / repeat;
}
```

**方案 B：条件编译**
```c
void benchmark_execution(int parameters_num, void **parameters)
{
    uint64_t nsdiff;
    
#ifdef RT_THREAD_PLATFORM
    rt_tick_t start_tick = rt_tick_get();
    // ... 执行内存访问 ...
    rt_tick_t end_tick = rt_tick_get();
    nsdiff = (end_tick - start_tick) * (1000000000L / RT_TICK_PER_SECOND);
#elif defined(__linux__)
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    // ... 执行内存访问 ...
    clock_gettime(CLOCK_REALTIME, &end);
    nsdiff = get_elapsed(&start, &end);
#else
    #error "Unsupported platform"
#endif
    
    avglat = (double)nsdiff / workingset_size / repeat;
}
```

## 最佳实践建议

### 1. 优先使用标准 C 库

```c
// ✅ 推荐：标准 C
malloc(), free()
memset(), memcpy()
printf(), sprintf()

// ⚠️ 谨慎：POSIX，某些 RTOS 可能不支持
getopt(), gettimeofday()
```

### 2. 使用框架提供的功能

```c
// ✅ 推荐：使用框架的时间戳（已跨平台）
#include "get_cpu_timestamp.h"
get_timestamp(), get_rdtsc()

// ✅ 推荐：使用框架的日志（已跨平台）
#include "logging.h"
elogf(), flogf()
```

### 3. 扩展平台抽象层（如果需要）

如果多个 benchmark 都需要相同的平台特定功能，扩展平台抽象层：

```c
// 在 platform_abstraction.h 中添加
int rtbench_get_highres_time(long *sec, long *nsec);

// 在各平台实现
// platform/linux/benchmark_helpers.c
// platform/rt-thread/benchmark_helpers.c
```

### 4. 条件编译作为最后手段

如果只有少数地方需要适配，且功能简单，可以使用条件编译：

```c
#ifdef RT_THREAD_PLATFORM
    // RT-Thread 代码
#elif defined(__linux__)
    // Linux 代码
#endif
```

## 检查清单

在将 benchmark 移植到新平台时，检查以下内容：

- [ ] **时间测量**：`clock_gettime()`, `gettimeofday()` → 使用框架时间戳或条件编译
- [ ] **内存管理**：`malloc()/free()` → 确认 RTOS 是否支持，或使用平台抽象层
- [ ] **文件操作**：`fopen()/fclose()` → 确认 RTOS 是否支持文件系统
- [ ] **参数解析**：`getopt()` → 可能需要替换为简单的参数解析
- [ ] **系统调用**：`sys/mman.h`, `unistd.h` → 确认 RTOS 是否支持
- [ ] **多线程**：如果 benchmark 使用多线程，需要适配 RTOS 的线程 API

## 总结

| 层面 | 是否需要平台适配 | 解决方案 |
|------|----------------|----------|
| **rt-bench 框架** | ✅ 已通过平台抽象层解决 | 使用 `rtbench_*` 接口 |
| **Benchmark 代码** | ⚠️ **需要检查并适配** | 1. 使用标准 C<br>2. 使用框架功能<br>3. 扩展抽象层<br>4. 条件编译 |

**关键点**：
- 框架已经隔离了平台差异
- **但 benchmark 代码如果使用了平台特定 API，仍然需要适配**
- 优先使用标准 C 库和框架提供的功能
- 必要时扩展平台抽象层或使用条件编译


