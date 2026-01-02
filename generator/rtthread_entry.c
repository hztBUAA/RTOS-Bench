#ifdef RT_THREAD_PLATFORM

#include "periodic_benchmark.h"
#include "logging.h"
#include <stdlib.h>
#include <string.h>

/* Minimal RT-Thread entry point to avoid argp/perf dependencies.
 * Usage: rtbench [-p <period_sec>] [-t <tasks>] [-f <prio>] [-c <cpu>]
 * Defaults: period 1s, run until SIGINT, skip priority/affinity changes.
 */

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
	/* Keep logging minimal on RT-Thread to avoid heavy stdio usage. */
	benchmark_verbosity = LOG_LEVEL_ERR;
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
		}
	}
}

int rtbench_rtthread_entry(int argc, char **argv)
{
	struct execution_options opts;

	set_default_exec_opts(&opts);
	parse_rtthread_args(argc, argv, &opts);
	return periodic_benchmark(&opts);
}

#endif /* RT_THREAD_PLATFORM */
