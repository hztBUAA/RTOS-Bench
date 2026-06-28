################################################################################
# RTOS-Bench integration for Dongtu/Intewell vm_3588 and x86 projects.
#
# Include this file from the Intewell project-local config_os.mk:
#
#   RTOS_BENCH_ROOT := A:/path/to/RTOS-Bench
#   include $(RTOS_BENCH_ROOT)/platforms/dongtu/intewell.mk
################################################################################

RTBENCH_DONGTU_MK_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
RTOS_BENCH_ROOT ?= $(abspath $(RTBENCH_DONGTU_MK_DIR)/../..)
RTBENCH_EXT_OBJ_DIR ?= ./rtosbench_ext
ARCH ?= __ARM64__

RTBENCH_DONGTU_IS_X86 := $(filter _X86_ _X86_32_ _I386_ __X86__,$(ARCH))
RTBENCH_DONGTU_IS_ARM64 := $(filter __ARM64__ __AARCH64__ __aarch64__ _AARCH64_,$(ARCH))

RTBENCH_GENERATOR_SRCS := \
	dongtu_entry.c \
	logging.c \
	memory_watcher.c \
	periodic_benchmark.c \
	result_export.c \
	rtbench_command.c \
	test_cmd.c \
	test_realtime.c \
	test_schedule.c \
	test_stress.c \
	uunifast.c \
	workload_busywait.c \
	workload_registry.c \
	workload_stub.c \
	platform/dongtu/scheduler.c \
	platform/dongtu/signal.c \
	platform/dongtu/sync.c \
	platform/dongtu/timer.c \
	platform/dongtu/timestamp.c \
	test_schedule/sched_modbus_wrapper.c \
	test_schedule/sched_mqtt_wrapper.c \
	test_schedule/sched_workloads.c \
	test_schedule/sched_compute_wrappers.c \
	stress_orig/common/core-helper.c \
	stress_orig/common/stress-ng.c \
	stress_orig/common/stress_stored_job.c \
	stress_orig/osal/os_intewell.c \
	stress_orig/stress_bench.c \
	stress_orig/stressor/stress-atomic.c \
	stress_orig/stressor/stress-bitops.c \
	stress_orig/stressor/stress-bsearch.c \
	stress_orig/stressor/stress-context.c \
	stress_orig/stressor/stress-copy-file.c \
	stress_orig/stressor/stress-cpu.c \
	stress_orig/stressor/stress-dentry.c \
	stress_orig/stressor/stress-fp.c \
	stress_orig/stressor/stress-fstat.c \
	stress_orig/stressor/stress-hdd.c \
	stress_orig/stressor/stress-malloc.c \
	stress_orig/stressor/stress-matrix.c \
	stress_orig/stressor/stress-memcpy.c \
	stress_orig/stressor/stress-memthrash.c \
	stress_orig/stressor/stress-open.c \
	stress_orig/stressor/stress-pipe.c \
	stress_orig/stressor/stress-prime.c \
	stress_orig/stressor/stress-ptr-chase.c \
	stress_orig/stressor/stress-qsort.c \
	stress_orig/stressor/stress-rename.c \
	stress_orig/stressor/stress-stack.c \
	stress_orig/stressor/stress-str.c \
	stress_orig/stressor/stress-stream.c \
	stress_orig/stressor/stress-trig.c \
	stress_orig/stressor/stress-unlink.c \
	stress_orig/stressor/stress-vecmath.c \
	stress_orig/stressor/stress-vm.c

RTBENCH_REALTIME_SRCS := \
	les/bench_init.c \
	les/cpu_affinity.c \
	les/data_tools.c \
	les/les.c \
	les/intewell_init.c \
	les/reworks_int.c \
	les/safe_sleep.c \
	multicore/ipc_bw.c \
	multicore/mem_bw.c \
	multicore/task_lt.c \
	realtime/test1.c \
	realtime/test10_1.c \
	realtime/test2.c \
	realtime/test3.c \
	realtime/test4_1.c \
	realtime/test4_2.c \
	realtime/test5_3.c \
	realtime/test6_0.c \
	realtime/test6_1.c \
	realtime/test6_2.c \
	realtime/test6_3.c \
	realtime/test6_4.c \
	realtime/test7_3.c \
	realtime/test8_1.c \
	realtime/test8_2.c \
	realtime/test9_3.c \
	verify/all_realtime_verify.c \
	verify/cpu_bind_verify.c \
	verify/freq_verify.c \
	verify/interrupt_stub_verify.c \
	verify/schedule_stub_verify.c \
	verify/time_verify.c

