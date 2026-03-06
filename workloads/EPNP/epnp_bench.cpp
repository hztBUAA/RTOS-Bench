
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

using namespace std;
using namespace Eigen;
using namespace opengv;

#define THREAD_STACK_SIZE   64 * 1024

static double diff_timespec_us(const struct timespec *start, const struct timespec *end)
{
    double start_us = (double)start->tv_sec * 1000000.0 + (double)start->tv_nsec / 1000.0;
    double end_us   = (double)end->tv_sec   * 1000000.0 + (double)end->tv_nsec   / 1000.0;
    return end_us - start_us;
}

static void* epnp_thread_entry(void* parameter) {
    std::cout << "[POSIX] Starting ePnP Benchmark..." << std::endl;

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    srand((unsigned int)ts.tv_nsec);
    initializeRandomSeed();

    size_t numberPoints = 100;
    const size_t iterations = 1000;

    double noise = 0.0;
    double outlierFraction = 0.0;

    translation_t position = generateRandomTranslation(2.0);
    rotation_t rotation = generateRandomRotation(0.5);

    translations_t camOffsets;
    rotations_t camRotations;
    generateCentralCameraSystem( camOffsets, camRotations );

    bearingVectors_t bearingVectors;
    points_t points;
    std::vector<int> camCorrespondences;
    Eigen::MatrixXd gt(3,numberPoints);

    generateRandom2D3DCorrespondences(
        position, rotation, camOffsets, camRotations, numberPoints, noise, outlierFraction,
        bearingVectors, points, camCorrespondences, gt );

    printExperimentCharacteristics( position, rotation, noise, outlierFraction );

    absolute_pose::CentralAbsoluteAdapter adapter(
        bearingVectors,
        points,
        rotation );

    std::cout << "Running ePnP (using all " << numberPoints << " correspondences)..." << std::endl;
    
    transformation_t epnp_transformation;
    
    struct timespec start_time = {0, 0};
    struct timespec end_time = {0, 0};

    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    for(size_t i = 0; i < iterations; i++) {
        epnp_transformation = absolute_pose::epnp(adapter);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    double total_time_us = diff_timespec_us(&start_time, &end_time);
    double avg_time_us = total_time_us / iterations;
    double score = 1000000.0 / avg_time_us;

    std::cout << "---------------------------------------------" << std::endl;
    std::cout << "RTOS Benchmark Result (ePnP 100 points):" << std::endl;
    std::cout << "Total Time: " << (total_time_us/1000.0) << " ms" << std::endl;
    std::cout << "Avg Latency: " << avg_time_us << " us" << std::endl;
    std::cout << "Performance: " << (int)score << " Runs/Sec" << std::endl;
    std::cout << "---------------------------------------------" << std::endl;

    return nullptr;
}

extern "C" int epnp_test(int argc, char** argv) {
    pthread_t tid;
    pthread_attr_t attr;
    int ret;

    pthread_attr_init(&attr);

    pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE);

    struct sched_param param;

    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = 20;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    ret = pthread_create(&tid, &attr, epnp_thread_entry, NULL);

    pthread_attr_destroy(&attr);
    
    if (ret == 0) {
        pthread_detach(tid);
        printf("ePnP benchmark thread created successfully (POSIX).\n");
    }
    else {
        printf("Failed to create epnp benchmark thread. Error: %d\n", ret);
    }
    
    return 0;
}
