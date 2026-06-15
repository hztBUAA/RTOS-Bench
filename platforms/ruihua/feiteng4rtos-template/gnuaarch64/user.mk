################################################################################
# RTOS-Bench integration for the Ruihua/ReWorks Feiteng sample project.
#
# Copy this file to:
#   C:/rtos/6.1.1-ARM/workspace/feiteng4rtos/gnuaarch64/user.mk
#
# The generated FTE2000_SMP-64/subdir.mk includes $(ROOT)/user.mk.
################################################################################

RTOSBENCH_SRC := C:/Users/hzt/yihui-workspace/rtos-bench/RTOS-Bench
RTOSBENCH_PORT := ../../rtosbench_port
RTOSBENCH_OBJDIR := ./rtosbench

RTOSBENCH_CFLAGS := \
	-DRUIHUA_PLATFORM \
	-DRTOSBENCH_USE_MANUAL_WORKLOAD_REGISTRATION \
	-DRTBENCH_NO_STANDALONE_MAIN \
	-I"$(RTOSBENCH_SRC)/generator" \
	-I"$(RTOSBENCH_SRC)/generator/test_schedule" \
	-I"$(RTOSBENCH_SRC)/generator/realtime_orig/les" \
	-I"$(RTOSBENCH_SRC)/generator/realtime_orig/realtime" \
	-I"$(RTOSBENCH_SRC)/generator/realtime_orig/multicore" \
	-I"$(RTOSBENCH_SRC)/generator/realtime_orig/verify" \
	-I"$(RTOSBENCH_SRC)/generator/stress_orig" \
	-I"$(RTOSBENCH_SRC)/generator/stress_orig/common" \
	-I"$(RTOSBENCH_SRC)/generator/stress_orig/osal" \
	-I"$(RTOSBENCH_SRC)/generator/stress_orig/stressor" \
	-I"$(RTOSBENCH_SRC)/workloads" \
	-I"$(RTOSBENCH_SRC)/workloads/FAST" \
	-I"$(RTOSBENCH_SRC)/workloads/EPNP" \
	-I"$(RTOSBENCH_SRC)/workloads/EPNP/Eigen" \
	-I"$(RTOSBENCH_SRC)/workloads/EKF" \
	-I"$(RTOSBENCH_SRC)/workloads/EKF/include" \
	-I"$(RTOSBENCH_SRC)/workloads/EKF/include/matrix" \
	-I"$(RTOSBENCH_SRC)/workloads/EKF/geo" \
	-I"$(RTOSBENCH_SRC)/workloads/EKF/geo_lookup" \
	-I"$(RTOSBENCH_SRC)/workloads/ICP" \
	-I"$(RTOSBENCH_SRC)/workloads/MODBUS" \
	-I"$(RTOSBENCH_SRC)/workloads/MQTT" \
	-I"$(RTOSBENCH_SRC)/workloads/PID" \
	-I"$(RTOSBENCH_SRC)/workloads/CUSUM" \
	-I"$(RTOSBENCH_SRC)/workloads/EWMA" \
	-I"$(RTOSBENCH_PORT)"

RTOSBENCH_CXXFLAGS := \
	$(RTOSBENCH_CFLAGS) \
	-std=c++14 \
	-Wno-error \
	-Wno-literal-suffix \
	-Wno-cpp \
	-DEIGEN_DONT_VECTORIZE \
	-DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT \
	-DECL_STANDALONE \
	-D__STDC_FORMAT_MACROS \
	-D__STDC_LIMIT_MACROS \
	-D_GLIBCXX_USE_C99 \
	-D_GLIBCXX_USE_C99_MATH \
	-D_USE_MATH_DEFINES \
	-include "$(RTOSBENCH_SRC)/workloads/EPNP/fix_opengv.h"

rtosbench_port/%.o: CPPFLAGS += $(RTOSBENCH_CFLAGS)
les/%.o: CPPFLAGS += $(RTOSBENCH_CFLAGS)

RTOSBENCH_MODULE_SRCS := \
	test_realtime.c \
	test_stress.c \
	realtime_orig/multicore/ipc_bw.c \
	realtime_orig/multicore/mem_bw.c \
	realtime_orig/multicore/task_lt.c \
	realtime_orig/realtime/test1.c \
	realtime_orig/realtime/test10_1.c \
	realtime_orig/realtime/test2.c \
	realtime_orig/realtime/test3.c \
	realtime_orig/realtime/test4_1.c \
	realtime_orig/realtime/test4_2.c \
	realtime_orig/realtime/test5_3.c \
	realtime_orig/realtime/test6_0.c \
	realtime_orig/realtime/test6_1.c \
	realtime_orig/realtime/test6_2.c \
	realtime_orig/realtime/test6_3.c \
	realtime_orig/realtime/test6_4.c \
	realtime_orig/realtime/test7_3.c \
	realtime_orig/realtime/test8_1.c \
	realtime_orig/realtime/test8_2.c \
	realtime_orig/realtime/test9_3.c \
	realtime_orig/verify/all_realtime_verify.c \
	realtime_orig/verify/cpu_bind_verify.c \
	realtime_orig/verify/freq_verify.c \
	realtime_orig/verify/interrupt_stub_verify.c \
	realtime_orig/verify/schedule_stub_verify.c \
	realtime_orig/verify/time_verify.c \
	stress_orig/common/core-helper.c \
	stress_orig/common/stress-ng.c \
	stress_orig/common/stress_stored_job.c \
	stress_orig/osal/os_rede.c \
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

