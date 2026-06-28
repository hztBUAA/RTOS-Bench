# Ruihua/ReWorks Feiteng 调试手册

本文档记录当前已验证的 Ruihua/ReWorks 飞腾派接入方式。远端仓库里的旧模板曾面向独立 `rtosbench.out` 工程；当前实测可用路径是把 RTOS-Bench 链进 Ruihua 自启动工程 `feiteng4rtos`，生成 `reworks.elf` 后由 U-Boot/TFTP 启动。

## 当前已验证环境

- RTOS: Ruihua ReWorks 6.1.1 ARM
- Board: 飞腾派 / Phytium Pi, AArch64 SMP
- Ruihua SDK: `C:\rtos\6.1.1-ARM`
- 样板工程: `C:\rtos\6.1.1-ARM\workspace\feiteng4rtos`
- 构建目录: `C:\rtos\6.1.1-ARM\workspace\feiteng4rtos\gnuaarch64\FTE2000_SMP-64`
- 启动镜像: `C:\rtos\6.1.1-ARM\workspace\feiteng4rtos\gnuaarch64\FTE2000_SMP-64\reworks.elf`
- 板端 telnet: `192.168.2.100:23`
- Shell prompt: `reworks>`

最新一次命令行构建成功的 `reworks.elf` SHA256:

```text
31BC67B133FBDEE3869D646D7B6A95BC6DACF9CB899157F3222B9B0C06E6E71C
```

## 目录关系

建议同事先从当前可用样板工程开始，不要重新从空工程摸索。

```text
C:\rtos\6.1.1-ARM\workspace\feiteng4rtos
├── usrInit.c                         # ReWorks 用户入口，调用 RTOS-Bench boot smoke
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
    └── sched_workloads_stub.c        # 让 test-schedule 只使用注册表 workload
```

RTOS-Bench 源码仍使用本仓库：

```text
C:\Users\hzt\yihui-workspace\rtos-bench\RTOS-Bench
```

关键仓库源码：

- `generator/ruihua_entry.c`
- `generator/rtbench_command.c`
- `generator/platform/ruihua/*`
- `generator/test_schedule.c`
- `generator/test_cmd.c`
- `generator/result_export.c`
- `generator/workload_registry.*`
- `generator/workload_stub.c`
- `generator/workload_busywait.c`

## 第一次接入步骤

1. 确认本地 RTOS-Bench 使用最新 `main`：

   ```bat
   cd /d C:\Users\hzt\yihui-workspace\rtos-bench\RTOS-Bench
   git switch main
   git pull --ff-only origin main
   ```

2. 打开 Ruihua 样板工程：

   ```text
   C:\rtos\6.1.1-ARM\workspace\feiteng4rtos
   ```

   如果同事新建工程，建议先复制这个工程，再改工程名和路径。不要直接从旧 `platforms/ruihua/rtosbench-project-template` 重新生成。

3. 检查 `makefile.conf` 里必须有 AArch64 CSP 配置。关键项如下：

   ```makefile
   -B"$(REDE_HOME)/user resource/aarch64_smp_csp_config/runtimelib/AARCH64_SMP/lib"
   -I"$(REDE_HOME)/resource/h/arch/AARCH64_SMP"
   -specs csp_specs -qrtos
   -L"$(REDE_HOME)/user resource/aarch64_smp_csp_config/runtimelib/AARCH64_SMP/lib"
   -l"aarch64_smp_csp"
   ```

   如果缺少这些项，常见现象是 `cpu.h` 找不到、`csp_specs` 找不到，或链接时报 `int_cpu_lock`、`context_switch` 等底层符号 undefined。

4. 检查 `gnuaarch64/user.mk` 的 `RTOSBENCH_SRC` 指向当前仓库：

   ```makefile
   RTOSBENCH_SRC := C:/Users/hzt/yihui-workspace/rtos-bench/RTOS-Bench
   RTOSBENCH_PORT := ../../rtosbench_port
   RTOSBENCH_OBJDIR := ./rtosbench
   ```

