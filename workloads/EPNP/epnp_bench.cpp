#ifdef RT_THREAD_PLATFORM
#include <rtthread.h>
#include <finsh.h>
#else
#define MSH_CMD_EXPORT(cmd, desc)
#endif
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

// POSIX Includes
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>

// OpenGV Includes
#include <opengv/absolute_pose/methods.hpp>
#include <opengv/absolute_pose/CentralAbsoluteAdapter.hpp>
#include <opengv/math/cayley.hpp>

#include "random_generators.hpp"
#include "experiment_helpers.hpp"
// #include "time_measurement.hpp"

using namespace std;
using namespace Eigen;
using namespace opengv;

// 定义测试线程的栈大小，Eigen 矩阵运算给大一点，这里给 16KB
#define THREAD_STACK_SIZE   16384

static double diff_timespec_us(const struct timespec *start, const struct timespec *end)
{
    double start_us = (double)start->tv_sec * 1000000.0 + (double)start->tv_nsec / 1000.0;
    double end_us   = (double)end->tv_sec   * 1000000.0 + (double)end->tv_nsec   / 1000.0;
    return end_us - start_us;
}

extern "C" int epnp_bench_run(size_t iterations) {
    std::cout << "[POSIX] Starting ePnP Benchmark..." << std::endl;

    // 1. 初始化随机种子
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    srand((unsigned int)ts.tv_nsec);
    initializeRandomSeed();

    // 2. 设置实验参数
    size_t numberPoints = 100;
    if (iterations == 0) iterations = 1000;

    // 噪声与外点：设为0，专注于纯粹的算力测试
    double noise = 0.0;
    double outlierFraction = 0.0;

    // 3. 生成随机位姿 (Ground Truth)
    translation_t position = generateRandomTranslation(2.0);
    rotation_t rotation = generateRandomRotation(0.5);

    // 4. 创建虚拟中心相机系统
    translations_t camOffsets;
    rotations_t camRotations;
    generateCentralCameraSystem( camOffsets, camRotations );

    // 5. 生成对应的 2D-3D 数据
    bearingVectors_t bearingVectors;
    points_t points;
    std::vector<int> camCorrespondences;
    Eigen::MatrixXd gt(3,numberPoints);

    generateRandom2D3DCorrespondences(
        position, rotation, camOffsets, camRotations, numberPoints, noise, outlierFraction,
        bearingVectors, points, camCorrespondences, gt );

    // 打印实验配置 (使用了 opengv 的 helper)
    printExperimentCharacteristics( position, rotation, noise, outlierFraction );

    // 6. 创建 Adapter
    absolute_pose::CentralAbsoluteAdapter adapter(
        bearingVectors,
        points,
        rotation );

    // 7. 运行 ePnP 算法测试
    std::cout << "Running ePnP (using all " << numberPoints << " correspondences)..." << std::endl;
    
    transformation_t epnp_transformation;
    
    struct timespec start_time = {0, 0}; // 初始化为0
    struct timespec end_time = {0, 0};

    // 记录开始时间
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    size_t loops = iterations > 0 ? iterations : 1;
    for(size_t i = 0; i < loops; i++) {
        epnp_transformation = absolute_pose::epnp(adapter);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    // 计算结果
    double total_time_us = diff_timespec_us(&start_time, &end_time);
    double avg_time_us = total_time_us / loops;

    /* Unified format timing output */
    printf("[epnp] samples=%zu total_time=%.3f ms avg_latency=%.3f us/iter\n",
           loops, total_time_us / 1000.0, avg_time_us);

    return 0;
}

/**
 * 实际执行 ePnP 测试的线程入口函数
 */
static void* epnp_thread_entry(void* parameter) {
    size_t iterations = parameter ? *((size_t*)parameter) : 1;
    epnp_bench_run(iterations);
    return nullptr;
}

extern "C" int epnp_test(void) {
    pthread_t tid;
    pthread_attr_t attr;
    int ret;

    // 初始化线程属性
    pthread_attr_init(&attr);

    // 设置栈大小
    pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE);

    struct sched_param param;

    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = 20;
    pthread_attr_setschedparam(&attr, &param);
    // 显式继承调度属性
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    // 创建线程
    ret = pthread_create(&tid, &attr, epnp_thread_entry, NULL);

    // 销毁属性对象
    pthread_attr_destroy(&attr);
    
    if (ret == 0) {
        pthread_join(tid, NULL);
        printf("ePnP benchmark thread created successfully (POSIX).\n");
    }
    else {
        printf("Failed to create epnp benchmark thread. Error: %d\n", ret);
    }
    
    return 0;
}
MSH_CMD_EXPORT(epnp_test, run ePnP Benchmark);
