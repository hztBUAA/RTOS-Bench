# 压力/功耗测试 (test-stress) 使用指南

本文档说明 RTOS-Bench 中的压力与功耗测试功能 (`test-stress`)，该功能基于工业操作系统通用基准检测指标体系指导书实现。

## 概述

`test-stress` 是 RTOS-Bench 的独立子命令，集成了 `rtos_stress`（stress-ng 的 RTOS 移植版），用于对系统进行高负载压力测试，同时配合外部功耗测量仪器评估系统在高负载下的功耗特性。

**核心能力**：
- 多种压力源 (stressor) 覆盖 CPU 计算、内存、I/O 等维度
- 可配置持续时间和并发 worker 数
- 输出 bogo-ops（标准化操作数）作为性能基准
- 支持逐个或全部 stressor 顺序执行

## 命令用法

### 基本用法

```bash
# RT-Thread msh 中
msh /> rtbench test-stress

# Linux 终端中
./rtbench test-stress
```

### 可选参数

```bash
rtbench test-stress [OPTIONS]

OPTIONS:
  -s <name>     指定 stressor 名称 (默认: cpu)
  -t <sec>      持续时间，单位秒 (默认: 10)
  -l, --list    列出所有可用 stressors
  -q            安静模式，减少输出
```

### 示例

```bash
# 默认 CPU 压力，10 秒
msh /> rtbench test-stress

# 指定 matrix stressor，30 秒
msh /> rtbench test-stress -s matrix -t 30

# 内存分配压力，5 秒
msh /> rtbench test-stress -s malloc -t 5

# 顺序运行全部 stressor
msh /> rtbench test-stress -s all -t 10

# 浮点运算压力
msh /> rtbench test-stress -s fp -t 15

# 列出所有可用 stressor
msh /> rtbench test-stress -l
```

## 可用 Stressors

### CPU 密集型

| Stressor | 说明 |
|----------|------|
| `cpu` | 综合 CPU 计算压力（整数运算、移位、逻辑等） |
| `matrix` | 矩阵乘法运算 |
| `prime` | 素数计算（计算密集型） |
| `trig` | 三角函数运算（sin/cos/tan） |
| `fp` | 浮点运算（加减乘除、平方根等） |
| `bitops` | 位操作（移位、翻转、计数） |
| `bsearch` | 二分查找 |
| `qsort` | 快速排序 |
| `str` | 字符串操作 (strlen/strcpy/strcmp) |
| `vecmath` | 向量数学运算 |

### 内存密集型

| Stressor | 说明 |
|----------|------|
| `vm` | 虚拟内存压力（大块内存读写） |
| `malloc` | 频繁 malloc/free 分配释放 |
| `memcpy` | 内存拷贝吞吐量 |
| `memthrash` | 内存读写模式压力 |
| `stream` | STREAM 内存带宽测试 |
| `stack` | 栈空间深度压力 |
| `ptr-chase` | 指针追踪（缓存命中率压力） |

### I/O 密集型

| Stressor | 说明 |
|----------|------|
| `hdd` | 磁盘/存储 I/O |
| `open` | 文件打开/关闭 |
| `pipe` | 管道读写 |
| `copy-file` | 文件拷贝 |
| `fstat` | 文件状态查询 |
| `rename` | 文件重命名 |
| `dentry` | 目录项操作 |
| `unlink` | 文件删除 |

### 其他

| Stressor | 说明 |
|----------|------|
| `context` | 上下文切换压力 |
| `atomic` | 原子操作 |
| `all` | 顺序执行全部 stressor |

> **注意**: 并非所有 stressor 在每个 RTOS 平台上都可用。I/O 类 stressor 需要文件系统支持，内存类 stressor 受限于可用 RAM 大小。

## 输出格式

```
[test-stress] Starting stress/power test
  Stressor: cpu, Duration: 10 seconds

rtos_stress: Starting CPU stressor...
rtos_stress: info: [1] cpu stressor running
rtos_stress: info: [1] cpu: 12345 bogo ops in 10.00s (1234.50 bogo ops/s)

[test-stress] Completed
```

