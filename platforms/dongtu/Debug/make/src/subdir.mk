OTHER_OPTION :=
ARCH = __X86__

C_SRCS += \
../../src/userAppInit.c \
../../RTOS-Bench/generator/test_schedule.c \
../../RTOS-Bench/generator/test_realtime.c \
../../RTOS-Bench/generator/test_stress.c \
../../RTOS-Bench/generator/test_cmd.c \
../../RTOS-Bench/generator/result_export.c \
../../RTOS-Bench/generator/uunifast.c \
../../RTOS-Bench/generator/stress_orig/common/stress-ng.c \
../../RTOS-Bench/generator/stress_orig/common/core-helper.c \
../../RTOS-Bench/generator/stress_orig/common/stress_stored_job.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-atomic.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-bitops.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-bsearch.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-context.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-copy-file.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-cpu.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-dentry.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-fp.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-fstat.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-hdd.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-malloc.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-matrix.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-memcpy.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-memthrash.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-open.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-pipe.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-prime.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-ptr-chase.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-qsort.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-rename.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-stack.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-str.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-stream.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-trig.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-unlink.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-vecmath.c \
../../RTOS-Bench/generator/stress_orig/stressor/stress-vm.c \
../../RTOS-Bench/generator/realtime_orig/les/bench_init.c \
../../RTOS-Bench/generator/realtime_orig/les/cpu_affinity.c \
../../RTOS-Bench/generator/realtime_orig/les/data_tools.c \
../../RTOS-Bench/generator/realtime_orig/les/les.c \
../../RTOS-Bench/generator/realtime_orig/les/reworks_int.c \
../../RTOS-Bench/generator/realtime_orig/les/safe_sleep.c \
../../RTOS-Bench/generator/realtime_orig/multicore/ipc_bw.c \
../../RTOS-Bench/generator/realtime_orig/multicore/mem_bw.c \
../../RTOS-Bench/generator/realtime_orig/multicore/task_lt.c \
../../RTOS-Bench/generator/realtime_orig/realtime/test1.c \
../../RTOS-Bench/generator/realtime_orig/realtime/test10_1.c \
../../RTOS-Bench/generator/realtime_orig/realtime/test2.c \
../../RTOS-Bench/generator/realtime_orig/realtime/test3.c \
../../RTOS-Bench/generator/realtime_orig/realtime/test4_1.c \
../../RTOS-Bench/generator/realtime_orig/realtime/test4_2.c \
../../RTOS-Bench/generator/realtime_orig/realtime/test5_3.c \
../../RTOS-Bench/generator/realtime_orig/realtime/test6_0.c \
../../RTOS-Bench/generator/realtime_orig/realtime/test6_1.c \
../../RTOS-Bench/generator/realtime_orig/realtime/test6_2.c \
../../RTOS-Bench/generator/realtime_orig/realtime/test6_3.c \
../../RTOS-Bench/generator/realtime_orig/realtime/test6_4.c \
../../RTOS-Bench/generator/realtime_orig/realtime/test7_3.c \
../../RTOS-Bench/generator/realtime_orig/realtime/test8_1.c \
../../RTOS-Bench/generator/realtime_orig/realtime/test8_2.c \
../../RTOS-Bench/generator/realtime_orig/realtime/test9_3.c \
../../RTOS-Bench/generator/realtime_orig/verify/all_realtime_verify.c \
../../RTOS-Bench/generator/realtime_orig/verify/cpu_bind_verify.c \
../../RTOS-Bench/generator/realtime_orig/verify/freq_verify.c \
../../RTOS-Bench/generator/realtime_orig/verify/interrupt_stub_verify.c \
../../RTOS-Bench/generator/realtime_orig/verify/schedule_stub_verify.c \
../../RTOS-Bench/generator/realtime_orig/verify/time_verify.c \
../../RTOS-Bench/generator/periodic_benchmark.c \
../../RTOS-Bench/generator/workload_registry.c \
../../RTOS-Bench/generator/workload_stub.c \
../../RTOS-Bench/generator/workload_busywait.c \
../../RTOS-Bench/generator/logging.c \
../../RTOS-Bench/generator/memory_watcher.c \
../../RTOS-Bench/workloads/CUSUM/cusum_bench.c \
../../RTOS-Bench/workloads/EWMA/ewma_bench.c \
../../RTOS-Bench/workloads/FAST/fast.c \
../../RTOS-Bench/workloads/FAST/fast_9.c \
../../RTOS-Bench/workloads/FAST/fast_bench.c \
../../RTOS-Bench/workloads/MODBUS/modbus_bench.c \
../../RTOS-Bench/workloads/MODBUS/nanomodbus.c \
../../RTOS-Bench/workloads/MQTT/mongoose.c \
../../RTOS-Bench/workloads/MQTT/mqtt_bench.c \
../../RTOS-Bench/generator/stress_orig/osal/os_intewell.c \
../../RTOS-Bench/generator/platform/dongtu/timer.c \
../../RTOS-Bench/generator/platform/dongtu/sync.c \
../../RTOS-Bench/generator/platform/dongtu/scheduler.c \
../../RTOS-Bench/generator/platform/dongtu/timestamp.c \
../../RTOS-Bench/generator/platform/dongtu/signal.c \
../../RTOS-Bench/generator/dongtu_entry.c