RTOSBENCH_MODULE_OBJS := $(addprefix $(RTOSBENCH_OBJDIR)/,$(RTOSBENCH_MODULE_SRCS:.c=.o))

RTOSBENCH_WORKLOAD_C_SRCS := \
	FAST/fast_bench.c \
	FAST/fast.c \
	FAST/fast_9.c \
	CUSUM/cusum_bench.c \
	EWMA/ewma_bench.c \
	MODBUS/modbus_bench.c \
	MODBUS/nanomodbus.c \
	MQTT/mqtt_bench.c \
	MQTT/mongoose.c

RTOSBENCH_WORKLOAD_CPP_SRCS := \
	PID/pid_bench.cpp \
	PID/PID_v1.cpp \
	PID/pid_wrapper.cpp \
	ICP/icp_bench.cpp \
	ICP/matrix.cpp \
	ICP/icp.cpp \
	ICP/icpPointToPlane.cpp \
	ICP/icpPointToPoint.cpp \
	ICP/kdtree.cpp \
	EPNP/epnp_bench.cpp \
	EPNP/methods.cpp \
	EPNP/Epnp.cpp \
	EPNP/random_generators.cpp \
	EPNP/time_measurement.cpp \
	EPNP/experiment_helpers.cpp \
	EPNP/cayley.cpp \
	EPNP/CentralAbsoluteAdapter.cpp \
	EKF/ekf_bench.cpp \
	EKF/geo/geo.cpp \
	EKF/geo_lookup/geo_mag_declination.cpp \
	EKF/EKF_core/vel_pos_fusion.cpp \
	EKF/EKF_core/utils.cpp \
	EKF/EKF_core/sideslip_fusion.cpp \
	EKF/EKF_core/terrain_estimator.cpp \
	EKF/EKF_core/optflow_fusion.cpp \
	EKF/EKF_core/sensor_range_finder.cpp \
	EKF/EKF_core/mag_control.cpp \
	EKF/EKF_core/mag_fusion.cpp \
	EKF/EKF_core/gps_checks.cpp \
	EKF/EKF_core/gps_yaw_fusion.cpp \
	EKF/EKF_core/imu_down_sampler.cpp \
	EKF/EKF_core/estimator_interface.cpp \
	EKF/EKF_core/ekf_helper.cpp \
	EKF/EKF_core/covariance.cpp \
	EKF/EKF_core/drag_fusion.cpp \
	EKF/EKF_core/ekf.cpp \
	EKF/EKF_core/control.cpp \
	EKF/EKF_core/airspeed_fusion.cpp \
	EKF/EKF_core/EKFGSF_yaw.cpp

RTOSBENCH_WORKLOAD_OBJS := \
	$(addprefix $(RTOSBENCH_OBJDIR)/workloads/,$(RTOSBENCH_WORKLOAD_C_SRCS:.c=.o)) \
	$(addprefix $(RTOSBENCH_OBJDIR)/workloads/,$(RTOSBENCH_WORKLOAD_CPP_SRCS:.cpp=.o))

RTOSBENCH_OBJS += \
	$(RTOSBENCH_OBJDIR)/ruihua_entry.o \
	$(RTOSBENCH_OBJDIR)/rtbench_command.o \
	$(RTOSBENCH_OBJDIR)/periodic_benchmark.o \
	$(RTOSBENCH_OBJDIR)/logging.o \
	$(RTOSBENCH_OBJDIR)/memory_watcher.o \
	$(RTOSBENCH_OBJDIR)/uunifast.o \
	$(RTOSBENCH_OBJDIR)/test_schedule.o \
	$(RTOSBENCH_OBJDIR)/test_cmd.o \
	$(RTOSBENCH_OBJDIR)/result_export.o \
	$(RTOSBENCH_OBJDIR)/workload_registry.o \
	$(RTOSBENCH_OBJDIR)/workload_stub.o \
	$(RTOSBENCH_OBJDIR)/workload_busywait.o \
	$(RTOSBENCH_OBJDIR)/ruihua_timer.o \
	$(RTOSBENCH_OBJDIR)/ruihua_sync.o \
	$(RTOSBENCH_OBJDIR)/ruihua_timestamp.o \
	$(RTOSBENCH_OBJDIR)/ruihua_scheduler.o \
	$(RTOSBENCH_OBJDIR)/ruihua_signal.o \
	$(RTOSBENCH_MODULE_OBJS) \
	$(RTOSBENCH_WORKLOAD_OBJS)

