#ifndef _CLOCK_T_DECLARED
typedef unsigned long clock_t;
#define _CLOCK_T_DECLARED
#endif
#ifndef _SUSECONDS_T_DECLARED
typedef long suseconds_t;
#define _SUSECONDS_T_DECLARED
#endif
#ifndef _CLOCKID_T_DECLARED
typedef int clockid_t;
#define _CLOCKID_T_DECLARED
#endif

#ifdef RT_THREAD_PLATFORM

#include "periodic_benchmark.h"
#include "logging.h"
#include "workload_registry.h"
#include "test_schedule.h"
#include "test_realtime.h"
#include "test_stress.h"
#include "test_cmd.h"
#include "result_export.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <rtthread.h>
#include <rtsched.h>

/* Stringify RT-Thread version from rtdef.h macros */
#define _RTBENCH_STR(x) #x
#define _RTBENCH_XSTR(x) _RTBENCH_STR(x)
#define RT_VERSION_STRING \
    _RTBENCH_XSTR(RT_VERSION_MAJOR) "." \
    _RTBENCH_XSTR(RT_VERSION_MINOR) "." \
    _RTBENCH_XSTR(RT_VERSION_PATCH)

/* Minimal RT-Thread entry point to avoid argp/perf dependencies.
 * Usage: rtosbench [-p <period_sec>] [-t <tasks>] [-f <prio>] [-c <cpu>] [-b <workload>]
 *        rtosbench test-schedule [--cycles <n>] [--util-start <pct>] [--util-end <pct>]
 *        rtosbench test-all [-o <output_path>] [--no-realtime] [--no-schedule] [--no-stress] [--no-cmd]
 * Defaults: period 1s, run until SIGINT, skip priority/affinity changes.
 */

/* Default result output path */
#define RTBENCH_DEFAULT_OUTPUT_PATH "/rtbench_result.json"

/* Stack size for worker threads (32KB to handle deep call chains, avoids tshell 4KB overflow) */
#define RTBENCH_TEST_ALL_STACK_SIZE (32 * 1024)
#define RTBENCH_TEST_STRESS_STACK_SIZE (32 * 1024)

/* Register packaged workloads (must be linked in) */
void rtosbench_register_rtos_workloads(void);

/* Forward declarations for result collection */
static void collect_realtime_result(int run_multicore);
static void collect_schedule_result(void);
static void collect_stress_result(const char *job_name);
static void collect_cmd_result(void);
static void collect_workload_results(int quick_mode);

static void set_default_exec_opts(struct execution_options *opts)
{
	memset(opts, 0, sizeof(*opts));
	opts->period_sec = 1;
	opts->period_nsec = 0;
	opts->deadline_sec = 0;
	opts->deadline_nsec = 0;
	opts->tasks_to_launch = 1;   /* default: run one job */
	opts->prio = 100;            /* 100 means skip set_priority (matches main.c default) */
	opts->runtime = 0;
	opts->deadline = 0;
	opts->period = 0;
	opts->memory_profiling_enable = 0;
	CPU_ZERO(&opts->core_affinity);
	CPU_ZERO(&opts->memory_profiling_core_affinity);
	opts->workload_name = NULL;
	opts->category_filter = NULL;
	opts->run_all_workloads = 0;
	opts->list_only = 0;
	/* 默认打开 TRACE，定位问题；若要安静可通过 -q 下调 */
	benchmark_verbosity = LOG_LEVEL_TRACE;
}

static void parse_rtthread_args(int argc, char **argv,
				struct execution_options *opts)
{
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-p") && (i + 1 < argc)) {
			double v = atof(argv[++i]);
			long sec = (long)v;
			long nsec = (long)((v - (double)sec) * 1000000000.0);
			opts->period_sec = sec;
			opts->period_nsec = nsec;
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
		} else if (!strcmp(argv[i], "-A")) {
			opts->run_all_workloads = 1;
		} else if (!strcmp(argv[i], "-G") && (i + 1 < argc)) {
			opts->category_filter = argv[++i];
		} else if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--list")) {
			opts->list_only = 1;
		}
	}
}

static void debug_print_context(const struct execution_options *opts)
{
	rt_thread_t self = rt_thread_self();
	void *sp = NULL;
#if defined(__aarch64__)
	asm volatile("mov %0, sp" : "=r"(sp));
#endif
	rt_kprintf("[rtbench] thread=%s prio=%d sp=%p\n",
		   self ? self->parent.name : "NULL",
		   self ? (int)RT_SCHED_PRIV(self).current_priority : -1,
		   sp);
	rt_kprintf("[rtbench] workload=%s period=%ld.%09ld tasks=%llu\n",
		   rtosbench_current_workload(),
		   opts->period_sec, opts->period_nsec,
		   (unsigned long long)opts->tasks_to_launch);
}

