# OneOS 哪吒派 D1H 接入与实地验收 SOP

这份 SOP 记录 OneOS 哪吒派 D1H 的 RTOS-Bench 接入方式、当前验证状态，以及后续拿到真实板卡后应该怎么启动、联网、拉取 `.out` 模块、通过 Telnet 复测。飞腾派 OneOS 的已验证流程也放在这里，作为哪吒派实地验收的参照。

## 当前状态

- OneOS 飞腾派已经在真实板卡上通过 Telnet/TFTP 跑通，`stub`、`epnp` 等典型负载可以返回 shell。
- OneOS 哪吒派 D1H 已经在 WSL/vendor 工程副本中完成全量 `.out` 编译验证。
- OneOS 哪吒派 D1H 尚未做真实板卡运行验收。

哪吒派全量编译已经覆盖：

- 工作负载：`stub`、`busywait`、`cusum`、`ewma`、`fast`、`epnp`、`ekf`、`icp`、`pid`、`modbus`、`mqtt`
- 测试模块：`test-schedule`、`test-realtime`、`test-stress`、`test-cmd`
- 注册与分发代码：`rtbench_workloads.cpp`、`run_all_workloads.cpp`

`workloads/main.cpp` 没有编进 OneOS `.out`，这是预期行为。OneOS 动态模块不是 Linux 可执行程序，不通过 `main()` 入口启动，而是加载 `.out` 后由 shell 命令和模块符号进入 RTOS-Bench。

## OneOS 板卡识别与宏策略

当前代码没有用“哪吒派”“飞腾派”这种板卡名来硬编码条件编译，而是按平台能力和 ABI/API 家族区分：

- `ONEOS_PLATFORM`：由 OneOS 工程编译时定义，表示当前目标是 OneOS。
- `RTBENCH_PLATFORM_ONEOS`：RTOS-Bench 内部的平台层标记，进入 `generator/platform/oneos/` 的实现。
- `ONEOS_V2_MUSL_LIBC`：表示 OneOS V2 风格、使用 musl/libc 已预定义 POSIX 类型的目标。当前自动识别 `__aarch64__`、`_M_ARM64`、`__riscv`。
- `ONEOS_V2_ARM64`：保留给已有 ARM64 OneOS V2 代码路径的兼容宏。

这样做的原因是：RTOS-Bench 真正关心的是 libc/POSIX 类型、OneOS API 形态、C++ 头文件兼容性，而不是板卡商品名。哪吒派 D1H 是 RISC-V musl，所以会走 `ONEOS_V2_MUSL_LIBC`；飞腾派 OneOS V2 ARM64 也会走同类路径。后续如果有新的 OneOS V2/musl 板卡，大概率可以复用这套条件编译，只需要补 BSP 工程、镜像加载地址、网卡名、模块路径等板级 SOP。

## 编译环境

哪吒派 D1H 使用 OneOS multi-CMake/Linux 命令行工程，和飞腾派 OneOS Studio 工程不同。我们本地是在 WSL 里验证的：

```bash
export ONEOS_BUILDER=/home/hzt/oneos-benchmark-v1.6/OneOS-Benchmark-V1.6/oneos-multi-tools
export ONEOS_SCRIPTS=/home/hzt/oneos-benchmark-v1.6/OneOS-Benchmark-V1.6/oneos-multi-cmake
export PATH=/home/hzt/oneos-tools/bin:/home/hzt/oneos-tools/cmake-3.27.9-linux-x86_64/bin:$PATH
```

编译内核镜像工程：

```bash
cd /home/hzt/oneos-benchmark-v1.6/OneOS-Benchmark-V1.6/projects/d1h-nezha
bash ../../oneos-multi-cmake/oos.sh build
```

编译 `.out` 动态模块工程：

```bash
cd /home/hzt/oneos-benchmark-v1.6/OneOS-Benchmark-V1.6/projects/d1h-nezha_out
bash ../../oneos-multi-cmake/oos.sh build
```

预期产物：

```text
projects/d1h-nezha_out/out/d1h-nezha_out.out
```

本地已经归档的哪吒派全量编译证据：

