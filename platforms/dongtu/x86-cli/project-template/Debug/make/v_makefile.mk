################################################################################
# 自动生成的文件。不要编辑！
################################################################################

#Basic variables defined here:
PLATFORM:=@INTEWELL_HOME@
RTOS:=RTCore
RTOS_PATH:=$(PLATFORM)/target/$(RTOS)
BSPS_PATH:=@INTEWELL_HOME@/target/RTCore/kbsp/board
TOOLS_CHAIN_PATH:=@INTEWELL_HOME@/host/gnu
BIN_PATH:=$(PLATFORM)/host/bin
PROJECT_PATH:=@PROJECT_DIR@
PROJECT_NAME:=rtosbench_x86
CONFIG_NAME:=Debug
CONFIG_PATH:=@PROJECT_DIR@/Debug
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
EXECUTABLES +=@PROJECT_DIR@/Debug/make/rtosbench_x86
TARGET_EXT +=elf
LIB_PATH := $(CONFIG_PATH)/lib
IS_INCREMENTAL_BUILD_WITHBSP := N
# BSP目标文件存放的路径
BSP_OBJ_PATH :=$(CONFIG_PATH)/obj

IS_COMPILE_SOURCE := N
HAS_CPP := N

COMPILE_SYMBOL := -D_X86_ -DOS_FAULT_STACK_DEEPTH=4 -DTTOS_RUN_IN_USER -DCONFIG_DEVICE_COM1=1 -DCONFIG_DEVICE_COM2=1 -DCONFIG_CONSOLE_STDIN=CONFIG_DEVICE_VIRTUAL_TERMINAL_NAME -DCONFIG_CONSOLE_STDOUT=CONFIG_DEVICE_VIRTUAL_TERMINAL_NAME -DDEBUG_INFO=1 -DTTOS_POSIX_API -DENABLE_CPLUSPLUS=0 
COMPILE_INCLUDE := -I@PROJECT_DIR@ -I@PROJECT_DIR@/$(CONFIG_NAME) -I@INTEWELL_HOME@/target/RTCore/include/rtl/c++ -I@INTEWELL_HOME@/target/RTCore/include/rtl/c++/x86_64-intewell-elf -I@INTEWELL_HOME@/target/RTCore/include/rtl -I@INTEWELL_HOME@/target/RTCore/bsp/include -I@INTEWELL_HOME@/target/RTCore/bsp/include/sysDriver -I@INTEWELL_HOME@/target/RTCore/bsp/include/sysDriver/$(CONFIG_ARCH)/$(CONFIG_SUB_ARCH) -I@INTEWELL_HOME@/target/RTCore/include/components/tcpip/lwip -I@INTEWELL_HOME@/target/RTCore/include/components/tcpip/netif -I@INTEWELL_HOME@/target/RTCore/include/components/dfs -I../ -I@INTEWELL_HOME@/target/RTCore/bsp/board/$(CONFIG_ARCH)/start/partition/ttos -I@INTEWELL_HOME@/target/RTCore/include -I@INTEWELL_HOME@/target/RTCore/include/igh -I@INTEWELL_HOME@/target/RTCore/include/igh/osal -I@INTEWELL_HOME@/target/RTCore/include/components/json -I@INTEWELL_HOME@/target/RTCore/include/smipc -I@INTEWELL_HOME@/target/RTCore/include/rpc -I@INTEWELL_HOME@/target/RTCore/include/nfs -I@INTEWELL_HOME@/target/RTCore/include/common -I@INTEWELL_HOME@/target/RTCore/include/driver -I@INTEWELL_HOME@/target/RTCore/include/ta/$(CONFIG_ARCH) -I@INTEWELL_HOME@/target/RTCore/include/rtedebug -I@INTEWELL_HOME@/target/RTCore/include/ttos -I@INTEWELL_HOME@/target/RTCore/include/ssk -I@INTEWELL_HOME@/target/RTCore/include/partition -I@INTEWELL_HOME@/target/RTCore/include/components -I@INTEWELL_HOME@/target/RTCore/include/components/sysDriver -I@INTEWELL_HOME@/target/RTCore/include/components/sysDriver/$(CONFIG_ARCH)/$(CONFIG_SUB_ARCH) -I@INTEWELL_HOME@/target/RTCore/include/components/tcpip -I@INTEWELL_HOME@/target/RTCore/include/components/tcpip/arch -I@INTEWELL_HOME@/target/RTCore/include/ta -I@INTEWELL_HOME@/target/RTCore/include/ta/$(CONFIG_ARCH) -I@INTEWELL_HOME@/target/RTCore/include/ta/$(CONFIG_ARCH)/$(CONFIG_SUB_ARCH) -I@PROJECT_DIR@/src -I@INTEWELL_HOME@/target/RTCore/include/components/dfs -I@INTEWELL_HOME@/target/RTCore/include/cfg -I@INTEWELL_HOME@/target/RTCore/include/posix -I@INTEWELL_HOME@/target/RTCore/include/shell -I@INTEWELL_HOME@/target/RTCore/init/partition/ttos -I@INTEWELL_HOME@/host/pub -I@INTEWELL_HOME@/target/RTCore/include/components/soem 
COMPILE_OPTIMIZATION := -O0 
COMPILE_DEBUG := -g 
COMPILE_WARNING := -Wall 
COMPILE_OTHER := -c -fno-builtin -ffunction-sections -fdata-sections 


#prebuild target defined here:
