# RT-Bench RT-Thread 移植实施计划

## 当前任务状态

根据你的待办事项，以下是具体的实施步骤：

## 任务 1: 【配置环境】rt-bench 在 rt-thread 上跑通

### 1.1 创建平台抽象层结构

**目标**：创建可扩展的平台抽象层，支持 Linux 和 RT-Thread

**步骤**：

1. **创建目录结构**：
```bash
mkdir -p generator/platform/linux
mkdir -p generator/platform/rt-thread
```

2. **创建平台抽象头文件**（已完成）：
   - `generator/platform_abstraction.h` - 定义统一接口

3. **实现 Linux 平台适配**（基于现有代码）：
   - `generator/platform/linux/timer.c`
   - `generator/platform/linux/sync.c`
   - `generator/platform/linux/scheduler.c`
   - `generator/platform/linux/timestamp.c`

4. **实现 RT-Thread 平台适配**：
   - `generator/platform/rt-thread/timer.c`
   - `generator/platform/rt-thread/sync.c`
   - `generator/platform/rt-thread/scheduler.c`
   - `generator/platform/rt-thread/timestamp.c`

### 1.2 修改核心文件使用抽象层

**需要修改的文件**：

1. **`generator/periodic_benchmark.c`**：
   - 替换 `timer_create/timer_settime` → `rtbench_timer_create/rtbench_timer_settime`
   - 替换 `sem_init/sem_wait/sem_post` → `rtbench_sem_*`
   - 替换信号处理 → 使用定时器回调

2. **`generator/main.c`**：
   - 替换 `sched_setattr` → `rtbench_set_priority/rtbench_set_deadline`
   - 替换 `sched_setaffinity` → `rtbench_set_affinity`

3. **`generator/get_cpu_timestamp.c`**：
   - 替换 `get_rdtsc/get_timestamp` → `rtbench_get_rdtsc/rtbench_get_timestamp`

### 1.3 修改编译系统

**`generator/Makefile`** 需要添加：

```makefile
# Platform selection
PLATFORM ?= linux

ifeq ($(PLATFORM),rt-thread)
  CFLAGS += -DRT_THREAD_PLATFORM
  CFLAGS += -I$(RT_THREAD_ROOT)/include
  LDFLAGS += -L$(RT_THREAD_ROOT)/lib
  PLATFORM_SRC = platform/rt-thread/*.c
else
  CFLAGS += -DLINUX_PLATFORM
  PLATFORM_SRC = platform/linux/*.c
endif

# Include platform abstraction
CFLAGS += -I.
OBJS += $(PLATFORM_SRC:.c=.o)
```

### 1.4 测试最小化版本

创建一个简单的测试负载验证框架：

```c
// test_benchmark.c
#include "periodic_benchmark.h"

int benchmark_init(int parameters_num, void **parameters) {
    return 0;
}

void benchmark_execution(int parameters_num, void **parameters) {
    // 简单的空循环，验证框架能正常运行
    volatile int i;
    for (i = 0; i < 1000; i++);
}

void benchmark_teardown(int parameters_num, void **parameters) {
}
```

编译并运行：
```bash
# Linux 平台
make PLATFORM=linux test_benchmark

# RT-Thread 平台
make PLATFORM=rt-thread test_benchmark
```

## 任务 2: 【调研】rt-bench在不同rtos上抽象层适配

### 2.1 分析需要抽象的接口

**核心接口分类**：

1. **时间管理**：
   - 定时器创建/设置/删除
   - 时间戳获取（高精度和低精度）

2. **同步原语**：
   - 信号量
   - 事件/信号（可选）

3. **调度管理**：
   - 优先级设置
   - 调度策略（FIFO, Deadline）
   - CPU 亲和性

4. **系统服务**：
   - 内存管理（可选，用于内存监控）
   - 性能计数器（可选，硬件相关）

### 2.2 设计通用抽象层

参考 `platform_abstraction.h` 的设计，确保：
- 接口简洁，易于实现
- 支持条件编译（通过宏定义）
- 提供合理的默认实现

### 2.3 其他 RTOS 支持

未来可以考虑支持：
- **FreeRTOS**：类似 RT-Thread，使用软件定时器和信号量
- **Zephyr**：有丰富的 API，可能需要更多适配
- **VxWorks**：商业 RTOS，API 较完善

## 任务 3: 【代码】负载转换

### 3.1 分析现有负载

**步骤**：

1. **找到学长的负载代码**（应该在某个 application 目录）
2. **分析代码结构**：
   - 识别初始化代码
   - 识别执行代码
   - 识别清理代码
   - 识别全局状态

3. **确定转换策略**：
   - 哪些数据在 `benchmark_init` 中初始化
   - 哪些状态需要在 `benchmark_execution` 中重置
   - 哪些资源在 `benchmark_teardown` 中释放

### 3.2 实现转换

**模板**：

