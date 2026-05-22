/**
 * @file oneos_entry.c
 * @brief OneOS shell command entry for RTOS-Bench.
 *
 * Provides comprehensive test suite for OneOS platform.
 * Usage:
 *   rtbench -l                    List all workloads
 *   rtbench -b <name>             Run specific workload
 *   rtbench -A                    Run all workloads
 *   rtbench test-all              Run comprehensive test suite
 *   rtbench test-realtime         Run realtime performance test
 *   rtbench test-schedule         Run schedulability test
 *   rtbench test-stress           Run stress test
 *   rtbench test-cmd              Run shell command support test
 *   rtbench export-result         Export results to JSON
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <os_task.h>
#include <os_sem.h>
#include <os_clock.h>
#include <oneos_config.h>
#include <shell.h>

#include "platform_abstraction.h"
#include "periodic_benchmark.h"
#include "logging.h"
#include "workload_registry.h"
#include "test_schedule.h"
#include "test_realtime.h"
#include "test_stress.h"
#include "test_cmd.h"
#include "result_export.h"

/* API compatibility: V2.0 uses os_tick_get_value(), V1.x uses os_tick_get() */
#if defined(ONEOS_V2_ARM64)
    #define RTBENCH_GET_TICK()  os_tick_get_value()
#else
    #define RTBENCH_GET_TICK()  os_tick_get()
#endif

/* External workload declarations */
extern const struct rtosbench_workload rtosbench_stub_workload;
extern const struct rtosbench_workload rtosbench_busywait_workload;

/* Forward declarations for C workloads */
extern int cusum_bench_run(void);
extern int ewma_bench_run(void);

/* Default result output path */
#define RTBENCH_DEFAULT_OUTPUT_PATH "/rtbench_result.json"

/* Stack size for worker threads (32KB to handle deep call chains) */
#define RTBENCH_TEST_ALL_STACK_SIZE (32 * 1024)

/* Forward declarations for result collection */
static void collect_realtime_result(int run_multicore);
static void collect_schedule_result(void);
static void collect_stress_result(const char *job_name);
static void collect_cmd_result(void);
static void collect_workload_results(int quick_mode);

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
 * Help & Utilities
 * ============================================================================ */

static int ci_equal(const char *a, const char *b)
{
    if (a == NULL || b == NULL) {
        return -1;
    }
    while (*a && *b) {
        int da = tolower((unsigned char)*a++);
        int db = tolower((unsigned char)*b++);
        if (da != db) {
            return da - db;
        }
    }
    return (unsigned char)*a - (unsigned char)*b;
}