CPP_SRCS += \
../../RTOS-Bench/workloads/rtbench_workloads.cpp \
../../RTOS-Bench/workloads/PID/PID_v1.cpp \
../../RTOS-Bench/workloads/PID/pid_bench.cpp \
../../RTOS-Bench/workloads/PID/pid_wrapper.cpp \
../../RTOS-Bench/workloads/EKF/ekf_bench.cpp \
../../RTOS-Bench/workloads/EKF/EKF_core/ekf.cpp \
../../RTOS-Bench/workloads/EKF/EKF_core/ekf_helper.cpp \
../../RTOS-Bench/workloads/EKF/EKF_core/estimator_interface.cpp \
../../RTOS-Bench/workloads/EKF/EKF_core/EKFGSF_yaw.cpp \
../../RTOS-Bench/workloads/EKF/EKF_core/airspeed_fusion.cpp \
../../RTOS-Bench/workloads/EKF/EKF_core/control.cpp \
../../RTOS-Bench/workloads/EKF/EKF_core/covariance.cpp \
../../RTOS-Bench/workloads/EKF/EKF_core/drag_fusion.cpp \
../../RTOS-Bench/workloads/EKF/EKF_core/gps_checks.cpp \
../../RTOS-Bench/workloads/EKF/EKF_core/gps_yaw_fusion.cpp \
../../RTOS-Bench/workloads/EKF/EKF_core/imu_down_sampler.cpp \
../../RTOS-Bench/workloads/EKF/EKF_core/mag_control.cpp \
../../RTOS-Bench/workloads/EKF/EKF_core/mag_fusion.cpp \
../../RTOS-Bench/workloads/EKF/EKF_core/optflow_fusion.cpp \
../../RTOS-Bench/workloads/EKF/EKF_core/sensor_range_finder.cpp \
../../RTOS-Bench/workloads/EKF/EKF_core/sideslip_fusion.cpp \
../../RTOS-Bench/workloads/EKF/EKF_core/terrain_estimator.cpp \
../../RTOS-Bench/workloads/EKF/EKF_core/utils.cpp \
../../RTOS-Bench/workloads/EKF/EKF_core/vel_pos_fusion.cpp \
../../RTOS-Bench/workloads/EKF/geo/geo.cpp \
../../RTOS-Bench/workloads/EKF/geo_lookup/geo_mag_declination.cpp \
../../RTOS-Bench/workloads/EPNP/cayley.cpp \
../../RTOS-Bench/workloads/EPNP/CentralAbsoluteAdapter.cpp \
../../RTOS-Bench/workloads/EPNP/Epnp.cpp \
../../RTOS-Bench/workloads/EPNP/epnp_bench.cpp \
../../RTOS-Bench/workloads/EPNP/experiment_helpers.cpp \
../../RTOS-Bench/workloads/EPNP/methods.cpp \
../../RTOS-Bench/workloads/EPNP/random_generators.cpp \
../../RTOS-Bench/workloads/EPNP/time_measurement.cpp \
../../RTOS-Bench/workloads/ICP/icp.cpp \
../../RTOS-Bench/workloads/ICP/icp_bench.cpp \
../../RTOS-Bench/workloads/ICP/icpPointToPlane.cpp \
../../RTOS-Bench/workloads/ICP/icpPointToPoint.cpp \
../../RTOS-Bench/workloads/ICP/kdtree.cpp \
../../RTOS-Bench/workloads/ICP/matrix.cpp

