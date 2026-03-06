/**
 * @file cusum_workload.c
 * @brief Example: Custom workload registration for RTOS-Bench
 * @note This is an optional example file. You can remove it or use it
 *       as a template for your own custom workloads.
 */

#include "workload_registry.h"

extern int cusum_bench_run(void);

static int cusum_init(int parameters_num, void **parameters)
{
    (void)parameters_num;
    (void)parameters;
    return 0;
}

static void cusum_exec(int parameters_num, void **parameters)
{
    (void)parameters_num;
    (void)parameters;
    cusum_bench_run();
}

static void cusum_teardown(int parameters_num, void **parameters)
{
    (void)parameters_num;
    (void)parameters;
}

static const struct rtosbench_workload cusum_workload = {
    .name = "cusum",
    .description = "CUSUM change-point detection benchmark",
    .category = "signal",
    .init = cusum_init,
    .exec = cusum_exec,
    .teardown = cusum_teardown,
};

RTOSBENCH_REGISTER_WORKLOAD(cusum_workload);
