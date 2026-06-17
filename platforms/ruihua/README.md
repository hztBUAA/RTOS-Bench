# Ruihua/ReWorks 飞腾派 SOP

本文档记录当前可用的锐华飞腾派调试方式。结论先行：**不要从空工程重新摸索，优先复制并使用已经跑通的样板工程**：

```text
<FEITENG_PROJECT_ROOT>
```

RTOS-Bench 源码仍使用本仓库：

```text
<RTOS_BENCH_ROOT>
```

当前接入方式不是运行独立 `.out` 动态模块，而是把 RTOS-Bench 直接链接进 ReWorks 自启动镜像。本地构建产物文件名是 `reworks.elf`，但上传到共享 rtbench TFTP 时必须改用飞腾派专用文件名；不要覆盖 `/tftp/reworks.elf`，该通用文件可能属于龙芯派或其他板子。板子重启后由 U-Boot 通过 TFTP 拉取 `bootcmd` 指向的飞腾专用镜像，ReWorks 启动完成后通过 telnet 进入 `reworks>` shell，调用 `rtbench_*` wrapper 完成验证。

## 当前环境

本文档用占位变量描述路径，避免绑定到某一台 Windows 电脑：

| 变量 | 含义 | 当前验收机器示例 |
| --- | --- | --- |
| `<REWORKS_SDK_ROOT>` | Ruihua/ReWorks SDK 根目录 | `C:\rtos\6.1.1-ARM` |
| `<FEITENG_PROJECT_ROOT>` | 飞腾派 ReWorks 样板工程根目录 | `<REWORKS_SDK_ROOT>\workspace\feiteng4rtos` |
| `<RTOS_BENCH_ROOT>` | RTOS-Bench 仓库根目录 | `...\rtos-bench\RTOS-Bench` |
| `<BUILD_DIR>` | 飞腾派镜像构建目录 | `<FEITENG_PROJECT_ROOT>\gnuaarch64\FTE2000_SMP-64` |
| `<ACCEPTED_ARTIFACT_ROOT>` | Windows 本机验收二进制归档根目录 | `...\rtos-bench-artifacts\ruihua\feiteng` |

`user.mk` 中建议把 Windows 路径写成正斜杠形式，例如把 `<RTOS_BENCH_ROOT>` 转成 `<RTOS_BENCH_ROOT_WITH_FORWARD_SLASHES>`，避免 Makefile 把反斜杠当作转义字符。

| 项目 | 当前值 |
| --- | --- |
| RTOS | Ruihua ReWorks 6.1.1 ARM |
| Board | 飞腾派 / Phytium Pi, AArch64 SMP |
| SDK | `<REWORKS_SDK_ROOT>` |
| 样板工程 | `<FEITENG_PROJECT_ROOT>` |
| 仓库模板 | `platforms/ruihua/feiteng4rtos-template` |
| 构建目录 | `<BUILD_DIR>` |
| 输出镜像 | `...\FTE2000_SMP-64\reworks.elf` |
| 校园网板端 IP | `192.168.31.210:23` |
| 直连调试板端 IP | `192.168.2.100:23` |
| ReWorks prompt | `reworks>` |

校园网部署时，当前 rtbench 主机 TFTP 文件为：

```text
rtbench@10.134.151.45:/tftp/ruihua-feiteng-reworks-192.168.31.210-20260602.elf
```

同一目录下保留了按时间戳命名的历史镜像备份。共享 TFTP 上的 `/tftp/reworks.elf` 不属于飞腾验收流程，禁止覆盖。

完整验收通过的飞腾二进制在 Windows 本机归档为：

```text
<ACCEPTED_ARTIFACT_ROOT>\20260615_134402\reworks.elf
```

对应信息：

```text
remote: rtbench@10.134.151.45:/tftp/ruihua-feiteng-reworks-192.168.31.210-20260602.elf
size:   11092248
sha256: 90BCF57E9B4FBABC6FE1FDC13E9E2D165A6DD475089608CBFC1598740F000DD5
logs:   utils/remote-test/logs/ruihua-rtosbench-acceptance-20260615_134402
```

## 最短闭环 SOP