当使用 `-s all` 时，每个 stressor 依次运行并输出结果：

```
[test-stress] Starting stress/power test
  Stressor: all, Duration: 10 seconds

--- Stressor: cpu ---
rtos_stress: info: [1] cpu: 12345 bogo ops in 10.00s

--- Stressor: matrix ---
rtos_stress: info: [1] matrix: 8901 bogo ops in 10.00s

--- Stressor: prime ---
rtos_stress: info: [1] prime: 3456 bogo ops in 10.00s

... (依次执行每个 stressor)
```

## 结果说明

### Bogo Ops

`bogo-ops`（bogus operations）是 stress-ng 定义的标准化操作计数，表示在测试时间内完成的迭代次数。数值本身没有绝对含义，但可用于：

- **同平台纵向对比**: 相同硬件上不同 RTOS 的压力吞吐量
- **性能回归检测**: 固件更新前后的性能变化
- **功耗关联分析**: 配合功耗仪读数，分析"每焦耳操作数"

### JSON 导出

在 `test-all` 模式下，stress 结果会自动纳入 JSON 导出：

```json
{
  "modules": {
    "test-stress": {
      "status": "passed",
      "duration_sec": 10.000,
      "stressors": [
        {
          "name": "cpu",
          "type": "cpu",
          "bogo_ops": 12345,
          "duration_sec": 10.000
        }
      ]
    }
  }
}
```

## 架构设计

### 与 stress-ng 的关系

`rtos_stress` 是 stress-ng 的 RTOS 移植版本，保留了核心 stressor 实现并替换了 OS 层：

```
┌─────────────────────────────────────────────────────┐
│                   rtbench CLI                        │
│   rtbench test-stress -s cpu -t 10                  │
├─────────────────────────────────────────────────────┤
│              test_stress.h / .c                      │
│   test_stress_run_stressor("cpu", 10)               │
├─────────────────────────────────────────────────────┤
│            stress_bench.c (adapter)                  │
│   stress_bench_run_stressor("cpu", 10)              │
│   -> stress_ng_main(argc, argv)                     │
├─────────────────────────────────────────────────────┤
│              rtos_stress 核心                        │
│   ┌──────────┐  ┌──────────┐  ┌──────────┐         │
│   │stress-cpu│  │stress-vm │  │stress-hdd│  ...     │
│   └──────────┘  └──────────┘  └──────────┘         │
├─────────────────────────────────────────────────────┤
│                    OSAL 层                           │
│   platforms/  -> RT-Thread / POSIX                   │
│   (线程创建、内存分配、时间获取等)                    │
└─────────────────────────────────────────────────────┘
```

### 数据结构

```c
/* 压力源类型枚举 */
typedef enum {
    STRESS_TYPE_CPU,        /* CPU 计算 */
    STRESS_TYPE_MATRIX,     /* 矩阵运算 */
    STRESS_TYPE_VM,         /* 虚拟内存 */
    STRESS_TYPE_MALLOC,     /* 内存分配 */
    STRESS_TYPE_MEMCPY,     /* 内存拷贝 */
    STRESS_TYPE_PRIME,      /* 素数计算 */
    STRESS_TYPE_TRIG,       /* 三角函数 */
    STRESS_TYPE_FP,         /* 浮点运算 */
    STRESS_TYPE_ALL,        /* 全部顺序执行 */
} stress_type_t;

/* 配置 */
struct test_stress_config {
    stress_type_t type;     /* stressor 类型 */
    int duration_sec;       /* 持续时间(秒) */
    int num_workers;        /* 并发 worker 数 (0=自动) */
    int quiet;              /* 安静模式 */
};

/* 单 stressor 结果 */
struct test_stress_result {
    const char *name;       /* stressor 名称 */
    double duration;        /* 实际持续时间(秒) */
    uint64_t bogo_ops;      /* bogo 操作数 */
    double bogo_ops_per_sec;/* 每秒操作数 */
    int success;            /* 0=成功 */
};
```

