# CLAUDE.md

本文件用于指导后续在本仓库内集成/扩展 rt-bench，重点面向新增 RTOS 平台和新的 workload 打包。
开发过程中必须严格确保：确保代码和文档的强一致性和复利性!

## 核心约定（统一入口 + POSIX 合同）
- 仅使用 workload registry（`generator/workload_registry.*`），强符号禁用 benchmark_* 覆盖；新增负载必须注册 `rtosbench_register_workload()`。
- 默认 POSIX 契约（pthread/clock/socket/sem）；RTOS 若 POSIX 不完备则在 `generator/platform/<platform>/` 做最小垫片。
- 打包开关：根目录 `Makefile` 默认 `RTOS_WORKLOADS=1`，自动把 `workloads/` 下源码编入 Linux/SylixOS；RT-Thread 由 BSP SConscript 引用。
- 工作目录：`workloads/` 集中存放所有负载；`workloads/rtbench_workloads.cpp` 统一注册和 `register_all_workloads()`。
- 入口层架构：平台 `*_entry.c` 应保持薄适配层；命令解析、`test-all` 编排、结果收集和默认输出路径统一放在 `generator/rtbench_command.c/h`。平台差异通过 `rtbench_platform_default_output_path()`、`rtbench_platform_get_env()`、`rtbench_platform_run_test_all()` 等 hook 处理。
- 入口 CLI：
  - Linux/SylixOS：`rtbench -p <period_sec> -t <tasks> -b <workload> [-q] [-d <deadline>]`
  - RT-Thread：`rtosbench`/`rtbench` msh，同参数；默认 TRACE，可用 `-q` 降噪（RT-Thread 目前未实现 -d，需要扩展）。
- 内置工作负载：busywait, stub, fast, epnp, ekf, icp, pid, modbus, mqtt。网络类在无网络时自动离线仿真/pack-only，并打印 offline 提示。

## 当前任务上下文（2026-05）
- 本地唯一真相工作树为 `C:\Users\hzt\yihui-workspace\rtos-bench\RTOS-Bench`。`RTOS-Bench-ruihua` 只允许作为历史/临时 worktree 参考，不再单独维护新改动；后续代码、文档、验证日志和提交都应落到 canonical 工作树。
- 正在进行多平台入口统一：SylixOS、Dongtu、OneOS、RT-Thread、Posixlite 均应收敛到 `rtbench_command` dispatcher，保留平台专属 worker/stack/默认输出路径。
- 验收必须以真实板卡为准：至少覆盖 `--help`、`export-result -o <writable>`、`test-schedule --cycles 3`、非 quick 的完整 `test-all`、错误参数回归。
- 真实板卡连接、历史基线、已验证日志和当前可用二进制路径维护在 `utils/remote-test/ENTRY_UNIFICATION_BASELINE.md`，更新连接或部署状态时必须同步更新该文件。

## 远端板卡与二进制版本管理
- 板卡上的 `/apps/hzt/`、OneOS 的 `/user/` 等目标目录是验收用二进制资产库，不只是临时上传目录。远端服务器、人工验收和自动化脚本都应优先调用这些已归档路径。
- 执行 `make clean`、IDE clean、删除 `Debug/` 或覆盖 `Debug/strip/` 前，必须先归档当前可运行产物：记录本地路径、目标板卡、远端路径、文件大小、SHA256、构建时间、源码分支/提交和最小验收命令。
- 归档命名使用可追溯后缀，如 `/apps/hzt/nezha-rtos-bench-r7`、`<board>-rtos-bench-default3`、`<board>-rtos-bench-YYYYMMDD-HHMM`。不要只覆盖无后缀主路径而不保留可运行旧版本。
- SylixOS 上同名覆盖后可能触发 loader/module 缓存问题；若覆盖后的同名路径异常，但同一文件换唯一名字可运行，应优先使用唯一名字继续验收，并把异常和可运行路径记入基线文档。
- 上传流程优先使用跳板机：本地 `sftp` 到 `10.134.151.45:1026`，再从跳板机 `curl -T` 到板卡 FTP，随后 telnet 到板卡执行 `chmod 755`、`ll`、`--help` 或 `test-all --help` 做短确认。
- SylixOS 结果默认路径必须是可写目录，例如 `/apps/hzt/rtbench_result.json`；完整测试建议显式加 `-o /apps/hzt/<board>_<case>_<timestamp>.json`，避免写根目录失败。
- 当前哪吒最新已验证入口统一二进制：`/apps/hzt/nezha-rtos-bench-r7`，对应本地未裁剪产物 `C:\Users\hzt\yihui-workspace\nezha-rtos-bench\Debug\nezha-rtos-bench`。完整验收命令：`/apps/hzt/nezha-rtos-bench-r7 test-all -o /apps/hzt/nezha_testall_full_r7_<timestamp>.json`。

## 脚本与命令
- RT-Thread (QEMU virt aarch64, BSP: `qemu-virt64-aarch64`)：`./run-rtthread.sh`
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
2. 根目录 `Makefile` 添加平台分支；BSP 侧链接平台源码与 `workloads/rtbench_workloads.cpp`。

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
- **可调度性测试**：
  - `rtbench test-schedule` => 完整测试（30%-100%利用率，默认 3 周期）
  - `rtbench test-schedule --cycles 1` => 快速 smoke（减少周期数）
  - 详见 [docs/SCHEDULE.md](docs/SCHEDULE.md)
- **实时性能测试**：
  - `rtbench test-realtime` => 测量上下文切换、信号量、互斥锁、内存分配延迟
  - `rtbench test-realtime --multicore` => 包含多核测试（需 SMP 支持）
  - 详见 [docs/REALTIME.md](docs/REALTIME.md)