```text
C:\Users\hzt\yihui-workspace\oneos-nezha-logs\24-build-full-workloads-attempt6-cxx-includes.log
C:\Users\hzt\yihui-workspace\oneos-nezha-logs\25-full-rtosbench-build-summary.md
C:\Users\hzt\yihui-workspace\oneos-nezha-artifacts\d1h-nezha_out-full-rtosbench.out
```

## 哪吒派需要的兼容处理

全量编译时主要遇到的是 OneOS V2/musl、C++ 负载和 D1H BSP 头文件之间的兼容问题，不是 RTOS-Bench POSIX 框架的大改。

1. 把 RISC-V/musl 识别为 OneOS V2 libc 目标。
   - RISC-V musl 已经定义了 `clock_t`、`clockid_t`、`timer_t`、pthread 相关类型。
   - RTOS-Bench 不能再重复 typedef，否则会编译冲突。

2. D1H 使用 OneOS V2 风格 API。
   - 当前任务：`os_get_current_task()` / `os_task_id`
   - tick 获取：`os_tick_get_value()`

3. 避免 BSP 的 C 风格 `min/max` 宏污染 C++ 负载。
   - EKF/ECL 和 EPNP/Eigen 会使用 `math::max`、`matrix::min`、`std::min` 这类 C++ 名字空间函数。
   - OneOS 兼容头 `oneos_cxx_compat.hpp` 会预先包含标准 C++ 头，并清理 `min/max` 宏。

4. C++ 文件不要混入 OneOS kernel libc 的 include response 文件。
   - vendor out 模板不应把 `processed_kernel_include.txt`、`processed_board_include.txt` 传给使用 libstdc++/sysroot 的 C++ 文件。
   - 否则 `pthread`、`timespec`、`cpu_set_t` 会同时从 OneOS kernel libc 和工具链 sysroot 出现，导致重复定义。

5. D1H BSP 头文件建议由厂家修正。
   - `sunxi_hal_common.h`、`driver.h` 不应在 `__cplusplus` 下定义 `min/max` 宏。
   - 最好在 BSP 模板里修；如果厂家暂时不改，我们可以在本地带一个小补丁。

## 飞腾派 OneOS 已验证流程

当前已知配置：

```text
板卡 IP：192.168.31.205
Telnet：23
TFTP 服务器：192.168.31.110
板端模块路径：/user/phytium_pi_out.out
```

串口/U-Boot 启动内核：

```text
tftp 0x80100000 oneos.bin
go 0x80100000
```

OneOS 启动后的网络、Telnet、文件系统和模块加载：

```text
set_if e01 192.168.31.205 192.168.31.1 255.255.255.0
default_netif e01
telnetd start
mkdir /user
mount -t fatfs sdmmc0a2 /user
tftp_client 192.168.31.110 get phytium_pi_out.out /user/phytium_pi_out.out
ld /user/phytium_pi_out.out
```

飞腾派冒烟测试命令：

```text
list_lmodule
rtbench --help
rtbench -L
rtbench -b stub -t 1
rtbench -b busywait -t 1
rtbench -b epnp -t 1
```

飞腾派缩减版典型负载验收命令：

```text
rtbench test-all --no-realtime --no-schedule --no-stress --no-cmd
```

如果 `rtbench -b stub -t 1` 卡在 `exec atexit` 附近，优先怀疑板上加载的是旧 `.out`。处理方式是重新编译、上传、卸载旧模块并加载新模块：

```text
unld /user/phytium_pi_out.out
unld /user/phytium_pi_out.out
tftp_client 192.168.31.110 get phytium_pi_out.out /user/phytium_pi_out.out
ld /user/phytium_pi_out.out
```

正常跑完的典型输出会包含 `Execution environment setup complete`、`Job completed`、`Cleaning up job environment`，最后回到 `sh /user>`。

## 哪吒派 D1H 实地验收计划

哪吒派建议按飞腾派流程验收，但镜像名、模块名、网卡名、挂载分区和 U-Boot 加载地址要以哪吒派 vendor 模板为准。

多人协作时，TFTP 源文件不要直接放在 `/tftp` 根目录裸名覆盖。推荐使用共享主机
`rtbench:/tftp/oneos/nezha-d1h/<YYYYMMDD_HHMMSS>/` 归档产物，并将通过基础检查的一组产物发布到
`/tftp/oneos/nezha-d1h/current/`。板端 `/user` 仍使用稳定短名，例如 `/user/ctest.out`，
因为 runner 会按固定模块路径查找主模块。完整共享 TFTP 流程见
[NEZHA_D1H_SHARED_TFTP_SOP.md](NEZHA_D1H_SHARED_TFTP_SOP.md)。

