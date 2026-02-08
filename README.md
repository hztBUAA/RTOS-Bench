# RTOS-Bench（精简版总览）

RTOS-Bench（rtbench CLI）是一套周期性实时基准框架，已统一为“多 workload 注册表”模式，支持 Linux / RT-Thread / SylixOS，默认以 POSIX 风格为合同（pthread/clock/socket 等）。本仓库将所有打包负载集中在 `workloads/`，不再依赖外部子目录。

## 快速使用
- RT-Thread（QEMU aarch64，交互）：
  ```
  ./run-rtthread.sh          # 编译并运行
  ./run-rtthread.sh -b       # 仅编译
  ./run-rtthread.sh -r       # 仅运行已有镜像
  # msh 内运行：
  rtbench -b <workload> -p <period_sec> -t 1 -q
  ```
- SylixOS：`./run-sylixos.sh`（默认 qemu-x86_64，可用 `-n` 无图形）。
- OneOS / 东土 / 锐华（POSIX-lite 口径）：交叉编译时指定 `PLATFORM=oneos|dongtu|ruihua`，入口为 `generator/posixlite_entry.c`，支持 `-b/-p/-t/-f/-c/-A/-G/-L/-q`。
- 手动 QEMU（RT-Thread）：`qemu-system-aarch64 -M virt,gic-version=2 -cpu cortex-a53 -m 128M -smp 4 -kernel extern/rt-thread/bsp/qemu-virt64-aarch64/rtthread.bin -nographic`

### 已内置工作负载
- 计算类：busywait, stub, fast, epnp, ekf, icp, pid, cusum, ewma
- 网络类：modbus, mqtt（无网络时自动离线仿真/pack-only，并打印 offline 提示）

### 常用命令示例（无网环境）
```
rtbench -b busywait -p 0.5 -t 1 -q
rtbench -b fast     -p 1   -t 1 -q
rtbench -b epnp     -p 1   -t 1 -q
rtbench -b ekf      -p 2   -t 1 -q
rtbench -b icp      -p 5   -t 1 -q
rtbench -b pid      -p 0.5 -t 1 -q
rtbench -b cusum    -p 0.5 -t 1 -q
rtbench -b ewma     -p 0.5 -t 1 -q
rtbench -b modbus   -p 2   -t 1 -q   # 无网则离线仿真
rtbench -b mqtt     -p 2   -t 1 -q   # 无网则 pack-only
```

### 新的工作负载调度选项（Linux/SylixOS 入口）
- `-w, --workload <name>`：只跑单个 workload（默认是注册表首个）。
- `-A, --all-workloads`：顺序跑完所有已注册 workload。
- `-G, --category <cat[,cat2]>`：按类别过滤运行（例如 `detection,network`）。
- `-L, --list`：列出名称/类别/描述并退出。
RT-Thread 路径同样支持 `-A/-G/-L`，但仍由 BSP 的 SCons 构建。

## 架构要点
- **Workload registry**：`workloads/rtbench_workloads.cpp` 注册全部负载，强符号禁用旧 benchmark_* 覆盖。
- **平台抽象**：`generator/platform/<platform>/` 提供 timer/sync/scheduler/timestamp/signal。POSIX 完整平台可直接复用 Linux 实现；其他平台用最小垫片兜底。
- **入口**：
  - Linux/SylixOS：`generator/main.c`（argp，全量参数、可写 CSV）。
  - OneOS/东土/锐华（POSIX-lite）：`generator/posixlite_entry.c`（精简 CLI，无 argp/perf 依赖）。
  - RT-Thread：`generator/rtthread_entry.c`（精简 CLI，默认 TRACE，可 `-q` 降噪）。
- **定时器**：RT-Thread 使用软定时器避免早期调度崩溃；Linux/SylixOS 用 POSIX timer。

## 新增工作负载
1. 在 `workloads/<NAME>/` 添加源码，提供 `foo_bench_run()` 入口（`extern "C"`）。
2. 在 `workloads/rtbench_workloads.cpp` 增加 `rtosbench_workload` 并在 `register_all_workloads()` 注册。
3. 避免自定义 benchmark_* 强符号；必要参数/循环次数在输出中提示。

## 新增平台（若非纯 POSIX）
1. 在 `generator/platform/<new>/` 实现：
   - `timer.c`（优先用内置定时器/软定时器）
   - `sync.c`（信号量）
   - `scheduler.c`（priority/affinity；deadline 不支持可返回 -1）
   - `timestamp.c`（尽量高精度）
   - `signal.c`（无则返回 -1）
2. 在 `generator/Makefile` 添加 PLATFORM 分支并指向目录；入口可参考 `rtthread_entry.c`。

## 功能缺口（相较原始 Linux 版）
- PMU/perf 采样、memory_watcher 在 RTOS 平台为 stub。
- deadline 调度：仅 Linux SCHED_DEADLINE；RTOS 需自实现或提示不支持。
- RT-Thread 入口未完整支持 JSON/文件输出；若需 CSV 落盘需扩展入口或在 workload 内输出。
- 时间戳精度：RT-Thread 目前 tick 级，需硬件计时可自行扩展。

## OneOS 平台集成指南

### 概述
OneOS（中国移动物联网操作系统）平台使用原生API实现，经过 OneOS Studio IDE (SCons) 编译验证。

### 支持的 Workloads
| Workload | 状态 | 备注 |
|----------|------|------|
| stub | ✅ | 基础测试 |
| busywait | ✅ | CPU 负载测试 |
| CUSUM | ✅ | 变化检测算法 |
| EWMA | ✅ | 指数加权移动平均 |
| FAST | ✅ | 特征检测（含 benchmark） |
| PID | ✅ | PID 控制器（含 benchmark） |
| MODBUS | ❌ | 需要 socket |
| MQTT | ❌ | 需要 socket |
| EKF/EPNP/ICP | ❌ | 需要 C++11 / Eigen |

### 平台抽象层实现

| 组件 | 文件 | 实现方式 |
|------|------|----------|
| 定时器 | timer.c | OneOS 原生 `os_timer_t` |
| 信号量 | sync.c | OneOS 原生 `os_sem_t` |
| 时间戳 | timestamp.c | OneOS 原生 `os_tick_get()` |
| 调度器 | scheduler.c | OneOS 原生 `os_task_set_priority()` |
| 信号 | signal.c | 空实现（RTOS 无信号机制） |

### POSIX 兼容性适配

OneOS 提供了部分 POSIX 支持，但有以下缺口需要适配：

**1. pthread_attr_setinheritsched() 缺失**
- OneOS 的 `pthread.h` 定义了 `PTHREAD_EXPLICIT_SCHED` 常量
- 但未实现 `pthread_attr_setinheritsched()` 函数
- 解决方案：`posix_sched_adapter.c` 提供实现

**2. CLOCK_MONOTONIC 未实现**
- OneOS 的 `clock_time.h` 定义了 `CLOCK_MONOTONIC=4`
- 但 `clock_gettime()` 只处理 `CLOCK_REALTIME`
- 解决方案：`include/time.h` 影子头文件将 `CLOCK_MONOTONIC` 映射到 `CLOCK_REALTIME`

### 集成步骤

1. **添加为 Git Submodule**
   ```bash
   cd your_oneos_project
   git submodule add https://github.com/user/RTOS-Bench.git rtos-bench-src
   ```

2. **复制 SConscript**
   ```bash
   cp rtos-bench-src/generator/platform/oneos/SConscript.example rtos-bench-src/SConscript
   ```

3. **配置 OneOS**
   - 在 menuconfig 中启用 `OS_USING_PTHREADS`
   - 确保 `OS_USING_SEMAPHORE` 和 `OS_USING_TIMER` 已启用

4. **编译**
   ```bash
   scons -j4
   ```

### OneOS POSIX 支持情况检查方法

```bash
# 查看 POSIX 头文件
ls oneos/osal/posix/include/

# 查看 POSIX 实现
ls oneos/osal/posix/source/

# 搜索特定函数声明
grep -r "clock_gettime" oneos/osal/posix/

# 查看 Kconfig 配置选项
grep -r "POSIX\|pthread" oneos/Kconfig*
```

### 已知限制
- 时间精度受限于 `OS_TICK_PER_SECOND`（通常 100-1000 Hz）
- 无真正的 CLOCK_MONOTONIC（使用 CLOCK_REALTIME 替代）
- 无 deadline 调度支持

## 许可证
RTOS-Bench 基于 MIT 许可证（见 LICENSE），子基准套件遵循各自目录下的 LICENSES。

## 参考论文
Nicolella et al., “RT-Bench: An Extensible Benchmark Framework for the Analysis and Management of Real-Time Applications,” RTNS 2022. DOI: 10.1145/3534879.3534888
