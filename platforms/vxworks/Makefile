# ============================================================================
# VxWorks 构建模板（RTOS-Bench 参考实现）
#
# 说明：本文件**不被仓库任何构建引用**，是一份供移植方复制/改写到自己
#       VxWorks 工程里的模板。仓库自身的构建（根 Makefile / CMake / SConscript /
#       platforms/{sylixos,dongtu,ruihua}）都不会 include 它。
#
# 用法：
#   1. 复制 platforms/vxworks/ 到你的 VxWorks 工程（或保持软链接）。
#   2. 按你的 SDK 设置下面的 VXWORKS_ROOT / TOOLCHAIN_PREFIX / CPU / TOOL。
#   3. 在生成工作区用 `make -f Makefile` 编译出 rt-bench 目标文件。
#
# 前置：平台层 5 个文件（timer/sync/scheduler/timestamp/signal.c）当前是**空编译单元**——
#       它们被 `#if defined(RTBENCH_PLATFORM_VXWORKS)` 守卫，而该宏在
#       generator/platform_abstraction.h 中**故意未定义**（以保证其他平台构建不变）。
#       启用方法见 README.md 的「激活步骤」。未激活时链接会报
#       `undefined reference to rtbench_timer_create` 等，属预期现象。
# ============================================================================

# ---------------------------------------------------------------------------
# 工具链配置（按需修改）
# ---------------------------------------------------------------------------
VXWORKS_ROOT      ?= /opt/windriver
WIND_HOME         ?= $(VXWORKS_ROOT)
# VxWorks 7 用 wr- 前缀；VxWorks 6.9 常见 arm-none-eabi- / pentium4-
TOOLCHAIN_PREFIX  ?= wr-
CPU               ?= V7ARM
TOOL              ?= diab
RTOS_BENCH_ROOT   ?= $(abspath ../..)

CC  := $(TOOLCHAIN_PREFIX)gcc
CXX := $(TOOLCHAIN_PREFIX)g++
AR  := $(TOOLCHAIN_PREFIX)ar

# ---------------------------------------------------------------------------
# 源文件清单
#
# 注意：仓库内所有构建清单都是**显式列举**（非 glob）。下面这 4 个 test_schedule
#      包装器是 test-schedule 的必需项，任何新建的构建文件都不能漏：
# ---------------------------------------------------------------------------
SCHED_SRCS := \
	$(RTOS_BENCH_ROOT)/generator/test_schedule/sched_workloads.c \
	$(RTOS_BENCH_ROOT)/generator/test_schedule/sched_compute_wrappers.c \
	$(RTOS_BENCH_ROOT)/generator/test_schedule/sched_mqtt_wrapper.c \
	$(RTOS_BENCH_ROOT)/generator/test_schedule/sched_modbus_wrapper.c

CORE_SRCS := \
	$(RTOS_BENCH_ROOT)/generator/periodic_benchmark.c \
	$(RTOS_BENCH_ROOT)/generator/logging.c \
	$(RTOS_BENCH_ROOT)/generator/memory_watcher.c \
	$(RTOS_BENCH_ROOT)/generator/uunifast.c \
	$(RTOS_BENCH_ROOT)/generator/test_schedule.c \
	$(RTOS_BENCH_ROOT)/generator/result_export.c \
	$(RTOS_BENCH_ROOT)/generator/workload_registry.c \
	$(RTOS_BENCH_ROOT)/generator/workload_stub.c \
	$(RTOS_BENCH_ROOT)/generator/workload_busywait.c

# 入口：先用 posixlite_entry.c 打通（无 argp/perf 依赖，最省事）。
# 若需要 shell 子命令（rtbench test-realtime / test-schedule / …），
# 改为 rtbench_command.c + 对应 *_entry.c，参考 generator/Makefile 的 RUIHUA_* 分支。
ENTRY_SRCS := \
	$(RTOS_BENCH_ROOT)/generator/posixlite_entry.c \
	$(RTOS_BENCH_ROOT)/generator/rtbench_command.c \
	$(SCHED_SRCS)

PLATFORM_SRCS := \
	$(RTOS_BENCH_ROOT)/platforms/vxworks/timer.c \
	$(RTOS_BENCH_ROOT)/platforms/vxworks/sync.c \
	$(RTOS_BENCH_ROOT)/platforms/vxworks/scheduler.c \
	$(RTOS_BENCH_ROOT)/platforms/vxworks/timestamp.c \
	$(RTOS_BENCH_ROOT)/platforms/vxworks/signal.c