OBJS += \
./src/userAppInit.o \
./RTOS-Bench/generator/test_schedule.o \
./RTOS-Bench/generator/test_realtime.o \
./RTOS-Bench/generator/test_stress.o \
./RTOS-Bench/generator/test_cmd.o \
./RTOS-Bench/generator/result_export.o \
./RTOS-Bench/generator/uunifast.o \
./RTOS-Bench/generator/stress_orig/common/stress-ng.o \
./RTOS-Bench/generator/stress_orig/common/core-helper.o \
./RTOS-Bench/generator/stress_orig/common/stress_stored_job.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-atomic.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-bitops.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-bsearch.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-context.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-copy-file.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-cpu.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-dentry.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-fp.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-fstat.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-hdd.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-malloc.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-matrix.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-memcpy.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-memthrash.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-open.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-pipe.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-prime.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-ptr-chase.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-qsort.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-rename.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-stack.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-str.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-stream.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-trig.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-unlink.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-vecmath.o \
./RTOS-Bench/generator/stress_orig/stressor/stress-vm.o \
./RTOS-Bench/generator/realtime_orig/les/bench_init.o \
./RTOS-Bench/generator/realtime_orig/les/cpu_affinity.o \
./RTOS-Bench/generator/realtime_orig/les/data_tools.o \
./RTOS-Bench/generator/realtime_orig/les/les.o \
./RTOS-Bench/generator/realtime_orig/les/reworks_int.o \
./RTOS-Bench/generator/realtime_orig/les/safe_sleep.o \
./RTOS-Bench/generator/realtime_orig/multicore/ipc_bw.o \
./RTOS-Bench/generator/realtime_orig/multicore/mem_bw.o \
./RTOS-Bench/generator/realtime_orig/multicore/task_lt.o \
./RTOS-Bench/generator/realtime_orig/realtime/test1.o \
./RTOS-Bench/generator/realtime_orig/realtime/test10_1.o \
./RTOS-Bench/generator/realtime_orig/realtime/test2.o \
./RTOS-Bench/generator/realtime_orig/realtime/test3.o \
./RTOS-Bench/generator/realtime_orig/realtime/test4_1.o \
./RTOS-Bench/generator/realtime_orig/realtime/test4_2.o \
./RTOS-Bench/generator/realtime_orig/realtime/test5_3.o \
./RTOS-Bench/generator/realtime_orig/realtime/test6_0.o \
./RTOS-Bench/generator/realtime_orig/realtime/test6_1.o \
./RTOS-Bench/generator/realtime_orig/realtime/test6_2.o \
./RTOS-Bench/generator/realtime_orig/realtime/test6_3.o \
./RTOS-Bench/generator/realtime_orig/realtime/test6_4.o \
./RTOS-Bench/generator/realtime_orig/realtime/test7_3.o \
./RTOS-Bench/generator/realtime_orig/realtime/test8_1.o \
./RTOS-Bench/generator/realtime_orig/realtime/test8_2.o \
./RTOS-Bench/generator/realtime_orig/realtime/test9_3.o \
./RTOS-Bench/generator/realtime_orig/verify/all_realtime_verify.o \
./RTOS-Bench/generator/realtime_orig/verify/cpu_bind_verify.o \
./RTOS-Bench/generator/realtime_orig/verify/freq_verify.o \
./RTOS-Bench/generator/realtime_orig/verify/interrupt_stub_verify.o \
./RTOS-Bench/generator/realtime_orig/verify/schedule_stub_verify.o \
./RTOS-Bench/generator/realtime_orig/verify/time_verify.o \
./RTOS-Bench/generator/periodic_benchmark.o \
./RTOS-Bench/generator/workload_registry.o \
./RTOS-Bench/generator/workload_stub.o \
./RTOS-Bench/generator/workload_busywait.o \
./RTOS-Bench/generator/logging.o \
./RTOS-Bench/generator/memory_watcher.o \
./RTOS-Bench/workloads/CUSUM/cusum_bench.o \
./RTOS-Bench/workloads/EWMA/ewma_bench.o \
./RTOS-Bench/workloads/FAST/fast.o \
./RTOS-Bench/workloads/FAST/fast_9.o \
./RTOS-Bench/workloads/FAST/fast_bench.o \
./RTOS-Bench/workloads/MODBUS/modbus_bench.o \
./RTOS-Bench/workloads/MODBUS/nanomodbus.o \
./RTOS-Bench/workloads/MQTT/mongoose.o \
./RTOS-Bench/workloads/MQTT/mqtt_bench.o \
./RTOS-Bench/generator/stress_orig/osal/os_intewell.o \
./RTOS-Bench/generator/platform/dongtu/timer.o \
./RTOS-Bench/generator/platform/dongtu/sync.o \
./RTOS-Bench/generator/platform/dongtu/scheduler.o \
./RTOS-Bench/generator/platform/dongtu/timestamp.o \
./RTOS-Bench/generator/platform/dongtu/signal.o \
./RTOS-Bench/generator/dongtu_entry.o \
./RTOS-Bench/workloads/rtbench_workloads.o \
./RTOS-Bench/workloads/PID/PID_v1.o \
./RTOS-Bench/workloads/PID/pid_bench.o \
./RTOS-Bench/workloads/PID/pid_wrapper.o \
./RTOS-Bench/workloads/EKF/ekf_bench.o \
./RTOS-Bench/workloads/EKF/EKF_core/ekf.o \
./RTOS-Bench/workloads/EKF/EKF_core/ekf_helper.o \
./RTOS-Bench/workloads/EKF/EKF_core/estimator_interface.o \
./RTOS-Bench/workloads/EKF/EKF_core/EKFGSF_yaw.o \
./RTOS-Bench/workloads/EKF/EKF_core/airspeed_fusion.o \
./RTOS-Bench/workloads/EKF/EKF_core/control.o \
./RTOS-Bench/workloads/EKF/EKF_core/covariance.o \
./RTOS-Bench/workloads/EKF/EKF_core/drag_fusion.o \
./RTOS-Bench/workloads/EKF/EKF_core/gps_checks.o \
./RTOS-Bench/workloads/EKF/EKF_core/gps_yaw_fusion.o \
./RTOS-Bench/workloads/EKF/EKF_core/imu_down_sampler.o \
./RTOS-Bench/workloads/EKF/EKF_core/mag_control.o \
./RTOS-Bench/workloads/EKF/EKF_core/mag_fusion.o \
./RTOS-Bench/workloads/EKF/EKF_core/optflow_fusion.o \
./RTOS-Bench/workloads/EKF/EKF_core/sensor_range_finder.o \
./RTOS-Bench/workloads/EKF/EKF_core/sideslip_fusion.o \
./RTOS-Bench/workloads/EKF/EKF_core/terrain_estimator.o \
./RTOS-Bench/workloads/EKF/EKF_core/utils.o \
./RTOS-Bench/workloads/EKF/EKF_core/vel_pos_fusion.o \
./RTOS-Bench/workloads/EKF/geo/geo.o \
./RTOS-Bench/workloads/EKF/geo_lookup/geo_mag_declination.o \
./RTOS-Bench/workloads/EPNP/cayley.o \
./RTOS-Bench/workloads/EPNP/CentralAbsoluteAdapter.o \
./RTOS-Bench/workloads/EPNP/Epnp.o \
./RTOS-Bench/workloads/EPNP/epnp_bench.o \
./RTOS-Bench/workloads/EPNP/experiment_helpers.o \
./RTOS-Bench/workloads/EPNP/methods.o \
./RTOS-Bench/workloads/EPNP/random_generators.o \
./RTOS-Bench/workloads/EPNP/time_measurement.o \
./RTOS-Bench/workloads/ICP/icp.o \
./RTOS-Bench/workloads/ICP/icp_bench.o \
./RTOS-Bench/workloads/ICP/icpPointToPlane.o \
./RTOS-Bench/workloads/ICP/icpPointToPoint.o \
./RTOS-Bench/workloads/ICP/kdtree.o \
./RTOS-Bench/workloads/ICP/matrix.o

