#*********************************************************************************************************
#
#                                    涓浗杞欢锟�??婧愮粍锟�??
#
#                                   宓屽叆寮忓疄鏃舵搷浣滅郴锟�??
#
#                                SylixOS(TM)  LW : long wing
#
#                               Copyright All Rights Reserved
#
#--------------鏂囦欢淇℃伅--------------------------------------------------------------------------------
#
# 锟�??   锟�??   锟�??: rtos-bench.mk
#
# 锟�??   锟�??   锟�??: RealEvo-IDE
#
# 鏂囦欢鍒涘缓鏃ユ湡: 2026 锟�?? 02 锟�?? 10 锟�??
#
# 锟�??        锟�??: 鏈枃浠剁敱 RealEvo-IDE 鐢熸垚锛岀敤浜庨厤锟�?? Makefile 鍔熻兘锛岃鍕挎墜鍔ㄤ慨锟�??
#*********************************************************************************************************

#*********************************************************************************************************
# Clear setting
#*********************************************************************************************************
include $(CLEAR_VARS_MK)

#*********************************************************************************************************
# Target
#*********************************************************************************************************
LOCAL_TARGET_NAME := rtos-bench

#*********************************************************************************************************
# Source list
#*********************************************************************************************************
LOCAL_SRCS := \
RTOS-Bench/generator/sylixos_entry.c \
RTOS-Bench/generator/test_schedule.c \
RTOS-Bench/generator/test_realtime.c \
RTOS-Bench/generator/test_stress.c \
RTOS-Bench/generator/test_cmd.c \
RTOS-Bench/generator/result_export.c \
RTOS-Bench/generator/uunifast.c \
RTOS-Bench/generator/stress_orig/common/stress-ng.c \
RTOS-Bench/generator/stress_orig/common/core-helper.c \
RTOS-Bench/generator/stress_orig/common/stress_stored_job.c \
RTOS-Bench/generator/stress_orig/osal/os_sylixos.c \
RTOS-Bench/generator/stress_orig/stressor/stress-atomic.c \
RTOS-Bench/generator/stress_orig/stressor/stress-bitops.c \
RTOS-Bench/generator/stress_orig/stressor/stress-bsearch.c \
RTOS-Bench/generator/stress_orig/stressor/stress-context.c \
RTOS-Bench/generator/stress_orig/stressor/stress-copy-file.c \
RTOS-Bench/generator/stress_orig/stressor/stress-cpu.c \
RTOS-Bench/generator/stress_orig/stressor/stress-dentry.c \
RTOS-Bench/generator/stress_orig/stressor/stress-fp.c \
RTOS-Bench/generator/stress_orig/stressor/stress-fstat.c \
RTOS-Bench/generator/stress_orig/stressor/stress-hdd.c \
RTOS-Bench/generator/stress_orig/stressor/stress-malloc.c \
RTOS-Bench/generator/stress_orig/stressor/stress-matrix.c \
RTOS-Bench/generator/stress_orig/stressor/stress-memcpy.c \
RTOS-Bench/generator/stress_orig/stressor/stress-memthrash.c \
RTOS-Bench/generator/stress_orig/stressor/stress-open.c \
RTOS-Bench/generator/stress_orig/stressor/stress-pipe.c \
RTOS-Bench/generator/stress_orig/stressor/stress-prime.c \
RTOS-Bench/generator/stress_orig/stressor/stress-ptr-chase.c \
RTOS-Bench/generator/stress_orig/stressor/stress-qsort.c \
RTOS-Bench/generator/stress_orig/stressor/stress-rename.c \
RTOS-Bench/generator/stress_orig/stressor/stress-stack.c \
RTOS-Bench/generator/stress_orig/stressor/stress-str.c \
RTOS-Bench/generator/stress_orig/stressor/stress-stream.c \
RTOS-Bench/generator/stress_orig/stressor/stress-trig.c \
RTOS-Bench/generator/stress_orig/stressor/stress-unlink.c \
RTOS-Bench/generator/stress_orig/stressor/stress-vecmath.c \
RTOS-Bench/generator/stress_orig/stressor/stress-vm.c \
RTOS-Bench/generator/realtime_orig/les/bench_init.c \
RTOS-Bench/generator/realtime_orig/les/cpu_affinity.c \
RTOS-Bench/generator/realtime_orig/les/data_tools.c \
RTOS-Bench/generator/realtime_orig/les/les.c \
RTOS-Bench/generator/realtime_orig/les/reworks_int.c \
RTOS-Bench/generator/realtime_orig/les/safe_sleep.c \
RTOS-Bench/generator/realtime_orig/multicore/ipc_bw.c \
RTOS-Bench/generator/realtime_orig/multicore/mem_bw.c \
RTOS-Bench/generator/realtime_orig/multicore/task_lt.c \
RTOS-Bench/generator/realtime_orig/realtime/test1.c \
RTOS-Bench/generator/realtime_orig/realtime/test10_1.c \
RTOS-Bench/generator/realtime_orig/realtime/test2.c \
RTOS-Bench/generator/realtime_orig/realtime/test3.c \
RTOS-Bench/generator/realtime_orig/realtime/test4_1.c \
RTOS-Bench/generator/realtime_orig/realtime/test4_2.c \
RTOS-Bench/generator/realtime_orig/realtime/test5_3.c \
RTOS-Bench/generator/realtime_orig/realtime/test6_0.c \
RTOS-Bench/generator/realtime_orig/realtime/test6_1.c \
RTOS-Bench/generator/realtime_orig/realtime/test6_2.c \
RTOS-Bench/generator/realtime_orig/realtime/test6_3.c \
RTOS-Bench/generator/realtime_orig/realtime/test6_4.c \
RTOS-Bench/generator/realtime_orig/realtime/test7_3.c \
RTOS-Bench/generator/realtime_orig/realtime/test8_1.c \
RTOS-Bench/generator/realtime_orig/realtime/test8_2.c \
RTOS-Bench/generator/realtime_orig/realtime/test9_3.c \
RTOS-Bench/generator/realtime_orig/verify/all_realtime_verify.c \
RTOS-Bench/generator/realtime_orig/verify/cpu_bind_verify.c \
RTOS-Bench/generator/realtime_orig/verify/freq_verify.c \
RTOS-Bench/generator/realtime_orig/verify/interrupt_stub_verify.c \
RTOS-Bench/generator/realtime_orig/verify/schedule_stub_verify.c \
RTOS-Bench/generator/realtime_orig/verify/time_verify.c \
RTOS-Bench/generator/periodic_benchmark.c \
RTOS-Bench/generator/workload_registry.c \
RTOS-Bench/generator/workload_stub.c \
RTOS-Bench/generator/workload_busywait.c \
RTOS-Bench/generator/logging.c \
RTOS-Bench/generator/memory_watcher.c \
RTOS-Bench/generator/platform/sylixos/timer.c \
RTOS-Bench/generator/platform/sylixos/sync.c \
RTOS-Bench/generator/platform/sylixos/scheduler.c \
RTOS-Bench/generator/platform/sylixos/timestamp.c \
RTOS-Bench/generator/platform/sylixos/signal.c \
RTOS-Bench/workloads/CUSUM/cusum_bench.c \
RTOS-Bench/workloads/EWMA/ewma_bench.c \
RTOS-Bench/workloads/FAST/fast.c \
RTOS-Bench/workloads/FAST/fast_9.c \
RTOS-Bench/workloads/FAST/fast_bench.c \
RTOS-Bench/workloads/MODBUS/modbus_bench.c \
RTOS-Bench/workloads/MODBUS/nanomodbus.c \
RTOS-Bench/workloads/MQTT/mongoose.c \
RTOS-Bench/workloads/MQTT/mqtt_bench.c \
src/cusum_workload.c

