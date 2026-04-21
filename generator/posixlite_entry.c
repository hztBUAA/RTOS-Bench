/**
 * @file posixlite_entry.c
 * @brief Generic POSIX-lite CLI entry point
 *
 * This is a generic entry point for systems with minimal POSIX support.
 * For specific RTOS platforms, use the dedicated entry files:
 *   - RT-Thread: rtthread_entry.c
 *   - SylixOS: sylixos_entry.c
 *   - OneOS: oneos_entry.c
 *   - Dongtu: dongtu_entry.c
 *   - Ruihua: ruihua_entry.c
 *
 * Avoids glibc argp/performance counters so it can build with minimal libc.
 * Supported options:
 *   -p <sec>   Period (seconds, float)
 *   -t <n>     Tasks to launch
 *   -f <prio>  FIFO priority (skip if 100, consistent with main.c default)
 *   -c <cpu>   Bind to CPU index
 *   -b <name>  Select workload by name
 *   -A         Run all workloads
 *   -G <cats>  Comma separated category filter
 *   -l|--list  List workloads and exit
 *   -q         Quiet logs (INFO)
 */

#include "periodic_benchmark.h"
#include "logging.h"
#include "workload_registry.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
	opts->prio = 100; /* 100 means "skip priority change" (see main.c) */
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

static void parse_posixlite_args(int argc, char **argv,
				 struct execution_options *opts)
{
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-p") && (i + 1 < argc)) {
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
			/* Alias to keep parity with Linux CLI */
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

int main(int argc, char **argv)
{
	struct execution_options opts;

	/* Intercept unsupported subcommands with clear error message */
	if (argc >= 2 && (strcmp(argv[1], "test-all") == 0 ||
	                  strcmp(argv[1], "test-realtime") == 0 ||
	                  strcmp(argv[1], "test-schedule") == 0 ||
	                  strcmp(argv[1], "test-stress") == 0 ||
	                  strcmp(argv[1], "test-cmd") == 0 ||
	                  strcmp(argv[1], "export-result") == 0)) {
		printf("\n");
		printf("******************************************************\n");
		printf("* ERROR: '%s' is not supported on this platform\n", argv[1]);
		printf("* This is a POSIX-lite entry with workload-only support.\n");
		printf("* Supported options: -b, -A, -G, -l, -p, -t, -f, -c, -q\n");
		printf("******************************************************\n");
		return -1;
	}
	/* Intercept other unknown subcommands */
	if (argc >= 2 && argv[1][0] != '-') {
		printf("\n");
		printf("******************************************************\n");
		printf("* ERROR: Unknown command '%s'\n", argv[1]);
		printf("******************************************************\n");
		return -1;
	}

	set_default_exec_opts(&opts);
	parse_posixlite_args(argc, argv, &opts);

	if (opts.list_only) {
		printf("Available workloads:\n");
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