static void print_usage(void)
{
    printf("\n");
    printf("RTOS-Bench - Cross-platform Industrial RTOS Benchmark Framework\n");
    printf("\n");
    printf("USAGE:\n");
    printf("  rtbench [OPTIONS]                    Run workload benchmark\n");
    printf("  rtbench test-all [OPTIONS]           Run comprehensive test suite\n");
    printf("  rtbench test-realtime [OPTIONS]      Run realtime performance test\n");
    printf("  rtbench test-schedule [OPTIONS]      Run schedulability test\n");
    printf("  rtbench test-stress [OPTIONS]        Run stress test\n");
    printf("  rtbench test-cmd                     Run shell command support test\n");
    printf("  rtbench export-result [OPTIONS]      Export results to JSON\n");
    printf("  rtbench -L | --list                  List available workloads\n");
    printf("  rtbench -h | --help                  Show this help message\n");
    printf("\n");
    printf("WORKLOAD BENCHMARK OPTIONS:\n");
    printf("  -b <name>        Workload name (use -L to list)\n");
    printf("  -p <sec>         Period in seconds (default: 1.0)\n");
    printf("  -t <count>       Number of tasks to launch (default: 1)\n");
    printf("  -q               Quiet mode (reduce verbosity)\n");
    printf("  -A               Run all registered workloads\n");
    printf("  -G <categories>  Filter workloads by comma-separated categories\n");
    printf("\n");
    printf("TEST-ALL OPTIONS:\n");
    printf("  -o, --output <path>   Output JSON path (default: %s)\n",
           RTBENCH_DEFAULT_OUTPUT_PATH);
    printf("  --no-realtime         Skip realtime performance test\n");
    printf("  --no-schedule         Skip schedulability test\n");
    printf("  --no-stress           Skip stress test\n");
    printf("  --no-cmd              Skip shell command support test\n");
    printf("  --no-workload         Skip typical workload tests\n");
    printf("  -m, --multicore       Enable multicore tests\n");
    printf("  --quick               Quick mode (smoke test)\n");
    printf("  -q                    Quiet mode\n");
    printf("\n");
    printf("TEST-REALTIME OPTIONS:\n");
    printf("  -v, --verify          Run verification tests\n");
    printf("  -m, --multicore       Enable multicore tests\n");
    printf("  -q                    Quiet mode\n");
    printf("\n");
    printf("TEST-SCHEDULE OPTIONS:\n");
    printf("  --cycles <n>          Number of test cycles (default: 100)\n");
    printf("  --util-start <pct>    Starting utilization percentage (default: 10)\n");
    printf("  --util-end <pct>      Ending utilization percentage (default: 100)\n");
    printf("  --util-step <pct>     Utilization step size (default: 10)\n");
    printf("  --quick               Quick mode (fewer cycles)\n");
    printf("  -q                    Quiet mode\n");
    printf("\n");
    printf("TEST-STRESS OPTIONS:\n");
    printf("  --job <name>          Run predefined stress job (default: all)\n");
    printf("  -s <stressor>         Run single stressor\n");
    printf("  -t <sec>              Duration in seconds (default: 10)\n");
    printf("  -c <workers>          Number of workers (default: 1)\n");
    printf("  --ops <count>         Maximum operations (default: unlimited)\n");
    printf("  --method <name>       Algorithm/method name for stressor\n");
    printf("  --opts <options>      Extra stressor-specific options\n");
    printf("  -l, --list            List available jobs and stressors\n");
    printf("  --quick               Quick mode\n");
    printf("  -q                    Quiet mode\n");
    printf("\n");
    printf("EXPORT-RESULT OPTIONS:\n");
    printf("  -o, --output <path>   Output JSON path (default: %s)\n",
           RTBENCH_DEFAULT_OUTPUT_PATH);
    printf("\n");
    printf("EXAMPLES:\n");
    printf("  rtbench -L                                  # List workloads\n");
    printf("  rtbench -b busywait -p 0.5 -t 10 -q         # Run busywait 10 times\n");
    printf("  rtbench test-all --quick                    # Quick comprehensive test\n");
    printf("  rtbench test-schedule --cycles 50           # Custom schedule test\n");
    printf("  rtbench test-stress -s cpu -t 30            # CPU stress for 30s\n");
    printf("\n");
}

static void print_workload_info(const char *name, const char *desc)
{
    printf("  %-12s - %s\n", name, desc ? desc : "(no description)");
}

static void set_default_exec_opts(struct execution_options *opts)
{
    memset(opts, 0, sizeof(*opts));
    opts->period_sec = 1;
    opts->period_nsec = 0;
    opts->deadline_sec = 0;
    opts->deadline_nsec = 0;
    opts->parsed_period = 0.0;
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
    benchmark_verbosity = LOG_LEVEL_TRACE;
}

static void parse_args(int argc, char **argv, struct execution_options *opts,
                       int *help_requested)
{
    *help_requested = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            print_usage();
            *help_requested = 1;
        } else if (!strcmp(argv[i], "-p") && (i + 1 < argc)) {
            double v = atof(argv[++i]);
            long sec = (long)v;
            long nsec = (long)((v - (double)sec) * 1000000000.0);
            opts->period_sec = sec;
            opts->period_nsec = nsec;
            opts->parsed_period = v;
        } else if (!strcmp(argv[i], "-d") && (i + 1 < argc)) {
            double v = atof(argv[++i]);
            long sec = (long)v;
            long nsec = (long)((v - (double)sec) * 1000000000.0);
            opts->deadline_sec = sec;
            opts->deadline_nsec = nsec;
            opts->parsed_deadline = v;
        } else if (!strcmp(argv[i], "-t") && (i + 1 < argc)) {
            opts->tasks_to_launch = strtoull(argv[++i], NULL, 10);
        } else if (!strcmp(argv[i], "-f") && (i + 1 < argc)) {
            opts->prio = (uint32_t)strtoul(argv[++i], NULL, 10);
        } else if (!strcmp(argv[i], "-c") && (i + 1 < argc)) {
            int cpu = (int)strtol(argv[++i], NULL, 10);
            if (cpu >= 0 && cpu < 32) {
                CPU_ZERO(&opts->core_affinity);
                CPU_SET((uint32_t)cpu, &opts->core_affinity);
            }
        } else if (!strcmp(argv[i], "-q")) {
            benchmark_verbosity = LOG_LEVEL_INFO;
        } else if (!strcmp(argv[i], "-b") && (i + 1 < argc)) {
            opts->workload_name = argv[++i];
        } else if (!strcmp(argv[i], "-w") && (i + 1 < argc)) {
            opts->workload_name = argv[++i];
        } else if (!strcmp(argv[i], "-A")) {
            opts->run_all_workloads = 1;
        } else if (!strcmp(argv[i], "-G") && (i + 1 < argc)) {
            opts->category_filter = argv[++i];
        } else if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--list") ||
                   !strcmp(argv[i], "-L")) {
            opts->list_only = 1;
        }
    }
}

