# OneOS 哪吒派 D1H Win 直连调试闭包文档

本文面向后续远端 ChatGPT/同事查询，记录当前 OneOS 哪吒派 D1H 在“只依赖 Windows 电脑 + 板子直连”的快速 debug、编译、部署、验收背景。目标是摆脱 `rtbench` 共享 TFTP 主机故障对单板调试的影响，同时解释 OneOS 启动镜像和 `.out` 动态模块的区别与联系。

## 1. 当前网络拓扑

推荐直连拓扑：

```text
Windows 有线网口 192.168.31.100/24
        |
        | 直连网线或同一小交换机
        |
OneOS 哪吒派 e00 192.168.31.211/24
```

Windows 侧已验证：

```powershell
ping -S 192.168.31.100 -n 4 192.168.31.211
Test-NetConnection 192.168.31.211 -Port 23
arp -a | findstr 192.168.31.211
```

成功标志：

```text
192.168.31.211 ping 0% loss
TcpTestSucceeded : True
192.168.31.211  00-e0-4c-80-f6-fc
```

板端串口确认：

```sh
ifconfig
default_netif e00
telnetd start
ping 192.168.31.100
```

当前已确认：Windows 直连板子可 ping 通，Telnet 23 端口可达。

## 2. Windows 本地 TFTP

当 `rtbench:/tftp` 不可用时，使用 Windows 本地只读 TFTP server 直接给板子发 `.out`。脚本：

```text
utils/remote-test/win_tftp_ro_server.py
```

启动命令：

```powershell
python utils\remote-test\win_tftp_ro_server.py `
  --root "C:\Users\hzt\yihui-workspace\oneos-nezha-artifacts\accepted\oneos-nezha-d1h-acceptance-20260610_132750" `
  --bind 0.0.0.0 `
  --port 69
```

如果 Windows 防火墙拦截 UDP 69，管理员 PowerShell 添加规则：

```powershell
New-NetFirewallRule -DisplayName "OneOS Local TFTP UDP69" -Direction Inbound -Protocol UDP -LocalPort 69 -Action Allow
```

板端验证命令：

```sh
ping 192.168.31.100
rm /user/wintftp_test.out
tftp_client 192.168.31.100 get wlfull.out /user/wintftp_test.out
ls /user
```

成功标志：

```text
TFTP client get file end, err=0
wintftp_test.out
```

本地 TFTP server 兼容以下别名：

```text
oneos-nezha-d1h-current.out            -> ctest.out
oneos-nezha-d1h-schedule-current.out   -> schedrun.out
oneos-nezha-d1h-realtime-current.out   -> rtrt.out
oneos-nezha-d1h-workloads-current.out  -> wlrun.out
oneos-nezha-d1h-stress-current.out     -> strun.out
oneos-nezha-d1h-testall-current.out    -> allrun.out
wlfull.out                             -> wlfull.out
```

## 3. 已验收二进制归档

最终通过验收的 `.out` 文件保存在 Windows：

```text
C:\Users\hzt\yihui-workspace\oneos-nezha-artifacts\accepted\oneos-nezha-d1h-acceptance-20260610_132750
```

仓库记录：

```text
platforms/oneos/NEZHA_D1H_ACCEPTED_BINARIES_20260610.md
```

核心文件：

```text
ctest.out     主 RTOS-Bench 模块
schedrun.out  test-schedule runner
rtrt.out      test-realtime runner
wlrun.out     workloads quick runner
strun.out     test-stress runner
allrun.out    quick test-all + export runner
wlfull.out    non-quick workloads-only runner
SHA256SUMS.txt
```

## 4. OneOS 启动镜像 vs `.out` 动态模块

### 4.1 启动镜像是什么

OneOS 启动镜像是板子启动时运行的系统本体，包含：

- OneOS kernel
- BSP/board 驱动
- 网络栈、shell、telnetd
- 应用入口 `application/main.c`
- 如果静态编入，还可以包含 `rtbench_cmd_stub.c`

当前哪吒派 image 工程路径：

```text
\\wsl.localhost\Ubuntu\home\hzt\oneos-benchmark-v1.6\OneOS-Benchmark-V1.6\projects\d1h-nezha
```

工程类型：

```cmake
set(PRO_TYPE image)
set(PRO_ARCH riscv64-c900)
```

构建输出：

```text
out/oneos.elf
out/oneos.bin
out/oneos.img
out/oneos_symtbl.txt
```

启动镜像一旦运行，串口出现 `sh />` 或 `sh /user>`，说明 OneOS image 已启动。后续在 shell 中执行 `ld` 加载 `.out`，不是重新启动系统。

### 4.2 `.out` 是什么

`.out` 是 OneOS 运行态动态模块，通过 shell 中的 `ld` 加载：

```sh
ld /user/ctest.out
```

`.out` 加载后会：

1. 被 OneOS 模块加载器映射到内核地址空间。
2. 解析模块符号。
3. 执行模块内的 `module_init(...)` 初始化函数。

本项目中：

```text
ctest.out
  = OneOS out 模板代码
  + RTOS-Bench OneOS 入口
  + test-schedule
  + test-realtime
  + test-stress
  + test-cmd
  + result_export
  + 当前可编译 workloads