5. `user.mk` 里需要定义 Ruihua 编译宏和 include：

   ```makefile
   RTOSBENCH_CFLAGS := \
   	-DRUIHUA_PLATFORM \
   	-DRTOSBENCH_USE_MANUAL_WORKLOAD_REGISTRATION \
   	-DRTBENCH_NO_STANDALONE_MAIN \
   	-I"$(RTOSBENCH_SRC)/generator" \
   	-I"$(RTOSBENCH_SRC)/generator/test_schedule" \
   	-I"$(RTOSBENCH_SRC)/workloads" \
   	-I"$(RTOSBENCH_PORT)"
   ```

6. 如果 `rtosbench_port` 目录已被 IDE 扫描进工程，自动生成的 `rtosbench_port/subdir.mk` 会负责编译 `rtosbench_port/*.c`。这时必须给该自动规则补 include，但不要把这些对象再重复加进 `RTOSBENCH_OBJS`：

   ```makefile
   rtosbench_port/%.o: CPPFLAGS += $(RTOSBENCH_CFLAGS)
   ```

   这是为了避免 `ruihua_workloads.c` 报：

   ```text
   fatal error: workload_registry.h: No such file or directory
   ```

7. `RTOSBENCH_OBJS` 只列仓库内 RTOS-Bench 框架源码和 Ruihua platform 源码。例如当前样板工程列到：

   ```makefile
   $(RTOSBENCH_OBJDIR)/ruihua_entry.o
   $(RTOSBENCH_OBJDIR)/rtbench_command.o
   $(RTOSBENCH_OBJDIR)/periodic_benchmark.o
   $(RTOSBENCH_OBJDIR)/logging.o
   $(RTOSBENCH_OBJDIR)/memory_watcher.o
   $(RTOSBENCH_OBJDIR)/uunifast.o
   $(RTOSBENCH_OBJDIR)/test_schedule.o
   $(RTOSBENCH_OBJDIR)/test_cmd.o
   $(RTOSBENCH_OBJDIR)/result_export.o
   $(RTOSBENCH_OBJDIR)/workload_registry.o
   $(RTOSBENCH_OBJDIR)/workload_stub.o
   $(RTOSBENCH_OBJDIR)/workload_busywait.o
   $(RTOSBENCH_OBJDIR)/ruihua_timer.o
   $(RTOSBENCH_OBJDIR)/ruihua_sync.o
   $(RTOSBENCH_OBJDIR)/ruihua_timestamp.o
   $(RTOSBENCH_OBJDIR)/ruihua_scheduler.o
   $(RTOSBENCH_OBJDIR)/ruihua_signal.o
   ```

8. 在 `usrInit.c` 里接入 RTOS-Bench 开机 smoke：

   ```c
   extern void rtosbench_boot_smoke(void);

   void UserInit(void)
   {
       printf("[feiteng4rtos] UserInit enter\n");
       rtosbench_boot_smoke();
       printf("[feiteng4rtos] UserInit leave\n");
       return;
   }
   ```

9. `rtosbench_port/rtosbench_boot.c` 当前会在开机后跑最小框架检查：

   ```c
   extern int rtbench_help(void);
   extern int rtbench_list(void);
   extern int rtbench_test_schedule_quick(void);

   void rtosbench_boot_smoke(void)
   {
       printf("[rtos-bench] Ruihua boot integration active\n");
       rtbench_help();
       rtbench_list();
       rtbench_test_schedule_quick();
   }
   ```

   如果调试阶段不希望每次重启都跑 schedule quick，可以临时只保留打印和 `rtbench_help()`，但提交前需要恢复可验证入口。

## 命令行构建

在 PowerShell 或 cmd 中进入构建目录：

```bat
cd /d C:\rtos\6.1.1-ARM\workspace\feiteng4rtos\gnuaarch64\FTE2000_SMP-64
cmd /c "set PATH=C:\rtos\6.1.1-ARM\tools\bin;C:\rtos\6.1.1-ARM\tools\build\gnuaarch64\bin;%PATH%&& gnu_make all"
```

成功后会看到：

```text
Size of reworks.elf:
   text    data     bss     dec     hex filename
...
Successfully!
```

可计算镜像 hash：