```text
内核镜像工程：d1h-nezha
out 模块工程：d1h-nezha_out
预期模块名：d1h-nezha_out.out
建议板端路径：/user/d1h-nezha_out.out
```

OneOS 启动后，先配置网络、启动 Telnet、挂载 `/user`，再从 TFTP 服务器拉取 `.out`：

```text
set_if <netif> <board-ip> <gateway-ip> <netmask>
default_netif <netif>
telnetd start
mkdir /user
mount -t fatfs <storage-partition> /user
tftp_client <tftp-server-ip> get d1h-nezha_out.out /user/d1h-nezha_out.out
ld /user/d1h-nezha_out.out
```

如果使用共享 TFTP 的 `current` 目录，主模块部署命令示例：

```text
tftp_client 192.168.31.110 get oneos/nezha-d1h/current/ctest.out /user/ctest.out
ld /user/ctest.out
```

如果内核镜像里使用了 `rtbench_cmd_stub.c` 这种固定转发入口，要确认它查找的模块路径和实际 `ld` 的路径一致。例如 stub 仍然查 `/user/phytium_pi_out.out`，那就要么沿用这个板端文件名，要么把 stub 改成 `/user/d1h-nezha_out.out` 后重新编译内核镜像。

哪吒派最小运行验收命令：

```text
list_lmodule
rtbench --help
rtbench -L
rtbench -b stub -t 1
rtbench -b busywait -t 1
rtbench -b epnp -t 1
rtbench -b ekf -t 1
rtbench -b icp -t 1
rtbench test-schedule --cycles 1 --util-start 30 --util-end 30 --util-step 30
```

基础负载都能返回 shell 后，再跑缩减版典型负载验收：

```text
rtbench test-all --no-realtime --no-schedule --no-stress --no-cmd
```

第一次上板时，不建议一开始就跑完整 `test-all`。`test-realtime`、`test-schedule`、`test-stress` 建议分开跑，避免长时间测试掩盖模块加载、C++ 栈空间或网络配置问题。

## 校园网/实验室路由器接入建议

目标是让同一个实验室网络里的开发者都能 Telnet 到板卡，并且板卡能从固定 TFTP 服务器拉取镜像或 `.out`。

1. 把 TFTP 服务器、开发机和板卡放到同一个二层网络。
   - 示例网段：`192.168.31.0/24`
   - 路由器/网关：`192.168.31.1`
   - TFTP 服务器：`192.168.31.110`
   - 飞腾派：`192.168.31.205`
   - 哪吒派建议预留：`192.168.31.206`

2. 如果 BSP 没有持久化网络配置，每次启动后都手动设置静态 IP：

```text
set_if <netif> <board-ip> 192.168.31.1 255.255.255.0
default_netif <netif>
telnetd start
```

3. 从开发机确认网络连通：

```text
ping <board-ip>
telnet <board-ip> 23
```

4. 从板卡确认能访问 TFTP：

```text
tftp_client 192.168.31.110 get <module>.out /user/<module>.out
```

5. 如果需要跨宿舍/办公室远程访问，建议通过 VPN 或跳板机进实验室网络。不要把 Telnet 直接暴露到校园公网。如果一定要做路由器端口映射，可以把外部端口如 `3021`、`3023` 映射到不同板卡的 `23`，但只建议在受控内网里使用。

## 每次验收要保存的证据

每次真实板卡验收建议保存这些材料：

- 内核镜像名称、大小和 SHA256
- `.out` 模块名称、大小和 SHA256
- 串口启动日志
- Telnet 操作日志，包含模块卸载、加载和冒烟命令
- `rtbench -L` 输出
- `stub`、`busywait`、`epnp`、`ekf`、`icp` 的运行结果
- `rtbench test-all --no-realtime --no-schedule --no-stress --no-cmd` 输出

建议日志放在：

```text
utils/remote-test/logs/<board>-<date>/
```

并在同目录写一个 `SUMMARY.md`，把镜像、模块、日志和结论路径列出来，方便后续 PR 审查和厂家沟通。
