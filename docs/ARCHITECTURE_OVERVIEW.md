# RTOS-Bench 架构概览

> 文档创建时间: 2026-02-23
> 最后更新: 2026-02-23

本文档描述 RTOS-Bench 框架的整体架构、各组件定位及跨平台编译策略。

---

## 一、框架整体定位

根据《工业操作系统通用基准检测指标体系指导书 v1.5》，RTOS 基准测试分为三大类：

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    工业 RTOS 基准检测指标体系                             │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐         │
│  │   实时性能指标   │  │   可调度性指标   │  │    功耗指标     │         │
│  │  (test-realtime) │  │ (test-schedule) │  │  (test-stress)  │         │
│  └────────┬────────┘  └────────┬────────┘  └────────┬────────┘         │
│           │                    │                    │                   │
│  • 上下文切换延迟      • 截止时间错失率       • CPU/内存压力             │
│  • 中断响应延迟        • UUniFast 任务生成    • 功耗曲线测量             │
│  • 系统调用延迟        • 多任务并发调度       • 热稳定性                 │
│  • IPC 原语开销                                                         │
│    (sem/mutex/mq/mp)                                                   │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
                                    ↓
         ┌──────────────────────────────────────────────────────┐
         │                    典型负载 (Workloads)               │
         │  为上述测试提供"被测负载"，模拟真实工业应用场景          │
         ├──────────────────────────────────────────────────────┤
         │  FAST - 角点检测       EKF  - 卡尔曼滤波             │
         │  EPNP - 位姿估计       ICP  - 点云配准               │
         │  PID  - 控制循环       MODBUS/MQTT - 通信协议         │
         │  CUSUM/EWMA - 异常检测                               │
         └──────────────────────────────────────────────────────┘
```

---

## 二、四个组件的关系与定位

| 组件 | 性质 | 目录位置 | 入口命令 | 说明 |
|------|------|----------|----------|------|
| **test-realtime** | 独立子命令 | `generator/realtime_orig/` | `rtbench test-realtime` | 测量 RTOS 内核的实时性能指标 |
| **test-schedule** | 独立子命令 | `generator/test_schedule.c` | `rtbench test-schedule` | 使用 workloads 验证可调度性 |
| **test-stress** | 独立子命令 | `generator/stress_orig/` | `rtbench test-stress` | 压力测试，配合外部功耗仪使用 |
| **test-cmd** | 独立子命令 | `generator/test_cmd.c` | `rtbench test-cmd` | 测试 Shell 命令注册与执行能力 |
| **workloads** | 负载库 | `workloads/` | `rtbench -b <name>` | 为 test-schedule 和周期执行提供负载 |

**关键区别**：
- `test-realtime/stress` 是**独立测试工具**，不依赖 workloads，各有原始实现
- `test-schedule` **使用 workloads** 作为负载，验证调度器的截止时间保障能力
- `workloads` 是**被测对象**，也可单独运行 (`rtbench -b xxx -p 1 -t 100`)

```
命令结构:
rtbench
├── test-realtime [--multicore]      # 实时性能 → realtime_orig/
├── test-schedule [--cycles N]       # 可调度性 → 调用 workloads
├── test-stress -s <stressor>        # 功耗压力 → stress_orig/
├── test-cmd                         # Shell 命令测试 → test_cmd.c
├── test-all [-o <file>]             # 运行所有测试
├── -b <workload> -p <period>        # 周期负载 → workloads/
└── -L                               # 列出负载
```

---

## 三、代码组织结构

```
RTOS-Bench/
├── generator/                    # 核心框架
│   ├── rtthread_entry.c         # RT-Thread 入口（msh 命令）
│   ├── oneos_entry.c            # OneOS 入口
│   ├── sylixos_entry.c          # SylixOS 入口
│   ├── dongtu_entry.c           # 东土 (Intewell) 入口
│   ├── ruihua_entry.c           # 锐华 (ReWorks) 入口
│   ├── posixlite_entry.c        # 通用 POSIX 入口（无专用入口的平台使用）
│   │
│   ├── test_realtime.c/.h       # 实时性测试 wrapper
│   ├── realtime_orig/           # 实时性测试原始实现
│   │   ├── les/                 # 定时器基础设施
│   │   ├── realtime/            # 单核测试 (test1-test11)
│   │   └── multicore/           # 多核测试
│   │
│   ├── test_stress.c/.h         # 压力测试 wrapper
│   ├── stress_orig/             # stress-ng 移植
│   │   ├── common/
│   │   ├── osal/
│   │   └── stressor/
│   │
│   ├── test_schedule.c/.h       # 可调度性测试（独立实现）
│   ├── test_cmd.c/.h            # Shell 命令支持测试
│   ├── uunifast.c/.h            # UUniFast 算法
│   │
│   ├── workload_registry.c/.h   # 负载注册表
│   ├── periodic_benchmark.c     # 周期执行引擎
│   │
│   └── platform/                # 平台抽象层
│       ├── linux/
│       ├── rt-thread/
│       ├── sylixos/
│       ├── oneos/
│       ├── dongtu/
│       └── ruihua/
│
├── workloads/                   # 典型负载
│   ├── FAST/, EPNP/, EKF/, ICP/, PID/
│   ├── MODBUS/, MQTT/
│   ├── CUSUM/, EWMA/
│   └── rtbench_workloads.cpp    # 统一注册
│
├── run-rtthread.sh              # RT-Thread 一键脚本
├── run-sylixos.sh               # SylixOS 启动脚本
└── extern/                      # 外部依赖
    ├── rt-thread/               # RT-Thread 源码
    └── toolchains/              # 工具链