当前飞腾派不是上传 `.out` 动态模块运行，而是“重新编译完整自启动镜像 -> TFTP 部署 -> U-Boot 拉取镜像 -> telnet 进入 ReWorks shell 执行 `rtbench_*` 命令”。后续换到新的服务器或新的交换机时，只要保证服务器和板子在同一个可达网络里，并让 U-Boot 的 `serverip` 指向新的 TFTP 服务器即可复现。

整体链路：

```text
Windows/ReDe 工程编译 reworks.elf
  -> 上传到 TFTP 服务器的飞腾专用文件名
  -> 板子 U-Boot 通过 bootcmd/tftpboot 拉取该文件
  -> ReWorks 启动，运行期 IP 来自 usrArgs.h
  -> 从同网段服务器 telnet 192.168.31.210 23
  -> 在 reworks> 执行 rtbench_* 验收命令
```

新服务器 / 新交换机场景检查项：

1. 板子网口和新服务器接到同一交换机/VLAN。
2. 新服务器配置静态 IP，并能与板端运行期 IP 互通，例如 `ping 192.168.31.210`。
3. 新服务器启动 TFTP 服务，TFTP 根目录中放飞腾专用镜像文件，不使用 `/tftp/reworks.elf`。
4. 通过串口进入 U-Boot，把 `serverip` 改为新服务器 IP，`ipaddr` 改为板子 U-Boot 阶段 IP，`bootcmd` 指向飞腾专用文件名。
5. ReWorks 启动后，从新服务器执行 `telnet 192.168.31.210 23`，看到 `reworks>` 后再跑验收命令。

## 样板工程结构

这些文件是当前飞腾派适配需要的最小样板。仓库内已经提供对应模板：

```text
platforms/ruihua/feiteng4rtos-template
```

使用时先以 `<FEITENG_PROJECT_ROOT>` 这个可用 ReDe 工程为底座，再把模板中的手工维护文件覆盖到工程对应位置。`rtosbench_port` **仍然需要**，它不是可删除的临时目录，而是 Ruihua 工程侧 glue layer：负责开机 smoke、手工注册 workload，并让 `test-schedule` 使用 workload registry。

```text
<FEITENG_PROJECT_ROOT>
├── usrInit.c                         # ReWorks 用户入口，调用 rtosbench_boot_smoke()
├── usrArgs.h                         # ReWorks 运行期网卡 IP，例如 192.168.31.210
├── makefile.conf                     # IDE 生成/维护的 BSP 与 AArch64 CSP 配置
├── gnuaarch64\
│   ├── user.mk                       # RTOS-Bench 接入点，手工维护
│   └── FTE2000_SMP-64\
│       ├── makefile                  # IDE 生成，不手工编辑
│       ├── subdir.mk                 # 自动 include $(ROOT)/user.mk
│       └── rtosbench_port\subdir.mk  # IDE 生成 rtosbench_port/*.c 编译规则
├── les\
│   └── bench_init.c                  # 包含仓库维护的 realtime wrapper
└── rtosbench_port\
    ├── rtosbench_boot.c              # 开机后调用 help/list/schedule quick
    ├── ruihua_workloads.c            # 注册 stub/busywait/ruihua-smoke
    └── sched_workloads_stub.c        # 禁用旧 schedule wrapper，改走注册表 workload
```

### `user.mk` 必要点

`gnuaarch64/user.mk` 是手工维护入口。至少需要：

```makefile
RTOSBENCH_SRC := <RTOS_BENCH_ROOT_WITH_FORWARD_SLASHES>
RTOSBENCH_PORT := ../../rtosbench_port
RTOSBENCH_OBJDIR := ./rtosbench

RTOSBENCH_CFLAGS := \
	-DRUIHUA_PLATFORM \
	-DRTOSBENCH_USE_MANUAL_WORKLOAD_REGISTRATION \
	-DRTBENCH_NO_STANDALONE_MAIN \
	-I"$(RTOSBENCH_SRC)/generator" \
	-I"$(RTOSBENCH_SRC)/generator/test_schedule" \
	-I"$(RTOSBENCH_SRC)/generator/realtime_orig/les" \
	-I"$(RTOSBENCH_SRC)/generator/realtime_orig/realtime" \
	-I"$(RTOSBENCH_SRC)/generator/realtime_orig/multicore" \
	-I"$(RTOSBENCH_SRC)/generator/realtime_orig/verify" \
	-I"$(RTOSBENCH_SRC)/generator/stress_orig" \
	-I"$(RTOSBENCH_SRC)/generator/stress_orig/common" \
	-I"$(RTOSBENCH_SRC)/generator/stress_orig/osal" \
	-I"$(RTOSBENCH_SRC)/generator/stress_orig/stressor" \
	-I"$(RTOSBENCH_SRC)/workloads" \
	-I"$(RTOSBENCH_PORT)"

rtosbench_port/%.o: CPPFLAGS += $(RTOSBENCH_CFLAGS)
les/%.o: CPPFLAGS += $(RTOSBENCH_CFLAGS)
```

