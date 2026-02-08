/**
 * @file ruihua_entry.c
 * @brief RTOS-Bench entry point for Ruihua RTOS (ReWorks)
 *
 * This file provides a shell command interface for running RTOS-Bench
 * on Ruihua RTOS platform. ReWorks is VxWorks-compatible, so we use
 * VxWorks-style shell commands.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Platform abstraction must be included before other headers */
#include "platform_abstraction.h"
#include "periodic_benchmark.h"
#include "logging.h"
#include "workload_registry.h"

/* Forward declarations */
extern void rtosbench_register_rtos_workloads(void);

/**
 * @brief List available workloads
 */
static void ruihua_list_workloads(void)
{
    int i;
    printf("\n=== RTOS-Bench Available Workloads ===\n");
    for (i = 0; i < rtosbench_workload_count(); i++) {
        const struct rtosbench_workload *wl = rtosbench_get_workload(i);
        if (wl && wl->name) {
            printf("  [%d] %s [%s] - %s\n", i,
                   wl->name,
                   wl->category ? wl->category : "-",
                   wl->description ? wl->description : "");
        }
    }
    printf("======================================\n\n");
}

/**
 * @brief Initialize execution options with defaults
 */
static void set_default_opts(struct execution_options *opts)
{
    memset(opts, 0, sizeof(*opts));
    opts->period_sec = 1;
    opts->period_nsec = 0;
    opts->deadline_sec = 0;
    opts->deadline_nsec = 0;
    opts->parsed_period = 1.0;
    opts->parsed_deadline = 0.0;
    opts->tasks_to_launch = 1;
    opts->prio = 100; /* 100 means "skip priority change" */
    opts->runtime = 0;
    opts->deadline = 0;
    opts->period = 0;
    opts->bytes_to_preallocate = 0;
    opts->memory_profiling_enable = 0;
    CPU_ZERO(&opts->core_affinity);
    CPU_ZERO(&opts->memory_profiling_core_affinity);
    opts->memory_profiling_time_bucket = 10000000;
    opts->output_path = NULL;
    opts->workload_name = NULL;
    opts->category_filter = NULL;
    opts->run_all_workloads = 0;
    opts->list_only = 0;
}

/**
 * @brief Run a specific workload
 * @param workload_name Name of the workload to run
 * @param period_sec Period in seconds
 * @param num_tasks Number of tasks to launch
 * @return 0 on success, negative on error
 */
int ruihua_run_benchmark(const char *workload_name, double period_sec, int num_tasks)
{
    struct execution_options opts;
    long sec;
    long nsec;

    /* Initialize default options */
    set_default_opts(&opts);

    sec = (long)period_sec;
    nsec = (long)((period_sec - (double)sec) * 1000000000.0);
    opts.period_sec = sec;
    opts.period_nsec = nsec;
    opts.parsed_period = period_sec;
    opts.tasks_to_launch = num_tasks > 0 ? (unsigned long long)num_tasks : 1;

    benchmark_verbosity = LOG_LEVEL_TRACE;

    /* Select and run workload */
    if (workload_name == NULL) {
        workload_name = "stub";
    }

    if (rtosbench_select_workload(workload_name) != 0) {
        printf("Error: workload '%s' not found\n", workload_name);
        ruihua_list_workloads();
        return -1;
    }

    printf("[ruihua_rtbench] Running workload: %s, period=%.3fs, tasks=%d\n",
           workload_name, period_sec, num_tasks);

    return periodic_benchmark(&opts);
}

/**
 * @brief Shell command entry point (VxWorks-style)
 *
 * Usage from Ruihua shell:
 *   -> rtbench                    - List available workloads
 *   -> rtbench "stub"             - Run stub workload with defaults
 *   -> rtbench "pid", 1.0, 2      - Run PID workload, 1s period, 2 tasks
 */
int rtbench(const char *workload, double period, int tasks)
{
    /* Register workloads on first call */
    static int initialized = 0;
    if (!initialized) {
        rtosbench_register_rtos_workloads();
        initialized = 1;
    }

    if (workload == NULL || workload[0] == '\0') {
        ruihua_list_workloads();
        printf("Usage: rtbench \"<workload>\", [period_sec], [num_tasks]\n");
        printf("Example: rtbench \"pid\", 1.0, 2\n");
        return 0;
    }

    if (period <= 0.0) {
        period = 1.0;
    }
    if (tasks <= 0) {
        tasks = 1;
    }

    return ruihua_run_benchmark(workload, period, tasks);
}

/**
 * @brief Simple test entry point
 *
 * Call from shell: -> rtbench_test
 */
int rtbench_test(void)
{
    static int initialized = 0;
    if (!initialized) {
        rtosbench_register_rtos_workloads();
        initialized = 1;
    }

    printf("RTOS-Bench Test on Ruihua RTOS (ReWorks)\n");
    ruihua_list_workloads();

    /* Run stub workload as a simple test */
    return ruihua_run_benchmark("stub", 1.0, 1);
}

/**
 * @brief Quick workload runner
 *
 * Call from shell: -> rtbench_quick "workload_name"
 */
int rtbench_quick(const char *name)
{
    static int initialized = 0;
    if (!initialized) {
        rtosbench_register_rtos_workloads();
        initialized = 1;
    }

    if (name == NULL || name[0] == '\0') {
        name = "stub";
    }

    return ruihua_run_benchmark(name, 0.5, 3);
}
