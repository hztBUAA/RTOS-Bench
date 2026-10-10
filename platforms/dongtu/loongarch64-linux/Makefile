# ============================================================================
# RTOS-Bench — Dongtu LoongArch64 (Linux / musl) cross build
#
# Produces a standalone LoongArch64 Linux userland ELF from the Dongtu source
# set. This is the command-line counterpart of the Intewell IDE flow:
#   - IDE flow (partition image) : platforms/dongtu/intewell.mk, loaded by the
#     vendor toolchain via the project-local config_os.mk, output *.bin.
#   - this flow (userspace ELF)  : plain executable, SCP to the board and run.
#
# Usage:
#   CROSS_COMPILE=/path/to/loongarch64-unknown-linux-musl- make
#   ./build-loongarch64.sh                      # convenience wrapper
# ============================================================================

PROJECT_ROOT  := $(abspath $(CURDIR)/../..)
GENERATOR_DIR := $(PROJECT_ROOT)/generator
WORKLOADS_DIR := $(PROJECT_ROOT)/workloads
DONGTU_DIR    := $(PROJECT_ROOT)/platforms/dongtu

CROSS_COMPILE ?= loongarch64-unknown-linux-musl-
CC      := $(CROSS_COMPILE)gcc
CXX     := $(CROSS_COMPILE)g++
READELF := $(CROSS_COMPILE)readelf

BUILD_DIR ?= $(CURDIR)/build
OUTPUT    ?= $(BUILD_DIR)/rtbench-dongtu-loongarch64

# ---------------------------------------------------------------------------
# Defines / includes
#
# RTBENCH_DONGTU_LINUX selects the POSIX side of the Dongtu/Linux-vs-Intewell
# differences (network headers, sched_getcpu vs cpuIDGet).
# TEST_SCHEDULE_MAX_PERIOD_NS caps the schedulability period at 10 s so the
# low-utilization gradients stay bounded in wall-clock time.
# ---------------------------------------------------------------------------
DEFS := \
	-DDONGTU_PLATFORM \
	-DRTBENCH_DONGTU_LINUX \
	-DMULTI_WORKLOAD \
	-DTEST_SCHEDULE_MAX_PERIOD_NS=10000000000ULL \
	-D_GNU_SOURCE \
	-D__STDC_FORMAT_MACROS \
	-D__STDC_LIMIT_MACROS \
	-D_USE_MATH_DEFINES \
	-Dalloca=__builtin_alloca

INC := \
	-I$(DONGTU_DIR) \
	-I$(GENERATOR_DIR) \
	-I$(GENERATOR_DIR)/platform/dongtu \
	-I$(GENERATOR_DIR)/test_schedule \
	-I$(GENERATOR_DIR)/realtime_orig \
	-I$(GENERATOR_DIR)/realtime_orig/les \
	-I$(GENERATOR_DIR)/realtime_orig/realtime \
	-I$(GENERATOR_DIR)/realtime_orig/multicore \
	-I$(GENERATOR_DIR)/realtime_orig/verify \
	-I$(GENERATOR_DIR)/stress_orig \
	-I$(GENERATOR_DIR)/stress_orig/common \
	-I$(GENERATOR_DIR)/stress_orig/osal \
	-I$(GENERATOR_DIR)/stress_orig/stressor \
	-I$(WORKLOADS_DIR) \
	-I$(WORKLOADS_DIR)/CUSUM \
	-I$(WORKLOADS_DIR)/EWMA \
	-I$(WORKLOADS_DIR)/FAST \
	-I$(WORKLOADS_DIR)/FAST/include_imgs \
	-I$(WORKLOADS_DIR)/ICP \
	-I$(WORKLOADS_DIR)/MODBUS \
	-I$(WORKLOADS_DIR)/MQTT \
	-I$(WORKLOADS_DIR)/PID