```

---

## 四、跨平台编译架构

### 4.1 当前支持情况

| 平台 | 入口文件 | 构建系统 | 验证状态 |
|------|----------|----------|----------|
| **RT-Thread** | `rtthread_entry.c` | SCons | ✅ QEMU 验证 |
| **Linux** | `main.c` | Makefile | ✅ 原生支持 |
| **SylixOS** | `sylixos_entry.c` | RealEvo IDE | ⚠️ 需 Windows IDE |
| **OneOS** | `oneos_entry.c` | SCons (OneOS Cube) | ✅ 部分验证 |
| **东土 (Dongtu)** | `dongtu_entry.c` | 厂商 IDE | ⚠️ 待验证 |
| **锐华 (Ruihua)** | `ruihua_entry.c` | 厂商 IDE | ⚠️ VxWorks 兼容 |

### 4.2 跨平台编译的三个层次

```
┌─────────────────────────────────────────────────────────────────────────┐
│                        跨平台编译的三个层次                              │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  Layer 1: 工具链 (Toolchain)                                            │
│  ─────────────────────────────                                          │
│  • arm-none-eabi-gcc (Cortex-M)                                        │
│  • aarch64-none-elf-gcc (ARM64)                                        │
│  • x86_64-linux-gnu-gcc (x86)                                          │
│  → 由 CPU 架构决定，与 RTOS 无关                                        │
│                                                                         │
│  Layer 2: 构建系统 (Build System)                                       │
│  ──────────────────────────────                                         │
│  • SCons + SConscript (RT-Thread, OneOS)                               │
│  • CMake + CMakeLists.txt (FreeRTOS, Zephyr)                           │
│  • IDE 工程文件 (Keil, IAR, RealEvo)                                   │
│  → 由 RTOS 生态决定，需要适配                                           │
│                                                                         │
│  Layer 3: 入口与 API (Entry & Platform API)                             │
│  ──────────────────────────────────────────                             │
│  • 入口函数: main() vs msh_cmd vs INIT_APP_EXPORT                       │
│  • 定时器: POSIX timer vs rt_timer vs xTimerCreate                     │
│  • 同步: sem_t vs rt_sem_t vs SemaphoreHandle_t                        │
│  → 由 RTOS API 决定，需要平台抽象层                                     │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

### 4.3 SCons vs Makefile vs IDE 的区别

**Makefile** - 传统 Unix 构建
```makefile
CC = arm-none-eabi-gcc
CFLAGS = -mcpu=cortex-m4 -mthumb
OBJS = main.o workload.o

all: firmware.elf
    $(CC) $(CFLAGS) $(OBJS) -o $@
```

**SCons** - Python 驱动的构建（RT-Thread/OneOS 使用）
```python
# SConscript - 类似 Makefile 但用 Python 语法
from building import *
src = Glob('*.c')
group = DefineGroup('myapp', src, depend=[''], CPPPATH=[cwd])
Return('group')
```
- RT-Thread 使用 SCons 是因为它支持**模块化组件管理**（menuconfig + Kconfig）
- `DefineGroup()` 把源文件注册到全局构建系统

**IDE 工程** - 图形化封装
- 内置工具链（不需要手动配置 PATH）
- 点击按钮 = 执行 `make` 或 `scons`
- 工程文件（.project, .uvprojx）描述源文件列表、编译选项

### 4.4 适配一个新平台需要做什么

```
Step 1: 创建入口文件
        generator/<platform>_entry.c
        • 解析命令行参数
        • 调用 test_realtime_run() / test_stress_run() / 周期执行器

Step 2: 实现平台抽象层
        generator/platform/<platform>/
        ├── timer.c      # 定时器 API
        ├── sync.c       # 信号量/互斥锁
        ├── scheduler.c  # 优先级/绑核
        ├── timestamp.c  # 高精度时间
        └── signal.c     # 信号处理(可选)

Step 3: 集成到 RTOS 构建系统
        • RT-Thread: 添加到 BSP 的 SConscript
        • OneOS: 添加到项目的 SConscript
        • FreeRTOS: 添加到 CMakeLists.txt
        • 厂商 IDE: 手动添加源文件到工程

Step 4: 处理 POSIX 缺口
        • clock_gettime 不支持 CLOCK_MONOTONIC → shadow header
        • pthread_attr_setinheritsched 缺失 → stub 函数
```

