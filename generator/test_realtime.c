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
/* Forward declaration - implemented in realtime_orig/les/bench_init.c */
extern int realtime_benchmark_run_sections(unsigned int sections);
/* Forward declaration - implemented in realtime_orig/verify/all_realtime_verify.c */
extern void realtime_verify_all(void);

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

void test_realtime_verify(void) {
    realtime_verify_all();
}

/*
 * Sectioned entry used by the split test-realtime commands
 * (--delay / --cost / --multi-access / --multi-service).
 * The legacy test_realtime_run() above is intentionally left untouched.
 */
int test_realtime_run_ex(unsigned int sections)
{
    REALTIME_PRINTF("\n");
    REALTIME_PRINTF("=============================================================\n");
    REALTIME_PRINTF("[test-realtime] Starting realtime performance benchmark\n");
    REALTIME_PRINTF("=============================================================\n");

    REALTIME_PRINTF("Sections:");
    if (sections & RTBENCH_REALTIME_SECTION_DELAY) {
        REALTIME_PRINTF(" delay");
    }
    if (sections & RTBENCH_REALTIME_SECTION_COST) {
        REALTIME_PRINTF(" cost");
    }
    if (sections & RTBENCH_REALTIME_SECTION_MULTI_ACCESS) {
        REALTIME_PRINTF(" multi-access");
    }
    if (sections & RTBENCH_REALTIME_SECTION_MULTI_SERVICE) {
        REALTIME_PRINTF(" multi-service");
    }
    REALTIME_PRINTF("\n\n");

    int ret = realtime_benchmark_run_sections(sections);

    REALTIME_PRINTF("\n");
    REALTIME_PRINTF("=============================================================\n");
    REALTIME_PRINTF("[test-realtime] Benchmark completed with code: %d\n", ret);
    REALTIME_PRINTF("=============================================================\n");

    return ret;
}
