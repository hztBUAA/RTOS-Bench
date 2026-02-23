/**
 * @file test_realtime.c
 * @brief Wrapper for the original realtime benchmark implementation
 * @details This file provides a thin wrapper around the original realtime
 *          benchmark code located in generator/realtime_orig/
 *
 * The original implementation includes:
 * - realtime_orig/les/       - LES timer and benchmark infrastructure
 * - realtime_orig/realtime/  - Single-core realtime tests (test1-test11)
 * - realtime_orig/multicore/ - Multicore performance tests
 */

#include "test_realtime.h"

#include <stdio.h>

#ifdef RT_THREAD_PLATFORM
#include <rtthread.h>
#define REALTIME_PRINTF rt_kprintf
#else
#define REALTIME_PRINTF printf
#endif

/* Forward declaration - implemented in realtime_orig/les/bench_init.c */
extern int realtime_benchmark_run_all(int run_multicore);

int test_realtime_run(int run_multicore)
{
    REALTIME_PRINTF("\n");
    REALTIME_PRINTF("=============================================================\n");
    REALTIME_PRINTF("[test-realtime] Starting realtime performance benchmark\n");
    REALTIME_PRINTF("=============================================================\n");

    if (run_multicore) {
        REALTIME_PRINTF("Mode: Single-core + Multicore tests\n");
    } else {
        REALTIME_PRINTF("Mode: Single-core tests only\n");
    }
    REALTIME_PRINTF("\n");

    int ret = realtime_benchmark_run_all(run_multicore);

    REALTIME_PRINTF("\n");
    REALTIME_PRINTF("=============================================================\n");
    REALTIME_PRINTF("[test-realtime] Benchmark completed with code: %d\n", ret);
    REALTIME_PRINTF("=============================================================\n");

    return ret;
}
