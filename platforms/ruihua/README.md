# Ruihua/ReWorks IDE 工程模板

这个目录提供瑞华 IDE（ReDe/ReWorks）下编译 RTOS-Bench 的工程壳模板。

RTOS-Bench 的瑞华适配源码已经在仓库内维护：

- `generator/ruihua_entry.c`
- `generator/rtbench_command.c`
- `generator/platform/ruihua/*`
- `generator/test_schedule.c`
- `generator/test_realtime.c`
- `generator/test_stress.c`
- `generator/test_cmd.c`
- `generator/result_export.c`

新同学不需要自己写 entry，也不需要手工枚举框架源码。瑞华 IDE 工程只负责设置工具链、SDK 头文件和调用 Makefile。

## 模板文件说明

`rtosbench-project-template` 里几个文件的职责如下：

- `.project`：瑞华 IDE/Eclipse 工程描述文件。它告诉 IDE 这个目录是 `rtosbench` 工程、使用瑞华/Eclipse builder，并调用 `gnu_make`。它不负责列源码，也不应该放本机路径。
- `Makefile`：真正的编译入口。它维护 RTOS-Bench 瑞华构建所需的源码列表、宏定义、include 路径、架构参数和链接规则。
- `makefile.conf`：瑞华 IDE/板级配置片段，保留 BSP、runtime lib、SDK 头文件等工程配置。通常不需要新同学修改。
- `makefile.local.example`：本机配置示例。复制模板工程后，新同学应该把它复制成 `makefile.local`，然后在 `makefile.local` 里改自己的安装路径，源码路径和**构建架构**。
- `.gitignore`：忽略本机配置和编译产物，例如 `makefile.local`、`gnuarm/`、`gnuaarch64/`。

路径变量优先级是：命令行传入的变量最高，其次是 `makefile.local`，最后才是 `Makefile` 里的默认值。因此本机差异不要直接改 `Makefile` 或 `makefile.conf`，优先写到 `makefile.local`。

## 推荐使用方式

1. 克隆最新 RTOS-Bench 仓库。
2. 打开ReDe IDE，文件->新建->可下载工程，选择对应的BSP
2. 将 `platforms/ruihua/rtosbench-project-template`下的文件 复制到瑞华 workspace下相应的目录，例如：

   `C:/rtos/6.1.1-ARM/workspace/rtosbench`

3. 在复制后的工程目录里，将 `makefile.local.example` 复制为 `makefile.local`。
4. 修改 `makefile.local` 里的本机路径：

   ```makefile
   REDE_HOME := C:/rtos/6.1.1-ARM
   RTOSBENCH_SRC := D:/workspace/RTOS-Bench
   ARCH:= arm
   ```

   `REDE_HOME` 是瑞华 IDE/SDK 安装根目录。如果新同学安装到了其他盘或其他版本目录，需要改成自己的真实路径。
    `ARCH` 选择自己需要编译的架构，或者在项目属性->C/C++构建里配置
5. 在瑞华 IDE 中打开 `rtosbench` 工程。
6. 选择 `rtosbench [gnuarm]`，执行 Build Project。

默认输出：

`gnu$(ARCH)/rtosbench.out`

## 命令行验证

如果当前命令行环境已经能找到瑞华 IDE 的 `gnu_make`，可以在工程目录运行：

```bat
gnu_make ARCH=arm clean all
```

如果 `gnu_make` 只在 IDE 内可见，优先在 IDE 中构建。不要依赖翼辉/RealEvo 的 `make.exe` 作为瑞华正式构建入口。

## 配置说明

- `REDE_HOME`：瑞华 IDE/SDK 安装目录，默认 `C:/rtos/6.1.1-ARM`。
- `RTOSBENCH_SRC`：RTOS-Bench 源码根目录。
- `ARCH`：默认 `arm`，也可设为 `aarch64` 或者 `loongarch`
- `FULL_WORKLOADS`：默认 `0`，只构建框架验收路径：`test-all/test-schedule/test-realtime/test-stress/test-cmd/export-result`。如需尝试完整业务 workload，可设为 `1`，但需要额外处理瑞华 IPNet 与部分 workload 头文件兼容问题。

`REDE_HOME` 会在编译时被 `Makefile` 和 `makefile.conf` 用来拼接瑞华 SDK 路径，例如：

```makefile
-I"$(REDE_HOME)/user resource/h/vx"
-I"$(REDE_HOME)/resource/h"
-I"$(REDE_HOME)/resource/h/arch/ARMv7_SMP"
-B"$(REDE_HOME)/user resource/armv7_smp_csp_config/runtimelib/ARMv7_SMP/lib"
```

也就是说，它不是 RTOS-Bench 源码路径，而是瑞华工具链和 BSP/SDK 资源路径。`RTOSBENCH_SRC` 才是 RTOS-Bench 仓库根目录。

## 验收入口

加载 `rtosbench.out` 后，可从瑞华 shell 调用：

- `rtbench_help`
- `rtbench_test`
- `rtbench_test_all`
- `rtbench_test_all_quick`
- `rtbench_test_schedule`
- `rtbench_test_schedule_quick`
- `rtbench_test_realtime`
- `rtbench_test_stress`
- `rtbench_test_cmd`
- `rtbench_export_result`

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