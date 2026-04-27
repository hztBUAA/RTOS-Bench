/**
 * @file sylixos_entry.c
 * @brief RTOS-Bench entry point for SylixOS
 *
 * SylixOS provides good POSIX compatibility, so this entry uses standard
 * POSIX interfaces. Shell commands are registered via SylixOS's module system.
 *
 * Usage from SylixOS shell:
 *   # rtbench -l                    List available workloads
 *   # rtbench -b pid -p 0.1 -t 10   Run PID workload
 *   # rtbench -A                    Run all workloads
 *   # rtbench test-schedule         Run schedulability test
 *   # rtbench test-realtime         Run realtime performance test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

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
__attribute__((weak)) int __fdlib_version = -1;

#define RTBENCH_PARSE_HELP 1

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

static void print_usage(void)
{
	printf("\nRTOS-Bench - SylixOS entry\n\n");
	printf("Usage:\n");
	printf("  rtbench [OPTIONS]                         Run a workload benchmark\n");
	printf("  rtbench test-schedule [OPTIONS]           Run schedulability test\n");
	printf("  rtbench test-realtime [OPTIONS]           Run realtime performance test\n");
	printf("  rtbench test-stress [OPTIONS]             Run stress test\n");
	printf("  rtbench test-cmd                          Run shell command support test\n");
	printf("  rtbench -h | --help                       Show this help message\n");
	printf("\nWorkload options:\n");
	printf("  -b, -w, --workload <name>                 Workload name (use -L to list)\n");
	printf("  -p, --period <sec>                        Period in seconds (default: 1.0)\n");
	printf("  -d, --deadline <sec>                      Deadline in seconds\n");
	printf("  -t, --tasks <count>                       Number of task activations\n");
	printf("  -f, --fifo <prio>                         SylixOS FIFO priority\n");
	printf("  -c, --core-affinity <cpu>                 CPU affinity (0-31)\n");
	printf("  -A, --all-workloads                       Run all registered workloads\n");
	printf("  -G, --category <cat[,cat2]>               Filter workloads by category\n");
	printf("  -L, -l, --list                            List available workloads\n");
	printf("  -s, --run-workload-suite                  Run simple workload suite\n");
	printf("  -q                                        Quiet mode\n");
	printf("\nTest-schedule options:\n");
	printf("  --cycles <n>                              Cycles per task (default: %d)\n",
	       TEST_SCHEDULE_CYCLES);
	printf("  --util-start <pct>                        Starting utilization (default: %d)\n",
	       TEST_SCHEDULE_UTIL_START);
	printf("  --util-end <pct>                          Ending utilization (default: %d)\n",
	       TEST_SCHEDULE_UTIL_END);
	printf("  --util-step <pct>                         Utilization step (default: %d)\n",
	       TEST_SCHEDULE_UTIL_STEP);
	printf("  --quick                                   Smoke-test schedule profile\n");
	printf("  -h, --help                                Show test-schedule help\n");
	printf("\nTest-realtime options:\n");
	printf("  -v, --verify                              Run realtime self-checks first\n");
	printf("  -m, --multicore                           Enable multicore tests\n");
	printf("  -q                                        Quiet mode\n");
	printf("\nTest-stress options:\n");
	printf("  --job <cpu|memory|file|all>               Run a predefined stress job\n");
	printf("  -s <stressor>                             Run a single stressor\n");
	printf("  -t <sec>                                  Duration in seconds\n");
	printf("  -c <workers>                              Number of workers\n");
	printf("  --ops <count>                             Maximum operations\n");
	printf("  --method <name>                           Stressor method\n");
	printf("  --opts <options>                          Extra stressor-specific options\n");
	printf("  -l, --list                                List stress jobs and stressors\n");
	printf("\nExamples:\n");
	printf("  rtbench -L\n");
	printf("  rtbench -b pid -p 0.1 -t 10 -q\n");
	printf("  rtbench test-schedule --cycles 1\n");
	printf("  rtbench test-schedule\n\n");
}

static void print_test_schedule_usage(void)
{
	printf("\nUsage: rtbench test-schedule [OPTIONS]\n\n");
	printf("Options:\n");
	printf("  --cycles <n>            Cycles per task (default: %d)\n",
	       TEST_SCHEDULE_CYCLES);
	printf("  --util-start <pct>      Starting utilization percentage (default: %d)\n",
	       TEST_SCHEDULE_UTIL_START);
	printf("  --util-end <pct>        Ending utilization percentage (default: %d)\n",
	       TEST_SCHEDULE_UTIL_END);
	printf("  --util-step <pct>       Utilization step percentage (default: %d)\n",
	       TEST_SCHEDULE_UTIL_STEP);
	printf("  --quick                 Use smoke-test defaults: cycles=%d, util=%d-%d step %d\n",
	       TEST_SCHEDULE_QUICK_CYCLES,
	       TEST_SCHEDULE_QUICK_UTIL_START,
	       TEST_SCHEDULE_QUICK_UTIL_END,
	       TEST_SCHEDULE_QUICK_UTIL_STEP);
	printf("  -q                      Quiet mode\n");
	printf("  -h, --help              Show this help message\n\n");
}

static int is_help_arg(const char *arg)
{
	return arg && (!strcmp(arg, "-h") || !strcmp(arg, "--help") ||
		       !strcmp(arg, "help"));
}

static const char *inline_option_value(const char *arg, const char *opt)
{
	size_t len;

	if (!arg || !opt) {
		return NULL;
	}

	len = strlen(opt);
	if (strncmp(arg, opt, len) == 0 && arg[len] == '=') {
		return arg + len + 1;
	}

	return NULL;
}

static int match_value_option(int argc, char **argv, int *idx,
			      const char *opt, const char **value)
{
	const char *arg = argv[*idx];
	const char *inline_value = inline_option_value(arg, opt);

	*value = NULL;
	if (strcmp(arg, opt) == 0) {
		if (*idx + 1 >= argc) {
			printf("[rtbench] Missing value for %s\n", opt);
			return -1;
		}
		*value = argv[++(*idx)];
		return 1;
	}
	if (inline_value) {
		if (*inline_value == '\0') {
			printf("[rtbench] Missing value for %s\n", opt);
			return -1;
		}
		*value = inline_value;
		return 1;
	}

	return 0;
}

static int parse_int_value(const char *value, const char *name,
			   int min_value, int max_value, int *out)
{
	char *end = NULL;
	long parsed;

	errno = 0;
	parsed = strtol(value, &end, 10);
	if (errno != 0 || end == value || (end && *end != '\0') ||
	    parsed < min_value || parsed > max_value) {
		printf("[rtbench] Invalid %s: %s\n", name, value);
		return -1;
	}

	*out = (int)parsed;
	return 0;
}

static int parse_double_value(const char *value, const char *name, double *out)
{
	char *end = NULL;
	double parsed;

	errno = 0;
	parsed = strtod(value, &end);
	if (errno != 0 || end == value || (end && *end != '\0') ||
	    parsed < 0.0) {
		printf("[rtbench] Invalid %s: %s\n", name, value);
		return -1;
	}

	*out = parsed;
	return 0;
}

static int parse_schedule_args(int argc, char **argv, int *cycles,
			       int *util_start, int *util_end, int *util_step)
{
	for (int i = 2; i < argc; i++) {
		const char *value;
		int matched;

		if (is_help_arg(argv[i])) {
			print_test_schedule_usage();
			return RTBENCH_PARSE_HELP;
		}

		matched = match_value_option(argc, argv, &i, "--cycles", &value);
		if (matched < 0) {
			print_test_schedule_usage();
			return -1;
		} else if (matched) {
			if (parse_int_value(value, "cycles", 1, INT_MAX, cycles) != 0) {
				print_test_schedule_usage();
				return -1;
			}
			continue;
		}

		matched = match_value_option(argc, argv, &i, "--util-start", &value);
		if (matched < 0) {
			print_test_schedule_usage();
			return -1;
		} else if (matched) {
			if (parse_int_value(value, "util-start", 1, 100, util_start) != 0) {
				print_test_schedule_usage();
				return -1;
			}
			continue;
		}

		matched = match_value_option(argc, argv, &i, "--util-end", &value);
		if (matched < 0) {
			print_test_schedule_usage();
			return -1;
		} else if (matched) {
			if (parse_int_value(value, "util-end", 1, 100, util_end) != 0) {
				print_test_schedule_usage();
				return -1;
			}
			continue;
		}

		matched = match_value_option(argc, argv, &i, "--util-step", &value);
		if (matched < 0) {
			print_test_schedule_usage();
			return -1;
		} else if (matched) {
			if (parse_int_value(value, "util-step", 1, 100, util_step) != 0) {
				print_test_schedule_usage();
				return -1;
			}
			continue;
		}

		if (strcmp(argv[i], "--quick") == 0) {
			*cycles = TEST_SCHEDULE_QUICK_CYCLES;
			*util_start = TEST_SCHEDULE_QUICK_UTIL_START;
			*util_end = TEST_SCHEDULE_QUICK_UTIL_END;
			*util_step = TEST_SCHEDULE_QUICK_UTIL_STEP;
		} else if (strcmp(argv[i], "-q") == 0) {
			benchmark_verbosity = LOG_LEVEL_INFO;
		} else {
			printf("[test-schedule] Unknown option: %s\n", argv[i]);
			print_test_schedule_usage();
			return -1;
		}
	}

	if (*util_start > *util_end) {
		printf("[test-schedule] util-start must be <= util-end\n");
		print_test_schedule_usage();
		return -1;
	}

	return 0;
}

static int parse_args(int argc, char **argv, struct execution_options *opts,
		      int *run_workload_suite)
{
	for (int i = 1; i < argc; i++) {
		const char *value;
		int matched;

		if (is_help_arg(argv[i])) {
			print_usage();
			return RTBENCH_PARSE_HELP;
		}

		matched = match_value_option(argc, argv, &i, "-p", &value);
		if (!matched) {
			matched = match_value_option(argc, argv, &i, "--period", &value);
		}
		if (matched < 0) {
			print_usage();
			return -1;
		} else if (matched) {
			double v;
			if (parse_double_value(value, "period", &v) != 0) {
				print_usage();
				return -1;
			}
			long sec = (long)v;
			long nsec = (long)((v - (double)sec) * 1000000000.0);
			opts->period_sec = sec;
			opts->period_nsec = nsec;
			opts->parsed_period = v;
			continue;
		}

		matched = match_value_option(argc, argv, &i, "-d", &value);
		if (!matched) {
			matched = match_value_option(argc, argv, &i, "--deadline", &value);
		}
		if (matched < 0) {
			print_usage();
			return -1;
		} else if (matched) {
			double v;
			if (parse_double_value(value, "deadline", &v) != 0) {
				print_usage();
				return -1;
			}
			long sec = (long)v;
			long nsec = (long)((v - (double)sec) * 1000000000.0);
			opts->deadline_sec = sec;
			opts->deadline_nsec = nsec;
			opts->parsed_deadline = v;
			continue;
		}

		matched = match_value_option(argc, argv, &i, "-t", &value);
		if (!matched) {
			matched = match_value_option(argc, argv, &i, "--tasks", &value);
		}
		if (matched < 0) {
			print_usage();
			return -1;
		} else if (matched) {
			int tasks;
			if (parse_int_value(value, "tasks", 0, INT_MAX, &tasks) != 0) {
				print_usage();
				return -1;
			}
			opts->tasks_to_launch = (uint64_t)tasks;
			continue;
		}

		matched = match_value_option(argc, argv, &i, "-f", &value);
		if (!matched) {
			matched = match_value_option(argc, argv, &i, "--fifo", &value);
		}
		if (matched < 0) {
			print_usage();
			return -1;
		} else if (matched) {
			int prio;
			if (parse_int_value(value, "priority", 0, 255, &prio) != 0) {
				print_usage();
				return -1;
			}
			opts->prio = (uint32_t)prio;
			continue;
		}

		matched = match_value_option(argc, argv, &i, "-c", &value);
		if (!matched) {
			matched = match_value_option(argc, argv, &i, "--core-affinity", &value);
		}
		if (matched < 0) {
			print_usage();
			return -1;
		} else if (matched) {
			int cpu;
			if (parse_int_value(value, "core-affinity", 0, 31, &cpu) != 0) {
				print_usage();
				return -1;
			}
			if (cpu >= 0 && cpu < 32) {
				CPU_ZERO(&opts->core_affinity);
				CPU_SET((uint32_t)cpu, &opts->core_affinity);
			}
			continue;
		}

		matched = match_value_option(argc, argv, &i, "-b", &value);
		if (!matched) {
			matched = match_value_option(argc, argv, &i, "-w", &value);
		}
		if (!matched) {
			matched = match_value_option(argc, argv, &i, "--workload", &value);
		}
		if (matched < 0) {
			print_usage();
			return -1;
		} else if (matched) {
			opts->workload_name = value;
			continue;
		}

		matched = match_value_option(argc, argv, &i, "-G", &value);
		if (!matched) {
			matched = match_value_option(argc, argv, &i, "--category", &value);
		}
		if (matched < 0) {
			print_usage();
			return -1;
		} else if (matched) {
			opts->category_filter = value;
			continue;
		}

		if (!strcmp(argv[i], "-q")) {
			benchmark_verbosity = LOG_LEVEL_INFO;
		} else if (!strcmp(argv[i], "-A") || !strcmp(argv[i], "--all-workloads")) {
			opts->run_all_workloads = 1;
		} else if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--list") ||
			   !strcmp(argv[i], "-L")) {
			opts->list_only = 1;
		} else if (!strcmp(argv[i], "-s") ||
			   !strcmp(argv[i], "--run-workload-suite")) {
			*run_workload_suite = 1;
		} else {
			printf("[rtbench] Unknown option or command: %s\n", argv[i]);
			print_usage();
			return -1;
		}
	}

	return 0;
}

static void debug_print_context(const struct execution_options *opts)
{
	printf("[rtbench] workload=%s period=%ld.%09ld tasks=%llu\n",
	       rtosbench_current_workload(),
	       opts->period_sec, opts->period_nsec,
	       (unsigned long long)opts->tasks_to_launch);
}

extern int run_all_workloads(void);

/**
 * @brief Main entry point for SylixOS
 */