RTBENCH_WORKLOAD_SRCS := \
	CUSUM/cusum_bench.c \
	EWMA/ewma_bench.c \
	FAST/fast.c \
	FAST/fast_9.c \
	FAST/fast_bench.c \
	MODBUS/modbus_bench.c \
	MODBUS/nanomodbus.c \
	MQTT/mongoose.c \
	MQTT/mqtt_bench.c

RTBENCH_WORKLOAD_CXX_SRCS := \
	rtbench_workloads.cpp \
	EKF/EKF_core/airspeed_fusion.cpp \
	EKF/EKF_core/control.cpp \
	EKF/EKF_core/covariance.cpp \
	EKF/EKF_core/drag_fusion.cpp \
	EKF/EKF_core/ekf.cpp \
	EKF/EKF_core/EKFGSF_yaw.cpp \
	EKF/EKF_core/ekf_helper.cpp \
	EKF/EKF_core/estimator_interface.cpp \
	EKF/EKF_core/gps_checks.cpp \
	EKF/EKF_core/gps_yaw_fusion.cpp \
	EKF/EKF_core/imu_down_sampler.cpp \
	EKF/EKF_core/mag_control.cpp \
	EKF/EKF_core/mag_fusion.cpp \
	EKF/EKF_core/optflow_fusion.cpp \
	EKF/EKF_core/sensor_range_finder.cpp \
	EKF/EKF_core/sideslip_fusion.cpp \
	EKF/EKF_core/terrain_estimator.cpp \
	EKF/EKF_core/utils.cpp \
	EKF/EKF_core/vel_pos_fusion.cpp \
	EKF/geo/geo.cpp \
	EKF/geo_lookup/geo_mag_declination.cpp \
	EKF/ekf_bench.cpp \
	EPNP/cayley.cpp \
	EPNP/CentralAbsoluteAdapter.cpp \
	EPNP/Epnp.cpp \
	EPNP/epnp_bench.cpp \
	EPNP/experiment_helpers.cpp \
	EPNP/methods.cpp \
	EPNP/random_generators.cpp \
	EPNP/time_measurement.cpp \
	ICP/icp.cpp \
	ICP/icpPointToPlane.cpp \
	ICP/icpPointToPoint.cpp \
	ICP/icp_bench.cpp \
	ICP/kdtree.cpp \
	ICP/matrix.cpp \
	PID/PID_v1.cpp \
	PID/pid_bench.cpp

RTBENCH_DONGTU_PLATFORM_SRCS := \
	shell.c

ifneq ($(wildcard $(RTOS_BENCH_ROOT)/platforms/dongtu/compat.c),)
RTBENCH_DONGTU_PLATFORM_SRCS += compat.c
endif

RTBENCH_GENERATOR_OBJS := $(addprefix $(RTBENCH_EXT_OBJ_DIR)/generator/,$(RTBENCH_GENERATOR_SRCS:.c=.o))
RTBENCH_REALTIME_OBJS := $(addprefix $(RTBENCH_EXT_OBJ_DIR)/generator/realtime_orig/,$(RTBENCH_REALTIME_SRCS:.c=.o))
RTBENCH_WORKLOAD_OBJS := $(addprefix $(RTBENCH_EXT_OBJ_DIR)/workloads/,$(RTBENCH_WORKLOAD_SRCS:.c=.o))
RTBENCH_WORKLOAD_CXX_OBJS := $(addprefix $(RTBENCH_EXT_OBJ_DIR)/workloads/,$(RTBENCH_WORKLOAD_CXX_SRCS:.cpp=.o))
RTBENCH_DONGTU_PLATFORM_OBJS := $(addprefix $(RTBENCH_EXT_OBJ_DIR)/platforms/dongtu/,$(RTBENCH_DONGTU_PLATFORM_SRCS:.c=.o))

RTBENCH_GENERATOR_DEPS := $(RTBENCH_GENERATOR_OBJS:.o=.d)
RTBENCH_REALTIME_DEPS := $(RTBENCH_REALTIME_OBJS:.o=.d)
RTBENCH_WORKLOAD_DEPS := $(RTBENCH_WORKLOAD_OBJS:.o=.d)
RTBENCH_WORKLOAD_CXX_DEPS := $(RTBENCH_WORKLOAD_CXX_OBJS:.o=.d)
RTBENCH_DONGTU_PLATFORM_DEPS := $(RTBENCH_DONGTU_PLATFORM_OBJS:.o=.d)