/* ============================================================================
 * test-stress worker thread (avoids tshell 4KB stack overflow)
 * ============================================================================ */

struct test_stress_params {
	const char *job_name;
	const char *stressor_name;  /* For single stressor mode (-s) */
	int duration_sec;           /* -t parameter */
	int num_workers;            /* -c parameter */
	uint64_t max_ops;           /* --ops parameter */
	const char *method_name;    /* --method parameter */
	const char *extra_opts;     /* Extra stressor-specific options */
	int single_mode;            /* 1 = run single stressor, 0 = run job */
	int result;
	struct rt_semaphore done_sem;
};

static void test_stress_thread_entry(void *parameter)
{
	struct test_stress_params *p = (struct test_stress_params *)parameter;

	if (p->single_mode && p->stressor_name) {
		rt_kprintf("  Mode: Single stressor\n");
		p->result = test_stress_run_single(p->stressor_name, p->duration_sec,
		                                   p->num_workers, p->max_ops,
		                                   p->method_name, p->extra_opts);
	} else {
		rt_kprintf("  Job: %s\n", p->job_name);
		p->result = test_stress_run_job(p->job_name);
	}

	rt_sem_release(&p->done_sem);
}

/* ============================================================================
 * test-all worker thread (avoids tshell 4KB stack overflow)
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
	struct rt_semaphore done_sem;
};

static void test_all_thread_entry(void *parameter)
{
	struct test_all_params *p = (struct test_all_params *)parameter;

	/* Initialize result collection */
	rtbench_result_init();
	rtbench_result_set_env("RT-Thread", RT_VERSION_STRING, "QEMU-virt-aarch64",
	                       "cortex-a53", 0, RT_CPUS_NR);
	rtbench_result_start();

	/* Register workloads */
	rtosbench_register_rtos_workloads();

	rt_kprintf("\n");
	rt_kprintf("=============================================================\n");
	rt_kprintf("[RTOS-Bench] Comprehensive Test Suite\n");
	rt_kprintf("=============================================================\n");
	rt_kprintf("Output: %s\n", p->output_path);
	rt_kprintf("Modules: realtime=%s schedule=%s stress=%s cmd=%s workload=%s\n",
	           p->run_realtime ? "yes" : "no",
	           p->run_schedule ? "yes" : "no",
	           p->run_stress ? "yes" : "no",
	           p->run_cmd ? "yes" : "no",
	           p->run_workload ? "yes" : "no");
	if (p->quick_mode) {
		rt_kprintf("Mode: QUICK (smoke test)\n");
	}
	rt_kprintf("\n");

	/* Run realtime test */
	if (p->run_realtime) {
		rt_kprintf(">>> Running test-realtime...\n");
		test_realtime_run(p->run_multicore);
		collect_realtime_result(p->run_multicore);
	}

	/* Run schedule test */
	if (p->run_schedule) {
		rt_kprintf("\n>>> Running test-schedule%s...\n",
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
		rt_kprintf("\n>>> Running test-stress (job: %s)...\n", stress_job);
		test_stress_run_job(stress_job);
		collect_stress_result(stress_job);
	}

	/* Run command support test */
	if (p->run_cmd) {
		rt_kprintf("\n>>> Running test-cmd...\n");
		test_cmd_run();
		collect_cmd_result();
	}

	/* Run workload tests */
	if (p->run_workload) {
		rt_kprintf("\n>>> Running typical workloads%s...\n",
		           p->quick_mode ? " (quick)" : "");
		collect_workload_results(p->quick_mode);
	}

	/* Finalize and export */
	rtbench_result_end();
	p->result = rtbench_result_export_json(p->output_path);
	if (p->result == 0) {
		rt_kprintf("\n=============================================================\n");
		rt_kprintf("[RTOS-Bench] Results saved to: %s\n", p->output_path);
		rt_kprintf("=============================================================\n");
	} else {
		rt_kprintf("\n[RTOS-Bench] Failed to save results: %d\n", p->result);
	}

	rtbench_result_cleanup();

	/* Signal completion */
	rt_sem_release(&p->done_sem);
}