OBJS += $(RTOSBENCH_OBJS)
C_DEPS += $(RTOSBENCH_OBJS:.o=.d)

$(RTOSBENCH_OBJDIR):
	@mkdir -p "$@"

$(RTOSBENCH_OBJDIR)/%.o: $(RTOSBENCH_SRC)/generator/%.c | $(RTOSBENCH_OBJDIR)
	@mkdir -p "$(dir $@)"
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/workloads/%.o: $(RTOSBENCH_SRC)/workloads/%.c | $(RTOSBENCH_OBJDIR)
	@mkdir -p "$(dir $@)"
	@echo_rede 'Building workload: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/workloads/%.o: $(RTOSBENCH_SRC)/workloads/%.cpp | $(RTOSBENCH_OBJDIR)
	@mkdir -p "$(dir $@)"
	@echo_rede 'Building C++ workload: $<'
	aarch64-elf-g++ -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CXXFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/ruihua_entry.o: $(RTOSBENCH_SRC)/generator/ruihua_entry.c | $(RTOSBENCH_OBJDIR)
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/rtbench_command.o: $(RTOSBENCH_SRC)/generator/rtbench_command.c | $(RTOSBENCH_OBJDIR)
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/periodic_benchmark.o: $(RTOSBENCH_SRC)/generator/periodic_benchmark.c | $(RTOSBENCH_OBJDIR)
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/logging.o: $(RTOSBENCH_SRC)/generator/logging.c | $(RTOSBENCH_OBJDIR)
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/memory_watcher.o: $(RTOSBENCH_SRC)/generator/memory_watcher.c | $(RTOSBENCH_OBJDIR)
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/uunifast.o: $(RTOSBENCH_SRC)/generator/uunifast.c | $(RTOSBENCH_OBJDIR)
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/test_schedule.o: $(RTOSBENCH_SRC)/generator/test_schedule.c | $(RTOSBENCH_OBJDIR)
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/test_cmd.o: $(RTOSBENCH_SRC)/generator/test_cmd.c | $(RTOSBENCH_OBJDIR)
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/result_export.o: $(RTOSBENCH_SRC)/generator/result_export.c | $(RTOSBENCH_OBJDIR)
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/workload_registry.o: $(RTOSBENCH_SRC)/generator/workload_registry.c | $(RTOSBENCH_OBJDIR)
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/workload_stub.o: $(RTOSBENCH_SRC)/generator/workload_stub.c | $(RTOSBENCH_OBJDIR)
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/workload_busywait.o: $(RTOSBENCH_SRC)/generator/workload_busywait.c | $(RTOSBENCH_OBJDIR)
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/ruihua_timer.o: $(RTOSBENCH_SRC)/generator/platform/ruihua/timer.c | $(RTOSBENCH_OBJDIR)
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/ruihua_sync.o: $(RTOSBENCH_SRC)/generator/platform/ruihua/sync.c | $(RTOSBENCH_OBJDIR)
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/ruihua_timestamp.o: $(RTOSBENCH_SRC)/generator/platform/ruihua/timestamp.c | $(RTOSBENCH_OBJDIR)
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/ruihua_scheduler.o: $(RTOSBENCH_SRC)/generator/platform/ruihua/scheduler.c | $(RTOSBENCH_OBJDIR)
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/ruihua_signal.o: $(RTOSBENCH_SRC)/generator/platform/ruihua/signal.c | $(RTOSBENCH_OBJDIR)
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/sched_workloads_stub.o: $(RTOSBENCH_PORT)/sched_workloads_stub.c | $(RTOSBENCH_OBJDIR)
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/ruihua_workloads.o: $(RTOSBENCH_PORT)/ruihua_workloads.c | $(RTOSBENCH_OBJDIR)
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"

$(RTOSBENCH_OBJDIR)/rtosbench_boot.o: $(RTOSBENCH_PORT)/rtosbench_boot.c | $(RTOSBENCH_OBJDIR)
	@echo_rede 'Building file: $<'
	aarch64-elf-gcc -O0 -gdwarf-2 -Wall -c -fmessage-length=0 $(CONFIG_COMPLILE_FLAGS) $(CPPFLAGS) $(RTOSBENCH_CFLAGS) -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$@" -o"$@" "$<"
