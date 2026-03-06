# 压力/功耗测试 (test-stress) 使用指南

本文档说明 RTOS-Bench 中的压力与功耗测试功能 (`test-stress`)，该功能基于工业操作系统通用基准检测指标体系指导书实现。

## 概述

`test-stress` 是 RTOS-Bench 的独立子命令，集成了 `rtos_stress`（stress-ng 的 RTOS 移植版），通过内置 Jobfile 对系统进行 **5 级递增负载** 的压力测试，同时配合外部功耗测量仪器评估系统在高负载下的功耗特性。

**核心能力**：
- 3 类内置 Job（cpu / memory / file），覆盖 27 种 stressor
- 每个 Job 包含 5 个 Stage（递增并发），自动执行
- 输出 `duration_sec`（执行时间）作为主要度量指标
- 结构化 JSON 中间结果，含 `stage` 字段，便于分级对比

## 命令用法

### 基本用法

```bash
# RT-Thread msh 中
msh /> rtbench test-stress

# 等效于
msh /> rtbench test-stress --job all
```

### 可选参数

```bash
rtbench test-stress [OPTIONS]

OPTIONS:
  --job <name>    指定 job 名称: cpu, memory, file, all (默认: all)
  -l, --list      列出所有可用 jobs
  -q              安静模式，减少输出
```

### 示例

```bash
# 运行全部 job（CPU + Memory + File，共 135 个 stressor 运行）
msh /> rtbench test-stress

# 仅运行 CPU job（13 stressors x 5 stages = 65 次）
msh /> rtbench test-stress --job cpu

# 仅运行 Memory job（6 stressors x 5 stages = 30 次）
msh /> rtbench test-stress --job memory

# 仅运行 File I/O job（8 stressors x 5 stages = 40 次）
msh /> rtbench test-stress --job file

# 列出所有可用 job
msh /> rtbench test-stress -l
```

## 内置 Job 说明

### CPU Job (`--job cpu`)

13 个 stressor，5 级递增并发（Stage 1-5），共 65 次运行。

| Stressor | 说明 |
|----------|------|
| `cpu` | 综合 CPU 计算压力 |
| `matrix` | 矩阵乘法运算 |
| `qsort` | 快速排序 |
| `atomic` | 原子操作 |
| `bitops` | 位操作 |
| `bsearch` | 二分查找 |
| `context` | 上下文切换 |
| `fp` | 浮点运算 |
| `prime` | 素数计算 |
| `stack` | 栈深度压力 |
| `str` | 字符串操作 |
| `trig` | 三角函数 |
| `vecmath` | 向量数学运算 |

### Memory Job (`--job memory`)

6 个 stressor，5 级递增并发，共 30 次运行。

| Stressor | 说明 |
|----------|------|
| `memcpy` | 内存拷贝吞吐量 |
| `stream` | STREAM 内存带宽 |
| `vm` | 虚拟内存读写 |
| `malloc` | 频繁 malloc/free |
| `memthrash` | 内存读写模式压力 |
| `ptr-chase` | 指针追踪（缓存命中率） |

### File Job (`--job file`)

8 个 stressor，5 级递增并发，共 40 次运行。

| Stressor | 说明 |
|----------|------|
| `hdd` | 磁盘/存储 I/O |
| `open` | 文件打开/关闭 |
| `copy-file` | 文件拷贝 |
| `unlink` | 文件删除 |
| `fstat` | 文件状态查询 |
| `dentry` | 目录项操作 |
| `rename` | 文件重命名 |
| `pipe` | 管道读写 |

## Stage（负载级别）说明

每个 Job 的 Jobfile 定义了 5 个 Stage，Stage 编号 1-5 对应递增的并发度（`-c` 参数）：

| Stage | 并发度 | 含义 |
|-------|--------|------|
| 1 | 1 | 单线程基准 |
| 2 | 2 | 双线程并发 |
| 3 | 3 | 中等并发 |
| 4 | 4 | 较高并发 |
| 5 | 5 | 满载并发 |

> **注意**：部分 stressor 因资源限制，实际并发度可能低于 Stage 编号（如 `vm` 在 Stage 5 使用 `-c 3`）。

## 输出格式

### 终端输出

运行时终端会显示进度条和汇总表格：

```
Initializing Job: stored_jobfile_cpu.txt...
Running Job: [=========>              ] 5/65 (7%) -> prime

========================================================================
 Job: stored_jobfile_cpu.txt [Built-in]
========================================================================
 Stressor    | Stage | Bogo Ops     | Time(s)  | Metric
------------------------------------------------------------------------
 cpu         | 1     | 2800         | 12.34    | 226.80 ops/s
 matrix      | 1     | 6400         | 8.91     | N/A
 ...
 cpu         | 2     | 4900         | 18.50    | 264.86 ops/s
 ...
------------------------------------------------------------------------
Total Run Time: 25m 13s
```

### JSON 中间结果

在 `test-all` 或手动导出时，stress 结果保存为：

```json
{
  "modules": {
    "test-stress": {
      "status": "passed",
      "duration_sec": 1500.000,
      "stressors": [
        {
          "name": "cpu",
          "type": "cpu",
          "stage": 1,
          "bogo_ops": 2800,
          "duration_sec": 12.340,
          "metric_value": 226.800,
          "metric_unit": "ops/s"
        },
        {
          "name": "matrix",
          "type": "cpu",
          "stage": 1,
          "bogo_ops": 6400,
          "duration_sec": 8.910
        },
        {
          "name": "memcpy",
          "type": "memory",
          "stage": 1,
          "bogo_ops": 15000,
          "duration_sec": 5.670,
          "metric_value": 345.600,
          "metric_unit": "MB/s"
        }
      ]
    }
  }
}
```