DEPS += $(OBJS:%.o=%.d)

BENCH_DSYMBOL := \
-D_GNU_SOURCE \
-DECL_STANDALONE \
-D__STDC_FORMAT_MACROS \
-D__STDC_LIMIT_MACROS \
-D_USE_MATH_DEFINES \
-DEIGEN_DONT_VECTORIZE \
-DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT \
-DFIX_OPENGV_USE_STUB_LDOUBLE

BENCH_INC := \
-I"../../src" \
-I"../../src/osal" \
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

CXX_EXTRA_FLAGS := -std=c++14 -Wno-error -Wno-literal-suffix

TARGET_CPU_FLAGS := \
    -march=i386 -m32 \
    -D_X86_ -D_X86_32_ -D_I386_ -D__X86__ \
    -DCONFIG_CORE_SMP -DCONFIG_OS_LP32 -DCONFIG_VM_R_GLOBAL_INT_EN_MASK \
    -fshort-wchar \
    -DCONFIG_TTOS_SMP=1 -DCONFIG_ACCESS_VAR_MACRO_SMP=1 \
    -DCPU_BIT=32 -DINTEWELL \
    -D_LITTLE_ENDIAN_ -mhard-float -D_HARD_FLOAT_ \
    -DBOARD_newpre3101_i7

