################################################################################
# 自动生成的文件。不要编辑！
################################################################################

#Basic variables defined here:
PLATFORM:=A:/IntelWell-IDE
RTOS:=RTCore
RTOS_PATH:=$(PLATFORM)/target/$(RTOS)
BSPS_PATH:=A:/IntelWell-IDE/target/RTCore/kbsp/board
TOOLS_CHAIN_PATH:=A:/IntelWell-IDE/host/gnu
BIN_PATH:=$(PLATFORM)/host/bin
PROJECT_PATH:=A:/IntelWell-IDE/eclipse/workspace/rtos_bench
PROJECT_NAME:=rtos_bench
CONFIG_NAME:=Debug
CONFIG_PATH:=A:/IntelWell-IDE/eclipse/workspace/rtos_bench/Debug
CURRENT_BOARD:=newpre3101_i7
COMPANY:=kyland
BOARD_SUFFIX:=newpre3101_i7
CONFIG_BOARD_START := msl_debug
IS_BUILD_APP := Y

USE_DEFAULT_TOOLS_CHAIN = yes
TOOLS_VERSION := gcc-9.3.0

-include $(CONFIG_PATH)/config_cpu.mk

CROSS_COMPILE :=x86_64-intewell-elf-


# Every subdirectory with source files must be described here
SUBDIR := \
src \


FLAGS := --gc-sections 
PREFLAGS := --gc-sections 
LIBS_PATH := -L$(RTOS_PATH)/lib/$(TOOLS_VERSION)/x86/$(CONFIG_SUB_ARCH)/$(CONFIG_CPU_ENDIAN)
ARCHIVES += lib$(PROJECT_NAME).a
EXECUTABLES +=A:/IntelWell-IDE/eclipse/workspace/rtos_bench/Debug/make/rtos_bench
TARGET_EXT +=elf
LIB_PATH := $(CONFIG_PATH)/lib
IS_INCREMENTAL_BUILD_WITHBSP := N
# BSP目标文件存放的路径
BSP_OBJ_PATH :=$(CONFIG_PATH)/obj

IS_COMPILE_SOURCE := N
HAS_CPP := Y

COMPILE_SYMBOL := -D_X86_ -DOS_FAULT_STACK_DEEPTH=4 -DTTOS_RUN_IN_USER -DCONFIG_DEVICE_COM1=1 -DCONFIG_DEVICE_COM2=1 -DCONFIG_CONSOLE_STDIN=CONFIG_DEVICE_VIRTUAL_TERMINAL_NAME -DCONFIG_CONSOLE_STDOUT=CONFIG_DEVICE_VIRTUAL_TERMINAL_NAME -DDEBUG_INFO=1 -DTTOS_POSIX_API -DENABLE_CPLUSPLUS=0 
COMPILE_INCLUDE := -IA:/IntelWell-IDE/eclipse/workspace/rtos_bench -IA:/IntelWell-IDE/eclipse/workspace/rtos_bench/$(CONFIG_NAME) -IA:/IntelWell-IDE/target/RTCore/include/rtl/c++ -IA:/IntelWell-IDE/target/RTCore/include/rtl/c++/x86_64-intewell-elf -IA:/IntelWell-IDE/target/RTCore/include/rtl -IA:/IntelWell-IDE/target/RTCore/bsp/include -IA:/IntelWell-IDE/target/RTCore/bsp/include/sysDriver -IA:/IntelWell-IDE/target/RTCore/bsp/include/sysDriver/$(CONFIG_ARCH)/$(CONFIG_SUB_ARCH) -IA:/IntelWell-IDE/target/RTCore/include/components/tcpip/lwip -IA:/IntelWell-IDE/target/RTCore/include/components/tcpip/netif -IA:/IntelWell-IDE/target/RTCore/include/components/dfs -I../ -IA:/IntelWell-IDE/target/RTCore/bsp/board/$(CONFIG_ARCH)/start/partition/ttos -IA:/IntelWell-IDE/target/RTCore/include -IA:/IntelWell-IDE/target/RTCore/include/eigen -IA:/IntelWell-IDE/target/RTCore/include/igh -IA:/IntelWell-IDE/target/RTCore/include/igh/osal -IA:/IntelWell-IDE/target/RTCore/include/components/json -IA:/IntelWell-IDE/target/RTCore/include/smipc -IA:/IntelWell-IDE/target/RTCore/include/rpc -IA:/IntelWell-IDE/target/RTCore/include/nfs -IA:/IntelWell-IDE/target/RTCore/include/common -IA:/IntelWell-IDE/target/RTCore/include/driver -IA:/IntelWell-IDE/target/RTCore/include/ta/$(CONFIG_ARCH) -IA:/IntelWell-IDE/target/RTCore/include/rtedebug -IA:/IntelWell-IDE/target/RTCore/include/ttos -IA:/IntelWell-IDE/target/RTCore/include/ssk -IA:/IntelWell-IDE/target/RTCore/include/partition -IA:/IntelWell-IDE/target/RTCore/include/components -IA:/IntelWell-IDE/target/RTCore/include/components/sysDriver -IA:/IntelWell-IDE/target/RTCore/include/components/sysDriver/$(CONFIG_ARCH)/$(CONFIG_SUB_ARCH) -IA:/IntelWell-IDE/target/RTCore/include/components/tcpip -IA:/IntelWell-IDE/target/RTCore/include/components/tcpip/arch -IA:/IntelWell-IDE/target/RTCore/include/ta -IA:/IntelWell-IDE/target/RTCore/include/ta/$(CONFIG_ARCH) -IA:/IntelWell-IDE/target/RTCore/include/ta/$(CONFIG_ARCH)/$(CONFIG_SUB_ARCH) -IA:/IntelWell-IDE/eclipse/workspace/rtos_bench/src -IA:/IntelWell-IDE/target/RTCore/include/components/dfs -IA:/IntelWell-IDE/target/RTCore/include/cfg -IA:/IntelWell-IDE/target/RTCore/include/posix -IA:/IntelWell-IDE/target/RTCore/include/shell -IA:/IntelWell-IDE/target/RTCore/init/partition/ttos -IA:/IntelWell-IDE/host/pub -IA:/IntelWell-IDE/target/RTCore/include/components/soem 
COMPILE_OPTIMIZATION := -O0 
COMPILE_DEBUG := -g 
COMPILE_WARNING := -Wall 
COMPILE_OTHER := -c -fno-builtin -ffunction-sections -fdata-sections 