WORKLOAD_C_SRCS := \
	$(RTOS_BENCH_ROOT)/workloads/CUSUM/cusum_bench.c \
	$(RTOS_BENCH_ROOT)/workloads/EWMA/ewma_bench.c \
	$(RTOS_BENCH_ROOT)/workloads/FAST/fast.c \
	$(RTOS_BENCH_ROOT)/workloads/FAST/fast_9.c \
	$(RTOS_BENCH_ROOT)/workloads/FAST/fast_bench.c \
	$(RTOS_BENCH_ROOT)/workloads/MODBUS/modbus_bench.c \
	$(RTOS_BENCH_ROOT)/workloads/MODBUS/nanomodbus.c \
	$(RTOS_BENCH_ROOT)/workloads/MQTT/mongoose.c \
	$(RTOS_BENCH_ROOT)/workloads/MQTT/mqtt_bench.c

WORKLOAD_CXX_SRCS := \
	$(RTOS_BENCH_ROOT)/workloads/rtbench_workloads.cpp \
	$(RTOS_BENCH_ROOT)/workloads/run_all_workloads.cpp \
	$(RTOS_BENCH_ROOT)/workloads/ICP/icp.cpp \
	$(RTOS_BENCH_ROOT)/workloads/ICP/icp_bench.cpp \
	$(RTOS_BENCH_ROOT)/workloads/ICP/icpPointToPlane.cpp \
	$(RTOS_BENCH_ROOT)/workloads/ICP/icpPointToPoint.cpp \
	$(RTOS_BENCH_ROOT)/workloads/ICP/kdtree.cpp \
	$(RTOS_BENCH_ROOT)/workloads/ICP/matrix.cpp \
	$(RTOS_BENCH_ROOT)/workloads/PID/PID_v1.cpp \
	$(RTOS_BENCH_ROOT)/workloads/PID/pid_bench.cpp \
	$(RTOS_BENCH_ROOT)/workloads/PID/pid_wrapper.cpp

# ---------------------------------------------------------------------------
# 头文件路径
# ---------------------------------------------------------------------------
INC := \
	-I$(RTOS_BENCH_ROOT)/generator \
	-I$(RTOS_BENCH_ROOT)/generator/test_schedule \
	-I$(RTOS_BENCH_ROOT)/platforms/vxworks \
	-I$(RTOS_BENCH_ROOT)/workloads \
	-I$(RTOS_BENCH_ROOT)/workloads/CUSUM \
	-I$(RTOS_BENCH_ROOT)/workloads/EWMA \
	-I$(RTOS_BENCH_ROOT)/workloads/FAST \
	-I$(RTOS_BENCH_ROOT)/workloads/FAST/include_imgs \
	-I$(RTOS_BENCH_ROOT)/workloads/ICP \
	-I$(RTOS_BENCH_ROOT)/workloads/MODBUS \
	-I$(RTOS_BENCH_ROOT)/workloads/MQTT \
	-I$(RTOS_BENCH_ROOT)/workloads/PID

# ---------------------------------------------------------------------------
# 编译宏 / 选项
#
# 注意：这里**没有**任何 -DEIGEN_* 或 -include fix_opengv.h ——
#       EKF/EPnP 已从本分支剪裁，Eigen/OpenGV 依赖不复存在。
# ---------------------------------------------------------------------------
DEFS := \
	-DVXWORKS_PLATFORM \
	-DMULTI_WORKLOAD \
	-D_GNU_SOURCE \
	-D__STDC_FORMAT_MACROS \
	-D__STDC_LIMIT_MACROS \
	-D_USE_MATH_DEFINES

# 若你的 VxWorks 提供 POSIX 时钟（vxWorks.h 里有 clock_gettime/CLOCK_MONOTONIC），
# 打开下面这行可显著提升时间戳精度（见 timestamp.c）：
# DEFS += -DRTBENCH_VXWORKS_USE_POSIX_CLOCK

CFLAGS   := $(DEFS) $(INC) -O2 -g -Wall -Wno-unused-parameter
CXXFLAGS := $(DEFS) $(INC) -O2 -g -std=c++14 -fno-exceptions -fno-rtti \
            -Wno-error -Wno-literal-suffix

# ---------------------------------------------------------------------------
# 构建规则
# ---------------------------------------------------------------------------
OBJ_DIR := build
C_OBJS   := $(patsubst $(RTOS_BENCH_ROOT)/%.c,$(OBJ_DIR)/%.o,$(CORE_SRCS) $(ENTRY_SRCS) $(PLATFORM_SRCS) $(WORKLOAD_C_SRCS))
CXX_OBJS := $(patsubst $(RTOS_BENCH_ROOT)/%.cpp,$(OBJ_DIR)/%.o,$(WORKLOAD_CXX_SRCS))

.PHONY: all clean
all: $(C_OBJS) $(CXX_OBJS)

$(OBJ_DIR)/%.o: $(RTOS_BENCH_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(RTOS_BENCH_ROOT)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)
