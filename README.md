# RT-Bench（精简版总览）

rt-bench 是一套周期性实时基准框架，已统一为“多 workload 注册表”模式，支持 Linux / RT-Thread / SylixOS，默认以 POSIX 风格为合同（pthread/clock/socket 等）。本仓库将所有打包负载集中在 `workloads/`，不再依赖外部子目录。

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
- 手动 QEMU（RT-Thread）：`qemu-system-aarch64 -M virt,gic-version=2 -cpu cortex-a53 -m 128M -smp 4 -kernel extern/rt-thread/bsp/qemu-virt64-aarch64/rtthread.bin -nographic`

### 已内置工作负载
- 计算类：busywait, stub, fast, epnp, ekf, icp, pid
- 网络类：modbus, mqtt（无网络时自动离线仿真/pack-only，并打印 offline 提示）

### 常用命令示例（无网环境）
```
rtbench -b busywait -p 0.5 -t 1 -q
rtbench -b fast     -p 1   -t 1 -q
rtbench -b epnp     -p 1   -t 1 -q
rtbench -b ekf      -p 2   -t 1 -q
rtbench -b icp      -p 5   -t 1 -q
rtbench -b pid      -p 0.5 -t 1 -q
rtbench -b modbus   -p 2   -t 1 -q   # 无网则离线仿真
rtbench -b mqtt     -p 2   -t 1 -q   # 无网则 pack-only
```

## 架构要点
- **Workload registry**：`workloads/rtbench_workloads.cpp` 注册全部负载，强符号禁用旧 benchmark_* 覆盖。
- **平台抽象**：`generator/platform/<platform>/` 提供 timer/sync/scheduler/timestamp/signal。POSIX 完整平台可直接复用 Linux 实现；其他平台用最小垫片兜底。
- **入口**：
  - Linux/SylixOS：`generator/main.c`（argp，全量参数、可写 CSV）。
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

## 许可证
RT-Bench 基于 MIT 许可证（见 LICENSE），子基准套件遵循各自目录下的 LICENSES。

## 参考论文
Nicolella et al., “RT-Bench: An Extensible Benchmark Framework for the Analysis and Management of Real-Time Applications,” RTNS 2022. DOI: 10.1145/3534879.3534888
