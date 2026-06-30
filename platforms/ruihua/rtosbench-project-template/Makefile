# RTOS-Bench Makefile for Ruihua/ReWorks IDE.

# If this template is built in-place from the RTOS-Bench repository, this points
# at the repository root. If you copy the template into the Ruihua IDE workspace,
# create makefile.local and set RTOSBENCH_SRC to your cloned repository path.
RTOSBENCH_SRC ?= $(abspath ../../..)
REDE_HOME ?= C:/rtos/6.1.1-ARM

# Local developer overrides. This file is intentionally gitignored.
-include makefile.local

# Optional Ruihua IDE generated/project configuration.
-include makefile.conf

#==============================================================================
# Architecture selection: ARCH=arm (default) or ARCH=aarch64
#==============================================================================
ARCH ?= arm
FULL_WORKLOADS ?= 0

 ifeq ($(ARCH),loongarch64)
    CROSS_COMPILE ?= loongarch64-elf-
    BUILD_DIR := gnuloongarch64

    ARCH_FLAGS := -mabi=lp64d -march=loongarch64 -mstrict-align
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

CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
SIZE = $(CROSS_COMPILE)size

OUTPUT := $(BUILD_DIR)/rtosbench.out

#==============================================================================
# Compiler and linker flags
#==============================================================================
CFLAGS += -O0 -gdwarf-2 -Wall -fmessage-length=0
CFLAGS += $(ARCH_FLAGS) $(ARCH_DEFS)
CFLAGS += -D__multi_core__ -D__REWORKS__ -DUSE_REWORKS_IPNET_69 -DRUIHUA_PLATFORM
CFLAGS += -DMULTI_WORKLOAD
CFLAGS += -MMD -MP

# LoongArch ReWorks DKM loader does not support PLT/GOT/GP-relative
# relocations emitted by the default LoongArch code model.  These options match
# the ReDe-generated LoongArch downloadable project rules, for example testdown.
ifeq ($(ARCH),loongarch64)
CFLAGS += -fno-plt
CFLAGS += -Wa,-mla-local-with-abs
CFLAGS += -Wa,-mla-global-with-abs
CFLAGS += -DRTOSBENCH_USE_MANUAL_WORKLOAD_REGISTRATION
endif

# Ruihua SDK headers.
CFLAGS += -I"$(REDE_HOME)/user resource/h/vx"
CFLAGS += -I"$(REDE_HOME)/resource/h"
CFLAGS += -I"$(REDE_HOME)/user resource/h/io"
CFLAGS += -I"$(REDE_HOME)/user resource/h/ipnet"
CFLAGS += -I"$(REDE_HOME)/user resource/h/symtbl"

# RTOS-Bench headers.
CFLAGS += -I"$(RTOSBENCH_SRC)/generator"
CFLAGS += -I"$(RTOSBENCH_SRC)/workloads"
CFLAGS += -I"$(RTOSBENCH_SRC)/generator/test_schedule"
CFLAGS += -I"$(RTOSBENCH_SRC)/generator/realtime_orig/les"
CFLAGS += -I"$(RTOSBENCH_SRC)/generator/realtime_orig/realtime"
CFLAGS += -I"$(RTOSBENCH_SRC)/generator/realtime_orig/multicore"
CFLAGS += -I"$(RTOSBENCH_SRC)/generator/realtime_orig/verify"
CFLAGS += -I"$(RTOSBENCH_SRC)/generator/stress_orig"
CFLAGS += -I"$(RTOSBENCH_SRC)/generator/stress_orig/common"
CFLAGS += -I"$(RTOSBENCH_SRC)/generator/stress_orig/osal"

# Workload headers.
CFLAGS += -I"$(RTOSBENCH_SRC)/workloads/PID"
CFLAGS += -I"$(RTOSBENCH_SRC)/workloads/CUSUM"
CFLAGS += -I"$(RTOSBENCH_SRC)/workloads/EWMA"
CFLAGS += -I"$(RTOSBENCH_SRC)/workloads/FAST"
CFLAGS += -I"$(RTOSBENCH_SRC)/workloads/EKF"
CFLAGS += -I"$(RTOSBENCH_SRC)/workloads/EPNP"
CFLAGS += -I"$(RTOSBENCH_SRC)/workloads/ICP"
CFLAGS += -I"$(RTOSBENCH_SRC)/workloads/MODBUS"
CFLAGS += -I"$(RTOSBENCH_SRC)/workloads/MQTT"

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti -DRUIHUA_PLATFORM
CXXFLAGS += -I"$(RTOSBENCH_SRC)/workloads/EKF/include" \
			-I"$(RTOSBENCH_SRC)/workloads/EKF/include/matrix" \
			-I"$(RTOSBENCH_SRC)/workloads/EKF/geo_lookup" \
			-I"$(RTOSBENCH_SRC)/workloads/EKF/geo"
CXXFLAGS += -DEIGEN_DONT_VECTORIZE \
            -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT \
            -DEIGEN_MAX_ALIGN_BYTES=8 \
            -DEIGEN_HAS_C99_MATH=0

