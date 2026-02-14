/**
 * @file stress_bench.c
 * @brief rtbench adapter for rtos_stress
 *
 * This file provides the interface between rtbench framework and
 * the stress-ng test suite. It exposes stress_bench_run() for
 * integration with the workload registry.
 */

#include "common/stress-ng.h"
#include <stdio.h>

/**
 * @brief Run a single stress test iteration
 *
 * This function runs the stress-ng cpu test with a short duration
 * for integration with the rtbench periodic benchmark framework.
 *
 * For full stress testing, use the rtos_stress command directly.
 *
 * @return 0 on success
 */
int stress_bench_run(void)
{
    /* Run a quick CPU stress test (1 second) as the default benchmark */
    char *argv[] = {
        "rtos_stress",
        "cpu",
        "-t", "1s",
        "-c", "1"
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    return stress_ng_main(argc, argv);
}

/**
 * @brief Run a specific stress test by name
 *
 * @param stressor_name Name of the stressor (e.g., "cpu", "vm", "matrix")
 * @param duration_sec Duration in seconds (0 for default 1s)
 * @return 0 on success
 */
int stress_bench_run_stressor(const char *stressor_name, int duration_sec)
{
    char duration_str[16];

    if (duration_sec <= 0) {
        duration_sec = 1;
    }
    snprintf(duration_str, sizeof(duration_str), "%ds", duration_sec);

    char *argv[] = {
        "rtos_stress",
        (char *)stressor_name,
        "-t", duration_str,
        "-c", "1"
    };
    int argc = sizeof(argv) / sizeof(argv[0]);

    return stress_ng_main(argc, argv);
}