```powershell
Get-FileHash -Algorithm SHA256 "C:\rtos\6.1.1-ARM\workspace\feiteng4rtos\gnuaarch64\FTE2000_SMP-64\reworks.elf"
```

如果命令行提示找不到 `gnu_make`，说明当前 shell 没有 Ruihua SDK PATH，优先使用上面的 `cmd /c "set PATH=...&& gnu_make all"` 形式。

## 部署和板端连接

当前 U-Boot/TFTP 已配置为从本机工程输出目录读取：

```text
C:\rtos\6.1.1-ARM\workspace\feiteng4rtos\gnuaarch64\FTE2000_SMP-64\reworks.elf
```

因此构建成功后通常不需要手动复制镜像。重启板子，U-Boot 会重新取这个 `reworks.elf`。

板端 shell：

```bat
telnet 192.168.2.100
```

进入后看到：

```text
reworks>
```

如果 telnet 不通，先确认板子已重启完成、网线/IP 正常；如果上一次运行了不稳定命令导致 shell 卡死，通常需要给板子断电或硬重启。

## 可调用命令

`generator/ruihua_entry.c` 暴露了一组适合 ReWorks shell 直接调用的函数：

```text
rtbench_help
rtbench_list
rtbench_stub
rtbench_busywait
rtbench_ruihua_smoke
rtbench_test
rtbench_quick
rtbench_test_all
rtbench_test_all_quick
rtbench_test_all_multicore
rtbench_test_schedule
rtbench_test_schedule_quick
rtbench_test_schedule_cycles3
rtbench_test_realtime
rtbench_test_stress
rtbench_test_cmd
rtbench_export_result
```

建议调试顺序：

```text
rtbench_help
rtbench_list
rtbench_ruihua_smoke
rtbench_test_schedule_quick
rtbench_test_schedule_cycles3
```

`rtbench_test_schedule_cycles3` 是当前完整验证过的 schedule 路径，覆盖默认 30%-100% 利用率梯度，每个梯度 3 个 cycle。

## 当前验证结果

板端完整 schedule 验证命令：

```text
rtbench_help
rtbench_list
rtbench_test_schedule_cycles3
```

观察到的结果：

```text
[test-schedule] Found 1 industrial workloads (excluded 2 utility workloads)
[ruihua-smoke]: WCET = 50.000 ms (5 iters)
U= 30%: MR=0.0000 (0 misses / 3 jobs)
U= 40%: MR=0.0000 (0 misses / 3 jobs)
U= 50%: MR=0.0000 (0 misses / 3 jobs)
U= 60%: MR=0.0000 (0 misses / 3 jobs)
U= 70%: MR=0.0000 (0 misses / 3 jobs)
U= 80%: MR=0.0000 (0 misses / 3 jobs)
U= 90%: MR=0.0000 (0 misses / 3 jobs)
U=100%: MR=0.0000 (0 misses / 3 jobs)
Average Miss Rate: 0.0000
Final Score: 100.00 / 100
```

验证证据记录在：

```text
utils/remote-test/logs/ruihua-feiteng-20260528/SUMMARY.md
utils/remote-test/logs/build-records-20260529/ruihua-feiteng-build-20260530.log
C:\rtos\6.1.1-ARM\workspace\feiteng4rtos\logs\rtosbench-validation-20260527.md
```

## 当前实现限制

- `test-schedule` 当前使用 `ruihua-smoke` 作为确定性工业 workload，`stub` 和 `busywait` 作为 utility workload 会被排除。
- `test-realtime`、`test-stress`、完整工业 workload 套件没有作为 Ruihua 本轮验收目标，不要把它们的结果写成已通过。
- Ruihua 的普通 workload 命令没有走通用 `periodic_benchmark()` 生命周期，而是在 `RUIHUA_PLATFORM` 下走 finite direct periodic path。这是为了避免 ReWorks telnet shell 在 POSIX signal/atexit 清理路径上挂住。
- Ruihua timer platform 已改为 pthread-backed timer，因为测试 BSP 的 `SIGEV_THREAD` timer 不可靠。

## 常见问题

### `workload_registry.h` 找不到

现象：

```text
../../rtosbench_port/ruihua_workloads.c:1:10: fatal error: workload_registry.h: No such file or directory
```