LDFLAGS := -r $(CONFIG_LINK_DYNAMIC_FLAGS)

#==============================================================================
# Source files
#==============================================================================
CORE_SRC := \
	$(RTOSBENCH_SRC)/generator/periodic_benchmark.c \
	$(RTOSBENCH_SRC)/generator/logging.c \
	$(RTOSBENCH_SRC)/generator/memory_watcher.c \
	$(RTOSBENCH_SRC)/generator/uunifast.c \
	$(RTOSBENCH_SRC)/generator/test_schedule.c \
	$(RTOSBENCH_SRC)/generator/test_schedule/sched_workloads.c \
	$(RTOSBENCH_SRC)/generator/test_schedule/sched_compute_wrappers.c \
	$(RTOSBENCH_SRC)/generator/test_schedule/sched_mqtt_wrapper.c \
	$(RTOSBENCH_SRC)/generator/test_schedule/sched_modbus_wrapper.c \
	$(RTOSBENCH_SRC)/generator/rtbench_command.c \
	$(RTOSBENCH_SRC)/generator/test_realtime.c \
	$(RTOSBENCH_SRC)/generator/test_stress.c \
	$(RTOSBENCH_SRC)/generator/test_cmd.c \
	$(RTOSBENCH_SRC)/generator/result_export.c \
	$(RTOSBENCH_SRC)/generator/workload_registry.c \
	$(RTOSBENCH_SRC)/generator/workload_stub.c \
	$(RTOSBENCH_SRC)/generator/workload_busywait.c

PLATFORM_SRC := \
	$(RTOSBENCH_SRC)/generator/platform/ruihua/timer.c \
	$(RTOSBENCH_SRC)/generator/platform/ruihua/sync.c \
	$(RTOSBENCH_SRC)/generator/platform/ruihua/timestamp.c \
	$(RTOSBENCH_SRC)/generator/platform/ruihua/scheduler.c \
	$(RTOSBENCH_SRC)/generator/platform/ruihua/signal.c

ENTRY_SRC := $(RTOSBENCH_SRC)/generator/ruihua_entry.c