static void debug_print_context(const struct execution_options *opts)
{
    printf("[rtbench] workload=%s period=%ld.%09ld tasks=%llu\n",
           rtosbench_current_workload(),
           opts->period_sec, opts->period_nsec,
           (unsigned long long)opts->tasks_to_launch);
}

/* ============================================================================
 * test-all worker thread (uses os_task to get large stack)
 * ============================================================================ */

struct test_all_params {
    const char *output_path;
    int run_realtime;
    int run_schedule;
    int run_stress;
    int run_cmd;
    int run_workload;
    int run_multicore;
    int quick_mode;
    int result;
    os_semaphore_id done_sem;
};

static void test_all_thread_entry(void *parameter)
{
    struct test_all_params *p = (struct test_all_params *)parameter;

    /* Initialize result collection */
    rtbench_result_init();
    rtbench_result_set_env("OneOS", "3.x", "OneOS-Board", "ARM", 0, 1);
    rtbench_result_start();

    /* Register workloads */
    ensure_workloads_registered();

    printf("\n");
    printf("=============================================================\n");
    printf("[RTOS-Bench] Comprehensive Test Suite\n");
    printf("=============================================================\n");
    printf("Output: %s\n", p->output_path);
    printf("Modules: realtime=%s schedule=%s stress=%s cmd=%s workload=%s\n",
           p->run_realtime ? "yes" : "no",
           p->run_schedule ? "yes" : "no",
           p->run_stress ? "yes" : "no",
           p->run_cmd ? "yes" : "no",
           p->run_workload ? "yes" : "no");
    if (p->quick_mode) {
        printf("Mode: QUICK (smoke test)\n");
    }
    printf("\n");

    /* Run realtime test */
    if (p->run_realtime) {
        printf(">>> Running test-realtime...\n");
        test_realtime_run(p->run_multicore);
        collect_realtime_result(p->run_multicore);
    }

    /* Run schedule test */
    if (p->run_schedule) {
        printf("\n>>> Running test-schedule%s...\n",
               p->quick_mode ? " (quick)" : "");
        if (p->quick_mode) {
            test_schedule_run_custom(
                TEST_SCHEDULE_QUICK_CYCLES,
                TEST_SCHEDULE_QUICK_UTIL_START,
                TEST_SCHEDULE_QUICK_UTIL_END,
                TEST_SCHEDULE_QUICK_UTIL_STEP);
        } else {
            test_schedule_run();
        }
        collect_schedule_result();
    }

    /* Run stress test */
    if (p->run_stress) {
        const char *stress_job = p->quick_mode ? "all-quick" : "all";
        printf("\n>>> Running test-stress (job: %s)...\n", stress_job);
        test_stress_run_job(stress_job);
        collect_stress_result(stress_job);
    }

    /* Run command support test */
    if (p->run_cmd) {
        printf("\n>>> Running test-cmd...\n");
        test_cmd_run();
        collect_cmd_result();
    }

    /* Run workload tests */
    if (p->run_workload) {
        printf("\n>>> Running typical workloads%s...\n",
               p->quick_mode ? " (quick)" : "");
        collect_workload_results(p->quick_mode);
    }

    /* Finalize and export */
    rtbench_result_end();
    p->result = rtbench_result_export_json(p->output_path);
    if (p->result == 0) {
        printf("\n=============================================================\n");
        printf("[RTOS-Bench] Results saved to: %s\n", p->output_path);
        printf("=============================================================\n");
    } else {
        printf("\n[RTOS-Bench] Failed to save results: %d\n", p->result);
    }

    rtbench_result_cleanup();

    /* Signal completion */
    os_semaphore_post(p->done_sem);
}

/* ============================================================================
 * Shell Command Implementation
 * ============================================================================ */