`rtosbench_port/%.o` 这一行用于修复 IDE 自动规则缺少 RTOS-Bench include 的问题，否则 `ruihua_workloads.c` 会报 `workload_registry.h` 找不到。`les/%.o` 这一行用于确保 realtime wrapper 以 `RUIHUA_PLATFORM` 编译，避免 CPU affinity 类型走到 Linux 分支。

`RTOSBENCH_OBJS` 需要包含核心框架、Ruihua platform、`test_realtime.c`、`test_stress.c` 以及 stress/realtime 原始模块。不要把 IDE 自动生成的 `rtosbench_port/*.o` 再重复加进 `RTOSBENCH_OBJS`。

### `usrInit.c` 接入点

```c
extern void rtosbench_boot_smoke(void);

void UserInit(void)
{
    printf("[feiteng4rtos] UserInit enter\n");
    rtosbench_boot_smoke();
    printf("[feiteng4rtos] UserInit leave\n");
}
```

## 编译

编译发生在 Windows 的 Ruihua/ReDe SDK 工程中。修改 RTOS-Bench 仓库代码后，样板工程通过 `gnuaarch64/user.mk` 直接引用仓库源码，因此需要重新构建该 ReWorks 自启动工程。

在 Windows 上进入构建目录：

```bat
cd /d <BUILD_DIR>
```

完整重编：

```bat
cmd /c "set PATH=<REWORKS_SDK_ROOT>\tools\bin;<REWORKS_SDK_ROOT>\tools\build\gnuaarch64\bin;%PATH%&& gnu_make clean all"
```

只做增量重编时可用：

```bat
cmd /c "set PATH=<REWORKS_SDK_ROOT>\tools\bin;<REWORKS_SDK_ROOT>\tools\build\gnuaarch64\bin;%PATH%&& gnu_make all"
```

成功后会生成：

```text
reworks.elf
reworks
feiteng4rtos.obj
```

记录镜像 hash：

```powershell
Get-FileHash -Algorithm SHA256 "<BUILD_DIR>\reworks.elf"
```

## 部署

### 校园网 / rtbench TFTP

将本地构建出的 `reworks.elf` 上传到 rtbench TFTP，必须使用飞腾派专用文件名，不要覆盖其他板子的镜像，尤其不要写入 `/tftp/reworks.elf`：

```powershell
scp -P 1026 "<BUILD_DIR>\reworks.elf" `
  rtbench@10.134.151.45:/tftp/ruihua-feiteng-reworks-192.168.31.210-YYYYMMDD.elf
```

如果 U-Boot 的 `bootcmd` 已经指向固定文件名，需要更新或覆盖该**飞腾固定文件**。覆盖前先备份，仍然不要碰 `/tftp/reworks.elf`：

```bash
cp /tftp/ruihua-feiteng-reworks-192.168.31.210-20260602.elf \
   /tftp/ruihua-feiteng-reworks-192.168.31.210-YYYYMMDD.before-change.elf
cp /tftp/ruihua-feiteng-reworks-192.168.31.210-YYYYMMDD.elf \
   /tftp/ruihua-feiteng-reworks-192.168.31.210-20260602.elf