REALTIME_SRC := \
	$(wildcard $(RTOSBENCH_SRC)/generator/realtime_orig/les/*.c) \
	$(wildcard $(RTOSBENCH_SRC)/generator/realtime_orig/realtime/*.c) \
	$(wildcard $(RTOSBENCH_SRC)/generator/realtime_orig/multicore/*.c) \
	$(wildcard $(RTOSBENCH_SRC)/generator/realtime_orig/verify/*.c)

STRESS_SRC := \
	$(RTOSBENCH_SRC)/generator/stress_orig/stress_bench.c \
	$(wildcard $(RTOSBENCH_SRC)/generator/stress_orig/common/*.c) \
	$(RTOSBENCH_SRC)/generator/stress_orig/osal/os_rede.c \
	$(wildcard $(RTOSBENCH_SRC)/generator/stress_orig/stressor/*.c)

WORKLOAD_WRAPPER_SRC := $(RTOSBENCH_SRC)/workloads/rtbench_workloads.cpp

WORKLOAD_C_SRC += $(RTOSBENCH_SRC)/workloads/CUSUM/cusum_bench.c
WORKLOAD_C_SRC += $(RTOSBENCH_SRC)/workloads/EWMA/ewma_bench.c
WORKLOAD_C_SRC += \
	$(RTOSBENCH_SRC)/workloads/FAST/fast.c \
	$(RTOSBENCH_SRC)/workloads/FAST/fast_9.c \
	$(RTOSBENCH_SRC)/workloads/FAST/fast_bench.c
WORKLOAD_C_SRC += \
	$(RTOSBENCH_SRC)/workloads/MODBUS/modbus_bench.c \
	$(RTOSBENCH_SRC)/workloads/MODBUS/nanomodbus.c
WORKLOAD_C_SRC += \
	$(RTOSBENCH_SRC)/workloads/MQTT/mongoose.c \
	$(RTOSBENCH_SRC)/workloads/MQTT/mqtt_bench.c

WORKLOAD_CPP_SRC += \
	$(RTOSBENCH_SRC)/workloads/run_all_workloads.cpp
WORKLOAD_CPP_SRC += \
	$(RTOSBENCH_SRC)/workloads/PID/pid_bench.cpp \
	$(RTOSBENCH_SRC)/workloads/PID/PID_v1.cpp \
	$(RTOSBENCH_SRC)/workloads/PID/pid_wrapper.cpp
WORKLOAD_CPP_SRC += \
	$(RTOSBENCH_SRC)/workloads/EKF/ekf_bench.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/EKF_core/EKFGSF_yaw.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/EKF_core/airspeed_fusion.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/EKF_core/control.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/EKF_core/covariance.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/EKF_core/drag_fusion.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/EKF_core/ekf.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/EKF_core/ekf_helper.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/EKF_core/estimator_interface.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/EKF_core/gps_checks.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/EKF_core/gps_yaw_fusion.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/EKF_core/imu_down_sampler.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/EKF_core/mag_control.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/EKF_core/mag_fusion.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/EKF_core/optflow_fusion.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/EKF_core/sensor_range_finder.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/EKF_core/sideslip_fusion.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/EKF_core/terrain_estimator.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/EKF_core/utils.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/EKF_core/vel_pos_fusion.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/geo/geo.cpp \
	$(RTOSBENCH_SRC)/workloads/EKF/geo_lookup/geo_mag_declination.cpp
WORKLOAD_CPP_SRC += \
	$(RTOSBENCH_SRC)/workloads/EPNP/cayley.cpp \
	$(RTOSBENCH_SRC)/workloads/EPNP/CentralAbsoluteAdapter.cpp \
	$(RTOSBENCH_SRC)/workloads/EPNP/Epnp.cpp \
	$(RTOSBENCH_SRC)/workloads/EPNP/epnp_bench.cpp \
	$(RTOSBENCH_SRC)/workloads/EPNP/experiment_helpers.cpp \
	$(RTOSBENCH_SRC)/workloads/EPNP/methods.cpp \
	$(RTOSBENCH_SRC)/workloads/EPNP/random_generators.cpp \
	$(RTOSBENCH_SRC)/workloads/EPNP/time_measurement.cpp
WORKLOAD_CPP_SRC += \
	$(RTOSBENCH_SRC)/workloads/ICP/icp.cpp \
	$(RTOSBENCH_SRC)/workloads/ICP/icp_bench.cpp \
	$(RTOSBENCH_SRC)/workloads/ICP/icpPointToPlane.cpp \
	$(RTOSBENCH_SRC)/workloads/ICP/icpPointToPoint.cpp \
	$(RTOSBENCH_SRC)/workloads/ICP/kdtree.cpp \
	$(RTOSBENCH_SRC)/workloads/ICP/matrix.cpp

SRCS_C := $(CORE_SRC) $(PLATFORM_SRC) $(REALTIME_SRC) $(STRESS_SRC) $(ENTRY_SRC) $(WORKLOAD_C_SRC)
SRCS_CXX := $(WORKLOAD_WRAPPER_SRC) $(WORKLOAD_CPP_SRC)

define c_to_obj
$(BUILD_DIR)/$(subst :,,$(subst /,_,$(basename $(1)))).o
endef

define cpp_to_obj
$(BUILD_DIR)/$(subst :,,$(subst /,_,$(basename $(1)))).o
endef

OBJS_C := $(foreach src,$(SRCS_C),$(call c_to_obj,$(src)))
OBJS_CXX := $(foreach src,$(SRCS_CXX),$(call cpp_to_obj,$(src)))
OBJS := $(OBJS_C) $(OBJS_CXX)
DEPS := $(OBJS:.o=.d)

.PHONY: all clean init info arm aarch64 loongarch64

all: init $(BUILD_DIR) $(OUTPUT)
	@echo ""
	@echo "=== Build Complete ($(ARCH)) ==="
	@$(SIZE) $(OUTPUT)

arm:
	$(MAKE) ARCH=arm all

aarch64:
	$(MAKE) ARCH=aarch64 all

loongarch64:
	$(MAKE) ARCH=loongarch64 all

info:
	@echo "=== RTOS-Bench for Ruihua RTOS ==="
	@echo "RTOSBENCH_SRC: $(RTOSBENCH_SRC)"
	@echo "REDE_HOME: $(REDE_HOME)"
	@echo "Architecture: $(ARCH)"
	@echo "Compiler: $(CC)"
	@echo "Build dir: $(BUILD_DIR)"
	@echo "Full workloads: $(FULL_WORKLOADS)"
	@echo "C sources: $(words $(SRCS_C))"
	@echo "C++ sources: $(words $(SRCS_CXX))"

init:
	@if [ ! -f "$(RTOSBENCH_SRC)/generator/ruihua_entry.c" ]; then \
		echo "Error: RTOS-Bench source not found at $(RTOSBENCH_SRC)"; \
		echo "Set RTOSBENCH_SRC in makefile.local or environment."; \
		exit 1; \
	fi

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(OUTPUT): $(OBJS)
	@echo "Linking: $@"
	@$(LD) $(LDFLAGS) -o $@ $(OBJS)

define make_c_rule
$(call c_to_obj,$(1)): $(1)
	@echo "Compiling: $$<"
	@$$(CC) $$(CFLAGS) -MF"$$(@:.o=.d)" -MT"$$@" -c -o $$@ $$<
endef

$(foreach src,$(SRCS_C),$(eval $(call make_c_rule,$(src))))

define make_cpp_rule
$(call cpp_to_obj,$(1)): $(1)
	@echo "Compiling: $$<"
	@$$(CXX) $$(CXXFLAGS) -MF"$$(@:.o=.d)" -MT"$$@" -c -o $$@ $$<
endef

$(foreach src,$(SRCS_CXX),$(eval $(call make_cpp_rule,$(src))))

clean:
	@rm -rf gnuarm gnuaarch64 gnuloongarch64
	@echo "Clean complete"

-include $(DEPS)
