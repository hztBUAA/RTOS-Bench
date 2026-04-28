################################################################################
# RTOS-Bench integration for Dongtu/Intewell vm_3588 projects.
#
# Include this from the vm_3588 project-local config_os.mk:
#   RTOS_BENCH_ROOT ?= C:/path/to/RTOS-Bench
#   include $(RTOS_BENCH_ROOT)/platforms/dongtu/intewell_vm3588.mk
#
# The vm_3588 project keeps only board/project files. RTOS-Bench sources remain
# under RTOS_BENCH_ROOT as the single source of truth.
################################################################################

RTBENCH_DONGTU_MK_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
RTOS_BENCH_ROOT ?= $(abspath $(RTBENCH_DONGTU_MK_DIR)/../..)
RTBENCH_EXT_OBJ_DIR ?= ./rtosbench_ext
ARCH ?= __ARM64__

RTBENCH_GENERATOR_SRCS := \
	dongtu_entry.c \
	dongtu_workloads.c \
	logging.c \
	memory_watcher.c \
	periodic_benchmark.c \
	result_export.c \
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

RTBENCH_DONGTU_PLATFORM_SRCS := \
	compat.c \
	shell.c

RTBENCH_GENERATOR_OBJS := $(addprefix $(RTBENCH_EXT_OBJ_DIR)/generator/,$(RTBENCH_GENERATOR_SRCS:.c=.o))
RTBENCH_WORKLOAD_OBJS := $(addprefix $(RTBENCH_EXT_OBJ_DIR)/workloads/,$(RTBENCH_WORKLOAD_SRCS:.c=.o))
RTBENCH_DONGTU_PLATFORM_OBJS := $(addprefix $(RTBENCH_EXT_OBJ_DIR)/platforms/dongtu/,$(RTBENCH_DONGTU_PLATFORM_SRCS:.c=.o))
RTBENCH_GENERATOR_DEPS := $(RTBENCH_GENERATOR_OBJS:.o=.d)
RTBENCH_WORKLOAD_DEPS := $(RTBENCH_WORKLOAD_OBJS:.o=.d)
RTBENCH_DONGTU_PLATFORM_DEPS := $(RTBENCH_DONGTU_PLATFORM_OBJS:.o=.d)
RTBENCH_DONGTU_SHELL_OBJ := $(RTBENCH_EXT_OBJ_DIR)/platforms/dongtu/shell.o
RTBENCH_DONGTU_ARCHIVE_PLATFORM_OBJS := $(filter-out $(RTBENCH_DONGTU_SHELL_OBJ),$(RTBENCH_DONGTU_PLATFORM_OBJS))

C_SRCS += $(addprefix $(RTOS_BENCH_ROOT)/generator/,$(RTBENCH_GENERATOR_SRCS))
C_SRCS += $(addprefix $(RTOS_BENCH_ROOT)/workloads/,$(RTBENCH_WORKLOAD_SRCS))
C_SRCS += $(addprefix $(RTOS_BENCH_ROOT)/platforms/dongtu/,$(RTBENCH_DONGTU_PLATFORM_SRCS))
OBJS += $(RTBENCH_GENERATOR_OBJS) $(RTBENCH_WORKLOAD_OBJS) $(RTBENCH_DONGTU_PLATFORM_OBJS)
DEPS += $(RTBENCH_GENERATOR_DEPS) $(RTBENCH_WORKLOAD_DEPS) $(RTBENCH_DONGTU_PLATFORM_DEPS)
USER_OBJS += $(RTBENCH_DONGTU_SHELL_OBJ)

RTBENCH_DONGTU_FLAGS := \
	-DDONGTU_PLATFORM \
	-DMULTI_WORKLOAD \
	-Dalloca=__builtin_alloca \
	-I$(RTOS_BENCH_ROOT)/generator \
	-I$(RTOS_BENCH_ROOT)/generator/platform/dongtu \
	-I$(RTOS_BENCH_ROOT)/generator/test_schedule \
	-I$(RTOS_BENCH_ROOT)/generator/stress_orig \
	-I$(RTOS_BENCH_ROOT)/generator/stress_orig/common \
	-I$(RTOS_BENCH_ROOT)/generator/stress_orig/osal \
	-I$(RTOS_BENCH_ROOT)/generator/stress_orig/stressor \
	-I$(RTOS_BENCH_ROOT)/workloads \
	-I$(RTOS_BENCH_ROOT)/workloads/CUSUM \
	-I$(RTOS_BENCH_ROOT)/workloads/EWMA \
	-I$(RTOS_BENCH_ROOT)/workloads/FAST \
	-I$(RTOS_BENCH_ROOT)/workloads/MODBUS \
	-I$(RTOS_BENCH_ROOT)/workloads/MQTT

