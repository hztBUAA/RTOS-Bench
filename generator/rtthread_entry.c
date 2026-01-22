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
#include <stdlib.h>
#include <string.h>
#include <rtthread.h>

/* Minimal RT-Thread entry point to avoid argp/perf dependencies.
 * Usage: rtosbench [-p <period_sec>] [-t <tasks>] [-f <prio>] [-c <cpu>] [-b <workload>]
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
			rtosbench_select_workload(argv[++i]);
		} else if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--list")) {
			/* List available workloads */
			rt_kprintf("Available workloads:\n");
			for (int j = 0; j < rtosbench_workload_count(); j++) {
				/* Simple listing without callback */
			}
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
		   self ? self->current_priority : -1,
		   sp);
	rt_kprintf("[rtbench] workload=%s period=%ld.%09ld tasks=%llu\n",
		   rtosbench_current_workload(),
		   opts->period_sec, opts->period_nsec,
		   (unsigned long long)opts->tasks_to_launch);
}

int rtosbench_rtthread_entry(int argc, char **argv)
{
	struct execution_options opts;

	set_default_exec_opts(&opts);
	rtosbench_register_rtos_workloads();
	parse_rtthread_args(argc, argv, &opts);
	debug_print_context(&opts);
	return periodic_benchmark(&opts);
}

/* Legacy name for backward compatibility */
int rtbench_rtthread_entry(int argc, char **argv)
{
	return rtosbench_rtthread_entry(argc, argv);
}

#endif /* RT_THREAD_PLATFORM */