static int cmd_rtbench(int argc, char **argv)
{
    const char *output_path = NULL;

    ensure_workloads_registered();

    /* No arguments - show help */
    if (argc < 2) {
        print_usage();
        return 0;
    }

    /* Handle test-all subcommand - comprehensive test suite */
    if (argc >= 2 && strcmp(argv[1], "test-all") == 0) {
        static struct test_all_params params;
        memset(&params, 0, sizeof(params));
        params.run_realtime = 1;
        params.run_schedule = 1;
        params.run_stress = 1;
        params.run_cmd = 1;
        params.run_workload = 1;
        params.run_multicore = 0;
        params.quick_mode = 0;

        /* Parse optional arguments */
        for (int i = 2; i < argc; i++) {
            if ((strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) && (i + 1 < argc)) {
                output_path = argv[++i];
            } else if (strcmp(argv[i], "--no-realtime") == 0) {
                params.run_realtime = 0;
            } else if (strcmp(argv[i], "--no-schedule") == 0) {
                params.run_schedule = 0;
            } else if (strcmp(argv[i], "--no-stress") == 0) {
                params.run_stress = 0;
            } else if (strcmp(argv[i], "--no-cmd") == 0) {
                params.run_cmd = 0;
            } else if (strcmp(argv[i], "--no-workload") == 0) {
                params.run_workload = 0;
            } else if (strcmp(argv[i], "--multicore") == 0 || strcmp(argv[i], "-m") == 0) {
                params.run_multicore = 1;
            } else if (strcmp(argv[i], "--quick") == 0) {
                params.quick_mode = 1;
            } else if (strcmp(argv[i], "-q") == 0) {
                benchmark_verbosity = LOG_LEVEL_INFO;
            }
        }

        /* Use default output path if not specified */
        if (!output_path) {
            output_path = RTBENCH_DEFAULT_OUTPUT_PATH;
        }
        params.output_path = output_path;

        /* Create completion semaphore */
        params.done_sem = os_semaphore_create(NULL, "ta_done", 0, OS_SEM_MAX_VALUE);
        if (params.done_sem == NULL) {
            printf("[RTOS-Bench] Failed to create semaphore\n");
            return -1;
        }

        /* Spawn worker task with large stack */
        static os_task_dummy_t test_all_task_cb;
        os_task_id task = os_task_create(&test_all_task_cb,
                                          OS_NULL,
                                          RTBENCH_TEST_ALL_STACK_SIZE,
                                          "rtbench",
                                          test_all_thread_entry,
                                          &params,
                                          OS_TASK_PRIORITY_MAX / 2);
        if (task < 0) {
            printf("[RTOS-Bench] Failed to create test-all worker task\n");
            os_semaphore_destroy(params.done_sem);
            return -1;
        }
        os_task_startup(task);

        /* Wait for worker to finish */
        os_semaphore_wait(params.done_sem, OS_WAIT_FOREVER);
        os_semaphore_destroy(params.done_sem);

        return params.result;
    }

    /* Handle export-result subcommand */
    if (argc >= 2 && strcmp(argv[1], "export-result") == 0) {
        output_path = RTBENCH_DEFAULT_OUTPUT_PATH;
        for (int i = 2; i < argc; i++) {
            if ((strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) && (i + 1 < argc)) {
                output_path = argv[++i];
            }
        }
        int ret = rtbench_result_export_json(output_path);
        if (ret == 0) {
            printf("[RTOS-Bench] Results exported to: %s\n", output_path);
        }
        return ret;
    }

    /* Handle test-schedule subcommand */
    if (argc >= 2 && strcmp(argv[1], "test-schedule") == 0) {
        int cycles = TEST_SCHEDULE_CYCLES;
        int util_start = TEST_SCHEDULE_UTIL_START;
        int util_end = TEST_SCHEDULE_UTIL_END;
        int util_step = TEST_SCHEDULE_UTIL_STEP;
        int quick = 0;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "--cycles") == 0 && (i + 1 < argc)) {
                cycles = atoi(argv[++i]);
            } else if (strcmp(argv[i], "--util-start") == 0 && (i + 1 < argc)) {
                util_start = atoi(argv[++i]);
            } else if (strcmp(argv[i], "--util-end") == 0 && (i + 1 < argc)) {
                util_end = atoi(argv[++i]);
            } else if (strcmp(argv[i], "--util-step") == 0 && (i + 1 < argc)) {
                util_step = atoi(argv[++i]);
            } else if (strcmp(argv[i], "--quick") == 0) {
                quick = 1;
            } else if (strcmp(argv[i], "-q") == 0) {
                benchmark_verbosity = LOG_LEVEL_INFO;
            }
        }

        if (quick) {
            cycles = TEST_SCHEDULE_QUICK_CYCLES;
            util_start = TEST_SCHEDULE_QUICK_UTIL_START;
            util_end = TEST_SCHEDULE_QUICK_UTIL_END;
            util_step = TEST_SCHEDULE_QUICK_UTIL_STEP;
        }

        printf("[test-schedule] Starting schedulability test on OneOS%s\n",
               quick ? " (quick)" : "");
        printf("  Cycles: %d, Utilization: %d%% - %d%% (step %d%%)\n",
               cycles, util_start, util_end, util_step);

        return test_schedule_run_custom(cycles, util_start, util_end, util_step);
    }

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
    if (argc >= 2 && strcmp(argv[1], "test-stress") == 0) {
        const char *job_name = "all";
        const char *stressor_name = NULL;
        int list_jobs = 0;
        int quick = 0;
        int duration_sec = 10;
        int num_workers = 1;
        uint64_t max_ops = 0;
        const char *method_name = NULL;
        static char extra_opts_buf[512];
        int extra_opts_len = 0;
        int single_mode = 0;

        extra_opts_buf[0] = '\0';

        for (int i = 2; i < argc; i++) {
            const char *val;

            if (strcmp(argv[i], "--job") == 0 && (i + 1 < argc)) {
                job_name = argv[++i];
            } else if ((val = test_stress_parse_opt_arg(argv[i], "--job=")) != NULL) {
                job_name = val;
            } else if (strcmp(argv[i], "-s") == 0 && (i + 1 < argc)) {
                single_mode = 1;
                stressor_name = argv[++i];
            } else if ((val = test_stress_parse_opt_arg(argv[i], "-s=")) != NULL) {
                single_mode = 1;
                stressor_name = val;
            } else if (strcmp(argv[i], "-t") == 0 && (i + 1 < argc)) {
                duration_sec = atoi(argv[++i]);
            } else if ((val = test_stress_parse_opt_arg(argv[i], "-t=")) != NULL) {
                duration_sec = atoi(val);
            } else if (strcmp(argv[i], "-c") == 0 && (i + 1 < argc)) {
                num_workers = atoi(argv[++i]);
            } else if ((val = test_stress_parse_opt_arg(argv[i], "-c=")) != NULL) {
                num_workers = atoi(val);
            } else if (strcmp(argv[i], "--ops") == 0 && (i + 1 < argc)) {
                max_ops = strtoull(argv[++i], NULL, 10);
            } else if ((val = test_stress_parse_opt_arg(argv[i], "--ops=")) != NULL) {
                max_ops = strtoull(val, NULL, 10);
            } else if (strcmp(argv[i], "--method") == 0 && (i + 1 < argc)) {
                method_name = argv[++i];
            } else if ((val = test_stress_parse_opt_arg(argv[i], "--method=")) != NULL) {
                method_name = val;
            } else if (strcmp(argv[i], "--opts") == 0 && (i + 1 < argc)) {
                const char *optstr = argv[++i];
                int optstr_len = strlen(optstr);
                if (extra_opts_len + optstr_len + 2 < (int)sizeof(extra_opts_buf)) {
                    if (extra_opts_len > 0) {
                        extra_opts_buf[extra_opts_len++] = ' ';
                    }
                    strcpy(extra_opts_buf + extra_opts_len, optstr);
                    extra_opts_len += optstr_len;
                }
            } else if (strcmp(argv[i], "-l") == 0 ||
                       strcmp(argv[i], "--list") == 0) {
                list_jobs = 1;
            } else if (strncmp(argv[i], "--", 2) == 0) {
                int arg_len = strlen(argv[i]);
                if (strchr(argv[i], '=') == NULL && i + 1 < argc && argv[i+1][0] != '-') {
                    arg_len += 1 + strlen(argv[i+1]);
                }
                if (extra_opts_len + arg_len + 2 < (int)sizeof(extra_opts_buf)) {
                    if (extra_opts_len > 0) {
                        extra_opts_buf[extra_opts_len++] = ' ';
                    }
                    strcpy(extra_opts_buf + extra_opts_len, argv[i]);
                    extra_opts_len += strlen(argv[i]);
                    if (strchr(argv[i], '=') == NULL && i + 1 < argc && argv[i+1][0] != '-') {
                        extra_opts_buf[extra_opts_len++] = ' ';
                        strcpy(extra_opts_buf + extra_opts_len, argv[++i]);
                        extra_opts_len += strlen(argv[i]);
                    }
                }
            } else if (strcmp(argv[i], "--quick") == 0) {
                quick = 1;
            } else if (strcmp(argv[i], "-q") == 0) {
                benchmark_verbosity = LOG_LEVEL_INFO;
            }
        }

        const char *extra_opts = (extra_opts_len > 0) ? extra_opts_buf : NULL;

        if (list_jobs) {
            test_stress_list_jobs();
            test_stress_list_stressors();
            return 0;
        }

        /* Validate single stressor mode */
        if (single_mode && !stressor_name) {
            printf("[test-stress] Error: -s requires a stressor name\n");
            return -1;
        }

        /* In quick mode, append "-quick" to job name if not already a quick variant */
        static char quick_job_buf[64];
        if (quick && !single_mode && strstr(job_name, "-quick") == NULL) {
            snprintf(quick_job_buf, sizeof(quick_job_buf), "%s-quick", job_name);
            job_name = quick_job_buf;
        }

        if (single_mode && stressor_name) {
            printf("  Mode: Single stressor\n");
            return test_stress_run_single(stressor_name, duration_sec,
                                          num_workers, max_ops,
                                          method_name, extra_opts);
        } else {
            printf("  Job: %s\n", job_name);
            return test_stress_run_job(job_name);
        }
    }

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

    /* Intercept unknown subcommands before falling through to workload mode */
    if (argc >= 2 && argv[1][0] != '-') {
        printf("\n");
        printf("******************************************************\n");
        printf("* ERROR: Unknown command '%s'\n", argv[1]);
        printf("******************************************************\n");
        print_usage();
        return -1;
    }

    /* Standard workload execution */
    struct execution_options opts;
    set_default_exec_opts(&opts);
    int help_requested = 0;
    parse_args(argc, argv, &opts, &help_requested);
    if (help_requested) {
        return 0;
    }

    if (opts.list_only) {
        printf("Available workloads (%d):\n", rtosbench_workload_count());
        for (int j = 0; j < rtosbench_workload_count(); j++) {
            const struct rtosbench_workload *wl =
                rtosbench_get_workload(j);
            printf("  %s [%s] - %s\n",
                   (wl && wl->name) ? wl->name : "(null)",
                   (wl && wl->category) ? wl->category : "-",
                   (wl && wl->description) ? wl->description : "");
        }
        return 0;
    }

    /* Build selection list */
    const struct rtosbench_workload *selected[RTOSBENCH_MAX_WORKLOADS];
    int selected_num = 0;

    if (opts.workload_name && (opts.run_all_workloads || opts.category_filter)) {
        printf("Cannot mix -b with -A/-G\n");
        return -1;
    }

    if (opts.run_all_workloads || opts.category_filter) {
        char *cats_buf = NULL;
        char *cats[RTOSBENCH_MAX_WORKLOADS];
        int cat_count = 0;
        if (opts.category_filter) {
            cats_buf = strdup(opts.category_filter);
            char *tok = strtok(cats_buf, ",");
            while (tok && cat_count < RTOSBENCH_MAX_WORKLOADS) {
                cats[cat_count++] = tok;
                tok = strtok(NULL, ",");
            }
        }
        for (int j = 0; j < rtosbench_workload_count(); j++) {
            const struct rtosbench_workload *wl =
                rtosbench_get_workload(j);
            if (wl == NULL) {
                continue;
            }
            if (opts.run_all_workloads) {
                selected[selected_num++] = wl;
            } else if (cat_count > 0 && wl->category) {
                for (int k = 0; k < cat_count; k++) {
                    if (ci_equal(wl->category, cats[k]) == 0) {
                        selected[selected_num++] = wl;
                        break;
                    }
                }
            }
        }
        if (cats_buf) {
            free(cats_buf);
        }
    } else if (opts.workload_name) {
        for (int j = 0; j < rtosbench_workload_count(); j++) {
            const struct rtosbench_workload *wl =
                rtosbench_get_workload(j);
            if (wl && wl->name &&
                strcmp(wl->name, opts.workload_name) == 0) {
                selected[selected_num++] = wl;
                break;
            }
        }
    } else {
        selected[selected_num++] = rtosbench_get_workload(0);
    }

    if (selected_num == 0) {
        printf("No workload selected.\n");
        return -1;
    }

    int ret = 0;
    for (int i = 0; i < selected_num; i++) {
        const struct rtosbench_workload *wl = selected[i];
        if (!wl || !wl->name) {
            continue;
        }
        rtosbench_select_workload(wl->name);
        debug_print_context(&opts);
        ret = periodic_benchmark(&opts);
        if (ret != 0) {
            break;
        }
    }
    return ret;
}