# main (>= PR #46) registers EKF/EPNP workloads unconditionally. When those
# directories exist, compile them and add the Eigen/OpenGV include + flags.
EKF_DIR  := $(WORKLOADS_DIR)/EKF
EPNP_DIR := $(WORKLOADS_DIR)/EPNP
EKF_EPNP_CXX :=
ifneq ($(wildcard $(EKF_DIR)),)
DEFS += -DEIGEN_DONT_VECTORIZE -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT -DFIX_OPENGV_USE_STUB_LDOUBLE
INC += \
	-I$(EKF_DIR) -I$(EKF_DIR)/include -I$(EKF_DIR)/EKF_core \
	-I$(EKF_DIR)/geo -I$(EKF_DIR)/geo_lookup -I$(EKF_DIR)/include/matrix \
	-I$(EPNP_DIR) -I$(EPNP_DIR)/opengv -I$(EPNP_DIR)/Eigen
EKF_EPNP_CXX := $(shell find $(EKF_DIR) $(EPNP_DIR) -name '*.cpp')
endif

CFLAGS := $(DEFS) $(INC) -O2 -g -Wall -Wno-unused-parameter
# -fpermissive downgrades the fix_opengv.h "static after extern" conflict to a
# warning (musl declares fabsl()/powl()/... before that header redefines them).
CXXFLAGS := $(DEFS) $(INC) -O2 -g -std=c++14 -fno-exceptions -fno-rtti \
            -fpermissive -Wno-error -Wno-literal-suffix
# -static            : self-contained, no board libc/loader dependency.
# -no-pie            : plain ET_EXEC static (max kernel compatibility).
# --allow-multiple-definition : fix_opengv.h emits the same long-double stubs
#                     into several TUs; they are equivalent, so this is safe.
LDFLAGS := -static -no-pie -Wl,--allow-multiple-definition -lm -pthread -lstdc++ -lrt

