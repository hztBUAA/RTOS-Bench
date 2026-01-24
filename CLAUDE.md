# CLAUDE.md

本文件用于指导后续在本仓库内集成/扩展 rt-bench，重点面向新增 RTOS 平台和新的 workload 打包。

## 核心约定（统一入口 + POSIX 合同）
- 仅使用 workload registry（`generator/workload_registry.*`），强符号禁用 benchmark_* 覆盖；新增负载必须注册 `rtosbench_register_workload()`。
- 默认 POSIX 契约（pthread/clock/socket/sem）；Linux/SylixOS 直接使用，RTOS 若 POSIX 不完备则在各自平台目录做最小垫片。
- 打包开关：`generator/Makefile` 默认 `RTOS_WORKLOADS=1`，自动把 `workloads/` 下源码编入 Linux/SylixOS；RT-Thread 由 BSP SConscript 引用。
- 工作目录：`workloads/` 集中存放所有负载；`workloads/rtbench_workloads.cpp` 统一注册和 `register_all_workloads()`。
- 入口 CLI：
  - Linux/SylixOS：`rtbench -p <period_sec> -t <tasks> -b <workload> [-q] [-d <deadline>]`
  - RT-Thread：`rtosbench`/`rtbench` msh，同参数；默认 TRACE，可用 `-q` 降噪（RT-Thread 目前未实现 -d，需要扩展）。
- 内置工作负载：busywait, stub, fast, epnp, ekf, icp, pid, modbus, mqtt。网络类在无网络时自动离线仿真/pack-only，并打印 offline 提示。

## 脚本与命令
- RT-Thread(QEMU aarch64)：`./run-rtthread.sh`
  - `-b` 仅编译；`-r` 仅运行；交互：`rtbench -b <name> -p <sec> -t 1 -q`
  - 默认 4 核 128M，无网；如需网络自行加 QEMU 网卡，但 BSP 未保证；网络类已有离线兜底。
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

## 新增 RTOS 平台步骤（若非纯 POSIX）
1. 在 `generator/platform/<new-rtos>/` 实现：
   - `timer.c`（优先内置/软定时器，避免线程早期崩溃）
   - `sync.c`
   - `scheduler.c`（priority/affinity；deadline 不支持可返回 -1 并提示）
   - `timestamp.c`（尽量高精度）
   - `signal.c`（不支持则返回 -1）
2. `generator/Makefile` 添加平台分支；BSP 侧链接平台源码与 `workloads/rtbench_workloads.cpp`。

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
- Deadline：Linux 支持 SCHED_DEADLINE；RTOS 不支持时 `rtbench_set_deadline` 应返回 -1 并提示，统计层仍可用 deadline 判定 miss（需入口支持 -d）。
- 时间戳：RT-Thread 目前 tick 级，可扩展硬件计时；大量 CSV 可用 `-q` 减噪。
- 定时器：RT-Thread 使用软定时器，避免自建线程 + `rt_cpus_unlock` 空指针。
- 网络：提供离线 fallback（modbus 仿真、mqtt pack-only），即便无网也能跑；有网需正确配置网卡/IP/DNS。
- 禁用旧 benchmark_registry；一切通过 workload registry。

## 目录速览
- `generator/`：核心框架 + 平台抽象 + 入口（Linux/SylixOS/RT-Thread）。
- `workloads/`：全部打包的负载与注册表。
- `run-rtthread.sh` / `run-sylixos.sh`：一键构建/运行脚本。
- `extern/rt-thread/bsp/qemu-virt64-aarch64/`：RT-Thread BSP 与 `.config`。
- `docs/BUILD_GUIDE.md`：详细构建与部署指南（新人必读）。

## 工具链说明

### RT-Thread (aarch64)
| 项目 | 说明 |
|------|------|
| 工具链 | xpack-aarch64-none-elf-gcc-14.2.1-1.1 |
| 位置 | `extern/toolchains/` |
| 构建系统 | SCons (`extern/.venv/bin/scons`) |
| 构建命令 | `./run-rtthread.sh -b` |

### SylixOS (x86_64)
| 项目 | 说明 |
|------|------|
| 运行镜像 | qcow2 格式，位于 `extern/yihui/...` |
| 开发工具 | RealEvo IDE 6.5.0 (Windows) |
| 运行命令 | `./run-sylixos.sh` |
| 编译方式 | 见 `docs/BUILD_GUIDE.md` |

**SylixOS 编译和运行（2026-01-24 完整验证）**：
- 默认镜像**没有 GCC**
- **不支持 9p 文件共享**（No driver）
- SSH 默认未启动
- **QEMU user mode 网络不兼容**（SylixOS 使用 10.4.x 网段）
- **Linux GCC 编译的二进制不兼容 SylixOS**（即使静态编译也报 `Invalid format!`）
- **必须使用 RealEvo IDE (Windows) 编译**
- VMware Fusion 可运行 SylixOS 镜像，支持 vmhgfs 共享文件夹
- QEMU 磁盘扩展受限：只支持原有 2 个 IDE 设备（额外 IDE/AHCI/virtio/USB 均不支持）

## 跨平台架构
- **平台抽象层**：`generator/platform/<platform>/` 提供 timer/sync/scheduler/timestamp/signal
- **对 workload 开发者透明**：只需关心业务逻辑，不同架构由工具链处理，不同 RTOS API 由抽象层处理
- **POSIX 契约**：Linux/SylixOS 完整支持，RT-Thread 通过适配层支持

以上约定若有更新，请同步修改本文件。