### 展平输出

通过 `flatten_rtbench_result.py` 展平后，每个 stressor 的每个 stage 会生成唯一路径：

```
stressors.cpu_s1.duration_sec = 12.340 (sec, min)
stressors.cpu_s2.duration_sec = 18.500 (sec, min)
stressors.memcpy_s1.duration_sec = 5.670 (sec, min)
```

- `_s1` ~ `_s5` 后缀表示 Stage 编号
- `duration_sec` 为主要输出指标（执行时间）
- `bogo_ops` 和 `stage` 字段在展平时被跳过

## 架构设计

```
┌─────────────────────────────────────────────────────┐
│                   rtbench CLI                        │
│   rtbench test-stress --job cpu                     │
├─────────────────────────────────────────────────────┤
│              test_stress.h / .c                      │
│   test_stress_run_job("cpu")                        │
│   -> handle_job_command_ex("cpu", buf, max)         │
├─────────────────────────────────────────────────────┤
│           stress-ng.c (engine layer)                 │
│   handle_job_command_ex()                           │
│   -> stress_jobfile_exec_ex() per job               │
│   -> stress_run_one_job() per stressor line         │
├─────────────────────────────────────────────────────┤
│              rtos_stress 核心                        │
│   ┌──────────┐  ┌──────────┐  ┌──────────┐         │
│   │stress-cpu│  │stress-vm │  │stress-hdd│  ...     │
│   └──────────┘  └──────────┘  └──────────┘         │
├─────────────────────────────────────────────────────┤
│                    OSAL 层                           │
│   platforms/  -> RT-Thread / POSIX                   │
└─────────────────────────────────────────────────────┘
```

## 关键源文件

| 文件 | 说明 |
|------|------|
| `generator/test_stress.h` | 接口定义 (`test_stress_run_job`, `test_stress_get_job_results`) |
| `generator/test_stress.c` | test-stress 命令封装 |
| `generator/stress_orig/common/stress-ng.c` | 引擎层 (`handle_job_command_ex`, `stress_jobfile_exec_ex`) |
| `generator/stress_orig/common/stress_stored_job.c` | 内置 Jobfile 数据 (3 个 Job, 各 5 Stage) |
| `generator/stress_orig/stressor/` | 各 stressor 实现 (27 种) |
| `generator/stress_orig/osal/` | RTOS 抽象层 |
| `generator/result_export.h/c` | JSON 结果序列化 |
| `generator/rtthread_entry.c` | RT-Thread 命令入口 |
| `utils/flatten_rtbench_result.py` | 结果展平工具 |

## 平台支持

| 平台 | 状态 | 说明 |
|------|------|------|
| RT-Thread | 支持 | 通过 OSAL 层适配，QEMU aarch64 已验证 |
| Linux | 支持 | 原生 POSIX 接口 |
| SylixOS | 接入 | POSIX 兼容，入口已对接 |
| Dongtu | 接入 | POSIX 兼容，入口已对接 |
| OneOS | 待验证 | 部分 stressor 可能受限 |
| Ruihua | 待验证 | VxWorks 兼容层 |

## 功耗测试使用方法

`test-stress` 主要用于配合外部功耗测量仪器进行功耗评估：

### 测试流程

1. **连接功耗仪**: 将功耗测量设备连接到目标板电源引脚
2. **选择 Job**: 根据待测场景选择对应的 job
3. **执行测试**: 运行 job（自动包含 5 级递增负载）
4. **记录数据**: 同时记录各 Stage 的执行时间和功耗仪读数

### 典型功耗测试用例

```bash
# CPU 满载功耗（5 级递增）
msh /> rtbench test-stress --job cpu

# 内存密集型功耗
msh /> rtbench test-stress --job memory

# I/O 密集型功耗（需文件系统）
msh /> rtbench test-stress --job file

# 综合功耗画像
msh /> rtbench test-stress --job all
```

## 故障排查

### 问题：Stressor 运行后 duration 为 0

**可能原因**:
1. Jobfile 中 `--ops` 值设置过小
2. Stressor 内部遇到不支持的系统调用

**解决方案**:
- 检查 `stress_stored_job.c` 中的 ops 配置
- 检查 RTOS 是否支持 stressor 所需的 API

### 问题：I/O stressor 报错

**可能原因**:
1. RTOS 未启用文件系统
2. 存储设备未挂载

**解决方案**:
- 确认 RT-Thread `.config` 中启用了 DFS 和 elmFAT/RomFS
- 确认存储设备已正确挂载

### 问题：内存类 stressor 崩溃

**可能原因**:
1. RTOS 可用堆内存不足
2. 高 Stage 并发度超出系统容量

**解决方案**:
- 增大 RTOS 堆配置（如 `RT_HEAP_SIZE`）
- 使用单独的 `--job memory` 测试以排查

### 问题：线程创建失败

**可能原因**:
1. 系统线程数达到上限（高 Stage 并发更容易触发）
2. 线程栈配置不足

**解决方案**:
- 增大 `PTHREAD_NUM_MAX` 或 `RT_THREAD_NUM_MAX`
- 确认没有其他高负载任务同时运行

## 参考资料

1. 工业操作系统通用基准检测指标体系指导书 v1.5
2. [stress-ng](https://github.com/ColinIanKing/stress-ng) - 原始 Linux 压力测试工具
3. RTOS-Bench 架构概览: [ARCHITECTURE_OVERVIEW.md](ARCHITECTURE_OVERVIEW.md)