原因是 IDE 自动生成的 `rtosbench_port/subdir.mk` 编译端口源码时没有 RTOS-Bench include。检查 `gnuaarch64/user.mk` 是否有：

```makefile
rtosbench_port/%.o: CPPFLAGS += $(RTOSBENCH_CFLAGS)
```

同时确认 `RTOSBENCH_CFLAGS` 里有：

```makefile
-I"$(RTOSBENCH_SRC)/generator"
```

### `cpu.h` 找不到

检查 `makefile.conf` 是否有：

```makefile
-I"$(REDE_HOME)/resource/h/arch/AARCH64_SMP"
```

### `csp_specs` 找不到

检查 SDK 是否真的有：

```text
C:\rtos\6.1.1-ARM\user resource\aarch64_smp_csp_config\runtimelib\AARCH64_SMP\lib\csp_specs
```

并检查 `makefile.conf` 是否有：

```makefile
-B"$(REDE_HOME)/user resource/aarch64_smp_csp_config/runtimelib/AARCH64_SMP/lib" -specs csp_specs -qrtos
```

如果文件不存在，需要找 Ruihua 同事补齐 AArch64 SMP CSP 组件。

### 链接时报 `int_cpu_lock` / `context_switch` undefined

通常是缺少 AArch64 CSP 库路径或库：

```makefile
-L"$(REDE_HOME)/user resource/aarch64_smp_csp_config/runtimelib/AARCH64_SMP/lib"
-l"aarch64_smp_csp"
```

### `test-schedule` 报 timer 创建失败

不要回退到 `SIGEV_THREAD` timer。当前仓库 `generator/platform/ruihua/timer.c` 使用 pthread-backed timer，是已验证路径。

### 运行 workload 后 telnet 不可达

旧实现走通用 `periodic_benchmark()`，曾出现一次 benchmark 输出后 telnet 不再可达。当前 `RUIHUA_PLATFORM` 已在 `rtbench_command.c` 使用 finite direct periodic path。请确认构建使用的是最新 `main`，并优先从以下命令重新验证：

```text
rtbench_ruihua_smoke
rtbench_test_schedule_quick
```

### `les/bench_init.c` 出现中文乱码和字符串未闭合

样板工程里历史 `les/bench_init.c` 曾出现编码损坏，表现为 `stray '\xxx' in program` 或 `expected expression before static`。处理方式是把损坏的中文表头和注释改成 ASCII，确保字符串正常闭合。这个问题不属于 RTOS-Bench 框架本身。

## 维护建议

- Ruihua 相关 RTOS-Bench 框架代码以本仓库 `main` 为准，不再维护单独分叉。
- IDE 生成的 `gnuaarch64/FTE2000_SMP-64/*.mk` 不建议手工长期维护；优先把可复用逻辑放到 `gnuaarch64/user.mk` 和 `rtosbench_port/*.c`。
- 每次变更后至少记录：构建命令、`reworks.elf` SHA256、板端命令、telnet 输出和是否返回 `reworks>`。

## 附:独立可下载工程方式(rtosbench-project-template)

除了上面"链进 `feiteng4rtos` 启动镜像"的方式,仓库还提供一套**独立可下载工程**模板
`platforms/ruihua/rtosbench-project-template`,产出独立的 `gnu$(ARCH)/rtosbench.out`,
适合在 ReDe IDE 里作为单独的"可下载工程"构建后用 `ld` 加载。**龙芯派推荐用这套**
(详见 `锐华龙芯派-编译与校验指南.md`)。

`rtosbench-project-template` 里几个文件的职责:

- `.project`:瑞华 IDE/Eclipse 工程描述文件。告诉 IDE 这个目录是 `rtosbench` 工程、使用瑞华/Eclipse builder、调用 `gnu_make`。它不负责列源码,也不放本机路径。
- `Makefile`:真正的编译入口。维护 RTOS-Bench 瑞华构建所需的源码列表、宏定义、include 路径、架构参数和链接规则。
- `makefile.conf`:瑞华 IDE/板级配置片段,保留 BSP、runtime lib、SDK 头文件等工程配置。通常不需要新同学修改。
- `makefile.local.example`:本机配置示例。复制模板工程后,把它复制成 `makefile.local`,在里面改自己的安装路径、源码路径和**构建架构**。
- `.gitignore`:忽略本机配置和编译产物,例如 `makefile.local`、`gnuarm/`、`gnuaarch64/`、`gnuloongarch64/`。