#*********************************************************************************************************
# C++ Source list
#*********************************************************************************************************
LOCAL_SRCS += \
RTOS-Bench/workloads/rtbench_workloads.cpp \
RTOS-Bench/workloads/PID/PID_v1.cpp \
RTOS-Bench/workloads/PID/pid_bench.cpp \
RTOS-Bench/workloads/PID/pid_wrapper.cpp \
RTOS-Bench/workloads/EKF/ekf_bench.cpp \
RTOS-Bench/workloads/EKF/EKF_core/ekf.cpp \
RTOS-Bench/workloads/EKF/EKF_core/ekf_helper.cpp \
RTOS-Bench/workloads/EKF/EKF_core/estimator_interface.cpp \
RTOS-Bench/workloads/EKF/EKF_core/EKFGSF_yaw.cpp \
RTOS-Bench/workloads/EKF/EKF_core/airspeed_fusion.cpp \
RTOS-Bench/workloads/EKF/EKF_core/control.cpp \
RTOS-Bench/workloads/EKF/EKF_core/covariance.cpp \
RTOS-Bench/workloads/EKF/EKF_core/drag_fusion.cpp \
RTOS-Bench/workloads/EKF/EKF_core/gps_checks.cpp \
RTOS-Bench/workloads/EKF/EKF_core/gps_yaw_fusion.cpp \
RTOS-Bench/workloads/EKF/EKF_core/imu_down_sampler.cpp \
RTOS-Bench/workloads/EKF/EKF_core/mag_control.cpp \
RTOS-Bench/workloads/EKF/EKF_core/mag_fusion.cpp \
RTOS-Bench/workloads/EKF/EKF_core/optflow_fusion.cpp \
RTOS-Bench/workloads/EKF/EKF_core/sensor_range_finder.cpp \
RTOS-Bench/workloads/EKF/EKF_core/sideslip_fusion.cpp \
RTOS-Bench/workloads/EKF/EKF_core/terrain_estimator.cpp \
RTOS-Bench/workloads/EKF/EKF_core/utils.cpp \
RTOS-Bench/workloads/EKF/EKF_core/vel_pos_fusion.cpp \
RTOS-Bench/workloads/EKF/geo/geo.cpp \
RTOS-Bench/workloads/EKF/geo_lookup/geo_mag_declination.cpp \
RTOS-Bench/workloads/EPNP/cayley.cpp \
RTOS-Bench/workloads/EPNP/CentralAbsoluteAdapter.cpp \
RTOS-Bench/workloads/EPNP/Epnp.cpp \
RTOS-Bench/workloads/EPNP/epnp_bench.cpp \
RTOS-Bench/workloads/EPNP/experiment_helpers.cpp \
RTOS-Bench/workloads/EPNP/methods.cpp \
RTOS-Bench/workloads/EPNP/random_generators.cpp \
RTOS-Bench/workloads/EPNP/time_measurement.cpp \
RTOS-Bench/workloads/ICP/icp.cpp \
RTOS-Bench/workloads/ICP/icp_bench.cpp \
RTOS-Bench/workloads/ICP/icpPointToPlane.cpp \
RTOS-Bench/workloads/ICP/icpPointToPoint.cpp \
RTOS-Bench/workloads/ICP/kdtree.cpp \
RTOS-Bench/workloads/ICP/matrix.cpp