/* Register shell command */
SH_CMD_EXPORT(rtbench, cmd_rtbench, "RTOS-Bench workload runner");

int cmd_rtbench_stub(int argc, char **argv) {
    return cmd_rtbench(argc, argv);
}

/* ============================================================================
 * Result Collection Functions
 * ============================================================================ */

/* External references to realtime benchmark results (bench_init.c) */
extern uint64_t *get_realtime_service_cost(void);
extern uint64_t *get_realtime_interrupt(void);
extern uint64_t  get_realtime_context_switch(void);
extern uint64_t *get_realtime_syscall(void);
extern uint64_t *get_multicore_memory_bandwidth(void);
extern uint64_t *get_multicore_ipc_bandwidth(void);
extern uint64_t *get_multicore_intra_inter_bandwidth(void);
extern uint64_t *get_multicore_init_dlt_latency(void);

/* Convert nanoseconds to microseconds */
#define NS_TO_US(ns) ((double)(ns) / 1000.0)

/* Convert raw bench_init.c value to GB/s (values stored as x1000 representation) */
#define RAW_TO_GBS(v) ((double)(v) / 1000.0)

static void collect_realtime_result(int run_multicore)
{
    struct rtbench_result *r = rtbench_result_get();
    struct rtbench_realtime_result *rt = &r->realtime;

    rt->valid = 1;
    rt->multicore_valid = run_multicore ? 1 : 0;

    /* Read actual measured values from bench_init.c */
    uint64_t ctx_sw = get_realtime_context_switch();
    uint64_t *interrupt = get_realtime_interrupt();
    uint64_t *syscall = get_realtime_syscall();
    uint64_t *svc = get_realtime_service_cost();

    /* Single-core latency metrics */
    rt->context_switch_avg_us = NS_TO_US(ctx_sw);
    rt->interrupt_min_us = NS_TO_US(interrupt[0]);
    rt->interrupt_max_us = NS_TO_US(interrupt[1]);
    rt->interrupt_avg_us = NS_TO_US(interrupt[2]);
    rt->syscall_min_us = NS_TO_US(syscall[0]);
    rt->syscall_max_us = NS_TO_US(syscall[1]);
    rt->syscall_avg_us = NS_TO_US(syscall[2]);

    /* Service cost: svc is uint64_t[8][4], row=op, col=scenario */
    static const char *svc_ops[] = {
        "sem_take", "sem_release",
        "mq_send", "mq_recv",
        "mutex_take", "mutex_release",
        "mempool_alloc", "mempool_free"
    };
    for (int i = 0; i < 8; i++) {
        double immediate = NS_TO_US(svc[i * 4 + 0]);
        double suspend   = NS_TO_US(svc[i * 4 + 1]);
        double low_prio  = NS_TO_US(svc[i * 4 + 2]);
        double high_prio = NS_TO_US(svc[i * 4 + 3]);
        rtbench_realtime_add_service_cost(rt, svc_ops[i],
            immediate, suspend, low_prio, high_prio);
    }

    /* Multi-core metrics */
    if (run_multicore) {
        uint64_t *mem_bw = get_multicore_memory_bandwidth();
        uint64_t *ipc_bw = get_multicore_ipc_bandwidth();
        uint64_t *intra_inter = get_multicore_intra_inter_bandwidth();
        uint64_t *task_lat = get_multicore_init_dlt_latency();

        static const char *mem_types[] = {
            "rd", "wr", "cp", "frd", "fwr", "fcp", "memset", "memcpy"
        };
        for (int i = 0; i < 8; i++) {
            rtbench_realtime_add_mem_bw(rt, mem_types[i],
                RAW_TO_GBS(mem_bw[i * 4 + 0]),
                RAW_TO_GBS(mem_bw[i * 4 + 1]),
                RAW_TO_GBS(mem_bw[i * 4 + 2]),
                RAW_TO_GBS(mem_bw[i * 4 + 3]));
        }

        rt->ipc_bw_c1 = RAW_TO_GBS(ipc_bw[0]);
        rt->ipc_bw_c2 = RAW_TO_GBS(ipc_bw[1]);
        rt->ipc_bw_c4 = RAW_TO_GBS(ipc_bw[2]);
        rt->ipc_bw_c8 = RAW_TO_GBS(ipc_bw[3]);

        rt->core_comm_intra = RAW_TO_GBS(intra_inter[0]);
        rt->core_comm_inter = RAW_TO_GBS(intra_inter[1]);

        rt->task_lat_c1 = NS_TO_US(task_lat[0]);
        rt->task_lat_c2 = NS_TO_US(task_lat[1]);
        rt->task_lat_c4 = NS_TO_US(task_lat[2]);
        rt->task_lat_c8 = NS_TO_US(task_lat[3]);
    }
}