RTBENCH_DONGTU_SHELL_OBJ := $(RTBENCH_EXT_OBJ_DIR)/platforms/dongtu/shell.o
RTBENCH_DONGTU_ARCHIVE_PLATFORM_OBJS := $(filter-out $(RTBENCH_DONGTU_SHELL_OBJ),$(RTBENCH_DONGTU_PLATFORM_OBJS))

CONFIG_CPLUSPLUS := 1
HAS_CPP := Y

C_SRCS += $(addprefix $(RTOS_BENCH_ROOT)/generator/,$(RTBENCH_GENERATOR_SRCS))
C_SRCS += $(addprefix $(RTOS_BENCH_ROOT)/generator/realtime_orig/,$(RTBENCH_REALTIME_SRCS))
C_SRCS += $(addprefix $(RTOS_BENCH_ROOT)/workloads/,$(RTBENCH_WORKLOAD_SRCS))
C_SRCS += $(addprefix $(RTOS_BENCH_ROOT)/platforms/dongtu/,$(RTBENCH_DONGTU_PLATFORM_SRCS))
CXX_SRCS += $(addprefix $(RTOS_BENCH_ROOT)/workloads/,$(RTBENCH_WORKLOAD_CXX_SRCS))

OBJS += $(RTBENCH_GENERATOR_OBJS)
OBJS += $(RTBENCH_REALTIME_OBJS)
OBJS += $(RTBENCH_WORKLOAD_OBJS)
OBJS += $(RTBENCH_WORKLOAD_CXX_OBJS)
OBJS += $(RTBENCH_DONGTU_PLATFORM_OBJS)

DEPS += $(RTBENCH_GENERATOR_DEPS)
DEPS += $(RTBENCH_REALTIME_DEPS)
DEPS += $(RTBENCH_WORKLOAD_DEPS)
DEPS += $(RTBENCH_WORKLOAD_CXX_DEPS)
DEPS += $(RTBENCH_DONGTU_PLATFORM_DEPS)

# Only shell.o is linked directly into the final ELF.
# This keeps SHELL_CMD_REGISTER(rtbench, ...) alive without directly linking
# dongtu_entry.o and without whole-archiving librtosbench_*.a.
USER_OBJS += $(RTBENCH_DONGTU_SHELL_OBJ)

RTBENCH_DONGTU_FLAGS := \
	-DDONGTU_PLATFORM \
	-DMULTI_WORKLOAD \
	-Dalloca=__builtin_alloca \
	-I$(RTOS_BENCH_ROOT)/platforms/dongtu \
	-I$(RTOS_BENCH_ROOT)/generator \
	-I$(RTOS_BENCH_ROOT)/generator/platform/dongtu \
	-I$(RTOS_BENCH_ROOT)/generator/test_schedule \
	-I$(RTOS_BENCH_ROOT)/generator/realtime_orig/les \
	-I$(RTOS_BENCH_ROOT)/generator/realtime_orig/realtime \
	-I$(RTOS_BENCH_ROOT)/generator/realtime_orig/multicore \
	-I$(RTOS_BENCH_ROOT)/generator/realtime_orig/verify \
	-I$(RTOS_BENCH_ROOT)/generator/stress_orig \
	-I$(RTOS_BENCH_ROOT)/generator/stress_orig/common \
	-I$(RTOS_BENCH_ROOT)/generator/stress_orig/osal \
	-I$(RTOS_BENCH_ROOT)/generator/stress_orig/stressor \
	-I$(RTOS_BENCH_ROOT)/workloads \
	-I$(RTOS_BENCH_ROOT)/workloads/CUSUM \
	-I$(RTOS_BENCH_ROOT)/workloads/EWMA \
	-I$(RTOS_BENCH_ROOT)/workloads/FAST \
	-I$(RTOS_BENCH_ROOT)/workloads/EKF \
	-I$(RTOS_BENCH_ROOT)/workloads/EKF/include \
	-I$(RTOS_BENCH_ROOT)/workloads/EKF/include/matrix \
	-I$(RTOS_BENCH_ROOT)/workloads/EKF/geo_lookup \
	-I$(RTOS_BENCH_ROOT)/workloads/EKF/geo \
	-I$(RTOS_BENCH_ROOT)/workloads/EPNP \
	-I$(RTOS_BENCH_ROOT)/workloads/EPNP/opengv \
	-I$(RTOS_BENCH_ROOT)/workloads/EPNP/Eigen \
	-I$(RTOS_BENCH_ROOT)/workloads/ICP \
	-I$(RTOS_BENCH_ROOT)/workloads/MODBUS \
	-I$(RTOS_BENCH_ROOT)/workloads/MQTT \
	-I$(RTOS_BENCH_ROOT)/workloads/PID