EPNP_EXTRA_FLAGS := -include ../../RTOS-Bench/workloads/EPNP/fix_opengv.h


RTOS-Bench/%.o: ../../RTOS-Bench/%.c
	@mkdir -p $(dir $@)
	@echo '正在构建文件： $<'
	$(CC) $(COMPILE_SYMBOL) $(BENCH_DSYMBOL) \
	      $(COMPILE_INCLUDE) $(BENCH_INC) \
	      $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) \
	      $(COMPILE_WARNING) $(COMPILE_OTHER) \
	      ${OTHER_OPTION} -D${ARCH} \
	      -MMD -MP -MF $(@:%.o=%.d) -MT ./$@ \
	      -o $@ $<
	@echo '已结束构建： $<'
	@echo ' '

RTOS-Bench/workloads/EPNP/%.o: ../../RTOS-Bench/workloads/EPNP/%.cpp
	@mkdir -p $(dir $@)
	@echo '正在构建文件（C++/EPNP）： $<'
	$(CXX) $(COMPILE_SYMBOL) $(BENCH_DSYMBOL) \
	       $(COMPILE_INCLUDE) $(BENCH_INC) \
	       $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) \
	       $(COMPILE_WARNING) $(COMPILE_OTHER) \
	       $(CXX_EXTRA_FLAGS) $(EPNP_EXTRA_FLAGS) \
	       ${OTHER_OPTION} $(TARGET_CPU_FLAGS) -D${ARCH} \
	       -MMD -MP -MF $(@:%.o=%.d) -MT ./$@ \
	       -o $@ $<
	@echo '已结束构建（C++/EPNP）： $<'
	@echo ' '

RTOS-Bench/%.o: ../../RTOS-Bench/%.cpp
	@mkdir -p $(dir $@)
	@echo '正在构建文件（C++）： $<'
	$(CXX) $(COMPILE_SYMBOL) $(BENCH_DSYMBOL) \
	       $(COMPILE_INCLUDE) $(BENCH_INC) \
	       $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) \
	       $(COMPILE_WARNING) $(COMPILE_OTHER) \
	       $(CXX_EXTRA_FLAGS) \
	       ${OTHER_OPTION} $(TARGET_CPU_FLAGS) -D${ARCH} \
	       -MMD -MP -MF $(@:%.o=%.d) -MT ./$@ \
	       -o $@ $<
	@echo '已结束构建（C++）： $<'
	@echo ' '


src/%.o: ../../src/%.c
	@mkdir -p $(dir $@)
	@echo '正在构建文件： $<'
	$(CC) $(COMPILE_SYMBOL) $(BENCH_DSYMBOL) \
	      $(COMPILE_INCLUDE) $(BENCH_INC) \
	      $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) \
	      $(COMPILE_WARNING) $(COMPILE_OTHER) \
	      ${OTHER_OPTION} -D${ARCH} \
	      -MMD -MP -MF $(@:%.o=%.d) -MT ./$@ \
	      -o $@ $<
	@echo '已结束构建： $<'
	@echo ' '


COMPILE_COMMAND := $(CC) $(COMPILE_SYMBOL) $(BENCH_DSYMBOL) \
    $(COMPILE_INCLUDE) $(BENCH_INC) \
    $(COMPILE_OPTIMIZATION) $(COMPILE_DEBUG) \
    $(COMPILE_WARNING) $(COMPILE_OTHER) \
    ${OTHER_OPTION} $(USER_OPTION) -D${ARCH} -o