static void collect_schedule_result(void)
{
    struct rtbench_result *r = rtbench_result_get();
    struct rtbench_schedule_result *sched = &r->schedule;

    const struct test_schedule_result *ts_result = test_schedule_get_result();

    sched->valid = 1;
    sched->cycles = TEST_SCHEDULE_CYCLES;
    sched->util_start = TEST_SCHEDULE_UTIL_START;
    sched->util_end = TEST_SCHEDULE_UTIL_END;
    sched->util_step = TEST_SCHEDULE_UTIL_STEP;

    sched->average_miss_rate = ts_result->average_miss_rate;
    sched->final_score = ts_result->final_score;

    sched->gradient_count = ts_result->num_gradients;
    for (int i = 0; i < ts_result->num_gradients && i < RTBENCH_MAX_GRADIENTS; i++) {
        struct rtbench_gradient_result *dst = &sched->gradients[i];
        const struct schedule_gradient_result *src = &ts_result->gradients[i];

        dst->utilization_percent = src->utilization_percent;
        dst->actual_utilization = src->actual_utilization;
        dst->total_jobs = src->total_jobs;
        dst->deadline_misses = src->total_misses;
        dst->miss_rate = src->miss_rate;
        dst->task_count = src->num_tasks;

        for (int j = 0; j < src->num_tasks && j < RTBENCH_MAX_WORKLOADS; j++) {
            struct rtbench_task_stat *tdst = &dst->task_stats[j];
            const struct schedule_task_stats *tsrc = &src->task_stats[j];

            if (tsrc->name) {
                strncpy(tdst->name, tsrc->name, sizeof(tdst->name) - 1);
            }
            tdst->jobs = tsrc->total_jobs;
            tdst->misses = tsrc->deadline_misses;
            if (tsrc->total_jobs > 0) {
                tdst->max_response_ms = (double)tsrc->max_response_ns / 1000000.0;
            }
        }
    }
}