RTBENCH_DONGTU_CXX ?= $(subst -gcc,-g++,$(CC))

RTBENCH_DONGTU_CXX_COMPAT_FLAGS :=
ifneq ($(RTBENCH_DONGTU_IS_ARM64),)
RTBENCH_DONGTU_CXX_COMPAT_FLAGS := -include $(RTOS_BENCH_ROOT)/platforms/dongtu/cxx_compat.h
endif

RTBENCH_DONGTU_CXX_FLAGS := \
	$(RTBENCH_DONGTU_FLAGS) \
	$(RTBENCH_DONGTU_CXX_COMPAT_FLAGS) \
	-UENABLE_CPLUSPLUS \
	-DENABLE_CPLUSPLUS=1 \
	-D_SYS_REENT_H_ \
	-D_NOTHROW= \
	-D_GLIBCXX_HAVE_MBSTATE_T=1 \
	-DECL_STANDALONE \
	-D__STDC_FORMAT_MACROS \
	-D__STDC_LIMIT_MACROS \
	-D_GLIBCXX_USE_C99 \
	-D_GLIBCXX_USE_C99_MATH \
	-D_USE_MATH_DEFINES \
	-DEIGEN_DONT_VECTORIZE \
	-DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT \
	-std=c++14 \
	-Wno-error \
	-Wno-literal-suffix \
	-Wno-cpp

RTBENCH_DONGTU_EPNP_CXX_FLAGS := \
	$(RTBENCH_DONGTU_CXX_FLAGS) \
	-include $(RTOS_BENCH_ROOT)/workloads/EPNP/fix_opengv.h

$(RTBENCH_EXT_OBJ_DIR)/generator/%.o: RTBENCH_EXTRA_FLAGS := $(RTBENCH_DONGTU_FLAGS)
$(RTBENCH_EXT_OBJ_DIR)/generator/%.o: $(RTOS_BENCH_ROOT)/generator/%.c
	@mkdir -p $(dir $@)
	@echo 'Building RTOS-Bench file: $<'
	$(CC) $(COMPILE_SYMBOL) $(COMPILE_INCLUDE) $(RTBENCH_EXTRA_FLAGS) $(USER_OPTION) -D${ARCH} $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) $(COMPILE_WARNING) $(COMPILE_OTHER) -o $@ $< && \
	$(CC) $(COMPILE_SYMBOL) $(COMPILE_INCLUDE) $(RTBENCH_EXTRA_FLAGS) $(USER_OPTION) -D${ARCH} $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) $(COMPILE_WARNING) $(COMPILE_OTHER) -MM -MG -P -w -MT $@ $< > $(@:%.o=%.d)

$(RTBENCH_EXT_OBJ_DIR)/generator/realtime_orig/%.o: RTBENCH_EXTRA_FLAGS := $(RTBENCH_DONGTU_FLAGS)
$(RTBENCH_EXT_OBJ_DIR)/generator/realtime_orig/%.o: $(RTOS_BENCH_ROOT)/generator/realtime_orig/%.c
	@mkdir -p $(dir $@)
	@echo 'Building RTOS-Bench realtime file: $<'
	$(CC) $(COMPILE_SYMBOL) $(COMPILE_INCLUDE) $(RTBENCH_EXTRA_FLAGS) $(USER_OPTION) -D${ARCH} $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) $(COMPILE_WARNING) $(COMPILE_OTHER) -o $@ $< && \
	$(CC) $(COMPILE_SYMBOL) $(COMPILE_INCLUDE) $(RTBENCH_EXTRA_FLAGS) $(USER_OPTION) -D${ARCH} $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) $(COMPILE_WARNING) $(COMPILE_OTHER) -MM -MG -P -w -MT $@ $< > $(@:%.o=%.d)

