#include <stdio.h>

#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>

#include "pid_data.h"
#include "PID_v1.h"

static unsigned long g_mock_millis = 0;

extern "C" unsigned long millis(void) {
    return g_mock_millis;
}

static uint64_t get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

// static double diff_timespec_us(const struct timespec *start, const struct timespec *end) {
//     double start_us = (double)start->tv_sec * 1000000.0 + (double)start->tv_nsec / 1000.0;
//     double end_us   = (double)end->tv_sec   * 1000000.0 + (double)end->tv_nsec   / 1000.0;
//     return end_us - start_us;
// }

static void* pid_thread_entry(void *parameter) {
    printf("--- RT-Thread PID Simulation Start ---\n");

    double Kp = 5.5, Ki = 0.02, Kd = 2.5;
    double Setpoint, Input, Output;

    PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

    myPID.SetMode(AUTOMATIC);
    myPID.SetOutputLimits(-10000, 10000);
    myPID.SetSampleTime(100);

    const int TEST_ROUNDS = 100000;
    uint64_t total_duration_ns = 0;
    volatile double output_dummy = 0.0;

    uint64_t t_start = get_time_ns();

    for (int i = 0; i < TEST_ROUNDS; i++) {
        double target, measured;
        
        get_synth_point(i, &target, &measured);
        
        Setpoint = target;
        Input = measured;
        g_mock_millis += 100;
        
        myPID.Compute();
        output_dummy += Output;
    }

    uint64_t t_end = get_time_ns();
    total_duration_ns = t_end - t_start;

    double avg_latency_ns = (double)total_duration_ns / TEST_ROUNDS;

    printf("\nBenchmark Results:\n");
    printf("------------------------------------------------\n");
    printf("Total Iterations:   %d\n", TEST_ROUNDS);
    printf("Total Time Cost:    %ld ns\n", total_duration_ns);
    printf("Avg Latency:        %.3f ns (%.3f us) / op\n", avg_latency_ns, avg_latency_ns/1000.0);
    printf("------------------------------------------------\n");

    return NULL;
}

extern "C" int pid_test(int argc, char **argv) {
    pthread_t tid;
    pthread_attr_t attr;
    int ret;
    
    pthread_attr_init(&attr);
    
    pthread_attr_setstacksize(&attr, 64 * 1024); 
    
    struct sched_param param;
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = 20;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    printf("Creating POSIX thread for PID benchmark...\n");

    ret = pthread_create(&tid, &attr, pid_thread_entry, NULL);

    pthread_attr_destroy(&attr);
    if (ret != 0) {
        printf("Failed to create pthread. Error: %d\n", ret);
        return -1;
    }
    
    // Wait for thread to complete
    pthread_join(tid, NULL);

    return 0;
}