# 追加 RTOS-Bench 头文件路径
COMPILE_INCLUDE += \
		-I"../../src" \
		-I"../../RTOS-Bench/generator" \
		-I"../../RTOS-Bench/workloads" \
		-I"../../RTOS-Bench/workloads/CUSUM" \
		-I"../../RTOS-Bench/workloads/EWMA" \
		-I"../../RTOS-Bench/workloads/FAST" \
		-I"../../RTOS-Bench/workloads/FAST/include_imgs" \
		-I"../../RTOS-Bench/workloads/PID" \
		-I"../../RTOS-Bench/workloads/EKF" \
		-I"../../RTOS-Bench/workloads/EKF/EKF_core" \
		-I"../../RTOS-Bench/workloads/EKF/include" \
		-I"../../RTOS-Bench/workloads/EKF/include/matrix" \
		-I"../../RTOS-Bench/workloads/EKF/geo" \
		-I"../../RTOS-Bench/workloads/EKF/geo_lookup" \
		-I"../../RTOS-Bench/workloads/EPNP" \
		-I"../../RTOS-Bench/workloads/EPNP/opengv" \
		-I"../../RTOS-Bench/workloads/EPNP/Eigen" \
		-I"../../RTOS-Bench/workloads/ICP" \
		-I"../../RTOS-Bench/workloads/MODBUS" \
		-I"../../RTOS-Bench/workloads/MQTT" \
		-I"../../RTOS-Bench/generator/stress_orig/common" \
		-I"../../RTOS-Bench/generator/stress_orig/osal" \
		-I"../../RTOS-Bench/generator/stress_orig/stressor" \
		-I"../../RTOS-Bench/generator/realtime_orig/les"

# 追加用户宏定义
COMPILE_SYMBOL += \
    -D_GNU_SOURCE \
    -DECL_STANDALONE \
    -D__STDC_FORMAT_MACROS \
    -D__STDC_LIMIT_MACROS \
    -D_USE_MATH_DEFINES \
    -DEIGEN_DONT_VECTORIZE \
    -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT \
    -DFIX_OPENGV_USE_STUB_LDOUBLE \
	-DDONGTU_PLATFORM
#prebuild target defined here:
