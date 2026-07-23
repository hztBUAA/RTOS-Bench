#ifdef RT_THREAD_PLATFORM
#include <rtthread.h>
#include <finsh.h>
#else
#define MSH_CMD_EXPORT(cmd, desc)
#endif
#include <iostream>
#include <cstdlib>
#include <cmath>

/* POSIX */
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>

#include "icpPointToPlane.h"
#include "suzanne.h"

using namespace std;

static double diff_timespec_us(const struct timespec *start, const struct timespec *end) {
    double start_us = (double)start->tv_sec * 1000000.0 + (double)start->tv_nsec / 1000.0;
    double end_us   = (double)end->tv_sec   * 1000000.0 + (double)end->tv_nsec   / 1000.0;
    return end_us - start_us;
}

extern "C" int icp_bench_run(void) {
    struct timespec start_time, end_time;

    int32_t num = bench_num;
    int32_t dim = 3;

    /* Output suppressed to avoid affecting performance measurements */
    // printf("ICP Benchmark Start...\n");

    double* M = (double*)bench_source_points;
    double* T = (double*)bench_target_points;

    Matrix R = Matrix::eye(3);
    Matrix t(3, 1);
    t.zero();

    // run point-to-plane ICP (-1 = no outlier threshold)
    cout << endl << "[ICP] Running ICP (point-to-plane)" << endl;

    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    IcpPointToPlane icp(T, num, dim);
    double residual = icp.fit(M, num, R, t, -1);

    clock_gettime(CLOCK_MONOTONIC, &end_time);

    double total_time_us = diff_timespec_us(&start_time, &end_time);

    printf("[ICP] samples=1 total_time=%.3f ms avg_latency=%.3f ms/run\n",
           total_time_us / 1000.0, total_time_us / 1000.0);

    return 0;
}

static void* icp_thread_entry(void* parameter) {
    (void)parameter;
    icp_bench_run();
    return NULL;
}

extern "C" int icp_test(void) {
    pthread_t tid;
    pthread_attr_t attr;
    int ret;

    // 初始化线程属性
    pthread_attr_init(&attr);

    pthread_attr_setstacksize(&attr, 64 * 1024); /* OneOS aarch64: 过大栈(1MB)分配异常->坏栈/deadbeef; 保持 64KB 基线 */

    struct sched_param param;
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = 20;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    cout << "[POSIX Thread][ICP] Creating benchmark thread..." << endl;

    ret = pthread_create(&tid, &attr, icp_thread_entry, NULL);
    
    pthread_attr_destroy(&attr);

    if (ret != 0) {
        cout << "[POSIX Thread][ICP] Failed to create benchmark thread. Error: " << ret << endl;
    } else {
        pthread_join(tid, NULL);
        cout << "[POSIX Thread][ICP] Benchmark thread finished." << endl;
    }

    return 0;
}
// 导出 MSH 命令
MSH_CMD_EXPORT(icp_test, Run ICP benchmark);