## 关键源文件

| 文件 | 说明 |
|------|------|
| `generator/test_stress.h` | 接口定义和数据结构 |
| `generator/test_stress.c` | test-stress 命令封装 |
| `generator/stress_orig/stress_bench.c` | stress-ng 适配器 |
| `generator/stress_orig/stressor/` | 各 stressor 实现 (27 种) |
| `generator/stress_orig/osal/` | RTOS 抽象层 |
| `generator/stress_orig/common/stress-ng.h` | stress-ng 核心头文件 |
| `generator/rtthread_entry.c` | RT-Thread 命令入口 |

## 平台支持

| 平台 | 状态 | 说明 |
|------|------|------|
| RT-Thread | ✅ 支持 | 通过 OSAL 层适配，QEMU aarch64 已验证 |
| Linux | ✅ 支持 | 原生 POSIX 接口 |
| SylixOS | ✅ 接入 | POSIX 兼容，入口已对接 |
| Dongtu | ✅ 接入 | POSIX 兼容，入口已对接 |
| OneOS | 🔄 待验证 | 部分 stressor 可能受限 |
| Ruihua | 🔄 待验证 | VxWorks 兼容层 |

## 功耗测试使用方法

`test-stress` 主要用于配合外部功耗测量仪器进行功耗评估：

### 测试流程

1. **连接功耗仪**: 将功耗测量设备（如功耗分析仪、示波器+电流探头）连接到目标板电源引脚
2. **选择 stressor**: 根据待测场景选择对应的 stressor
3. **执行测试**: 运行较长时间（建议 ≥30 秒）以获得稳定的功耗读数
4. **记录数据**: 同时记录 bogo-ops 和功耗仪读数

### 典型功耗测试用例

```bash
# CPU 满载功耗
msh /> rtbench test-stress -s cpu -t 60

# 内存密集型功耗
msh /> rtbench test-stress -s memcpy -t 60

# I/O 密集型功耗（需文件系统）
msh /> rtbench test-stress -s hdd -t 60

# 综合功耗画像（逐项测试）
msh /> rtbench test-stress -s all -t 30
```

### 功耗指标计算

```
能效比 = bogo_ops / 平均功耗(W) / 时间(s)
       = bogo_ops_per_sec / 平均功耗(W)
       单位: bogo-ops/W
```

## 故障排查

### 问题：Stressor 运行后 bogo_ops 为 0

**可能原因**:
1. 持续时间太短（< 1 秒）
2. Stressor 内部遇到不支持的系统调用

**解决方案**:
- 增加 `-t` 时间（至少 5 秒）
- 检查 RTOS 是否支持 stressor 所需的 API（如 `malloc`、`open`）

### 问题：I/O stressor 报错

**可能原因**:
1. RTOS 未启用文件系统
2. 存储设备未挂载

**解决方案**:
- 确认 RT-Thread `.config` 中启用了 DFS 和 elmFAT/RomFS
- 确认存储设备（SD 卡、Flash）已正确挂载

### 问题：内存类 stressor 崩溃

**可能原因**:
1. RTOS 可用堆内存不足
2. `vm` stressor 请求的内存超出系统容量

**解决方案**:
- 增大 RTOS 堆配置（如 `RT_HEAP_SIZE`）
- 使用 `malloc` 替代 `vm`（分配粒度更小）

### 问题：线程创建失败

**可能原因**:
1. 系统线程数达到上限
2. 线程栈配置不足

**解决方案**:
- 增大 `PTHREAD_NUM_MAX` 或 `RT_THREAD_NUM_MAX`
- 确认没有其他高负载任务同时运行

## 参考资料

1. 工业操作系统通用基准检测指标体系指导书 v1.5
2. [stress-ng](https://github.com/ColinIanKing/stress-ng) - 原始 Linux 压力测试工具
3. RTOS-Bench 架构概览: [ARCHITECTURE_OVERVIEW.md](ARCHITECTURE_OVERVIEW.md)
