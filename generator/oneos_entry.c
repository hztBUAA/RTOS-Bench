/**
 * @file oneos_entry.c
 * @brief OneOS shell command entry for RTOS-Bench.
 *
 * Provides shell commands to list and run workloads on OneOS.
 * Usage:
 *   rtbench -l           List all workloads
 *   rtbench -b <name>    Run specific workload
 *   rtbench -A           Run all workloads
 *   rtbench test-realtime         Run realtime performance test
 *   rtbench test-cmd              Run cmd test
 *   # TODO
 *   rtbench test-schedule         Run schedulability test
 *   rtbench test-stress           Run stress test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os_task.h>
#include <shell.h>

#include "logging.h"
#include "workload_registry.h"
#include "test_schedule.h"
#include "test_realtime.h"
#include "test_stress.h"
#include "test_cmd.h"

/* External workload declarations */
extern const struct rtosbench_workload rtosbench_stub_workload;
extern const struct rtosbench_workload rtosbench_busywait_workload;

/* Forward declarations for C workloads */
extern int cusum_bench_run(void);
extern int ewma_bench_run(void);

/* ============================================================================
 * Workload Wrappers (minimal, reusing original benchmark functions)
 * ============================================================================ */

/* CUSUM wrapper */
static int cusum_init(int n, void **p) { (void)n; (void)p; return 0; }
static void cusum_exec(int n, void **p) { (void)n; (void)p; cusum_bench_run(); }
static void cusum_teardown(int n, void **p) { (void)n; (void)p; }

const struct rtosbench_workload rtosbench_cusum_workload = {
    .name = "cusum",
    .description = "CUSUM mean-shift detector",
    .category = "detection",
    .init = cusum_init,
    .exec = cusum_exec,
    .teardown = cusum_teardown,
};

/* EWMA wrapper */
static int ewma_init(int n, void **p) { (void)n; (void)p; return 0; }
static void ewma_exec(int n, void **p) { (void)n; (void)p; ewma_bench_run(); }
static void ewma_teardown(int n, void **p) { (void)n; (void)p; }

const struct rtosbench_workload rtosbench_ewma_workload = {
    .name = "ewma",
    .description = "EWMA residual thresholding",
    .category = "detection",
    .init = ewma_init,
    .exec = ewma_exec,
    .teardown = ewma_teardown,
};

/* ============================================================================
 * Workload Registration
 * ============================================================================ */

static int g_workloads_registered = 0;

static void ensure_workloads_registered(void)
{
    if (g_workloads_registered) {
        return;
    }
    g_workloads_registered = 1;

    rtosbench_register_workload(&rtosbench_stub_workload);
    rtosbench_register_workload(&rtosbench_busywait_workload);
    rtosbench_register_workload(&rtosbench_cusum_workload);
    rtosbench_register_workload(&rtosbench_ewma_workload);
}

/* ============================================================================
 * Shell Command Implementation
 * ============================================================================ */

static void print_workload_info(const char *name, const char *desc)
{
    printf("  %-12s - %s\n", name, desc ? desc : "(no description)");
}

static int run_single_workload(const char *name)
{
    printf("[rtbench] Running workload: %s\n", name);

    if (rtosbench_select_workload(name) != 0) {
        printf("[rtbench] Error: workload '%s' not found\n", name);
        return -1;
    }

    if (workload_init(0, NULL) != 0) {
        printf("[rtbench] Error: init failed\n");
        return -1;
    }

    workload_exec(0, NULL);
    workload_teardown(0, NULL);

    printf("[rtbench] Workload '%s' completed\n", name);
    return 0;
}

static int cmd_rtbench(int argc, char **argv)
{
    int i;
    int list_only = 0;
    int run_all = 0;
    const char *workload_name = NULL;

    ensure_workloads_registered();

    /* Parse arguments */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--list") == 0) {
            list_only = 1;
        } else if (strcmp(argv[i], "-A") == 0) {
            run_all = 1;
        } else if (strcmp(argv[i], "-b") == 0 && (i + 1 < argc)) {
            workload_name = argv[++i];
        }
    }

    /* Handle test-schedule subcommand */
    /* TODO */

	/* Handle test-realtime subcommand */
	if (argc >= 2 && strcmp(argv[1], "test-realtime") == 0) {
		int run_multicore = 0;
		int run_verify = 0;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "--verify") == 0 ||
                strcmp(argv[i], "-v") == 0) {
                run_verify = 1;
            } else if (strcmp(argv[i], "--multicore") == 0 ||
                strcmp(argv[i], "-m") == 0) {
                run_multicore = 1;
            } else if (strcmp(argv[i], "-q") == 0) {
                benchmark_verbosity = LOG_LEVEL_INFO;
            }
        }

		printf("[test-realtime] Starting realtime performance test on OneOS\n");
		if (run_multicore) {
			printf("  Multicore tests: enabled\n");
		}

        if (run_verify) {
            test_realtime_verify();
        }

		return test_realtime_run(run_multicore);
	}

	/* Handle test-stress subcommand */
	/* TODO */

	/* Handle test-cmd subcommand */
	if (argc >= 2 && strcmp(argv[1], "test-cmd") == 0) {
		for (int i = 2; i < argc; i++) {
			if (strcmp(argv[i], "-q") == 0) {
				benchmark_verbosity = LOG_LEVEL_INFO;
			}
		}
		printf("[test-cmd] Starting shell command support test on OneOS\n");
		return test_cmd_run();
	}

    /* List workloads */
    if (list_only) {
        printf("Available workloads (%d):\n", rtosbench_workload_count());
        rtosbench_list_workloads(print_workload_info);
        return 0;
    }

    /* Run all workloads */
    if (run_all) {
        int count = rtosbench_workload_count();
        printf("[rtbench] Running all %d workloads\n\n", count);
        for (i = 0; i < count; i++) {
            const struct rtosbench_workload *wl = rtosbench_get_workload(i);
            if (wl && wl->name) {
                run_single_workload(wl->name);
                printf("\n");
            }
        }
        return 0;
    }

    /* Run specific workload */
    if (workload_name) {
        return run_single_workload(workload_name);
    }

    /* No args: show usage */
    printf("RTOS-Bench for OneOS\n");
    printf("Usage: rtbench [options]\n");
    printf("  -l, --list    List available workloads\n");
    printf("  -b <name>     Run specific workload\n");
    printf("  -A            Run all workloads\n");
    return 0;
}

/* Register shell command */
SH_CMD_EXPORT(rtbench, cmd_rtbench, "RTOS-Bench workload runner");

int cmd_rtbench_stub(int argc, char **argv) {
    return cmd_rtbench(argc, argv);
}

/* Auto-init on system startup */
static int rtbench_auto_init(void)
{
    ensure_workloads_registered();
    printf("[rtbench] RTOS-Bench initialized, %d workloads available\n",
           rtosbench_workload_count());
    return 0;
}
OS_APP_INIT(rtbench_auto_init);
