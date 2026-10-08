PROJECT_ROOT := $(abspath $(CURDIR)/../..)
GENERATOR_DIR := $(PROJECT_ROOT)/generator
WORKLOADS_DIR := $(PROJECT_ROOT)/workloads

# The two supported boards share the openEuler userspace ABI and AArch64
# toolchain. BOARD only selects board metadata and keeps build outputs apart.
BOARD ?= feiteng
BOARD_CONFIG := $(CURDIR)/boards/$(BOARD).mk
ifeq ($(wildcard $(BOARD_CONFIG)),)
$(error unsupported BOARD='$(BOARD)'; use BOARD=feiteng or BOARD=orangepi)
endif
include $(BOARD_CONFIG)

CROSS_COMPILE ?= aarch64-buildroot-linux-gnu-
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
READELF := $(CROSS_COMPILE)readelf

BUILD_DIR ?= $(CURDIR)/build/$(BOARD)/aarch64
OUTPUT ?= $(BUILD_DIR)/rtos-bench-openeuler-$(BOARD)-aarch64
STATIC ?= 1

CPPFLAGS += \
	-D_GNU_SOURCE \
	-DLINUX_PLATFORM \
	-DMULTI_WORKLOAD \
	-DECL_STANDALONE \
	-D__STDC_FORMAT_MACROS \
	-D__STDC_LIMIT_MACROS \
	-D_USE_MATH_DEFINES \
	-DEIGEN_DONT_VECTORIZE \
	-DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT \
	-DFIX_OPENGV_USE_STUB_LDOUBLE \
	$(BOARD_CPPFLAGS) \
	-I$(GENERATOR_DIR) \
	-I$(GENERATOR_DIR)/test_schedule \
	-I$(GENERATOR_DIR)/realtime_orig/les \
	-I$(GENERATOR_DIR)/realtime_orig/realtime \
	-I$(GENERATOR_DIR)/realtime_orig/multicore \
	-I$(GENERATOR_DIR)/realtime_orig/verify \
	-I$(GENERATOR_DIR)/stress_orig \
	-I$(GENERATOR_DIR)/stress_orig/common \
	-I$(GENERATOR_DIR)/stress_orig/osal \
	-I$(WORKLOADS_DIR) \
	-I$(WORKLOADS_DIR)/CUSUM \
	-I$(WORKLOADS_DIR)/EWMA \
	-I$(WORKLOADS_DIR)/FAST \
	-I$(WORKLOADS_DIR)/FAST/include_imgs \
	-I$(WORKLOADS_DIR)/PID \
	-I$(WORKLOADS_DIR)/EKF \
	-I$(WORKLOADS_DIR)/EKF/EKF_core \
	-I$(WORKLOADS_DIR)/EKF/include \
	-I$(WORKLOADS_DIR)/EKF/include/matrix \
	-I$(WORKLOADS_DIR)/EKF/geo \
	-I$(WORKLOADS_DIR)/EKF/geo_lookup \
	-I$(WORKLOADS_DIR)/EPNP \
	-I$(WORKLOADS_DIR)/EPNP/opengv \
	-I$(WORKLOADS_DIR)/EPNP/Eigen \
	-I$(WORKLOADS_DIR)/ICP \
	-I$(WORKLOADS_DIR)/MODBUS \
	-I$(WORKLOADS_DIR)/MQTT

COMMON_FLAGS := -O2 -g -fPIC -ffunction-sections -fdata-sections -pthread
CFLAGS += $(COMMON_FLAGS) -std=gnu11
CXXFLAGS += $(COMMON_FLAGS) -std=gnu++14
LDLIBS += -pthread -lrt -lm -latomic
LDFLAGS += -Wl,--gc-sections -Wl,--wrap=malloc -Wl,--wrap=mmap
ifeq ($(STATIC),1)
LDFLAGS += -static
endif

CORE_C_SRCS := \
	$(GENERATOR_DIR)/linux_entry.c \
	$(GENERATOR_DIR)/periodic_benchmark.c \
	$(GENERATOR_DIR)/logging.c \
	$(GENERATOR_DIR)/memory_watcher.c \
	$(GENERATOR_DIR)/get_cpu_timestamp.c \
	$(GENERATOR_DIR)/uunifast.c \
	$(GENERATOR_DIR)/result_export.c \
	$(GENERATOR_DIR)/workload_registry.c \
	$(GENERATOR_DIR)/workload_stub.c \
	$(GENERATOR_DIR)/workload_busywait.c \
	$(GENERATOR_DIR)/test_realtime.c \
	$(GENERATOR_DIR)/test_stress.c \
	$(GENERATOR_DIR)/test_cmd.c \
	$(GENERATOR_DIR)/rtbench_command.c