```c
// 原始负载的全局状态
static struct workload_state {
    // 初始化时分配的数据
    void *data_buffer;
    size_t buffer_size;
    
    // 执行时的临时状态（需要重置）
    int iteration_count;
    // ...
} g_state;

int benchmark_init(int parameters_num, void **parameters) {
    // 1. 解析参数（从 parameters 数组）
    // 2. 分配内存
    g_state.buffer_size = parse_size(parameters);
    g_state.data_buffer = malloc(g_state.buffer_size);
    
    // 3. 初始化数据结构
    init_workload_data(g_state.data_buffer);
    
    return 0;
}

void benchmark_execution(int parameters_num, void **parameters) {
    // 1. 重置状态（重要！）
    g_state.iteration_count = 0;
    reset_workload_state(&g_state);
    
    // 2. 执行负载
    run_workload(&g_state);
}

void benchmark_teardown(int parameters_num, void **parameters) {
    // 1. 清理资源
    if (g_state.data_buffer) {
        free(g_state.data_buffer);
        g_state.data_buffer = NULL;
    }
    
    // 2. 其他清理工作
    cleanup_workload();
}
```

### 3.3 验证转换

**检查清单**：
- [ ] `benchmark_execution` 可以多次调用而不出错
- [ ] 每次执行的结果一致（或可预期）
- [ ] 没有内存泄漏
- [ ] 参数解析正确

## 任务 4: 测试逻辑验证

### 4.1 RT-Bench 测试流程

**标准流程**：

1. **启动测试**：
   ```bash
   ./benchmark -p 1.0 -d 0.5 -t 10 -o results.csv
   ```
   - `-p 1.0`: 周期 1 秒
   - `-d 0.5`: 截止时间 0.5 秒
   - `-t 10`: 执行 10 个周期
   - `-o results.csv`: 输出文件

2. **执行过程**：
   - 每个周期：启动 → 执行负载 → 记录时间戳 → 输出结果
   - 如果负载在 deadline 前完成：记录成功
   - 如果负载超过 deadline：记录失败

3. **结果输出**：
   - CSV 格式，包含：
     - 周期开始/结束时间
     - 任务完成时间
     - Deadline 状态（是否满足）
     - 性能计数器（如果支持）

### 4.2 在 RT-Thread 上运行

**步骤**：

1. **编译**：
   ```bash
   cd generator
   make PLATFORM=rt-thread
   ```

2. **运行**：
   - 如果 RT-Thread 支持 shell：直接在 shell 中运行
   - 否则：需要集成到 RT-Thread 应用中

3. **验证输出**：
   - 检查 CSV 文件格式
   - 验证时间戳合理性
   - 对比 Linux 版本的输出

## 实施优先级

### 第一阶段（核心功能）
1. ✅ 创建平台抽象层接口定义
2. ⏳ 实现 RT-Thread 定时器适配
3. ⏳ 实现 RT-Thread 信号量适配
4. ⏳ 实现 RT-Thread 时间戳适配
5. ⏳ 修改 `periodic_benchmark.c` 使用抽象层
6. ⏳ 测试最小化版本

### 第二阶段（完整功能）
1. ⏳ 实现调度策略适配
2. ⏳ 实现 CPU 亲和性适配
3. ⏳ 负载转换（学长的负载）
4. ⏳ 完整测试和验证

### 第三阶段（优化和扩展）
1. ⏳ 性能计数器支持（如果硬件支持）
2. ⏳ 内存监控支持
3. ⏳ 其他 RTOS 支持

## 下一步行动

**立即可以开始的工作**：

1. **创建 RT-Thread 平台适配实现**：
   ```bash
   # 创建文件
   touch generator/platform/rt-thread/timer.c
   touch generator/platform/rt-thread/sync.c
   touch generator/platform/rt-thread/timestamp.c
   ```

2. **实现 RT-Thread 定时器适配**（参考 RT-Thread 文档）：
   - 使用 `rt_timer_create()` 创建定时器
   - 在定时器回调中调用用户回调函数

3. **实现 RT-Thread 信号量适配**：
   - 使用 `rt_sem_create()` 创建信号量
   - 使用 `rt_sem_take/release()` 进行同步

4. **修改 `periodic_benchmark.c`**：
   - 添加 `#include "platform_abstraction.h"`
   - 替换所有平台特定调用

## 参考资源

- **RT-Thread 定时器 API**：https://www.rt-thread.org/document/site/programming-manual/timer/timer/
- **RT-Thread 信号量 API**：https://www.rt-thread.org/document/site/programming-manual/ipc1/ipc1/
- **RT-Bench 扩展指南**：`docs/source/3-Extending_rt-bench.markdown`

## 注意事项

1. **线程安全**：确保 RT-Thread 版本在多线程环境下正确工作
2. **时间精度**：RT-Thread 的时间精度可能与 Linux 不同，需要验证
3. **内存管理**：RT-Thread 可能使用不同的内存管理方式
4. **调试**：RT-Thread 的调试工具可能与 Linux 不同，需要熟悉

