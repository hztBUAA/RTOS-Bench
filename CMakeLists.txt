if(NOT TARGET ${PROJECT_NAME})
    message(FATAL_ERROR "Base target ${PROJECT_NAME} not found. Ensure project() is called in root.")
endif()

# 收集核心源文件
file(GLOB RTOS_BENCH_CORE 
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/oneos_entry.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/test_schedule.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/test_realtime.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/test_stress.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/test_cmd.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/result_export.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/uunifast.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/periodic_benchmark.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/workload_registry.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/workload_stub.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/workload_busywait.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/logging.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/memory_watcher.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/platform/oneos/posix_sched_adapter.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/platform/oneos/timer.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/platform/oneos/sync.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/platform/oneos/scheduler.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/platform/oneos/timestamp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/platform/oneos/signal.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/platform/oneos/module_compat.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/test_schedule/sched_workloads.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/test_schedule/sched_compute_wrappers.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/test_schedule/sched_modbus_wrapper.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/test_schedule/sched_mqtt_wrapper.c"
)

# 收集 stress 模块的源码
file(GLOB_RECURSE STRESS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/stress_orig/common/stress-ng.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/stress_orig/common/core-helper.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/stress_orig/common/stress_stored_job.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/stress_orig/osal/os_oneos.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/stress_orig/stressor/*.c"
)

# 收集 realtime 模块的源码
file(GLOB_RECURSE REALTIME_ORIG_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/generator/realtime_orig/*.c")

# 收集负载源文件
option(RTOS_WORKLOADS "Enable RTOS workloads" ON)
if(RTOS_WORKLOADS)

    file(GLOB_RECURSE RTOS_BENCH_WORKLOADS 
        "${CMAKE_CURRENT_SOURCE_DIR}/workloads/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/workloads/*.cpp"
    )
    # 排除模块独立入口
    list(REMOVE_ITEM RTOS_BENCH_WORKLOADS
        "${CMAKE_CURRENT_SOURCE_DIR}/workloads/main.cpp"
    )
else()
    set(RTOS_BENCH_WORKLOADS "")
endif()

# 将源文件关联到主项目目标
target_sources(${PROJECT_NAME} PRIVATE
    ${RTOS_BENCH_CORE}
    ${STRESS_SOURCES}
    ${REALTIME_ORIG_SOURCES}
    ${RTOS_BENCH_WORKLOADS}
)

# 添加算法和环境宏
target_compile_definitions(${PROJECT_NAME} PRIVATE
    _GNU_SOURCE
    ECL_STANDALONE
    __STDC_FORMAT_MACROS
    __STDC_LIMIT_MACROS
    _USE_MATH_DEFINES
    EIGEN_DONT_VECTORIZE
    EIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT
    FIX_OPENGV_USE_STUB_LDOUBLE
)

# 添加编译宏
target_compile_definitions(${PROJECT_NAME} PRIVATE
    ONEOS_PLATFORM
    MULTI_WORKLOAD
)

# 添加头文件路径
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/platform/oneos/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator"
    "${CMAKE_CURRENT_SOURCE_DIR}/workloads"
    "${CMAKE_CURRENT_SOURCE_DIR}/workloads/CUSUM"
    "${CMAKE_CURRENT_SOURCE_DIR}/workloads/EWMA"
    "${CMAKE_CURRENT_SOURCE_DIR}/workloads/FAST"
    "${CMAKE_CURRENT_SOURCE_DIR}/workloads/FAST/include_imgs"
    "${CMAKE_CURRENT_SOURCE_DIR}/workloads/PID"
    "${CMAKE_CURRENT_SOURCE_DIR}/workloads/EKF"
    "${CMAKE_CURRENT_SOURCE_DIR}/workloads/EKF/EKF_core"
    "${CMAKE_CURRENT_SOURCE_DIR}/workloads/EKF/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/workloads/EKF/include/matrix"
    "${CMAKE_CURRENT_SOURCE_DIR}/workloads/EKF/geo"
    "${CMAKE_CURRENT_SOURCE_DIR}/workloads/EKF/geo_lookup"
    "${CMAKE_CURRENT_SOURCE_DIR}/workloads/EPNP"
    "${CMAKE_CURRENT_SOURCE_DIR}/workloads/EPNP/opengv"
    "${CMAKE_CURRENT_SOURCE_DIR}/workloads/EPNP/Eigen"
    "${CMAKE_CURRENT_SOURCE_DIR}/workloads/ICP"
    "${CMAKE_CURRENT_SOURCE_DIR}/workloads/MODBUS"
    "${CMAKE_CURRENT_SOURCE_DIR}/workloads/MQTT"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/stress_orig/common"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/stress_orig/osal"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/stress_orig/stressor"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/realtime_orig/les"
    "${CMAKE_CURRENT_SOURCE_DIR}/generator/test_schedule"
)

# 强制包含头文件
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++14>
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-error>
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-literal-suffix>
    $<$<COMPILE_LANGUAGE:CXX>:-include cassert>
    #$<$<COMPILE_LANGUAGE:CXX>:-include "${CMAKE_CURRENT_SOURCE_DIR}/workloads/EPNP/fix_opengv.h">
)
