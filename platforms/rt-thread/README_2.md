# RT-Thread 飞腾派内核插桩记录

本文记录 2026-10-28 为 RTOS-Bench 实时性能测试接入 RT-Thread 内核插桩的改动、
构建开关、产物和校验结果。

## 1. 构建开关

新增全局编译宏：

```text
RTBENCH_KERNEL_INSTRUMENTATION
```

飞腾派 BSP 的 `aarch64/SConstruct` 仅在以下条件同时满足时启用插桩：

- `aarch64/rtos-bench` 链接存在；
- 链接目标中存在 `generator/platform/rt-thread/rtbench_kernel_probe.h`；
- 环境变量 `RTBENCH_KERNEL_INSTRUMENTATION` 不为 `0`。

默认启用。生成未插桩镜像时使用：

```bash
export RTBENCH_KERNEL_INSTRUMENTATION=0
```

切换该开关后必须清理旧对象，避免内核对象复用：

```bash
cd D:/OS/RTTHREAD/rt-thread/bsp/phytium/aarch64
export RTT_ROOT=D:/OS/RTTHREAD/rt-thread
export RTT_CC=gcc
export RTT_CC_PREFIX=aarch64-none-elf-
export RTT_EXEC_PATH=D:/OS/RTTHREAD/fix/rt-thread/xpack-aarch64-none-elf-gcc-14.2.1-1.1/bin
export RTBENCH_KERNEL_INSTRUMENTATION=1

scons -c
scons --useconfig=.config
scons -j4
```

WSL 中执行时使用对应的 `/mnt/d/...` 路径。

## 2. RTOS-Bench 仓库改动

| 文件 | 内容 |
|---|---|
| `generator/platform/rt-thread/rtbench_kernel_probe.h` | 内核可见的最小探针接口，不暴露完整 `les.h` |
| `generator/platform/rt-thread/rtbench_kernel_probe.c` | 调用现有 `LES_stub()`、`LES_interrupt_end_stub()`、`LES_syscall_stub()` |
| `generator/realtime_orig/multicore/task_lt.c` | 任务创建/删除重复次数恢复为 30 |

这三个接口分别为：

```c
void rtbench_kernel_sched_probe(void);
void rtbench_kernel_irq_end_probe(void);
void rtbench_kernel_syscall_probe(void);
```

`generator/SConscript` 已有的 `Glob('platform/rt-thread/*.c')` 会自动编译新增实现，
无需修改构建源文件清单。

任务创建/删除测试的重复次数为：

```c
#define TEST_REPETITION 30
```

## 3. RT-Thread 仓库修改

| 文件 | 修改 |
|---|---|
| `bsp/phytium/aarch64/SConstruct` | 检测 RTOS-Bench 探针头文件；添加全局探针 `CPPPATH` 和 `RTBENCH_KERNEL_INSTRUMENTATION` 宏；保留原有 `RT_THREAD_PLATFORM` |
| `src/scheduler_mp.c` | 在 `rt_schedule()` 中、`rt_hw_context_switch()` 前调用 `rtbench_kernel_sched_probe()` |
| `libcpu/aarch64/common/vector_gcc.S` | 在 IRQ 入口保存上下文后、`rt_interrupt_enter()` 前记录 CPU0 的 `CNTPCT_EL0` 到 `LES_interrupt_start_val` |
| `libcpu/aarch64/common/trap.c` | 在 `rt_hw_trap_irq()` 开始处调用 `rtbench_kernel_irq_end_probe()` |
| `components/libc/compilers/newlib/syscalls.c` | 在 `_getpid_r()` 返回前调用 `rtbench_kernel_syscall_probe()` |
| `bsp/phytium/aarch64/.config` | `PTHREAD_NUM_MAX` 从 32 提升到 320 |
| `bsp/phytium/aarch64/rtconfig.h` | 由 `.config` 重新生成，`PTHREAD_NUM_MAX=320` |

所有内核探针都由 `RTBENCH_KERNEL_INSTRUMENTATION` 条件编译保护。未启用插桩或未接入
RTOS-Bench 时，内核不引用上述探针符号。

`libcpu/aarch64/common/vector_gcc.S` 的 CPU 判断使用：

```asm
mrs x9, mpidr_el1
and x9, x9, #15
```

与 RT-Thread 当前 `rt_hw_cpu_id()` 对 `MPIDR_EL1` 低 4 位的映射保持一致。