int main(int argc, char **argv)
{
	struct execution_options opts;
	int run_workload_suite = 0;
	int parse_ret;

	/* Register workloads */
	rtosbench_register_rtos_workloads();

	if (argc <= 1 || is_help_arg(argv[1])) {
		print_usage();
		return 0;
	}

	/* Handle test-schedule subcommand */
	if (argc >= 2 && strcmp(argv[1], "test-schedule") == 0) {
		int cycles = TEST_SCHEDULE_CYCLES;
		int util_start = TEST_SCHEDULE_UTIL_START;
		int util_end = TEST_SCHEDULE_UTIL_END;
		int util_step = TEST_SCHEDULE_UTIL_STEP;

		parse_ret = parse_schedule_args(argc, argv, &cycles, &util_start,
						&util_end, &util_step);
		if (parse_ret == RTBENCH_PARSE_HELP) {
			return 0;
		}
		if (parse_ret != 0) {
			return -1;
		}

		printf("[test-schedule] Starting schedulability test on SylixOS\n");
		printf("  Cycles: %d, Utilization: %d%% - %d%% (step %d%%)\n",
		       cycles, util_start, util_end, util_step);

		return test_schedule_run_custom(cycles, util_start, util_end, util_step);
	}

	/* Handle test-realtime subcommand */
	if (argc >= 2 && strcmp(argv[1], "test-realtime") == 0) {
		int run_multicore = 0;
		int run_verify = 0;

		for (int i = 2; i < argc; i++) {
			if (is_help_arg(argv[i])) {
				print_usage();
				return 0;
			} else if (strcmp(argv[i], "--verify") == 0 ||
				   strcmp(argv[i], "-v") == 0) {
				run_verify = 1;
			} else if (strcmp(argv[i], "--multicore") == 0 ||
				   strcmp(argv[i], "-m") == 0) {
				run_multicore = 1;
			} else if (strcmp(argv[i], "-q") == 0) {
				benchmark_verbosity = LOG_LEVEL_INFO;
			} else {
				printf("[test-realtime] Unknown option: %s\n", argv[i]);
				print_usage();
				return -1;
			}
		}

		printf("[test-realtime] Starting realtime performance test on SylixOS\n");
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
		int duration_sec = 10;      /* Default 10 seconds */
		int num_workers = 1;        /* Default 1 worker */
		uint64_t max_ops = 0;       /* Default no limit */
		const char *method_name = NULL;   /* --method parameter */
		static char extra_opts_buf[512];    /* Extra stressor-specific options buffer */
		int extra_opts_len = 0;
		int single_mode = 0;        /* Default to job mode */

		extra_opts_buf[0] = '\0';

		for (int i = 2; i < argc; i++) {
			const char *val;
			
			if (is_help_arg(argv[i])) {
				print_usage();
				return 0;
			} else if (strcmp(argv[i], "--job") == 0 && (i +1 < argc)) {
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
			} else if (strcmp(argv[i], "-l") == 0 ||
			           strcmp(argv[i], "--list") == 0) {
				list_jobs = 1;
			} else if (strncmp(argv[i], "--", 2) == 0) {
				printf("[test-stress] Unknown option: %s\n", argv[i]);
				print_usage();
				return -1;
			} else if (strcmp(argv[i], "-q") == 0) {
				benchmark_verbosity = LOG_LEVEL_INFO;
			} else {
				printf("[test-stress] Unknown option: %s\n", argv[i]);
				print_usage();
				return -1;
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
			if (is_help_arg(argv[i])) {
				print_usage();
				return 0;
			} else if (strcmp(argv[i], "-q") == 0) {
				benchmark_verbosity = LOG_LEVEL_INFO;
			} else {
				printf("[test-cmd] Unknown option: %s\n", argv[i]);
				print_usage();
				return -1;
			}
		}
		printf("[test-cmd] Starting shell command support test on SylixOS\n");
		return test_cmd_run();
	}

	/* Standard workload execution */
	set_default_exec_opts(&opts);
	parse_ret = parse_args(argc, argv, &opts, &run_workload_suite);
	if (parse_ret == RTBENCH_PARSE_HELP) {
		return 0;
	}
	if (parse_ret != 0) {
		return -1;
	}

	if (run_workload_suite) {
		return run_all_workloads();
	}

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