# ---------------------------------------------------------------------------
# Source lists (explicit; mirrors platforms/dongtu/intewell.mk minus shell.c)
# ---------------------------------------------------------------------------
C_SRCS := \
	$(GENERATOR_DIR)/dongtu_entry.c \
	$(GENERATOR_DIR)/logging.c \
	$(GENERATOR_DIR)/memory_watcher.c \
	$(GENERATOR_DIR)/periodic_benchmark.c \
	$(GENERATOR_DIR)/result_export.c \
	$(GENERATOR_DIR)/rtbench_command.c \
	$(GENERATOR_DIR)/test_cmd.c \
	$(GENERATOR_DIR)/test_realtime.c \
	$(GENERATOR_DIR)/test_schedule.c \
	$(GENERATOR_DIR)/test_stress.c \
	$(GENERATOR_DIR)/uunifast.c \
	$(GENERATOR_DIR)/workload_busywait.c \
	$(GENERATOR_DIR)/workload_registry.c \
	$(GENERATOR_DIR)/workload_stub.c \
	$(GENERATOR_DIR)/platform/dongtu/scheduler.c \
	$(GENERATOR_DIR)/platform/dongtu/signal.c \
	$(GENERATOR_DIR)/platform/dongtu/sync.c \
	$(GENERATOR_DIR)/platform/dongtu/timer.c \
	$(GENERATOR_DIR)/platform/dongtu/timestamp.c \
	$(GENERATOR_DIR)/test_schedule/sched_modbus_wrapper.c \
	$(GENERATOR_DIR)/test_schedule/sched_mqtt_wrapper.c \
	$(GENERATOR_DIR)/test_schedule/sched_workloads.c \
	$(GENERATOR_DIR)/test_schedule/sched_compute_wrappers.c \
	$(GENERATOR_DIR)/stress_orig/common/core-helper.c \
	$(GENERATOR_DIR)/stress_orig/common/stress-ng.c \
	$(GENERATOR_DIR)/stress_orig/common/stress_stored_job.c \
	$(GENERATOR_DIR)/stress_orig/osal/os_intewell.c \
	$(GENERATOR_DIR)/stress_orig/stress_bench.c \
	$(wildcard $(GENERATOR_DIR)/stress_orig/stressor/*.c) \
	$(GENERATOR_DIR)/realtime_orig/les/bench_init.c \
	$(GENERATOR_DIR)/realtime_orig/les/cpu_affinity.c \
	$(GENERATOR_DIR)/realtime_orig/les/data_tools.c \
	$(GENERATOR_DIR)/realtime_orig/les/les.c \
	$(GENERATOR_DIR)/realtime_orig/les/intewell_init.c \
	$(GENERATOR_DIR)/realtime_orig/les/reworks_int.c \
	$(GENERATOR_DIR)/realtime_orig/les/safe_sleep.c \
	$(GENERATOR_DIR)/realtime_orig/multicore/ipc_bw.c \
	$(GENERATOR_DIR)/realtime_orig/multicore/mem_bw.c \
	$(GENERATOR_DIR)/realtime_orig/multicore/task_lt.c \
	$(wildcard $(GENERATOR_DIR)/realtime_orig/realtime/*.c) \
	$(wildcard $(GENERATOR_DIR)/realtime_orig/verify/*.c) \
	$(WORKLOADS_DIR)/CUSUM/cusum_bench.c \
	$(WORKLOADS_DIR)/EWMA/ewma_bench.c \
	$(WORKLOADS_DIR)/FAST/fast.c \
	$(WORKLOADS_DIR)/FAST/fast_9.c \
	$(WORKLOADS_DIR)/FAST/fast_bench.c \
	$(WORKLOADS_DIR)/MODBUS/modbus_bench.c \
	$(WORKLOADS_DIR)/MODBUS/nanomodbus.c \
	$(WORKLOADS_DIR)/MQTT/mongoose.c \
	$(WORKLOADS_DIR)/MQTT/mqtt_bench.c

CXX_SRCS := \
	$(WORKLOADS_DIR)/rtbench_workloads.cpp \
	$(WORKLOADS_DIR)/run_all_workloads.cpp \
	$(WORKLOADS_DIR)/ICP/icp.cpp \
	$(WORKLOADS_DIR)/ICP/icpPointToPlane.cpp \
	$(WORKLOADS_DIR)/ICP/icpPointToPoint.cpp \
	$(WORKLOADS_DIR)/ICP/icp_bench.cpp \
	$(WORKLOADS_DIR)/ICP/kdtree.cpp \
	$(WORKLOADS_DIR)/ICP/matrix.cpp \
	$(WORKLOADS_DIR)/PID/PID_v1.cpp \
	$(WORKLOADS_DIR)/PID/pid_bench.cpp \
	$(EKF_EPNP_CXX)

C_OBJS   := $(patsubst $(PROJECT_ROOT)/%.c,$(BUILD_DIR)/obj/%.c.o,$(C_SRCS))
CXX_OBJS := $(patsubst $(PROJECT_ROOT)/%.cpp,$(BUILD_DIR)/obj/%.cpp.o,$(CXX_SRCS))
OBJS     := $(C_OBJS) $(CXX_OBJS)

.PHONY: all clean check-toolchain print-config
all: check-toolchain $(OUTPUT)

check-toolchain:
	@command -v $(CC) >/dev/null || { \
		echo "error: $(CC) not found."; \
		echo "Put the LoongArch musl toolchain bin/ on PATH, or set CROSS_COMPILE."; \
		exit 1; }

$(OUTPUT): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)
	@echo "Built: $@"
	@$(READELF) -h $@ | grep -E 'Class:|Data:|Machine:|Type:'

$(BUILD_DIR)/obj/%.c.o: $(PROJECT_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/obj/%.cpp.o: $(PROJECT_ROOT)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

print-config:
	@echo "CROSS_COMPILE = $(CROSS_COMPILE)"
	@echo "OUTPUT        = $(OUTPUT)"
	@echo "sources: C=$(words $(C_SRCS)) CXX=$(words $(CXX_SRCS))"