- **压力测试**：
  - `rtbench test-stress -s cpu -t 5` => CPU 压力测试
  - `rtbench test-stress -s memory -t 5` => 内存压力测试
  - 详见 [docs/STRESS.md](docs/STRESS.md)
- **Shell 命令支持测试**：
  - `rtbench test-cmd` => 测试 Shell 命令注册与执行能力
  - 详见 [docs/CMD.md](docs/CMD.md)
- **综合测试**：
  - `rtbench test-all` => 运行所有测试子命令（test-realtime, test-schedule, test-stress, test-cmd）
  - `rtbench test-all -o /rtbench_result.json` => 运行所有测试并导出结果

## 迁移/调试提示
- Deadline：Linux 支持 SCHED_DEADLINE；RTOS 不支持时 `rtbench_set_deadline` 应返回 -1 并提示，统计层仍可用 deadline 判定 miss（需入口支持 -d）。
- 时间戳：RT-Thread 目前 tick 级，可扩展硬件计时；大量 CSV 可用 `-q` 减噪。
- 定时器：RT-Thread 使用软定时器，避免自建线程 + `rt_cpus_unlock` 空指针。
- 网络：提供离线 fallback（modbus 仿真、mqtt pack-only），即便无网也能跑；有网需正确配置网卡/IP/DNS。
- 禁用旧 benchmark_registry；一切通过 workload registry。

## 目录速览
- `Makefile`：跨平台构建入口（Linux/SylixOS/OneOS 等）。
- `generator/`：核心框架 + 平台抽象 + 入口（Linux/SylixOS/RT-Thread 等）。
- `workloads/`：全部打包的负载与注册表。
- `run-rtthread.sh` / `run-sylixos.sh`：一键构建/运行脚本。
- `extern/rt-thread/bsp/qemu-virt64-aarch64/`：RT-Thread BSP 与 `.config`。
- `docs/SCHEDULE.md`：可调度性测试 (test-schedule) 使用指南。
- `docs/REALTIME.md`：实时性能测试 (test-realtime) 使用指南。
- `docs/STRESS.md`：压力测试 (test-stress) 使用指南。
- `docs/CMD.md`：Shell 命令支持测试 (test-cmd) 使用指南。

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

### OneOS (ARM Cortex-M)
| 项目 | 说明 |
|------|------|
| 开发工具 | OneOS Studio IDE (基于 Eclipse) |
| 工具链 | arm-none-eabi-gcc (内置于 OneOS Studio) |
| 构建系统 | SCons (内置于 OneOS Cube) |
| 平台层 | `generator/platform/oneos/` |
| 入口文件 | `generator/oneos_entry.c` |
| 集成方式 | Git submodule + SConscript |

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
- **POSIX 契约**：各平台通过 `generator/platform/<platform>/` 提供 POSIX 垫片，RT-Thread 通过适配层支持，OneOS 通过原生 API + POSIX 补丁支持

## OneOS 集成经验（2026-02-09 验证）

### 编译验证结果
- **成功编译**：stub, busywait, CUSUM, EWMA, FAST(含benchmark), PID(含benchmark)
- **无法编译**：MODBUS/MQTT(需socket)、EKF/EPNP/ICP(需C++11/Eigen)

### OneOS POSIX 支持情况
OneOS 提供了较完整的 POSIX 支持，但有以下缺口：

| POSIX 功能 | OneOS 状态 | 解决方案 |
|------------|------------|----------|
| pthread 基础 | ✅ 完整支持 | 直接使用 |
| semaphore | ✅ 完整支持 | 平台层用原生 API 更稳定 |
| clock_gettime | ⚠️ 仅 CLOCK_REALTIME | shadow time.h 映射 CLOCK_MONOTONIC |
| pthread_attr_setinheritsched | ❌ 未实现 | posix_sched_adapter.c 补充 |
| mqueue | ✅ 完整支持 | 直接使用 |

### 关键技术点

**1. 影子头文件技术 (Shadow Header)**
```
include/time.h:
  - 使用 #include_next 包含真正的 time.h
  - 重定义 CLOCK_MONOTONIC 为 CLOCK_REALTIME
  - 提供 clock_gettime 声明
```

**2. LOCAL_CCFLAGS 注入**
```python
# SConscript 中使用 LOCAL_CCFLAGS 只影响本代码组
LOCAL_CCFLAGS = ' -include ' + sched_adapter_h
group = AddCodeGroup(..., LOCAL_CCFLAGS=LOCAL_CCFLAGS)
```

**3. 平台层选择策略**
- 优先使用 OneOS 原生 API（os_sem_t, os_timer_t）而非 POSIX wrapper
- 原生 API 更稳定，避免 POSIX 层的额外抽象开销

### 快速检查 RTOS POSIX 支持的方法
```bash
# 1. 查看 POSIX 头文件结构
ls <rtos>/osal/posix/include/

# 2. 搜索函数声明
grep -r "^extern\|^int " <rtos>/osal/posix/include/

# 3. 搜索函数实现
grep -r "EXPORT_SYMBOL" <rtos>/osal/posix/source/

# 4. 检查 switch 语句了解实际支持的选项
grep -A10 "switch.*clockid" <rtos>/osal/posix/source/
```

### 常见问题
1. **"implicit declaration" 警告**：C 代码可继续链接，C++ 会报错
2. **"multiple definition" 错误**：检查是否与 RTOS 内置实现冲突
3. **影子头文件影响全局**：使用 LOCAL_CPPPATH 或 LOCAL_CCFLAGS 限制作用域

以上约定若有更新，请同步修改本文件。
