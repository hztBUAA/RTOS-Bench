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
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <rtthread.h>
#include <rtsched.h>

/* Minimal RT-Thread entry point to avoid argp/perf dependencies.
 * Usage: rtosbench [-p <period_sec>] [-t <tasks>] [-f <prio>] [-c <cpu>] [-b <workload>]
 *        rtosbench test-schedule [--cycles <n>] [--util-start <pct>] [--util-end <pct>]
 * Defaults: period 1s, run until SIGINT, skip priority/affinity changes.
 */

/* Register packaged workloads (must be linked in) */
void rtosbench_register_rtos_workloads(void);

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

int rtosbench_rtthread_entry(int argc, char **argv)
{
	struct execution_options opts;

	/* Check for test-schedule subcommand */
	if (argc >= 2 && strcmp(argv[1], "test-schedule") == 0) {
		int cycles = TEST_SCHEDULE_CYCLES;
		int util_start = TEST_SCHEDULE_UTIL_START;
		int util_end = TEST_SCHEDULE_UTIL_END;
		int util_step = TEST_SCHEDULE_UTIL_STEP;

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
			} else if (strcmp(argv[i], "-q") == 0) {
				benchmark_verbosity = LOG_LEVEL_INFO;
			}
		}

		/* Register workloads before running test */
		rtosbench_register_rtos_workloads();

		rt_kprintf("[test-schedule] Starting schedulability test\n");
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
		const char *stressor = "cpu";
		int duration = 10;
		int list_stressors = 0;

		/* Parse optional test-stress arguments */
		for (int i = 2; i < argc; i++) {
			if (strcmp(argv[i], "-s") == 0 && (i + 1 < argc)) {
				stressor = argv[++i];
			} else if (strcmp(argv[i], "-t") == 0 && (i + 1 < argc)) {
				duration = atoi(argv[++i]);
			} else if (strcmp(argv[i], "-l") == 0 ||
			           strcmp(argv[i], "--list") == 0) {
				list_stressors = 1;
			} else if (strcmp(argv[i], "-q") == 0) {
				benchmark_verbosity = LOG_LEVEL_INFO;
			}
		}

		if (list_stressors) {
			test_stress_list_stressors();
			return 0;
		}

		rt_kprintf("[test-stress] Starting stress/power test\n");
		rt_kprintf("  Stressor: %s, Duration: %d seconds\n", stressor, duration);

		if (strcmp(stressor, "all") == 0) {
			struct test_stress_config config = {
				.type = STRESS_TYPE_ALL,
				.duration_sec = duration,
				.num_workers = 1,
				.quiet = 0
			};
			return test_stress_run_config(&config);
		}

		return test_stress_run_stressor(stressor, duration);
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

#endif /* RT_THREAD_PLATFORM */