$(RTBENCH_EXT_OBJ_DIR)/generator/%.o: RTBENCH_EXTRA_FLAGS := $(RTBENCH_DONGTU_FLAGS)
$(RTBENCH_EXT_OBJ_DIR)/generator/%.o: $(RTOS_BENCH_ROOT)/generator/%.c
	@mkdir -p $(dir $@)
	@echo 'Building RTOS-Bench file: $<'
	$(CC) $(COMPILE_SYMBOL) $(COMPILE_INCLUDE) $(RTBENCH_EXTRA_FLAGS) $(USER_OPTION) -D${ARCH} $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) $(COMPILE_WARNING) $(COMPILE_OTHER) -o $@ $< && \
	$(CC) $(COMPILE_SYMBOL) $(COMPILE_INCLUDE) $(RTBENCH_EXTRA_FLAGS) $(USER_OPTION) -D${ARCH} $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) $(COMPILE_WARNING) $(COMPILE_OTHER) -MM -MG -P -w -MT $@ $< > $(@:%.o=%.d)

$(RTBENCH_EXT_OBJ_DIR)/workloads/%.o: RTBENCH_EXTRA_FLAGS := $(RTBENCH_DONGTU_FLAGS)
$(RTBENCH_EXT_OBJ_DIR)/workloads/%.o: $(RTOS_BENCH_ROOT)/workloads/%.c
	@mkdir -p $(dir $@)
	@echo 'Building RTOS-Bench workload: $<'
	$(CC) $(COMPILE_SYMBOL) $(COMPILE_INCLUDE) $(RTBENCH_EXTRA_FLAGS) $(USER_OPTION) -D${ARCH} $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) $(COMPILE_WARNING) $(COMPILE_OTHER) -o $@ $< && \
	$(CC) $(COMPILE_SYMBOL) $(COMPILE_INCLUDE) $(RTBENCH_EXTRA_FLAGS) $(USER_OPTION) -D${ARCH} $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) $(COMPILE_WARNING) $(COMPILE_OTHER) -MM -MG -P -w -MT $@ $< > $(@:%.o=%.d)

$(RTBENCH_EXT_OBJ_DIR)/platforms/dongtu/%.o: RTBENCH_EXTRA_FLAGS := $(RTBENCH_DONGTU_FLAGS)
$(RTBENCH_EXT_OBJ_DIR)/platforms/dongtu/%.o: $(RTOS_BENCH_ROOT)/platforms/dongtu/%.c
	@mkdir -p $(dir $@)
	@echo 'Building RTOS-Bench Dongtu platform file: $<'
	$(CC) $(COMPILE_SYMBOL) $(COMPILE_INCLUDE) $(RTBENCH_EXTRA_FLAGS) $(USER_OPTION) -D${ARCH} $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) $(COMPILE_WARNING) $(COMPILE_OTHER) -o $@ $< && \
	$(CC) $(COMPILE_SYMBOL) $(COMPILE_INCLUDE) $(RTBENCH_EXTRA_FLAGS) $(USER_OPTION) -D${ARCH} $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) $(COMPILE_WARNING) $(COMPILE_OTHER) -MM -MG -P -w -MT $@ $< > $(@:%.o=%.d)

RTBENCH_VM3588_BASE_OBJS := \
	./src/les/bench_init.o \
	./src/les/cpu_affinity.o \
	./src/les/data_tools.o \
	./src/les/intewell_init.o \
	./src/les/les.o \
	./src/les/reworks_int.o \
	./src/les/safe_sleep.o \
	./src/multicore/ipc_bw.o \
	./src/multicore/mem_bw.o \
	./src/multicore/task_lt.o \
	./src/realtime/test1.o \
	./src/realtime/test10_1.o \
	./src/realtime/test2.o \
	./src/realtime/test3.o \
	./src/realtime/test4_1.o \
	./src/realtime/test4_2.o \
	./src/realtime/test5_3.o \
	./src/realtime/test6_0.o \
	./src/realtime/test6_1.o \
	./src/realtime/test6_2.o \
	./src/realtime/test6_3.o \
	./src/realtime/test6_4.o \
	./src/realtime/test7_3.o \
	./src/realtime/test8_1.o \
	./src/realtime/test8_2.o \
	./src/realtime/test9_3.o \
	./src/userAppInit.o \
	./src/verify/all_realtime_verify.o \
	./src/verify/cpu_bind_verify.o \
	./src/verify/freq_verify.o \
	./src/verify/interrupt_stub_verify.o \
	./src/verify/schedule_stub_verify.o \
	./src/verify/time_verify.o

RTBENCH_VM3588_PRJ_OBJS := $(RTBENCH_VM3588_BASE_OBJS) $(RTBENCH_GENERATOR_OBJS) $(RTBENCH_WORKLOAD_OBJS) $(RTBENCH_DONGTU_ARCHIVE_PLATFORM_OBJS)

$(ARCHIVES): prjObjs.lst

.PHONY: rtosbench_prjobjs prjObjs.lst
rtosbench_prjobjs prjObjs.lst:
	@printf '%s\n' $(RTBENCH_VM3588_PRJ_OBJS) > prjObjs.lst