---

## 五、待完善的点

### 5.1 Workloads 跨平台编译

**现状**：Workloads 已经在 RT-Thread 上编译运行

**问题**：
- C++ workloads (EPNP, EKF, ICP) 需要 `extern "C"` 和 `.ctors` 链接脚本
- 网络 workloads (MODBUS, MQTT) 需要 SAL/socket 支持
- OneOS 验证：CUSUM, EWMA, FAST, PID 可编译；EKF/EPNP/ICP 需要 Eigen 移植

**解决方案**：
```python
# 在各平台的 SConscript 中条件编译
if GetDepend(['PKG_USING_EIGEN']):
    src += Glob('workloads/EKF/*.cpp')
```

### 5.2 统一的跨平台构建入口

**理想方案**：
```
RTOS-Bench/
├── CMakeLists.txt           # 顶层 CMake (用于 Linux/FreeRTOS/Zephyr)
├── SConscript               # 顶层 SCons (用于 RT-Thread/OneOS)
├── Kconfig                  # 组件配置
└── ports/
    ├── rtthread/            # RT-Thread BSP 集成示例
    ├── oneos/               # OneOS 项目集成示例
    ├── dongtu/              # 东土项目集成示例
    ├── ruihua/              # 锐华项目集成示例
    └── sylixos/             # SylixOS IDE 工程模板
```

**当前差距**：
- 没有统一的 CMakeLists.txt
- 各平台适配散落在各处
- 缺少"如何集成到厂商 IDE"的详细指南

### 5.3 厂商 IDE 集成模式

**现实情况**：
```
┌─────────────────────────────────────────────────────────────────────────┐
│  厂商 RTOS 通常提供自己的 IDE，不支持外部构建系统                         │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  厂商 IDE 的工作流：                                                     │
│  1. 创建工程 → 生成工程文件 (.project, .uvprojx)                         │
│  2. 添加源文件 → 手动拖入或 import                                       │
│  3. 配置编译选项 → GUI 界面                                             │
│  4. 一键编译 → IDE 调用内置工具链                                        │
│                                                                         │
│  我们的代码如何适配？                                                    │
│  ─────────────────────                                                  │
│  1. 提供"源文件清单"让用户手动添加                                       │
│  2. 提供"需要定义的宏"(RT_THREAD_PLATFORM, ONEOS_PLATFORM)              │
│  3. 提供"需要包含的头文件路径"                                           │
│  4. 提供"入口函数适配示例"                                              │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

### 5.4 建议的改进方向

| 优先级 | 改进项 | 说明 |
|--------|--------|------|
| P0 | 完善 `docs/PORTING_GUIDE.md` | 详细的移植指南 |
| P0 | 提供源文件清单脚本 | 自动生成各平台需要的文件列表 |
| P1 | 添加 CMakeLists.txt | 支持 FreeRTOS/Zephyr 生态 |
| P1 | 完善 stress/realtime 功能 | 目前是移植代码，可能需要调试 |
| P2 | 单元测试框架 | 验证各平台抽象层正确性 |
| P2 | CI/CD 多平台构建 | GitHub Actions 自动验证 |

---

## 六、厂商 IDE 适配指南

当你在 Windows 上使用厂商 IDE 编译时，需要做以下适配：

```
你的代码仓库                    厂商 IDE 工程
─────────────                   ─────────────
generator/                      → 添加到 "Source Files"
├── test_schedule.c            → 添加
├── test_realtime.c            → 添加
├── workload_registry.c        → 添加
├── platform/xxx/              → 添加对应平台
│
workloads/                      → 添加到 "Source Files"
├── CUSUM/*.c                  → 添加
├── EWMA/*.c                   → 添加
│
需要的宏定义                    → 在 IDE 的 "Preprocessor" 设置
├── XXX_PLATFORM               → 添加
│
入口适配                        → 使用对应平台的 xxx_entry.c
├── RT-Thread: rtthread_entry.c
├── SylixOS: sylixos_entry.c
├── 东土: dongtu_entry.c
└── 锐华: ruihua_entry.c
```

**关键点**：
- 每个 RTOS 的 shell/命令注册方式不同
- RT-Thread: `MSH_CMD_EXPORT(rtbench, ...)`
- OneOS: `SH_CMD_EXPORT(rtbench, ...)`
- FreeRTOS: 没有内置 shell，需要自己实现或用 FreeRTOS+CLI

这不是"改动代码"，而是**写一个平台入口适配器**。代码主体是跨平台的。

---

## 七、参考资料

- [AGENTS.md](../AGENTS.md) - 开发资产与约定
- [SCHEDULE.md](SCHEDULE.md) - 可调度性测试指南
- [REALTIME.md](REALTIME.md) - 实时性能测试指南
- [STRESS.md](STRESS.md) - 压力测试指南
- [CMD.md](CMD.md) - Shell 命令支持测试指南
- 工业操作系统通用基准检测指标体系指导书 v1.5
