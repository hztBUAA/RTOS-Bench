-/**
 * @file dongtu_entry.c
 * @brief RTOS-Bench entry point for Dongtu (Intewell) RTOS
 *
 * Dongtu RTOS provides partial POSIX support. This entry uses the available
 * POSIX-lite interfaces and integrates with Dongtu's shell system.
 *
 * Usage from Dongtu shell:
 *   rtbench -l                    List available workloads
 *   rtbench -b pid -p 0.1 -t 10   Run PID workload
 *   rtbench -A                    Run all workloads
 *   rtbench test-schedule         Run schedulability test
 *   rtbench test-realtime         Run realtime performance test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "platform_abstraction.h"
#include "periodic_benchmark.h"
#include "logging.h"
#include "workload_registry.h"
#include "test_schedule.h"
#include "test_realtime.h"
#include "test_stress.h"
#include "test_cmd.h"

/* Forward declarations */
extern void rtosbench_register_rtos_workloads(void);

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

static void parse_args(int argc, char **argv, struct execution_options *opts)
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

/**
 * @brief Shell command entry point for Dongtu RTOS
 *
 * This function can be registered as a shell command in Dongtu's shell system.
 * The exact registration method depends on the Dongtu version and configuration.
 */
int rtbench_dongtu_entry(int argc, char **argv)
{
	struct execution_options opts;

	/* Register workloads on first call */
	static int initialized = 0;
	if (!initialized) {
		rtosbench_register_rtos_workloads();
		initialized = 1;
	}

	/* Handle test-schedule subcommand */
	if (argc >= 2 && strcmp(argv[1], "test-schedule") == 0) {
		int cycles = TEST_SCHEDULE_CYCLES;
		int util_start = TEST_SCHEDULE_UTIL_START;
		int util_end = TEST_SCHEDULE_UTIL_END;
		int util_step = TEST_SCHEDULE_UTIL_STEP;

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

		printf("[test-schedule] Starting schedulability test on Dongtu\n");
		printf("  Cycles: %d, Utilization: %d%% - %d%% (step %d%%)\n",
		       cycles, util_start, util_end, util_step);

		return test_schedule_run_custom(cycles, util_start, util_end, util_step);
	}

	/* Handle test-realtime subcommand */
	if (argc >= 2 && strcmp(argv[1], "test-realtime") == 0) {
		int run_multicore = 0;

		for (int i = 2; i < argc; i++) {
			if (strcmp(argv[i], "--multicore") == 0 ||
			    strcmp(argv[i], "-m") == 0) {
				run_multicore = 1;
			} else if (strcmp(argv[i], "-q") == 0) {
				benchmark_verbosity = LOG_LEVEL_INFO;
			}
		}

		printf("[test-realtime] Starting realtime performance test on Dongtu\n");
		if (run_multicore) {
			printf("  Multicore tests: enabled\n");
		}

		return test_realtime_run(run_multicore);
	}

	/* Handle test-stress subcommand */
	if (argc >= 2 && strcmp(argv[1], "test-stress") == 0) {
		const char *stressor = "cpu";
		int duration = 10;
		int num_workers = 1;        /* Default 1 worker */
		uint64_t max_ops = 0;       /* Default no limit */
		const char *method_name = NULL;   /* --method parameter */
		static char extra_opts_buf[512];    /* Extra stressor-specific options buffer */
		int extra_opts_len = 0;
		int list_stressors = 0;

		extra_opts_buf[0] = '\0';

		for (int i = 2; i < argc; i++) {
			const char *val;
			
			if (strcmp(argv[i], "-s") == 0 && (i +1 < argc)) {
				stressor = argv[++i];
			} else if ((val = test_stress_parse_opt_arg(argv[i], "-s=")) != NULL) {
				stressor = val;
			} else if (strcmp(argv[i], "-t") == 0 && (i +1 < argc)) {
				duration = atoi(argv[++i]);
			} else if ((val = test_stress_parse_opt_arg(argv[i], "-t=")) != NULL) {
				duration = atoi(val);
			} else if (strcmp(argv[i], "-c") == 0 && (i +1 < argc)) {
				num_workers = atoi(argv[++i]);
			} else if ((val = test_stress_parse_opt_arg(argv[i], "-c=")) != NULL) {
				num_workers = atoi(val);
			} else if (strcmp(argv[i], "--ops") == 0 && (i +1 < argc)) {
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
				list_stressors = 1;
			} else if (strcmp(argv[i], "-q") == 0) {
				benchmark_verbosity = LOG_LEVEL_INFO;
			}
		}

		const char *extra_opts = (extra_opts_len > 0) ? extra_opts_buf : NULL;

		if (list_stressors) {
			test_stress_list_jobs();
			test_stress_list_stressors();
			return 0;
		}

		printf("  Mode: Single stressor\n");

		return test_stress_run_single(stressor, duration, num_workers, max_ops,
		                              method_name, extra_opts);
	}

	/* Handle test-cmd subcommand */
	if (argc >= 2 && strcmp(argv[1], "test-cmd") == 0) {
		for (int i = 2; i < argc; i++) {
			if (strcmp(argv[i], "-q") == 0) {
				benchmark_verbosity = LOG_LEVEL_INFO;
			}
		}
		printf("[test-cmd] Starting shell command support test on Dongtu\n");
		return test_cmd_run();
	}

	/* Standard workload execution */
	set_default_exec_opts(&opts);
	parse_args(argc, argv, &opts);

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
		char cats_buf[256];
		char *cats[RTOSBENCH_MAX_WORKLOADS];
		int cat_count = 0;
		if (opts.category_filter) {
			strncpy(cats_buf, opts.category_filter, sizeof(cats_buf) - 1);
			cats_buf[sizeof(cats_buf) - 1] = '\0';
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

/**
 * @brief Convenience wrapper for main()-style entry
 *
 * Use this if Dongtu supports standard main() entry point.
 */
int main(int argc, char **argv)
{
	return rtbench_dongtu_entry(argc, argv);
}
