# CLAUDE.md

本文件用于指导后续在本仓库内集成/扩展 rt-bench，重点面向新增 RTOS 平台和新的 workload 打包。

## 当前整体约定
- **统一入口**：仅使用 workload registry（`generator/workload_registry.*`），强符号禁用 benchmark_* 覆盖。新增负载必须通过 `rtosbench_register_workload()`。
- **打包开关**：`generator/Makefile` 默认 `RTOS_WORKLOADS=1`，自动把 `workloads/` 下所有源码编入 Linux/SylixOS；RT-Thread 由 BSP SConscript 引用。
- **工作目录**：所有可复用负载放在 `workloads/`，不再依赖外部子目录（如 hsw）。`workloads/rtbench_workloads.cpp` 统一注册表。
- **入口 CLI**：
  - Linux/SylixOS：`rtbench -p <period_sec> -t <tasks> -b <workload> [-q]`
  - RT-Thread：同上，通过 `rtosbench`/`rtbench` msh 命令；默认 TRACE，可用 `-q` 降噪。
- **内置工作负载**：busywait, stub, fast, epnp, ekf, icp, pid, modbus, mqtt。网络类在无网络时自动进入离线仿真（modbus/mqtt 会打印 offline 提示）。

## 脚本与命令
- RT-Thread(QEMU aarch64)：`./run-rtthread.sh`
  - `-b` 仅编译；`-r` 仅运行；交互模式可在 msh 输入 `rtbench -b <name> -p <sec> -t 1 -q`
  - QEMU 默认 4 核 128M，无网络；如需网络可自行添加 `-netdev user ...`，但 BSP 未保证稳定。
- SylixOS：`./run-sylixos.sh`（默认 qemu-x86_64，支持 `-n` 无图形）。
- 手工 QEMU（RT-Thread）：  
  ```bash
  /usr/local/bin/qemu-system-aarch64 -M virt,gic-version=2 -cpu cortex-a53 -m 128M -smp 4 \
    -kernel extern/rt-thread/bsp/qemu-virt64-aarch64/rtthread.bin -nographic
  # msh:
  rtbench -b busywait -p 0.5 -t 1 -q
  ```

## 新增工作负载指南
1. 在 `workloads/<NAME>/` 放源代码；提供可调用的 `*_bench_run()` 入口（C 或 `extern "C"`）。
2. 在 `workloads/rtbench_workloads.cpp` 增加对应的 `rtosbench_workload` 并注册：
   ```c
   const struct rtosbench_workload rtosbench_foo = {
     .name = "foo",
     .description = "...",
     .init = foo_init,
     .exec = foo_exec,
     .teardown = foo_teardown,
   };
   // 在 register_all_workloads() 中调用 rtosbench_register_workload(&rtosbench_foo);
   ```
3. 若需要特定参数/循环次数，请在 README 或 workload 代码内打印提示；RT-Thread 默认 `-t 1`、`-p 1s`。
4. 不要定义 `benchmark_init/benchmark_execution` 等旧接口；统一通过 registry。

## 新增 RTOS 平台步骤
1. 在 `generator/platform/<new-rtos>/` 实现以下最小接口：
   - `timer.c`：`rtbench_timer_create/settime/delete`（推荐使用本 RTOS 内建定时器/软定时器）。
   - `sync.c`：`rtbench_sem_*`。
   - `scheduler.c`：`rtbench_set_priority/deadline/affinity`（不支持可返回 -1）。
   - `timestamp.c`：`rtbench_get_rdtsc/timestamp`（尽量高精度）。
   - `signal.c`：若无信号支持返回 -1。
2. 在 `generator/Makefile` 增加 `ifeq ($(PLATFORM),<new-rtos>)` 分支，设置 `PLATFORM_DIR` 和必要的 CFLAGS/LDFLAGS。
3. 提供入口（参考 `generator/rtthread_entry.c`）：解析少量参数，调用 `rtosbench_register_rtos_workloads()` 和 `periodic_benchmark()`.
4. 平台 BSP 侧（如 RT-Thread SConscript）包含生成的 .c/.cpp 并链接 `workloads/rtbench_workloads.cpp`。

## RT-Thread 集成要点
- BSP 已开启 POSIX/pthread/SAL/LWIP/virtio 配置（见 `extern/rt-thread/bsp/qemu-virt64-aarch64/.config`）；如需网络真实可用，需在 QEMU 侧配置网卡并确保 DHCP/静态地址匹配，否则 modbus/mqtt 会走离线模式。
- C++ workload（epnp/ekf/icp/pid）已用 `extern "C"` 暴露；链接脚本已包含 .ctors。
- 定时器实现使用软定时器（非自建线程），避免早期调度崩溃；日志默认 TRACE，可用 `-q`。

## 已验证命令示例（无网络环境）
- 计算类：
  - `rtbench -b busywait -p 0.5 -t 1 -q`
  - `rtbench -b fast -p 1 -t 1 -q`
  - `rtbench -b epnp -p 1 -t 1 -q`
  - `rtbench -b ekf -p 2 -t 1 -q`
  - `rtbench -b icp -p 5 -t 1 -q`
  - `rtbench -b pid -p 0.5 -t 1 -q`
- 网络类（离线降级）：
  - `rtbench -b modbus -p 2 -t 1 -q`  => offline 仿真，打印 TPS
  - `rtbench -b mqtt -p 2 -t 1 -q`    => 无会话则 pack-only 并打印统计

## 迁移/调试提示
- 若周期输出大量 CSV，可用 `-q` 或调整 `benchmark_verbosity`。
- RT-Thread 若遇定时器/线程问题，先检查是否用软定时器实现，避免手工线程 + `rt_cpus_unlock` 空指针。
- 新 workload 如需网络，优先提供离线 fallback，保证在无网场景也能运行。
- 不再使用旧的 `benchmark_registry`；删除或忽略任何单负载强符号覆盖的实现。

## 目录速览
- `generator/`：核心框架 + 平台抽象 + 入口（Linux/SylixOS/RT-Thread）。
- `workloads/`：全部打包的负载与注册表。
- `run-rtthread.sh` / `run-sylixos.sh`：一键构建/运行脚本。
- `extern/rt-thread/bsp/qemu-virt64-aarch64/`：RT-Thread BSP 与 `.config`。

以上约定若有更新，请同步修改本文件。***