static void collect_stress_result(const char *job_name)
{
    struct rtbench_result *r = rtbench_result_get();
    struct rtbench_stress_result *stress = &r->stress;

    int count = 0;
    const struct test_stress_job_result *results = test_stress_get_job_results(&count);

    stress->valid = 1;

    double total_dur = 0;
    for (int i = 0; i < count; i++) {
        total_dur += results[i].duration_sec;
    }
    stress->duration_sec = total_dur;

    for (int i = 0; i < count; i++) {
        const struct test_stress_job_result *jr = &results[i];
        double metric_val = 0;
        const char *metric_unit = "";

        if (jr->metric_value > 0.00001 && jr->metric_unit[0] != '\0') {
            metric_val = jr->metric_value;
            metric_unit = jr->metric_unit;
        }

        rtbench_stress_add_stressor(stress,
            jr->name, jr->type, jr->stage,
            jr->bogo_ops, jr->duration_sec,
            metric_val, metric_unit);
    }

    (void)job_name;
}

static void collect_cmd_result(void)
{
    struct rtbench_result *r = rtbench_result_get();
    const struct test_cmd_module_result *src = test_cmd_get_result();

    if (!src || !src->valid) {
        return;
    }

    struct rtbench_cmd_module_result *dst = &r->cmd;
    dst->valid = 1;
    dst->cmd_count = src->cmd_count;
    dst->pass_count = src->pass_count;

    for (int i = 0; i < src->cmd_count && i < RTBENCH_MAX_CMD_COMMANDS; i++) {
        strncpy(dst->results[i].command, src->results[i].command,
                sizeof(dst->results[i].command) - 1);
        strncpy(dst->results[i].name, src->results[i].name,
                sizeof(dst->results[i].name) - 1);
        dst->results[i].supported = src->results[i].supported;
    }
}