#*********************************************************************************************************
# Header file search path (eg. LOCAL_INC_PATH := -I"Your header files search path")
#*********************************************************************************************************
LOCAL_INC_PATH :=  \
-I"./RTOS-Bench/generator" \
-I"./RTOS-Bench/workloads" \
-I"./RTOS-Bench/workloads/CUSUM" \
-I"./RTOS-Bench/workloads/EWMA" \
-I"./RTOS-Bench/workloads/FAST" \
-I"./RTOS-Bench/workloads/FAST/include_imgs" \
-I"./RTOS-Bench/workloads/PID" \
-I"./RTOS-Bench/workloads/EKF" \
-I"./RTOS-Bench/workloads/EKF/EKF_core" \
-I"./RTOS-Bench/workloads/EKF/include" \
-I"./RTOS-Bench/workloads/EKF/include/matrix" \
-I"./RTOS-Bench/workloads/EKF/geo" \
-I"./RTOS-Bench/workloads/EKF/geo_lookup" \
-I"./RTOS-Bench/workloads/EPNP" \
-I"./RTOS-Bench/workloads/EPNP/opengv" \
-I"./RTOS-Bench/workloads/EPNP/Eigen" \
-I"./RTOS-Bench/workloads/ICP" \
-I"./RTOS-Bench/workloads/MODBUS" \
-I"./RTOS-Bench/workloads/MQTT" \
-I"./RTOS-Bench/generator/stress_orig/common" \
-I"./RTOS-Bench/generator/stress_orig/osal" \
-I"./RTOS-Bench/generator/stress_orig/stressor" \
-I"./RTOS-Bench/generator/realtime_orig/les"

#*********************************************************************************************************
# Pre-defined macro (eg. -DYOUR_MARCO=1)
#*********************************************************************************************************
LOCAL_DSYMBOL :=  \
-DSYLIXOS_PLATFORM \
-D_GNU_SOURCE \
-DECL_STANDALONE \
-D__STDC_FORMAT_MACROS \
-D__STDC_LIMIT_MACROS \
-D_USE_MATH_DEFINES \
-DEIGEN_DONT_VECTORIZE \
-DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT \
-DFIX_OPENGV_USE_STUB_LDOUBLE

#*********************************************************************************************************
# Compiler flags
#*********************************************************************************************************
LOCAL_CFLAGS := 
LOCAL_CXXFLAGS := -std=c++14 -Wno-error -Wno-literal-suffix -include ./RTOS-Bench/workloads/EPNP/fix_opengv.h
LOCAL_LINKFLAGS := 

#*********************************************************************************************************
# Depend library (eg. LOCAL_DEPEND_LIB := -la LOCAL_DEPEND_LIB_PATH := -L"Your library search path")
#*********************************************************************************************************
LOCAL_DEPEND_LIB :=  \
-lm
LOCAL_DEPEND_LIB_PATH := 

#*********************************************************************************************************
# Linker specific
#*********************************************************************************************************
LOCAL_NO_UNDEF_SYM := no

#*********************************************************************************************************
# C++ config
#*********************************************************************************************************
LOCAL_USE_CXX        := yes
LOCAL_USE_CXX_EXCEPT := no

#*********************************************************************************************************
# Code coverage config
#*********************************************************************************************************
LOCAL_USE_GCOV := no

#*********************************************************************************************************
# OpenMP config
#*********************************************************************************************************
LOCAL_USE_OMP := no

#*********************************************************************************************************
# Use short command for link and ar
#*********************************************************************************************************
LOCAL_USE_SHORT_CMD := no

#*********************************************************************************************************
# User link command
#*********************************************************************************************************
LOCAL_PRE_LINK_CMD := 
LOCAL_POST_LINK_CMD := 
LOCAL_PRE_STRIP_CMD := 
LOCAL_POST_STRIP_CMD := 

#*********************************************************************************************************
# Depend target
#*********************************************************************************************************
LOCAL_DEPEND_TARGET := 

include $(APPLICATION_MK)

#*********************************************************************************************************
# End
#*********************************************************************************************************