# Keep this list explicit. These five files are the complete test-schedule
# implementation and are required by RTOS-Bench/AGENTS.md.
SCHEDULE_C_SRCS := \
	$(GENERATOR_DIR)/test_schedule.c \
	$(GENERATOR_DIR)/test_schedule/sched_workloads.c \
	$(GENERATOR_DIR)/test_schedule/sched_compute_wrappers.c \
	$(GENERATOR_DIR)/test_schedule/sched_mqtt_wrapper.c \
	$(GENERATOR_DIR)/test_schedule/sched_modbus_wrapper.c

PLATFORM_C_SRCS := \
	$(GENERATOR_DIR)/platform/linux/timer.c \
	$(GENERATOR_DIR)/platform/linux/sync.c \
	$(GENERATOR_DIR)/platform/linux/timestamp.c \
	$(GENERATOR_DIR)/platform/linux/scheduler.c \
	$(GENERATOR_DIR)/platform/linux/signal.c

REALTIME_C_SRCS := \
	$(wildcard $(GENERATOR_DIR)/realtime_orig/les/*.c) \
	$(wildcard $(GENERATOR_DIR)/realtime_orig/realtime/*.c) \
	$(wildcard $(GENERATOR_DIR)/realtime_orig/multicore/*.c) \
	$(wildcard $(GENERATOR_DIR)/realtime_orig/verify/*.c)

STRESS_C_SRCS := \
	$(GENERATOR_DIR)/stress_orig/stress_bench.c \
	$(GENERATOR_DIR)/stress_orig/common/stress-ng.c \
	$(GENERATOR_DIR)/stress_orig/common/core-helper.c \
	$(GENERATOR_DIR)/stress_orig/common/stress_stored_job.c \
	$(GENERATOR_DIR)/stress_orig/osal/os_openeuler.c \
	$(wildcard $(GENERATOR_DIR)/stress_orig/stressor/*.c)

WORKLOAD_C_SRCS := $(shell find $(WORKLOADS_DIR) -type f -name '*.c')
WORKLOAD_CXX_SRCS := $(shell find $(WORKLOADS_DIR) -type f -name '*.cpp')

C_SRCS := $(CORE_C_SRCS) $(SCHEDULE_C_SRCS) $(PLATFORM_C_SRCS) \
	$(REALTIME_C_SRCS) $(STRESS_C_SRCS) $(WORKLOAD_C_SRCS)
CXX_SRCS := $(WORKLOAD_CXX_SRCS)

C_OBJS := $(patsubst $(PROJECT_ROOT)/%.c,$(BUILD_DIR)/obj/%.c.o,$(C_SRCS))
CXX_OBJS := $(patsubst $(PROJECT_ROOT)/%.cpp,$(BUILD_DIR)/obj/%.cpp.o,$(CXX_SRCS))
OBJS := $(C_OBJS) $(CXX_OBJS)
DEPS := $(OBJS:.o=.d)

.PHONY: all clean check-toolchain check-schedule-sources print-config

all: check-toolchain check-schedule-sources $(OUTPUT)

check-toolchain:
	@command -v $(CC) >/dev/null || { \
		echo "error: $(CC) not found; set OPENEULER_AARCH64_SDK or run build-aarch64.sh"; \
		exit 1; \
	}

check-schedule-sources:
	@for src in $(SCHEDULE_C_SRCS); do \
		test -f "$$src" || { echo "error: missing schedule source $$src"; exit 1; }; \
	done

$(OUTPUT): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)
	@echo "Built $(BOARD_NAME): $@"
	@file $@
	@$(READELF) -h $@ | grep -E 'Class:|Data:|Machine:|Type:'

$(BUILD_DIR)/obj/%.c.o: $(PROJECT_ROOT)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/obj/%.cpp.o: $(PROJECT_ROOT)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

print-config:
	@echo "BOARD=$(BOARD) ($(BOARD_NAME))"
	@echo "BOARD_DEPLOY_DIR=$(BOARD_DEPLOY_DIR)"
	@echo "CC=$(CC)"
	@echo "CXX=$(CXX)"
	@echo "BUILD_DIR=$(BUILD_DIR)"
	@echo "OUTPUT=$(OUTPUT)"
	@echo "STATIC=$(STATIC)"
	@echo "STRESS_OSAL=$(GENERATOR_DIR)/stress_orig/osal/os_openeuler.c"

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