路径变量优先级:**命令行传入 > `makefile.local` > `Makefile` 默认值**。本机差异不要直接改
`Makefile`/`makefile.conf`,优先写到 `makefile.local`。

### 推荐使用方式

1. 克隆最新 RTOS-Bench 仓库。
2. 打开 ReDe IDE,`文件 -> 新建 -> 可下载工程`,选择对应的 BSP(龙芯派选 `loongarch2k1000la_64bits_smp_bsp_config`)。
3. 将 `platforms/ruihua/rtosbench-project-template` 下的文件复制到瑞华 workspace 下相应目录,例如 `C:/rtos/6.1.1-ARM/workspace/rtosbench`。
4. 在复制后的工程目录里,将 `makefile.local.example` 复制为 `makefile.local`,修改本机路径:

   ```makefile
   REDE_HOME := C:/rtos/6.1.1-ARM
   RTOSBENCH_SRC := D:/workspace/RTOS-Bench
   ARCH := arm          # 龙芯派改为 loongarch64
   ```

5. 在瑞华 IDE 中打开 `rtosbench` 工程。
6. 选择对应构建配置(如 `rtosbench [gnuloongarch64]`),执行 Build Project。产物在 `gnu$(ARCH)/rtosbench.out`。

工程变量说明:

- `REDE_HOME`:瑞华 IDE/SDK 安装目录,默认 `C:/rtos/6.1.1-ARM`。
- `RTOSBENCH_SRC`:RTOS-Bench 源码根目录。
- `ARCH`:默认 `arm`,也可设为 `aarch64` 或 `loongarch64`,也可在项目属性 -> C/C++ 构建里配置。
- `FULL_WORKLOADS`:此变量历史上用于 feiteng `user.mk` 路径裁剪业务 workload;在 `rtosbench-project-template/Makefile` 中业务 workload 默认参与构建,以构建实际行为为准。

## 后续增加架构方式

1.在Makefile增加相应架构的工具链和符号：
```makefile
 ifeq ($(ARCH),loongarch64)
    CROSS_COMPILE ?= loongarch64-elf-
    BUILD_DIR := gnuloongarch64

    ARCH_FLAGS := -mabi=lp64d -march=loongarch64 -mstrict-align -G0
    ARCH_FLAGS += -I"$(REDE_HOME)/resource/h/arch/LOONGARCH64_SMP"

    ARCH_DEFS := -DCPU=MIPS64 -DREWORKS_LP64 -D_LOONGARCH_64_
else ifeq ($(ARCH),aarch64)
    CROSS_COMPILE ?= aarch64-elf-
    BUILD_DIR := gnuaarch64
    ARCH_FLAGS := -I"$(REDE_HOME)/resource/h/arch/AARCH64_SMP"
    ARCH_DEFS := -D__aarch64__ -D_FTE2000_64CORES_64BITS_BORAD_
else
    CROSS_COMPILE ?= arm-elf-
    BUILD_DIR := gnuarm
    ARCH_FLAGS := -mlong-calls -mcpu=cortex-a9 -mapcs-frame -mfpu=vfpv3 -mfloat-abi=softfp
    ARCH_FLAGS += -I"$(REDE_HOME)/resource/h/arch/ARMv7_SMP"
    ARCH_DEFS := -DCPU=ARMARCH7 -D__arm__ -D_CPU_ARM_ -D_IMX6Q_
endif
```
请确认\${ARCH}_SMP，CROSS_COMPILE的路径正确，ARCH_DEFS符号正确
可查看IDE模板工程的构建方式

2.在makefile.conf里增加相应的BSP：