static void collect_workload_results(int quick_mode)
{
    struct rtbench_result *r = rtbench_result_get();
    struct rtbench_workload_module_result *wl = &r->workload;

    wl->valid = 1;

    int count = rtosbench_workload_count();
    for (int i = 0; i < count && wl->workload_count < RTBENCH_MAX_WORKLOADS; i++) {
        const struct rtosbench_workload *w = rtosbench_get_workload(i);
        if (!w || !w->name || !w->exec) {
            continue;
        }

        /* Skip busywait and synthetic workloads for typical workload test */
        if (w->category && strcmp(w->category, "synthetic") == 0) {
            continue;
        }

        printf("  Running workload: %s\n", w->name);

        /* Initialize */
        if (w->init) {
            w->init(0, NULL);
        }

        /* Run workload and measure time using OneOS ticks */
        int rounds = quick_mode ? 5 : 10;
        os_tick_t start_tick = RTBENCH_GET_TICK();

        for (int j = 0; j < rounds; j++) {
            w->exec(0, NULL);
        }

        os_tick_t end_tick = RTBENCH_GET_TICK();
        double exec_time_ms = (double)(end_tick - start_tick) * 1000.0 / OS_TICK_PER_SECOND;
        double avg_time_ms = exec_time_ms / rounds;

        /* Teardown */
        if (w->teardown) {
            w->teardown(0, NULL);
        }

        /* Store result */
        rtbench_workload_add_result(wl, w->name,
                                    w->category ? w->category : "",
                                    1, rounds, exec_time_ms, avg_time_ms);

        printf("    %s: %.3f ms total, %.3f ms avg\n",
               w->name, exec_time_ms, avg_time_ms);
    }
}