```

runner `.out` 则是小启动器，例如 `schedrun.out`。它本身不实现完整测试，而是在 `module_init(runner_init)` 中找到 `/user/ctest.out` 的 `cmd_rtbench_stub` 并调用。

### 4.3 两者联系

```text
oneos.img / oneos.bin
  启动系统、提供 shell、网络、模块加载器、telnetd
        |
        | shell: tftp_client / ld
        v
/user/ctest.out
  动态加载 RTOS-Bench 主模块，暴露 cmd_rtbench_stub
        |
        | ld runner.out
        v
runner.out
  自动找 /user/ctest.out，构造 argv，调用测试入口
```

因此：

- 启动镜像坏了：板子进不了 OneOS shell，不能执行 `ifconfig`、`ld`。
- `.out` 坏了：OneOS shell 还在，但 `ld /user/ctest.out` 或 runner 执行失败。
- 网络坏了：串口仍可用，但 TFTP/Telnet 不通。

## 5. 内核 image 构建逻辑

内核 image 工程：

```text
projects/d1h-nezha
```

关键文件：

```text
project_path.cmake
CMakeLists.txt
application/CMakeLists.txt
application/main.c
application/rtbench_cmd_stub.c
board/setup/link.lds
```

根 `CMakeLists.txt` 数据流：

1. 读取 `project_path.cmake`，确定 `PRO_TYPE=image`、`KERNEL_LIB_DIR`、`BSP_LIB_DIR`。
2. include `toolchain_riscv64`，选择 RISC-V C906 工具链。
3. 设置 image 编译选项：`-march=rv64imafdxthead`、`-mabi=lp64d`、`-mtune=c906`、`-fno-pic`。
4. 处理 kernel/BSP `include.txt`，生成 response file 给 gcc。
5. 创建 `oneos` 可执行目标。
6. 加入 `board`、`application`。
7. 链接 `application`、`bsp`、`board`、`liboneos`。
8. 预处理 `board/setup/link.lds` 生成 `build/linker.lds`。
9. 链接生成 `out/oneos.elf`。
10. `objcopy` 生成 `out/oneos.bin`。
11. `mkbootimg` 生成 `out/oneos.img`。
12. 提取符号表。

对应 CMake 片段：

```cmake
add_subdirectory(board)
add_subdirectory(application)

target_link_libraries(oneos
    PRIVATE
    -Wl,--start-group
    -Wl,--whole-archive
    $<LINK_ONLY:application>
    $<LINK_ONLY:bsp>
    $<LINK_ONLY:board>
    ${OBJECT_LIBS}
    -Wl,--no-whole-archive
    $<LINK_ONLY:gcc>
    -static-libstdc++
    -Wl,--end-group
)
```

生成镜像：

```cmake
${CMAKE_OBJCOPY} -R .reserved_ram -O binary out/oneos.elf out/oneos.bin
${PRO_ROOT}/mkbootimg --kernel out/oneos.bin --base 0x43000000 --kernel_offset 0x0 --ramdisk /dev/null --board "d1h" -o out/oneos.img
```

典型构建命令：

```bash
cd ~/oneos-benchmark-v1.6/OneOS-Benchmark-V1.6/projects/d1h-nezha
/home/hzt/oneos-tools/cmake-3.27.9-linux-x86_64/bin/cmake -S . -B build
/usr/bin/make -C build -j
```

## 6. `.out` 模块构建逻辑

动态模块工程：

```text
projects/d1h-nezha_out
```

工程类型：

```cmake
set(PRO_TYPE out)
set(PRO_ARCH riscv64)
```

关键点：

- `add_executable(${PROJECT_NAME})`
- 链接选项包含 `-shared`、`-nostdlib`、`-e 0x0`、`-T link.lds`
- 输出后缀 `.out`
- RTOS-Bench 通过 `add_subdirectory(RTOS-Bench)` 编入主模块

典型构建命令：

```bash
cd ~/oneos-benchmark-v1.6/OneOS-Benchmark-V1.6/projects/d1h-nezha_out
/home/hzt/oneos-tools/cmake-3.27.9-linux-x86_64/bin/cmake -S . -B build
/usr/bin/make -C build -j
```

输出：

```text
out/d1h-nezha_out.out
```

当前验收中归档为：

```text
ctest.out
```

## 7. `rtbench` 命令注册逻辑

`ctest.out` 内部有：

```c
SH_CMD_EXPORT(rtbench, cmd_rtbench, "RTOS-Bench workload runner");