需要确认${REDE_HOME}/user resource文件夹下有相应的bsp
```makefile
################################################################################
# ARMv7 SMP
################################################################################
ifeq ($(ARCH),arm)

CROSS_PREFIX ?= gnuarm
CROSS_COMPILE ?= arm-elf-

RUIHUA_ARMV7_ARCH_DIR ?= ARMv7_SMP
RUIHUA_ARMV7_BSP_DIR  ?= fte2000_smp_bsp-32_config
RUIHUA_ARMV7_CSP_DIR  ?= armv7_smp_csp_config

RUIHUA_ARCH_FLAGS := \
	-DCPU=ARMARCH7 \
	-D__arm__ \
	-D_CPU_ARM_ \
	-D_IMX6Q_ \
	-I"$(REDE_HOME)/resource/h/arch/$(RUIHUA_ARMV7_ARCH_DIR)" \
	-B"$(REDE_HOME)/user resource/$(RUIHUA_ARMV7_BSP_DIR)/runtimelib/$(RUIHUA_ARMV7_ARCH_DIR)/lib" \
	-B"$(REDE_HOME)/user resource/$(RUIHUA_ARMV7_CSP_DIR)/runtimelib/$(RUIHUA_ARMV7_ARCH_DIR)/lib" \
	-I"$(REDE_HOME)/user resource/$(RUIHUA_ARMV7_BSP_DIR)/runtimelib/h"

################################################################################
# AArch64 SMP
################################################################################
else ifeq ($(ARCH),aarch64)

CROSS_PREFIX ?= gnuaarch64
CROSS_COMPILE ?= aarch64-elf-

RUIHUA_AARCH64_ARCH_DIR ?= AARCH64_SMP
RUIHUA_AARCH64_BSP_DIR  ?= fte2000_smp_bsp-64_config

RUIHUA_ARCH_FLAGS := \
	-D__aarch64__ \
	-D_FTE2000_64CORES_64BITS_BORAD_ \
	-I"$(REDE_HOME)/resource/h/arch/$(RUIHUA_AARCH64_ARCH_DIR)" \
	-B"$(REDE_HOME)/user resource/$(RUIHUA_AARCH64_BSP_DIR)/runtimelib/$(RUIHUA_AARCH64_ARCH_DIR)/lib" \
	-I"$(REDE_HOME)/user resource/$(RUIHUA_AARCH64_BSP_DIR)/runtimelib/h"

else ifeq ($(ARCH),loongarch64)

CROSS_PREFIX ?= gnuloongarch64
CROSS_COMPILE ?= loongarch64-elf-

RUIHUA_LOONGARCH64_ARCH_DIR ?= LOONGARCH64_SMP

RUIHUA_LOONGARCH64_BSP_DIR ?= loongarch2k1000la_64bits_smp_bsp_config

RUIHUA_LOONGARCH64_CSP_DIR ?= loongarch_64bits_smp_csp_config

RUIHUA_LOONGARCH64_BOARD_DEFS ?= -DLOONGARCH2K2000_BOARD

CONFIG_COMPLILE_FLAGS += \
	$(RUIHUA_COMMON_FLAGS) \
	-B"$(REDE_HOME)/user resource/$(RUIHUA_LOONGARCH64_BSP_DIR)/runtimelib/$(RUIHUA_LOONGARCH64_ARCH_DIR)/lib" \
	$(RUIHUA_LOONGARCH64_BOARD_DEFS) \
	-B"$(REDE_HOME)/user resource/$(RUIHUA_LOONGARCH64_CSP_DIR)/runtimelib/$(RUIHUA_LOONGARCH64_ARCH_DIR)/lib" \
	-mabi=lp64d \
	-march=loongarch64 \
	-mstrict-align \
	-D__multi_core__ \
	-DCPU=MIPS64 \
	-G0 \
	-DREWORKS_LP64 \
	-D_LOONGARCH_64_ \
	-I"$(REDE_HOME)/resource/h/arch/$(RUIHUA_LOONGARCH64_ARCH_DIR)"

CONFIG_LINK_DYNAMIC_FLAGS += \
	--oformat=elf64-loongarch

else
$(error Unsupported ARCH=$(ARCH). Use ARCH=arm, ARCH=aarch64, or ARCH=loongarch64)
endif
```
## Makefile修改提交要求

无需提交makefile.local.example，避免相关问题。