`PTHREAD_NUM_MAX` 是同时可占用的 pthread 元数据槽位数，不是生命周期内累计创建数。
joinable pthread 退出后必须 `pthread_join()` 才释放槽位。并发度 8 的任务创建/删除测试
最多同时需要约 250 个 pthread 槽位，因此当前配置保留 320 个槽位。

## 4. 平台宏说明

`RT_THREAD_PLATFORM` 已由构建系统以编译参数提供：

- `bsp/phytium/aarch64/SConstruct` 在 `PrepareBuilding()` 前添加全局
  `CPPDEFINES=['RT_THREAD_PLATFORM']`；
- `generator/SConscript` 为 RTOS-Bench 编译组添加相同的局部宏。

因此 `generator/realtime_orig/les/platform_macro.h` 中的
`//#define RT_THREAD_PLATFORM` 保持注释，平台宏以构建系统为唯一来源。

## 5. 插桩语义

- 调度插桩：只在 CPU0 且 `LES_flag=1` 时记录当前 `CNTPCT_EL0`。
- 中断插桩：IRQ 向量入口记录起点；进入 `rt_hw_trap_irq()` 时记录终点并清除
  `LES_interrupt_flag`。
- 系统调用插桩：在 newlib 的 `_getpid_r()` 返回前记录终点。
- 未插桩路径不会因缺失 LES 符号导致链接失败。

## 6. 构建结果

构建环境：

- 源码：RT-Thread `db6c0ddbf333be65f5ddc990ee63a200f768a9f7`
- 板卡：Phytium Pi / E2000Q / Firefly PE2204
- 工具链：xPack AArch64 GCC 14.2.1
- SCons：4.5.2
- 插桩开关：`RTBENCH_KERNEL_INSTRUMENTATION=1`
- 构建日志错误：0
- 构建日志警告：74（来自 RT-Thread、BSP 和 RTOS-Bench 现有代码的非阻塞警告）

输出目录：`D:\OS\RTTHREAD\output\Stub`

| 产物 | 大小（字节） | SHA-256 |
|---|---:|---|
| `rtthread_a64.elf` | 49,325,648 | `9258AF41366E6D72E5F1C671F995498B4D00318E06F002DB7DBD41487410A642` |
| `rtthread_a64.bin` | 6,324,176 | `EB2725A464E537234B07F738381F0927760B6BC64DF5665E5233AA7BCA3B2601` |
| `rtthread_a64.map` | 7,933,960 | `D60CA657F3211DFBCB54D91C503D7F6C8F6123F712D97A925FA8D8205DDD37D4` |

与 `output\NoStub` 的非插桩镜像相比：

- text 增加 584 字节；
- BIN 增加 512 字节；
- data 大小一致；
- bss 增加 4096 字节，主要来自扩展到 320 项的 `pth_table`。

## 7. 二进制校验

```text
Class:               ELF64
Machine:             AArch64
Entry point address: 0x80080000
```

关键符号：

```text
LES_stub
LES_interrupt_end_stub
LES_interrupt_flag
LES_interrupt_start_val
rtbench_kernel_sched_probe
rtbench_kernel_irq_end_probe
rtbench_kernel_syscall_probe
rtosbench_rtthread_entry
rtosbench_register_rtos_workloads
__fsym_rtos_bench
```

反汇编确认：

- `rt_schedule()` 调用 `rtbench_kernel_sched_probe()`，随后调用
  `rt_hw_context_switch()`；
- `rt_hw_trap_irq()` 开始处调用 `rtbench_kernel_irq_end_probe()`；
- `_getpid_r()` 返回前调用 `rtbench_kernel_syscall_probe()`；
- `vector_irq` 中存在 `MPIDR_EL1` 和 `CNTPCT_EL0` 读取；
- 探针实现分别调用 `LES_stub()`、`LES_interrupt_end_stub()` 和
  `LES_syscall_stub()`。

## 8. 板上验证建议

将 `output\Stub\rtthread_a64.bin` 下载到开发板后执行：

```text
rtos_bench test-realtime -v
rtos_bench test-realtime -v --delay
```

验收重点：

- `interrupt stub verification` 能获得有效数据且 unexpected data 为 0；
- `schedule stub verification` 的 buffer offset 至少为 2；
- `--delay` 输出的中断延迟 MIN/MAX/AVG 不全为 0；
- 完整 `test-realtime` 的系统调用延迟来自新的 `_getpid_r()` 探针。

本次只完成编译和二进制级验证，尚未在飞腾派实机上运行。
