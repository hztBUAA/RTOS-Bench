# Ruihua/ReWorks 飞腾派 SOP

本文档记录当前可用的锐华飞腾派调试方式。结论先行：**不要从空工程重新摸索，优先复制并使用已经跑通的样板工程**：

```text
C:\rtos\6.1.1-ARM\workspace\feiteng4rtos
```

RTOS-Bench 源码仍使用本仓库：

```text
C:\Users\hzt\yihui-workspace\rtos-bench\RTOS-Bench
```

当前接入方式不是运行独立 `.out` 动态模块，而是把 RTOS-Bench 直接链接进 ReWorks 自启动镜像 `reworks.elf`。板子重启后由 U-Boot 通过 TFTP 拉取 `reworks.elf`，ReWorks 启动完成后通过 telnet 进入 `reworks>` shell，调用 `rtbench_*` wrapper 完成验证。

## 当前环境

| 项目 | 当前值 |
| --- | --- |
| RTOS | Ruihua ReWorks 6.1.1 ARM |
| Board | 飞腾派 / Phytium Pi, AArch64 SMP |
| SDK | `C:\rtos\6.1.1-ARM` |
| 样板工程 | `C:\rtos\6.1.1-ARM\workspace\feiteng4rtos` |
| 构建目录 | `C:\rtos\6.1.1-ARM\workspace\feiteng4rtos\gnuaarch64\FTE2000_SMP-64` |
| 输出镜像 | `...\FTE2000_SMP-64\reworks.elf` |
| 校园网板端 IP | `192.168.31.210:23` |
| 直连调试板端 IP | `192.168.2.100:23` |
| ReWorks prompt | `reworks>` |

校园网部署时，当前 rtbench 主机 TFTP 文件为：

```text
rtbench@10.134.151.45:/tftp/ruihua-feiteng-reworks-192.168.31.210-20260602.elf
```

同一目录下保留了按时间戳命名的历史镜像备份，不要覆盖龙芯派或其他板子的 `reworks.elf`。

## 样板工程结构

这些文件是当前飞腾派适配需要的最小样板。`rtosbench_port` **仍然需要**，它不是可删除的临时目录，而是 Ruihua 工程侧 glue layer：负责开机 smoke、手工注册 workload，并让 `test-schedule` 使用 workload registry。

```text
C:\rtos\6.1.1-ARM\workspace\feiteng4rtos
├── usrInit.c                         # ReWorks 用户入口，调用 rtosbench_boot_smoke()
├── usrArgs.h                         # ReWorks 运行期网卡 IP，例如 192.168.31.210
├── makefile.conf                     # IDE 生成/维护的 BSP 与 AArch64 CSP 配置
├── gnuaarch64\
│   ├── user.mk                       # RTOS-Bench 接入点，手工维护
│   └── FTE2000_SMP-64\
│       ├── makefile                  # IDE 生成，不手工编辑
│       ├── subdir.mk                 # 自动 include $(ROOT)/user.mk
│       └── rtosbench_port\subdir.mk  # IDE 生成 rtosbench_port/*.c 编译规则
└── rtosbench_port\
    ├── rtosbench_boot.c              # 开机后调用 help/list/schedule quick
    ├── ruihua_workloads.c            # 注册 stub/busywait/ruihua-smoke
    └── sched_workloads_stub.c        # 禁用旧 schedule wrapper，改走注册表 workload
```

### `user.mk` 必要点

`gnuaarch64/user.mk` 是手工维护入口。至少需要：

```makefile
RTOSBENCH_SRC := C:/Users/hzt/yihui-workspace/rtos-bench/RTOS-Bench
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

在 Windows 上进入构建目录：

```bat
cd /d C:\rtos\6.1.1-ARM\workspace\feiteng4rtos\gnuaarch64\FTE2000_SMP-64
```

完整重编：

```bat
cmd /c "set PATH=C:\rtos\6.1.1-ARM\tools\bin;C:\rtos\6.1.1-ARM\tools\build\gnuaarch64\bin;%PATH%&& gnu_make clean all"
```

成功后会生成：

```text
reworks.elf
reworks
feiteng4rtos.obj
```

记录镜像 hash：

```powershell
Get-FileHash -Algorithm SHA256 "C:\rtos\6.1.1-ARM\workspace\feiteng4rtos\gnuaarch64\FTE2000_SMP-64\reworks.elf"
```

## 部署

### 校园网 / rtbench TFTP

将本地构建出的 `reworks.elf` 上传到 rtbench TFTP，建议使用飞腾派专用文件名，不要覆盖其他板子的镜像：

```powershell
scp -P 1026 "C:\rtos\6.1.1-ARM\workspace\feiteng4rtos\gnuaarch64\FTE2000_SMP-64\reworks.elf" `
  rtbench@10.134.151.45:/tftp/ruihua-feiteng-reworks-192.168.31.210-YYYYMMDD.elf
```

如果 U-Boot 的 `bootcmd` 已经指向固定文件名，需要同时更新或覆盖该固定文件。覆盖前先备份：

```bash
cp /tftp/ruihua-feiteng-reworks-192.168.31.210-20260602.elf \
   /tftp/ruihua-feiteng-reworks-192.168.31.210-YYYYMMDD.before-change.elf
```

板端 ReWorks 运行期 IP 由样板工程 `usrArgs.h` 中的 `GEM0_IPV4_ADDR` 决定。校园网验收使用：

```c
#define GEM0_IPV4_ADDR "192.168.31.210"
```

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
C:\rtos\6.1.1-ARM\workspace\feiteng4rtos\gnuaarch64\FTE2000_SMP-64
```

直连模式下 ReWorks 运行期 IP 需要与镜像匹配，常用值是：

```c
#define GEM0_IPV4_ADDR "192.168.2.100"
```

## 启动和连接

重启或重新上电板子，让 U-Boot 从 TFTP 拉取最新 `reworks.elf`。启动完成后连接 telnet：

```bat
telnet 192.168.31.210 23
```

或直连模式：

```bat
telnet 192.168.2.100 23
```

看到 `reworks>` 后再执行 RTOS-Bench 命令。`reboot` 命令在当前板上不够可靠，可能触发复位后不自动恢复；验收阶段优先使用现场复位或串口观察启动过程。

## 验收命令

建议按从轻到重执行：

```text
rtbench_help
rtbench_list
rtbench_test_cmd
rtbench_test_realtime
rtbench_test_schedule_cycles3
rtbench_test_stress
rtbench_test_all
```

当前 Ruihua wrapper 对 shell 验收做了有界化：

- `rtbench_test_stress` 默认执行 `test-stress --job all-quick`。
- `rtbench_test_all` 默认执行 `test-all --schedule-cycles 3 --stress-job all-quick`。

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

### `test-cmd` 全部失败

旧代码在 Ruihua 上会走 unsupported stub，表现为所有命令 failed。当前 `RUIHUA_PLATFORM` 已使用 ReWorks 内置命令能力列表做 `test-cmd` 验收，避免在 telnet 命令内部重入 shell parser。

### `test-stress` 时间太长

不要在交互验收默认跑 `test-stress --job all`。飞腾派上 `all` 可运行但耗时很长；PR 验收使用 `all-quick`。