```

板端 ReWorks 运行期 IP 由样板工程 `usrArgs.h` 中的 `GEM0_IPV4_ADDR` 决定。校园网验收使用：

```c
#define GEM0_IPV4_ADDR "192.168.31.210"
```

### 新 TFTP 服务器

如果后续换到新的服务器和交换机，部署方式保持不变，只替换服务器地址和 U-Boot 环境。假设新服务器 IP 是 `<NEW_TFTP_SERVER_IP>`，TFTP 根目录是 `/tftp`：

```bash
cp reworks.elf /tftp/ruihua-feiteng-reworks-192.168.31.210-YYYYMMDD.elf
```

串口进入 U-Boot 后设置示例：

```text
setenv ipaddr 192.168.31.210
setenv serverip <NEW_TFTP_SERVER_IP>
setenv netmask 255.255.255.0
setenv gatewayip <GATEWAY_IP>
setenv bootcmd 'tftpboot 0x80000000 ruihua-feiteng-reworks-192.168.31.210-YYYYMMDD.elf; bootelf 0x80000000'
saveenv
boot
```

实际加载地址和启动命令以现场串口里原有 `bootcmd` 为准；迁移时优先只替换文件名和 `serverip`，不要无依据改启动地址。

### Windows 直连调试

直连调试时，Windows 有线网卡建议配置为：

```text
IP address: 192.168.2.61
Netmask:    255.255.255.0
Gateway:    留空
DNS:        留空
```

TFTP 根目录指向：

```text
<BUILD_DIR>
```

直连模式下 ReWorks 运行期 IP 需要与镜像匹配，常用值是：

```c
#define GEM0_IPV4_ADDR "192.168.2.100"
```

## 启动和连接

重启或重新上电板子，让 U-Boot 从 TFTP 拉取 `bootcmd` 指向的飞腾派专用镜像，例如 `/tftp/ruihua-feiteng-reworks-192.168.31.210-20260602.elf`。启动完成后连接 telnet：

```bat
telnet 192.168.31.210 23
```

或直连模式：

```bat
telnet 192.168.2.100 23
```

看到 `reworks>` 后再执行 RTOS-Bench 命令。`reboot` 命令在当前板上不够可靠，可能触发复位后不自动恢复；验收阶段优先使用现场复位或串口观察启动过程。

从 `rtbench` 服务器或新的同网段服务器登录时：

```bash
telnet 192.168.31.210 23
```

进入后常用交互过程：

```text
reworks> rtbench_list
reworks> rtbench_test_realtime
reworks> rtbench_test_schedule_cycles3
reworks> rtbench_test_stress
reworks> rtbench_test_all_quick
reworks> rtbench_export_result_default
reworks> cat /rtbench_result.json
```

命令正常完成时会打印 `0x00000000 (0)` 并回到 `reworks>`。如果只是单独测试实时性能，执行 `rtbench_test_realtime` 即可。

## 验收命令

建议按从轻到重执行：

```text
rtbench_help
rtbench_list
rtbench_test_schedule_cycles3
rtbench_test_realtime
rtbench_fast
rtbench_epnp
rtbench_ekf
rtbench_icp
rtbench_modbus
rtbench_mqtt
rtbench_pid
rtbench_cusum
rtbench_ewma
rtbench_test_stress
rtbench_test_all_quick
rtbench_export_result_default
cat /rtbench_result.json
```

当前 Ruihua wrapper 对 shell 验收做了有界化：

- `rtbench_test_stress` 默认执行 `test-stress --job all-quick`。
- `rtbench_test_all` 和 `rtbench_test_all_quick` 默认执行 `test-all --schedule-cycles 3 --stress-job all-quick`。
- `rtbench_modbus` / `rtbench_mqtt` 在 Ruihua 飞腾 port 层使用有界离线模型，避免板端 TCP loopback 线程模型导致 shell 不稳定。

这样仍覆盖 realtime、schedule、stress、cmd、workload，但不会让 stress 的 `job=all` 在 telnet 会话中长时间占用。

验收通过时应看到：

```text
[test-schedule] ... Final Score: 100.00 / 100
[test-stress] Job 'all-quick' completed: 27 stressor runs
[test-cmd] Result: 10/10 commands supported
[RTOS-Bench] Results saved to: /rtbench_result.json
0x00000000 (0)
reworks>
```

## 日志归档

每次验收至少记录：

- 构建命令和构建日志。
- `reworks.elf` SHA256。
- TFTP 目标文件名。
- telnet 原始输出。
- 是否回到 `reworks>`。

仓库内建议按时间戳保存到：

```text
utils/remote-test/logs/ruihua-feiteng-fullmodules-YYYYMMDD_HHMM/
```

完整验收闭环建议保留这些文件：

```text
ENVIRONMENT.md
git_status.txt
test_schedule_*.log / .rc
test_realtime_*.log / .rc
realtime_metrics_*.md
workloads_serial_*.log
workload_<name>_*.log / .rc
workloads_matrix_*.md
test_stress_*.log / .rc
runner_testall_*.log
testall_*.rc
export_result_*.log / .rc
feiteng_export_*.json
feiteng_flattened_*.json
failures_and_fixes.md
SUMMARY.md
```

当前已验收通过的归档位置：

```text
binary: <ACCEPTED_ARTIFACT_ROOT>\20260615_134402\reworks.elf
logs:   utils/remote-test/logs/ruihua-rtosbench-acceptance-20260615_134402
json:   utils/remote-test/logs/ruihua-rtosbench-acceptance-20260615_134402/feiteng_export_20260615_1425.json
report: utils/remote-test/logs/ruihua-rtosbench-acceptance-20260615_134402/feiteng_flattened_20260615_1425.json
```

二进制归档原则：验收通过后把实际启动的飞腾专用 TFTP 镜像复制到 Windows 本机归档目录，记录 size、SHA256、TFTP 源路径和日志目录；二进制本身不提交到 Git 仓库。

## 常见问题

### `workload_registry.h` 找不到

原因是 `rtosbench_port/*.c` 由 IDE 自动规则编译，但没有继承 RTOS-Bench include。确认 `user.mk` 有：

```makefile
rtosbench_port/%.o: CPPFLAGS += $(RTOSBENCH_CFLAGS)
```

### `cpu.h` / `csp_specs` 找不到

检查 `makefile.conf` 是否包含 AArch64 SMP CSP include、`-B` specs 路径、`-specs csp_specs -qrtos`、`-laarch64_smp_csp`。缺少这些配置会导致 `cpu.h` 找不到或 `int_cpu_lock/context_switch` 等底层符号 undefined。

### telnet 不可达

先区分是启动期、IP 配错，还是 shell 被命令卡住：

1. 确认镜像里的 `usrArgs.h` IP 与当前网络模式一致。
2. 从同网段机器 ping 板端 IP。
3. 检查 `23/telnet` 端口。
4. 必要时接串口看 U-Boot 是否成功 TFTP、ReWorks 是否启动到 shell。

### `ld "*.out"` 提示 `Symbol ... already exists`

如果当前板子启动的是本文档验收通过的飞腾自启动镜像，RTOS-Bench 已经被静态链接进 `reworks.elf`，启动后直接在 `reworks>` 下执行 `rtbench_*` 命令即可，不需要再 `ld "rtosbench*.out"`。

在这种镜像上再次加载 `rtosbench.out`、`rtosbench-tbh-feiteng.out` 或其他包含同一套 RTOS-Bench 对象的动态模块，会看到大量：

```text
Symbol 'stress_ng_main' already exists
Symbol 'transport_write' already exists
Symbol 'rtbench_*' already exists
```

这不是 workload 本身运行失败，而是 ReWorks 符号表里已经存在同名函数/全局变量。解决方式：

1. 如果只是跑 RTOS-Bench 验收，不要执行 `ld`，直接运行：

   ```text
   rtbench_list
   rtbench_test_realtime
   rtbench_test_schedule_cycles3
   rtbench_test_stress
   rtbench_test_all_quick
   ```

2. 如果必须调试 `.out` 动态模块，先换回不内置 RTOS-Bench 的干净 ReWorks 基础镜像，再加载该 `.out`。
3. 不要同时加载 `rtosbench.out` 和 `rtosbench-tbh-feiteng.out`。如果前一次动态模块已经成功加载，先用对应模块名 `unld` 卸载；如果只是半加载后留下符号污染，通常需要重启/重新上电清空符号表。
4. 动态模块模式和当前完整验收使用的自启动镜像模式不要混用。飞腾验收以自启动镜像模式为准。

### `test-cmd` 全部失败

旧代码在 Ruihua 上会走 unsupported stub，表现为所有命令 failed。当前 `RUIHUA_PLATFORM` 已使用 ReWorks 内置命令能力列表做 `test-cmd` 验收，避免在 telnet 命令内部重入 shell parser。

### `test-stress` 时间太长

不要在交互验收默认跑 `test-stress --job all`。飞腾派上 `all` 可运行但耗时很长；PR 验收使用 `all-quick`。