$(RTBENCH_EXT_OBJ_DIR)/workloads/%.o: RTBENCH_EXTRA_FLAGS := $(RTBENCH_DONGTU_FLAGS)
$(RTBENCH_EXT_OBJ_DIR)/workloads/%.o: $(RTOS_BENCH_ROOT)/workloads/%.c
	@mkdir -p $(dir $@)
	@echo 'Building RTOS-Bench workload: $<'
	$(CC) $(COMPILE_SYMBOL) $(COMPILE_INCLUDE) $(RTBENCH_EXTRA_FLAGS) $(USER_OPTION) -D${ARCH} $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) $(COMPILE_WARNING) $(COMPILE_OTHER) -o $@ $< && \
	$(CC) $(COMPILE_SYMBOL) $(COMPILE_INCLUDE) $(RTBENCH_EXTRA_FLAGS) $(USER_OPTION) -D${ARCH} $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) $(COMPILE_WARNING) $(COMPILE_OTHER) -MM -MG -P -w -MT $@ $< > $(@:%.o=%.d)

$(RTBENCH_EXT_OBJ_DIR)/workloads/%.o: RTBENCH_EXTRA_CXX_FLAGS := $(RTBENCH_DONGTU_CXX_FLAGS)
$(RTBENCH_EXT_OBJ_DIR)/workloads/EPNP/%.o: RTBENCH_EXTRA_CXX_FLAGS := $(RTBENCH_DONGTU_EPNP_CXX_FLAGS)
$(RTBENCH_EXT_OBJ_DIR)/workloads/%.o: $(RTOS_BENCH_ROOT)/workloads/%.cpp
	@mkdir -p $(dir $@)
	@echo 'Building RTOS-Bench C++ workload: $<'
	$(RTBENCH_DONGTU_CXX) $(RTBENCH_EXTRA_CXX_FLAGS) $(COMPILE_SYMBOL) $(COMPILE_INCLUDE) $(USER_OPTION) -D${ARCH} $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) $(COMPILE_WARNING) $(COMPILE_OTHER) -o $@ $< && \
	$(RTBENCH_DONGTU_CXX) $(RTBENCH_EXTRA_CXX_FLAGS) $(COMPILE_SYMBOL) $(COMPILE_INCLUDE) $(USER_OPTION) -D${ARCH} $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) $(COMPILE_WARNING) $(COMPILE_OTHER) -MM -MG -P -w -MT $@ $< > $(@:%.o=%.d)

$(RTBENCH_EXT_OBJ_DIR)/platforms/dongtu/%.o: RTBENCH_EXTRA_FLAGS := $(RTBENCH_DONGTU_FLAGS)
$(RTBENCH_EXT_OBJ_DIR)/platforms/dongtu/%.o: $(RTOS_BENCH_ROOT)/platforms/dongtu/%.c
	@mkdir -p $(dir $@)
	@echo 'Building RTOS-Bench Dongtu platform file: $<'
	$(CC) $(COMPILE_SYMBOL) $(COMPILE_INCLUDE) $(RTBENCH_EXTRA_FLAGS) $(USER_OPTION) -D${ARCH} $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) $(COMPILE_WARNING) $(COMPILE_OTHER) -o $@ $< && \
	$(CC) $(COMPILE_SYMBOL) $(COMPILE_INCLUDE) $(RTBENCH_EXTRA_FLAGS) $(USER_OPTION) -D${ARCH} $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) $(COMPILE_WARNING) $(COMPILE_OTHER) -MM -MG -P -w -MT $@ $< > $(@:%.o=%.d)

RTBENCH_BASE_OBJS := \
	./src/userAppInit.o

RTBENCH_PRJ_OBJS := \
	$(RTBENCH_BASE_OBJS) \
	$(RTBENCH_GENERATOR_OBJS) \
	$(RTBENCH_REALTIME_OBJS) \
	$(RTBENCH_WORKLOAD_OBJS) \
	$(RTBENCH_WORKLOAD_CXX_OBJS) \
	$(RTBENCH_DONGTU_ARCHIVE_PLATFORM_OBJS)

RTBENCH_VM3588_PRJ_OBJS := $(RTBENCH_PRJ_OBJS)

$(ARCHIVES): prjObjs.lst $(RTBENCH_DONGTU_SHELL_OBJ)

.PHONY: rtosbench_prjobjs prjObjs.lst
rtosbench_prjobjs prjObjs.lst:
	@printf '%s\n' $(RTBENCH_PRJ_OBJS) > prjObjs.lst