int rtosbench_rtthread_entry(int argc, char **argv)
{
	struct execution_options opts;
	const char *output_path = NULL;

	/* Check for test-all subcommand - comprehensive test suite */
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

		/* Initialize completion semaphore */
		rt_sem_init(&params.done_sem, "ta_done", 0, RT_IPC_FLAG_PRIO);

		/* Spawn worker thread with large stack to avoid tshell stack overflow */
		rt_thread_t t = rt_thread_create("rtbench",
		                                  test_all_thread_entry,
		                                  &params,
		                                  RTBENCH_TEST_ALL_STACK_SIZE,
		                                  20, 10);
		if (t == RT_NULL) {
			rt_kprintf("[RTOS-Bench] Failed to create test-all worker thread\n");
			rt_sem_detach(&params.done_sem);
			return -1;
		}
		rt_thread_startup(t);

		/* Wait for worker to finish */
		rt_sem_take(&params.done_sem, RT_WAITING_FOREVER);
		rt_sem_detach(&params.done_sem);

		return params.result;
	}

	/* Check for export-result subcommand */
	if (argc >= 2 && strcmp(argv[1], "export-result") == 0) {
		output_path = RTBENCH_DEFAULT_OUTPUT_PATH;
		for (int i = 2; i < argc; i++) {
			if ((strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) && (i + 1 < argc)) {
				output_path = argv[++i];
			}
		}
		int ret = rtbench_result_export_json(output_path);
		if (ret == 0) {
			rt_kprintf("[RTOS-Bench] Results exported to: %s\n", output_path);
		}
		return ret;
	}

	/* Check for test-schedule subcommand */
	if (argc >= 2 && strcmp(argv[1], "test-schedule") == 0) {
		int cycles = TEST_SCHEDULE_CYCLES;
		int util_start = TEST_SCHEDULE_UTIL_START;
		int util_end = TEST_SCHEDULE_UTIL_END;
		int util_step = TEST_SCHEDULE_UTIL_STEP;
		int quick = 0;

		/* Parse optional test-schedule arguments */
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

		/* Register workloads before running test */
		rtosbench_register_rtos_workloads();

		rt_kprintf("[test-schedule] Starting schedulability test%s\n",
			   quick ? " (quick)" : "");
		rt_kprintf("  Cycles: %d, Utilization: %d%% - %d%% (step %d%%)\n",
			   cycles, util_start, util_end, util_step);

		return test_schedule_run_custom(cycles, util_start, util_end, util_step);
	}

	/* Check for test-realtime subcommand */
	if (argc >= 2 && strcmp(argv[1], "test-realtime") == 0) {
		int run_multicore = 0;

		/* Parse optional test-realtime arguments */
		for (int i = 2; i < argc; i++) {
			if (strcmp(argv[i], "--multicore") == 0 ||
			    strcmp(argv[i], "-m") == 0) {
				run_multicore = 1;
			} else if (strcmp(argv[i], "-q") == 0) {
				benchmark_verbosity = LOG_LEVEL_INFO;
			}
		}

		rt_kprintf("[test-realtime] Starting realtime performance test\n");
		if (run_multicore) {
			rt_kprintf("  Multicore tests: enabled\n");
		}

		return test_realtime_run(run_multicore);
	}

	/* Check for test-stress subcommand */
	if (argc >= 2 && strcmp(argv[1], "test-stress") == 0) {
		const char *job_name = "all";
		const char *stressor_name = NULL;
		int list_jobs = 0;
		int quick = 0;
		int duration_sec = 10;      /* Default 10 seconds */
		int num_workers = 1;        /* Default 1 worker */
		uint64_t max_ops = 0;       /* Default no limit */
		const char *method_name = NULL;   /* --method parameter */
		static char extra_opts_buf[512];    /* Extra stressor-specific options buffer */
		int extra_opts_len = 0;
		int single_mode = 0;        /* Default to job mode */
	
		extra_opts_buf[0] = '\0';
	
		/* Parse optional test-stress arguments */
		for (int i = 2; i < argc; i++) {
			const char *val;
			
			if (strcmp(argv[i], "--job") == 0 && (i +1 < argc)) {
				job_name = argv[++i];
			} else if ((val = test_stress_parse_opt_arg(argv[i], "--job=")) != NULL) {
				job_name = val;
			} else if (strcmp(argv[i], "-s") == 0 && (i +1 < argc)) {
				/* Single stressor mode */
				single_mode = 1;
				stressor_name = argv[++i];
			} else if ((val = test_stress_parse_opt_arg(argv[i], "-s=")) != NULL) {
				single_mode = 1;
				stressor_name = val;
			} else if (strcmp(argv[i], "-t") == 0 && (i +1 < argc)) {
				/* Duration in seconds */
				duration_sec = atoi(argv[++i]);
			} else if ((val = test_stress_parse_opt_arg(argv[i], "-t=")) != NULL) {
				duration_sec = atoi(val);
			} else if (strcmp(argv[i], "-c") == 0 && (i +1 < argc)) {
				/* Number of workers */
				num_workers = atoi(argv[++i]);
			} else if ((val = test_stress_parse_opt_arg(argv[i], "-c=")) != NULL) {
				num_workers = atoi(val);
			} else if (strcmp(argv[i], "--ops") == 0 && (i +1 < argc)) {
				/* Maximum operations */
				max_ops = strtoull(argv[++i], NULL, 10);
			} else if ((val = test_stress_parse_opt_arg(argv[i], "--ops=")) != NULL) {
				max_ops = strtoull(val, NULL, 10);
			} else if (strcmp(argv[i], "--method") == 0 && (i +1 < argc)) {
				/* Method/algorithm name */
				method_name = argv[++i];
			} else if ((val = test_stress_parse_opt_arg(argv[i], "--method=")) != NULL) {
				/* Method with equals sign: --method=ackermann */
				method_name = val;
			} else if (strcmp(argv[i], "--opts") == 0 && (i +1 < argc)) {
				/* Extra stressor-specific options */
				const char *opts = argv[++i];
				int opts_len = strlen(opts);
				if (extra_opts_len + opts_len +2 < sizeof(extra_opts_buf)) {
					if (extra_opts_len > 0) {
						extra_opts_buf[extra_opts_len++] = ' ';
					}
					strcpy(extra_opts_buf + extra_opts_len, opts);
					extra_opts_len += opts_len;
				}
			} else if (strncmp(argv[i], "--", 2) == 0) {
				/* Unknown --option: collect it and its argument if present */
				/* Support both --opt value and --opt=value forms */
				int arg_len = strlen(argv[i]);
				if (strchr(argv[i], '=') == NULL && i +1 < argc && argv[i+1][0] != '-') {
					/* Has separate argument: --opt value */
					arg_len += 1 + strlen(argv[i+1]);
				}
				if (extra_opts_len + arg_len +2 < sizeof(extra_opts_buf)) {
					if (extra_opts_len > 0) {
						extra_opts_buf[extra_opts_len++] = ' ';
					}
					strcpy(extra_opts_buf + extra_opts_len, argv[i]);
					extra_opts_len += strlen(argv[i]);
					if (strchr(argv[i], '=') == NULL && i +1 < argc && argv[i+1][0] != '-') {
						extra_opts_buf[extra_opts_len++] = ' ';
						strcpy(extra_opts_buf + extra_opts_len, argv[++i]);
						extra_opts_len += strlen(argv[i]);
					}
				}
			} else if (strcmp(argv[i], "-l") == 0 ||
			           strcmp(argv[i], "--list") == 0) {
				list_jobs = 1;
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
			rt_kprintf("[test-stress] Error: -s requires a stressor name\n");
			return -1;
		}

		/* In quick mode, append "-quick" to job name if not already a quick variant */
		static char quick_job_buf[64];
		if (quick && strstr(job_name, "-quick") == NULL) {
			snprintf(quick_job_buf, sizeof(quick_job_buf), "%s-quick", job_name);
			job_name = quick_job_buf;
		}

		/* Spawn worker thread with large stack to avoid tshell stack overflow */
		static struct test_stress_params stress_params;
		memset(&stress_params, 0, sizeof(stress_params));
		stress_params.job_name = job_name;
		stress_params.stressor_name = stressor_name;
		stress_params.duration_sec = duration_sec;
		stress_params.num_workers = num_workers;
		stress_params.max_ops = max_ops;
		stress_params.method_name = method_name;
		stress_params.extra_opts = extra_opts;
		stress_params.single_mode = single_mode;

		rt_sem_init(&stress_params.done_sem, "ts_done", 0, RT_IPC_FLAG_PRIO);

		rt_thread_t t = rt_thread_create("ts_work",
		                                  test_stress_thread_entry,
		                                  &stress_params,
		                                  RTBENCH_TEST_STRESS_STACK_SIZE,
		                                 20, 10);
		if (t == RT_NULL) {
			rt_kprintf("[test-stress] Failed to create worker thread\n");
			rt_sem_detach(&stress_params.done_sem);
			return -1;
		}
		rt_thread_startup(t);

		/* Wait for worker to finish */
		rt_sem_take(&stress_params.done_sem, RT_WAITING_FOREVER);
		rt_sem_detach(&stress_params.done_sem);

		return stress_params.result;
	}

	/* Check for test-cmd subcommand */
	if (argc >= 2 && strcmp(argv[1], "test-cmd") == 0) {
		for (int i = 2; i < argc; i++) {
			if (strcmp(argv[i], "-q") == 0) {
				benchmark_verbosity = LOG_LEVEL_INFO;
			}
		}
		rt_kprintf("[test-cmd] Starting shell command support test\n");
		return test_cmd_run();
	}

	set_default_exec_opts(&opts);
	rtosbench_register_rtos_workloads();
	parse_rtthread_args(argc, argv, &opts);
	if (opts.list_only) {
		rt_kprintf("Available workloads:\n");
		for (int j = 0; j < rtosbench_workload_count(); j++) {
			const struct rtosbench_workload *wl =
				rtosbench_get_workload(j);
			rt_kprintf("  %s [%s] - %s\n",
				   wl && wl->name ? wl->name : "(null)",
				   (wl && wl->category) ? wl->category : "-",
				   (wl && wl->description) ? wl->description : "");
		}
		return 0;
	}

	/* Build selection list */
	const struct rtosbench_workload *selected[RTOSBENCH_MAX_WORKLOADS];
	int selected_num = 0;

	if (opts.workload_name && (opts.run_all_workloads || opts.category_filter)) {
		rt_kprintf("Cannot mix -b with -A/-G\n");
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
					if (strcasecmp(wl->category, cats[k]) == 0) {
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
		rt_kprintf("No workload selected.\n");
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

/* Legacy name for backward compatibility */
int rtbench_rtthread_entry(int argc, char **argv)
{
	return rtosbench_rtthread_entry(argc, argv);
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

	/* Service cost: svc is uint64_t[8][4], row=op, col=scenario
	 * Columns: [0]=immediate, [1]=suspend, [2]=low_prio, [3]=high_prio */
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

		/* Memory bandwidth: mem_bw is [8][4], row=type, col=concurrency */
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

		/* IPC bandwidth: [4] concurrency levels */
		rt->ipc_bw_c1 = RAW_TO_GBS(ipc_bw[0]);
		rt->ipc_bw_c2 = RAW_TO_GBS(ipc_bw[1]);
		rt->ipc_bw_c4 = RAW_TO_GBS(ipc_bw[2]);
		rt->ipc_bw_c8 = RAW_TO_GBS(ipc_bw[3]);

		/* Intra/inter core communication */
		rt->core_comm_intra = RAW_TO_GBS(intra_inter[0]);
		rt->core_comm_inter = RAW_TO_GBS(intra_inter[1]);

		/* Task create/delete latency: [4] concurrency levels, in us */
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

	/* Get result from test_schedule module */
	const struct test_schedule_result *ts_result = test_schedule_get_result();

	sched->valid = 1;
	sched->cycles = TEST_SCHEDULE_CYCLES;
	sched->util_start = TEST_SCHEDULE_UTIL_START;
	sched->util_end = TEST_SCHEDULE_UTIL_END;
	sched->util_step = TEST_SCHEDULE_UTIL_STEP;

	/* Copy summary */
	sched->average_miss_rate = ts_result->average_miss_rate;
	sched->final_score = ts_result->final_score;

	/* Copy gradient results */
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

		/* Copy task stats */
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

	/* Calculate total duration from individual results */
	double total_dur = 0;
	for (int i = 0; i < count; i++) {
		total_dur += results[i].duration_sec;
	}
	stress->duration_sec = total_dur;

	/* Add each stressor result */
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

	/* Iterate through registered workloads and run them */
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

		rt_kprintf("  Running workload: %s\n", w->name);

		/* Initialize */
		if (w->init) {
			w->init(0, NULL);
		}

		/* Run workload and measure time */
		int rounds = quick_mode ? 5 : 100;
		uint64_t start_tick = rt_tick_get();

		for (int j = 0; j < rounds; j++) {
			w->exec(0, NULL);
		}

		uint64_t end_tick = rt_tick_get();
		double exec_time_ms = (double)(end_tick - start_tick) * 1000.0 / RT_TICK_PER_SECOND;
		double avg_time_ms = exec_time_ms / rounds;

		/* Teardown */
		if (w->teardown) {
			w->teardown(0, NULL);
		}

		/* Store result */
		rtbench_workload_add_result(wl, w->name,
		                            w->category ? w->category : "",
		                            1, rounds, exec_time_ms, avg_time_ms);

		rt_kprintf("    %s: %.3f ms total, %.3f ms avg\n",
		           w->name, exec_time_ms, avg_time_ms);
	}
}

#endif /* RT_THREAD_PLATFORM */