int cmd_rtbench_stub(int argc, char **argv) {
    return cmd_rtbench(argc, argv);
}
```

但在当前哪吒 image 下，动态 `.out` 里的 `SH_CMD_EXPORT` 不一定被主 shell 稳定注册，所以串口可能出现：

```sh
rtbench: Command not found.
```

这不代表 `ctest.out` 没加载，也不代表 RTOS-Bench 不可用。runner 绕开 shell 注册，直接找符号：

```c
handle = os_module_find("/user/ctest.out");
os_module_symbol_find_by_handle(handle, "cmd_rtbench_stub", &symbol_addr);
target_cmd(argc, argv);
```

如果希望串口直接支持：

```sh
rtbench -L
rtbench test-schedule
rtbench test-all --quick -o /user/nezha_testall.json
```

需要把 `application/rtbench_cmd_stub.c` 编进 **内核 image**，并确保它查找的模块路径是当前板端路径，例如：

```c
const char *module_name = "/user/ctest.out";
```

然后重新编译并启动新的 `oneos.img`。这样 `rtbench` 命令由 image 静态注册，动态模块只提供 `cmd_rtbench_stub` 实现。

## 8. Win 直连部署命令

先启动 Windows 本地 TFTP server，然后在板端执行：

```sh
tftp_client 192.168.31.100 get oneos-nezha-d1h-current.out /user/ctest.out
ld /user/ctest.out

tftp_client 192.168.31.100 get oneos-nezha-d1h-schedule-current.out /user/schedrun.out
ld /user/schedrun.out

tftp_client 192.168.31.100 get oneos-nezha-d1h-realtime-current.out /user/rtrt.out
ld /user/rtrt.out

tftp_client 192.168.31.100 get wlrun.out /user/wlrun.out
ld /user/wlrun.out

tftp_client 192.168.31.100 get strun.out /user/strun.out
ld /user/strun.out

tftp_client 192.168.31.100 get allrun.out /user/allrun.out
ld /user/allrun.out

tftp_client 192.168.31.100 get wlfull.out /user/wlfull.out
ld /user/wlfull.out
```

通过标志：

```text
test-schedule ret=0
test-realtime ret=0
workloads ret=0
test-stress ret=0
test-all ret=0
Results saved to: /user/nezha_testall.json
Results saved to: /user/wlfull.json
```

## 9. 已知边界

- 当前 OneOS D1H `.out` 实际注册 workloads 为 `stub`、`busywait`、`cusum`、`ewma`，不是 9 个。
- `schedrun.out` 是 quick schedule runner，只跑 `cycles=1`、`util=30%`。
- `wlfull.out` 是非 quick workloads-only runner，当前 4 个 workloads 均以 `rounds=10` 通过。
- `test-realtime` 当前 image 下 interrupt latency 采样为 `0/200`，作为平台中断插桩覆盖缺口记录。
- `test-cmd` 中 `ps` 不支持，整体命令支持率为 `10/11`。
- `stress all-quick` 中 bsearch/ternary 会打印 fail，但 stress 汇总仍返回 passed。

## 10. 关键日志和证据

完整验收摘要：

```text
utils/remote-test/logs/oneos-nezha-d1h-acceptance-20260610_132750/ACCEPTANCE_SUMMARY.md
```

通过验收的二进制 manifest：

```text
platforms/oneos/NEZHA_D1H_ACCEPTED_BINARIES_20260610.md
```

共享 TFTP 历史 SOP：

```text
platforms/oneos/NEZHA_D1H_SHARED_TFTP_SOP.md
```

Win 本地 TFTP server：

```text
utils/remote-test/win_tftp_ro_server.py
```
