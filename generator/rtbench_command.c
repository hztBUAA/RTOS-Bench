/**
 * @file rtbench_command.c
 * @brief Common RTOS-Bench command dispatcher used by platform entries.
 */

#include "rtbench_command.h"

#include "logging.h"
#include "periodic_benchmark.h"
#include "platform_abstraction.h"
#include "result_export.h"
#include "test_cmd.h"
#include "test_realtime.h"
#include "test_schedule.h"
#include "test_schedule/sched_workloads.h"
#include "test_stress.h"
#include "workload_registry.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(RUIHUA_PLATFORM)
#include <unistd.h>
#endif

#if defined(__GNUC__)
#define RTBENCH_WEAK __attribute__((weak))
#else
#define RTBENCH_WEAK
#endif

#define RTBENCH_PARSE_HELP 1

extern const struct rtosbench_workload rtosbench_stub_workload;
extern const struct rtosbench_workload rtosbench_busywait_workload;
extern void rtosbench_register_rtos_workloads(void) RTBENCH_WEAK;

extern int test_schedule_run_custom(int cycles, int util_start,
				    int util_end, int util_step) RTBENCH_WEAK;
extern int test_realtime_run(int run_multicore) RTBENCH_WEAK;
extern void test_realtime_verify(void) RTBENCH_WEAK;
extern int test_stress_run_job(const char *job_name) RTBENCH_WEAK;
extern int test_stress_run_single(const char *stressor_name, int duration_sec,
				  int num_workers, uint64_t max_ops,
				  const char *method_name,
				  const char *extra_opts) RTBENCH_WEAK;
extern void test_stress_list_jobs(void) RTBENCH_WEAK;
extern void test_stress_list_stressors(void) RTBENCH_WEAK;
extern const char *test_stress_parse_opt_arg(const char *arg,
					     const char *prefix) RTBENCH_WEAK;
extern int test_cmd_run(void) RTBENCH_WEAK;
extern const struct test_schedule_result *test_schedule_get_result(void) RTBENCH_WEAK;
extern const struct test_stress_job_result *test_stress_get_job_results(int *count_out) RTBENCH_WEAK;
extern const struct test_cmd_module_result *test_cmd_get_result(void) RTBENCH_WEAK;

extern uint64_t *get_realtime_service_cost(void) RTBENCH_WEAK;
extern uint64_t *get_realtime_interrupt(void) RTBENCH_WEAK;
extern uint64_t get_realtime_context_switch(void) RTBENCH_WEAK;
extern uint64_t *get_realtime_syscall(void) RTBENCH_WEAK;
extern uint64_t *get_multicore_memory_bandwidth(void) RTBENCH_WEAK;
extern uint64_t *get_multicore_ipc_bandwidth(void) RTBENCH_WEAK;
extern uint64_t *get_multicore_intra_inter_bandwidth(void) RTBENCH_WEAK;
extern uint64_t *get_multicore_init_dlt_latency(void) RTBENCH_WEAK;

struct test_all_params {
	const char *output_path;
	const char *xml_output_path;
	int run_realtime;
	int run_schedule;
	int run_stress;
	int run_cmd;
	int run_workload;
	int run_multicore;
	int quick_mode;
	int schedule_cycles;
	int schedule_util_start;
	int schedule_util_end;
	int schedule_util_step;
	int export_results;
	const char *stress_job;
};

static int g_workloads_registered;

const char *RTBENCH_WEAK rtbench_platform_default_output_path(void)
{
#if defined(SYLIXOS_PLATFORM)
	return "/apps/hzt/rtbench_result.json";
#else
	return "/rtbench_result.json";
#endif
}

void RTBENCH_WEAK rtbench_platform_get_env(struct rtbench_platform_env *env)
{
	if (env == NULL) {
		return;
	}
#if defined(SYLIXOS_PLATFORM)
	env->os_name = "SylixOS";
	env->os_version = "3.x";
	env->board = "SylixOS-Board";
	env->cpu_type = "Unknown";
	env->cpu_freq_mhz = 0;
	env->cpu_core_num = 1;
#elif defined(ONEOS_PLATFORM)
	env->os_name = "OneOS";
	env->os_version = "3.x";
	env->board = "OneOS-Board";
	env->cpu_type = "ARM";
	env->cpu_freq_mhz = 0;
	env->cpu_core_num = 1;
#elif defined(DONGTU_PLATFORM)
	env->os_name = "Dongtu";
	env->os_version = "Intewell";
	env->board = "Dongtu-Board";
	env->cpu_type = "x86_64";
	env->cpu_freq_mhz = 0;
	env->cpu_core_num = 1;
#elif defined(RT_THREAD_PLATFORM)
	env->os_name = "RT-Thread";
	env->os_version = "unknown";
	env->board = "RT-Thread-Board";
	env->cpu_type = "Unknown";
	env->cpu_freq_mhz = 0;
	env->cpu_core_num = 1;
#elif defined(RUIHUA_PLATFORM)
	env->os_name = "Ruihua ReWorks";
	env->os_version = "6.1.1";
	env->board = "Ruihua-Board";
	env->cpu_type = "ARM";
	env->cpu_freq_mhz = 0;
	env->cpu_core_num = 0;
#else
	env->os_name = "Unknown";
	env->os_version = "unknown";
	env->board = "Unknown-Board";
	env->cpu_type = "Unknown";
	env->cpu_freq_mhz = 0;
	env->cpu_core_num = 0;
#endif
}

int RTBENCH_WEAK rtbench_platform_run_test_all(rtbench_command_runner_fn runner,
					       void *ctx)
{
	return runner ? runner(ctx) : -1;
}

static const char *default_output_path(void)
{
	const char *path = rtbench_platform_default_output_path();
	return (path && path[0] != '\0') ? path : "/rtbench_result.json";
}

static int is_help_arg(const char *arg)
{
	return arg != NULL && (!strcmp(arg, "-h") || !strcmp(arg, "--help") ||
			       !strcmp(arg, "help"));
}

static const char *inline_option_value(const char *arg, const char *opt)
{
	size_t len;

	if (arg == NULL || opt == NULL) {
		return NULL;
	}
	len = strlen(opt);
	if (strncmp(arg, opt, len) == 0 && arg[len] == '=') {
		return arg + len + 1;
	}
	return NULL;
}

static int match_value_option(int argc, char **argv, int *idx,
			      const char *short_opt, const char *long_opt,
			      const char **value)
{
	const char *arg = argv[*idx];
	const char *inline_value;

	*value = NULL;
	if (short_opt != NULL && strcmp(arg, short_opt) == 0) {
		if (*idx + 1 >= argc) {
			printf("[rtbench] Missing value for %s\n", short_opt);
			return -1;
		}
		*value = argv[++(*idx)];
		return 1;
	}
	if (long_opt != NULL && strcmp(arg, long_opt) == 0) {
		if (*idx + 1 >= argc) {
			printf("[rtbench] Missing value for %s\n", long_opt);
			return -1;
		}
		*value = argv[++(*idx)];
		return 1;
	}
	if (long_opt != NULL && (inline_value = inline_option_value(arg, long_opt)) != NULL) {
		if (*inline_value == '\0') {
			printf("[rtbench] Missing value for %s\n", long_opt);
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

static const char *stress_opt_arg(const char *arg, const char *prefix)
{
	size_t len;

	if (test_stress_parse_opt_arg != NULL) {
		return test_stress_parse_opt_arg(arg, prefix);
	}
	len = strlen(prefix);
	return strncmp(arg, prefix, len) == 0 ? arg + len : NULL;
}

static void ensure_workloads_registered(void)
{
	if (g_workloads_registered) {
		return;
	}
	g_workloads_registered = 1;
	rtosbench_register_workload(&rtosbench_stub_workload);
	rtosbench_register_workload(&rtosbench_busywait_workload);
	if (rtosbench_register_rtos_workloads != NULL) {
		rtosbench_register_rtos_workloads();
	}
}

void rtbench_command_print_usage(void)
{
	printf("\nRTOS-Bench - Cross-platform Industrial RTOS Benchmark Framework\n\n");
	printf("USAGE:\n");
	printf("  rtbench [OPTIONS]                    Run workload benchmark\n");
	printf("  rtbench test-all [OPTIONS]           Run comprehensive test suite\n");
	printf("  rtbench test-realtime [OPTIONS]      Run realtime performance test\n");
	printf("  rtbench test-schedule [OPTIONS]      Run schedulability test\n");
	printf("  rtbench test-stress [OPTIONS]        Run stress test\n");
	printf("  rtbench test-cmd                     Run shell command support test\n");
	printf("  rtbench export-result [OPTIONS]      Export current results to JSON\n");
	printf("  rtbench -L | --list                  List available workloads\n");
	printf("  rtbench -h | --help                  Show this help message\n\n");
	printf("WORKLOAD OPTIONS:\n");
	printf("  -b, -w, --workload <name>  Workload name\n");
	printf("  -p, --period <sec>         Period in seconds (default: 1.0)\n");
	printf("  -d, --deadline <sec>       Deadline in seconds\n");
	printf("  -t, --tasks <count>        Number of activations (0 = infinite)\n");
	printf("  -f, --fifo <prio>          FIFO priority\n");
	printf("  -c, --core-affinity <cpu>  CPU affinity (0-31)\n");
	printf("  -A, --all-workloads        Run all registered workloads\n");
	printf("  -G, --category <list>      Run workload categories\n");
	printf("  -q                         Quiet mode\n\n");
}

static void print_test_all_usage(void)
{
	printf("\nUsage: rtbench test-all [OPTIONS]\n");
	printf("  -o, --output <path>        Output JSON path (default: %s)\n",
	       default_output_path());
	printf("  --xml-output <path>        Optional JUnit XML output path for Flow upload\n");
	printf("  --no-realtime|--no-schedule|--no-stress|--no-cmd|--no-workload\n");
	printf("  --no-export                 Do not write result JSON\n");
	printf("  -m, --multicore            Enable realtime multicore tests\n");
	printf("  --quick                    Use bounded smoke settings\n");
	printf("  --schedule-cycles <n>      Override schedule cycles\n");
	printf("  --util-start/end/step <p>  Override schedule utilization range\n");
	printf("  --stress-job <name>        Override stress job\n");
	printf("  -q, -h, --help\n\n");
}

static void print_test_schedule_usage(void)
{
	printf("\nUsage: rtbench test-schedule [OPTIONS]\n");
	printf("  --cycles <n>               Number of test cycles (default: %d)\n",
	       TEST_SCHEDULE_CYCLES);
	printf("  --util-start <pct>         Start utilization (default: %d)\n",
	       TEST_SCHEDULE_UTIL_START);
	printf("  --util-end <pct>           End utilization (default: %d)\n",
	       TEST_SCHEDULE_UTIL_END);
	printf("  --util-step <pct>          Utilization step (default: %d)\n",
	       TEST_SCHEDULE_UTIL_STEP);
	printf("  --quick, -q, -h, --help\n\n");
}

static void print_export_usage(void)
{
	printf("\nUsage: rtbench export-result [-o|--output <path>] [--xml-output <path>]\n");
	printf("  default output path: %s\n\n", default_output_path());
}

static int validate_schedule_args(int cycles, int util_start,
				  int util_end, int util_step,
				  void (*usage_fn)(void))
{
	if (cycles < 1) {
		printf("[test-schedule] cycles must be >= 1\n");
		usage_fn();
		return -1;
	}
	if (util_start < 1 || util_start > 100 ||
	    util_end < 1 || util_end > 100 ||
	    util_step < 1 || util_step > 100 ||
	    util_start > util_end) {
		printf("[test-schedule] invalid utilization range: %d-%d step %d\n",
		       util_start, util_end, util_step);
		usage_fn();
		return -1;
	}
	return 0;
}

static void set_default_exec_opts(struct execution_options *opts)
{
	memset(opts, 0, sizeof(*opts));
	opts->period_sec = 1;
	opts->period_nsec = 0;
	opts->deadline_sec = 0;
	opts->deadline_nsec = 0;
	opts->tasks_to_launch = 1;
	opts->prio = 100;
	CPU_ZERO(&opts->core_affinity);
	CPU_ZERO(&opts->memory_profiling_core_affinity);
	opts->memory_profiling_time_bucket = 10000000;
	benchmark_verbosity = LOG_LEVEL_TRACE;
}

static int parse_workload_args(int argc, char **argv,
			       struct execution_options *opts)
{
	for (int i = 1; i < argc; i++) {
		const char *value;
		int matched;

		if (is_help_arg(argv[i])) {
			rtbench_command_print_usage();
			return RTBENCH_PARSE_HELP;
		}
		matched = match_value_option(argc, argv, &i, "-p", "--period", &value);
		if (matched < 0) {
			rtbench_command_print_usage();
			return -1;
		} else if (matched) {
			double v;
			long sec;
			long nsec;
			if (parse_double_value(value, "period", &v) != 0) {
				rtbench_command_print_usage();
				return -1;
			}
			sec = (long)v;
			nsec = (long)((v - (double)sec) * 1000000000.0);
			opts->period_sec = sec;
			opts->period_nsec = nsec;
			opts->parsed_period = v;
			continue;
		}
		matched = match_value_option(argc, argv, &i, "-d", "--deadline", &value);
		if (matched < 0) {
			rtbench_command_print_usage();
			return -1;
		} else if (matched) {
			double v;
			long sec;
			long nsec;
			if (parse_double_value(value, "deadline", &v) != 0) {
				rtbench_command_print_usage();
				return -1;
			}
			sec = (long)v;
			nsec = (long)((v - (double)sec) * 1000000000.0);
			opts->deadline_sec = sec;
			opts->deadline_nsec = nsec;
			opts->parsed_deadline = v;
			continue;
		}
		matched = match_value_option(argc, argv, &i, "-t", "--tasks", &value);
		if (matched < 0) {
			rtbench_command_print_usage();
			return -1;
		} else if (matched) {
			int tasks;
			if (parse_int_value(value, "tasks", 0, INT_MAX, &tasks) != 0) {
				rtbench_command_print_usage();
				return -1;
			}
			opts->tasks_to_launch = (uint64_t)tasks;
			continue;
		}
		matched = match_value_option(argc, argv, &i, "-f", "--fifo", &value);
		if (matched < 0) {
			rtbench_command_print_usage();
			return -1;
		} else if (matched) {
			int prio;
			if (parse_int_value(value, "priority", 0, 255, &prio) != 0) {
				rtbench_command_print_usage();
				return -1;
			}
			opts->prio = (uint32_t)prio;
			continue;
		}
		matched = match_value_option(argc, argv, &i, "-c", "--core-affinity", &value);
		if (matched < 0) {
			rtbench_command_print_usage();
			return -1;
		} else if (matched) {
			int cpu;
			if (parse_int_value(value, "core-affinity", 0, 31, &cpu) != 0) {
				rtbench_command_print_usage();
				return -1;
			}
			CPU_ZERO(&opts->core_affinity);
			CPU_SET((uint32_t)cpu, &opts->core_affinity);
			continue;
		}
		matched = match_value_option(argc, argv, &i, "-b", "--workload", &value);
		if (!matched) {
			matched = match_value_option(argc, argv, &i, "-w", NULL, &value);
		}
		if (matched < 0) {
			rtbench_command_print_usage();
			return -1;
		} else if (matched) {
			opts->workload_name = value;
			continue;
		}
		matched = match_value_option(argc, argv, &i, "-G", "--category", &value);
		if (matched < 0) {
			rtbench_command_print_usage();
			return -1;
		} else if (matched) {
			opts->category_filter = value;
			continue;
		}
		if (!strcmp(argv[i], "-q")) {
			benchmark_verbosity = LOG_LEVEL_INFO;
		} else if (!strcmp(argv[i], "-A") || !strcmp(argv[i], "--all-workloads")) {
			opts->run_all_workloads = 1;
		} else if (!strcmp(argv[i], "-L") || !strcmp(argv[i], "-l") ||
			   !strcmp(argv[i], "--list")) {
			opts->list_only = 1;
		} else {
			printf("[rtbench] Unknown option or command: %s\n", argv[i]);
			rtbench_command_print_usage();
			return -1;
		}
	}
	return 0;
}

static int parse_test_all_args(int argc, char **argv,
			       struct test_all_params *params)
{
	int cycles_set = 0;
	int util_start_set = 0;
	int util_end_set = 0;
	int util_step_set = 0;

	for (int i = 2; i < argc; i++) {
		const char *value;
		int matched;

		if (is_help_arg(argv[i])) {
			print_test_all_usage();
			return RTBENCH_PARSE_HELP;
		}
		matched = match_value_option(argc, argv, &i, "-o", "--output", &value);
		if (matched < 0) {
			print_test_all_usage();
			return -1;
		} else if (matched) {
			params->output_path = value;
			continue;
		}
		matched = match_value_option(argc, argv, &i, NULL, "--xml-output", &value);
		if (matched < 0) {
			print_test_all_usage();
			return -1;
		} else if (matched) {
			params->xml_output_path = value;
			continue;
		}
		matched = match_value_option(argc, argv, &i, NULL, "--schedule-cycles", &value);
		if (matched < 0) {
			print_test_all_usage();
			return -1;
		} else if (matched) {
			if (parse_int_value(value, "schedule-cycles", 1, INT_MAX,
					    &params->schedule_cycles) != 0) {
				print_test_all_usage();
				return -1;
			}
			cycles_set = 1;
			continue;
		}
		matched = match_value_option(argc, argv, &i, NULL, "--util-start", &value);
		if (matched < 0) {
			print_test_all_usage();
			return -1;
		} else if (matched) {
			if (parse_int_value(value, "util-start", 1, 100,
					    &params->schedule_util_start) != 0) {
				print_test_all_usage();
				return -1;
			}
			util_start_set = 1;
			continue;
		}
		matched = match_value_option(argc, argv, &i, NULL, "--util-end", &value);
		if (matched < 0) {
			print_test_all_usage();
			return -1;
		} else if (matched) {
			if (parse_int_value(value, "util-end", 1, 100,
					    &params->schedule_util_end) != 0) {
				print_test_all_usage();
				return -1;
			}
			util_end_set = 1;
			continue;
		}
		matched = match_value_option(argc, argv, &i, NULL, "--util-step", &value);
		if (matched < 0) {
			print_test_all_usage();
			return -1;
		} else if (matched) {
			if (parse_int_value(value, "util-step", 1, 100,
					    &params->schedule_util_step) != 0) {
				print_test_all_usage();
				return -1;
			}
			util_step_set = 1;
			continue;
		}
		matched = match_value_option(argc, argv, &i, NULL, "--stress-job", &value);
		if (matched < 0) {
			print_test_all_usage();
			return -1;
		} else if (matched) {
			params->stress_job = value;
			continue;
		}
		if (!strcmp(argv[i], "--no-realtime")) {
			params->run_realtime = 0;
		} else if (!strcmp(argv[i], "--no-schedule")) {
			params->run_schedule = 0;
		} else if (!strcmp(argv[i], "--no-stress")) {
			params->run_stress = 0;
		} else if (!strcmp(argv[i], "--no-cmd")) {
			params->run_cmd = 0;
		} else if (!strcmp(argv[i], "--no-workload")) {
			params->run_workload = 0;
		} else if (!strcmp(argv[i], "--no-export")) {
			params->export_results = 0;
		} else if (!strcmp(argv[i], "--multicore") || !strcmp(argv[i], "-m")) {
			params->run_multicore = 1;
		} else if (!strcmp(argv[i], "--quick")) {
			params->quick_mode = 1;
		} else if (!strcmp(argv[i], "-q")) {
			benchmark_verbosity = LOG_LEVEL_INFO;
		} else {
			printf("[test-all] Unknown option: %s\n", argv[i]);
			print_test_all_usage();
			return -1;
		}
	}
	if (params->quick_mode) {
		if (!cycles_set) params->schedule_cycles = TEST_SCHEDULE_QUICK_CYCLES;
		if (!util_start_set) params->schedule_util_start = TEST_SCHEDULE_QUICK_UTIL_START;
		if (!util_end_set) params->schedule_util_end = TEST_SCHEDULE_QUICK_UTIL_END;
		if (!util_step_set) params->schedule_util_step = TEST_SCHEDULE_QUICK_UTIL_STEP;
	}
	return validate_schedule_args(params->schedule_cycles,
				      params->schedule_util_start,
				      params->schedule_util_end,
				      params->schedule_util_step,
				      print_test_all_usage);
}

static int run_test_schedule_with_args(int argc, char **argv)
{
	int cycles = TEST_SCHEDULE_CYCLES;
	int util_start = TEST_SCHEDULE_UTIL_START;
	int util_end = TEST_SCHEDULE_UTIL_END;
	int util_step = TEST_SCHEDULE_UTIL_STEP;
	int quick = 0;
	int cycles_set = 0;
	int util_start_set = 0;
	int util_end_set = 0;
	int util_step_set = 0;

	for (int i = 2; i < argc; i++) {
		const char *value;
		int matched;

		if (is_help_arg(argv[i])) {
			print_test_schedule_usage();
			return RTBENCH_PARSE_HELP;
		}
		matched = match_value_option(argc, argv, &i, NULL, "--cycles", &value);
		if (matched < 0) return -1;
		if (matched) {
			if (parse_int_value(value, "cycles", 1, INT_MAX, &cycles) != 0) return -1;
			cycles_set = 1;
			continue;
		}
		matched = match_value_option(argc, argv, &i, NULL, "--util-start", &value);
		if (matched < 0) return -1;
		if (matched) {
			if (parse_int_value(value, "util-start", 1, 100, &util_start) != 0) return -1;
			util_start_set = 1;
			continue;
		}
		matched = match_value_option(argc, argv, &i, NULL, "--util-end", &value);
		if (matched < 0) return -1;
		if (matched) {
			if (parse_int_value(value, "util-end", 1, 100, &util_end) != 0) return -1;
			util_end_set = 1;
			continue;
		}
		matched = match_value_option(argc, argv, &i, NULL, "--util-step", &value);
		if (matched < 0) return -1;
		if (matched) {
			if (parse_int_value(value, "util-step", 1, 100, &util_step) != 0) return -1;
			util_step_set = 1;
			continue;
		}
		if (!strcmp(argv[i], "--quick")) {
			quick = 1;
		} else if (!strcmp(argv[i], "-q")) {
			benchmark_verbosity = LOG_LEVEL_INFO;
		} else {
			printf("[test-schedule] Unknown option: %s\n", argv[i]);
			print_test_schedule_usage();
			return -1;
		}
	}
	if (quick) {
		if (!cycles_set) cycles = TEST_SCHEDULE_QUICK_CYCLES;
		if (!util_start_set) util_start = TEST_SCHEDULE_QUICK_UTIL_START;
		if (!util_end_set) util_end = TEST_SCHEDULE_QUICK_UTIL_END;
		if (!util_step_set) util_step = TEST_SCHEDULE_QUICK_UTIL_STEP;
	}
	if (validate_schedule_args(cycles, util_start, util_end, util_step,
				   print_test_schedule_usage) != 0) {
		return -1;
	}
	if (test_schedule_run_custom == NULL) {
		printf("[test-schedule] module is not linked\n");
		return -1;
	}
	ensure_workloads_registered();
	return test_schedule_run_custom(cycles, util_start, util_end, util_step);
}

#define NS_TO_US(ns) ((double)(ns) / 1000.0)
#define RAW_TO_GBS(v) ((double)(v) / 1000.0)

static void collect_realtime_result(int run_multicore)
{
	struct rtbench_result *r = rtbench_result_get();
	struct rtbench_realtime_result *rt;
	uint64_t ctx_sw = 0;
	uint64_t *interrupt = NULL;
	uint64_t *syscall = NULL;
	uint64_t *svc = NULL;

	if (r == NULL) return;
	rt = &r->realtime;
	rt->valid = 1;
	rt->multicore_valid = run_multicore ? 1 : 0;

	if (get_realtime_context_switch != NULL) ctx_sw = get_realtime_context_switch();
	if (get_realtime_interrupt != NULL) interrupt = get_realtime_interrupt();
	if (get_realtime_syscall != NULL) syscall = get_realtime_syscall();
	if (get_realtime_service_cost != NULL) svc = get_realtime_service_cost();

	rt->context_switch_avg_us = NS_TO_US(ctx_sw);
	if (interrupt != NULL) {
		rt->interrupt_min_us = NS_TO_US(interrupt[0]);
		rt->interrupt_max_us = NS_TO_US(interrupt[1]);
		rt->interrupt_avg_us = NS_TO_US(interrupt[2]);
	}
	if (syscall != NULL) {
		rt->syscall_min_us = NS_TO_US(syscall[0]);
		rt->syscall_max_us = NS_TO_US(syscall[1]);
		rt->syscall_avg_us = NS_TO_US(syscall[2]);
	}
	if (svc != NULL) {
		static const char *svc_ops[] = {
			"sem_take", "sem_release",
			"mq_send", "mq_recv",
			"mutex_take", "mutex_release",
			"mempool_alloc", "mempool_free"
		};
		for (int i = 0; i < 8; i++) {
			rtbench_realtime_add_service_cost(rt, svc_ops[i],
				NS_TO_US(svc[i * 4 + 0]),
				NS_TO_US(svc[i * 4 + 1]),
				NS_TO_US(svc[i * 4 + 2]),
				NS_TO_US(svc[i * 4 + 3]));
		}
	}

	if (run_multicore &&
	    get_multicore_memory_bandwidth != NULL &&
	    get_multicore_ipc_bandwidth != NULL &&
	    get_multicore_intra_inter_bandwidth != NULL &&
	    get_multicore_init_dlt_latency != NULL) {
		uint64_t *mem_bw = get_multicore_memory_bandwidth();
		uint64_t *ipc_bw = get_multicore_ipc_bandwidth();
		uint64_t *intra_inter = get_multicore_intra_inter_bandwidth();
		uint64_t *task_lat = get_multicore_init_dlt_latency();
		static const char *mem_types[] = {
			"rd", "wr", "cp", "frd", "fwr", "fcp", "memset", "memcpy"
		};

		if (mem_bw != NULL) {
			for (int i = 0; i < 8; i++) {
				rtbench_realtime_add_mem_bw(rt, mem_types[i],
					RAW_TO_GBS(mem_bw[i * 4 + 0]),
					RAW_TO_GBS(mem_bw[i * 4 + 1]),
					RAW_TO_GBS(mem_bw[i * 4 + 2]),
					RAW_TO_GBS(mem_bw[i * 4 + 3]));
			}
		}
		if (ipc_bw != NULL) {
			rt->ipc_bw_c1 = RAW_TO_GBS(ipc_bw[0]);
			rt->ipc_bw_c2 = RAW_TO_GBS(ipc_bw[1]);
			rt->ipc_bw_c4 = RAW_TO_GBS(ipc_bw[2]);
			rt->ipc_bw_c8 = RAW_TO_GBS(ipc_bw[3]);
		}
		if (intra_inter != NULL) {
			rt->core_comm_intra = RAW_TO_GBS(intra_inter[0]);
			rt->core_comm_inter = RAW_TO_GBS(intra_inter[1]);
		}
		if (task_lat != NULL) {
			rt->task_lat_c1 = NS_TO_US(task_lat[0]);
			rt->task_lat_c2 = NS_TO_US(task_lat[1]);
			rt->task_lat_c4 = NS_TO_US(task_lat[2]);
			rt->task_lat_c8 = NS_TO_US(task_lat[3]);
		}
	}
}

static void collect_schedule_result(int cycles, int util_start,
				    int util_end, int util_step)
{
	struct rtbench_result *r = rtbench_result_get();
	struct rtbench_schedule_result *sched;
	const struct test_schedule_result *ts_result;

	if (r == NULL || test_schedule_get_result == NULL) return;
	ts_result = test_schedule_get_result();
	if (ts_result == NULL) return;
	sched = &r->schedule;
	sched->valid = 1;
	sched->cycles = cycles;
	sched->util_start = util_start;
	sched->util_end = util_end;
	sched->util_step = util_step;
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
			if (tsrc->name != NULL) {
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
	struct rtbench_stress_result *stress;
	const struct test_stress_job_result *results;
	int count = 0;
	double total_dur = 0;

	(void)job_name;
	if (r == NULL || test_stress_get_job_results == NULL) return;
	results = test_stress_get_job_results(&count);
	if (results == NULL) return;
	stress = &r->stress;
	stress->valid = 1;
	for (int i = 0; i < count; i++) total_dur += results[i].duration_sec;
	stress->duration_sec = total_dur;
	for (int i = 0; i < count; i++) {
		const struct test_stress_job_result *jr = &results[i];
		double metric_val = 0;
		const char *metric_unit = "";
		if (jr->metric_value > 0.00001 && jr->metric_unit[0] != '\0') {
			metric_val = jr->metric_value;
			metric_unit = jr->metric_unit;
		}
		rtbench_stress_add_stressor(stress, jr->name, jr->type, jr->stage,
					    jr->bogo_ops, jr->duration_sec,
					    metric_val, metric_unit);
	}
}

static void collect_cmd_result(void)
{
	struct rtbench_result *r = rtbench_result_get();
	struct rtbench_cmd_module_result *dst;
	const struct test_cmd_module_result *src;

	if (r == NULL || test_cmd_get_result == NULL) return;
	src = test_cmd_get_result();
	if (src == NULL || !src->valid) return;
	dst = &r->cmd;
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
	struct rtbench_workload_module_result *wl;
	if (r == NULL) return;
	wl = &r->workload;
	memset(wl, 0, sizeof(*wl));
	wl->valid = 1;
	for (int i = 0; i < rtosbench_workload_count() &&
	     wl->workload_count < RTBENCH_MAX_WORKLOADS; i++) {
		const struct rtosbench_workload *w = rtosbench_get_workload(i);
		const struct sched_workload_wrapper *wrapper =
			(quick_mode && w && w->name) ? sched_get_wrapper(w->name) : NULL;
		int rounds = quick_mode ? 5 : 10;
		long double start_ts, end_ts;
		double total_ms;

		if (w == NULL || w->name == NULL ||
		    (w->exec == NULL && (wrapper == NULL || wrapper->quick_exec == NULL))) {
			continue;
		}
		if (w->category != NULL && strcmp(w->category, "synthetic") == 0) {
			continue;
		}
		printf("  Running workload: %s%s\n", w->name,
		       wrapper ? " (quick wrapper)" : "");
		if (wrapper != NULL && wrapper->init != NULL) {
			wrapper->init();
		} else if (w->init != NULL) {
			w->init(0, NULL);
		}
		start_ts = rtbench_get_timestamp();
		for (int j = 0; j < rounds; j++) {
			if (wrapper != NULL && wrapper->quick_exec != NULL) {
				wrapper->quick_exec();
			} else {
				w->exec(0, NULL);
			}
		}
		end_ts = rtbench_get_timestamp();
		if (wrapper != NULL && wrapper->teardown != NULL) {
			wrapper->teardown();
		} else if (w->teardown != NULL) {
			w->teardown(0, NULL);
		}
		total_ms = (double)(end_ts - start_ts) * 1000.0;
		printf("    %s: %.3f ms total, %.3f ms avg\n",
		       w->name, total_ms, total_ms / (double)rounds);
		rtbench_workload_add_result(wl, w->name,
					    w->category ? w->category : "",
					    1, rounds, total_ms,
					    total_ms / (double)rounds);
	}
}

static int run_test_all_impl(void *ctx)
{
	struct test_all_params *p = (struct test_all_params *)ctx;
	struct rtbench_platform_env env;
	int ret = 0;

	rtbench_result_init();
	memset(&env, 0, sizeof(env));
	rtbench_platform_get_env(&env);
	rtbench_result_set_env(env.os_name, env.os_version, env.board,
			       env.cpu_type, env.cpu_freq_mhz,
			       env.cpu_core_num);
	rtbench_result_start();
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

	if (p->run_realtime) {
		if (test_realtime_run != NULL) {
			printf(">>> Running test-realtime...\n");
			ret |= test_realtime_run(p->run_multicore);
			collect_realtime_result(p->run_multicore);
		} else {
			printf("[test-realtime] module is not linked\n");
		}
	}
	if (p->run_schedule) {
		if (test_schedule_run_custom != NULL) {
			printf("\n>>> Running test-schedule%s (cycles=%d util=%d-%d step %d)...\n",
			       p->quick_mode ? " (quick)" : "",
			       p->schedule_cycles, p->schedule_util_start,
			       p->schedule_util_end, p->schedule_util_step);
			ret |= test_schedule_run_custom(p->schedule_cycles,
							p->schedule_util_start,
							p->schedule_util_end,
							p->schedule_util_step);
			collect_schedule_result(p->schedule_cycles,
						p->schedule_util_start,
						p->schedule_util_end,
						p->schedule_util_step);
		} else {
			printf("[test-schedule] module is not linked\n");
		}
	}
	if (p->run_stress) {
		const char *job = p->stress_job ? p->stress_job :
#ifdef DONGTU_PLATFORM
			"all-quick";
#else
			(p->quick_mode ? "all-quick" : "all");
#endif
		if (test_stress_run_job != NULL) {
			printf("\n>>> Running test-stress (job: %s)...\n", job);
			ret |= test_stress_run_job(job);
			collect_stress_result(job);
		} else {
			printf("[test-stress] module is not linked\n");
		}
	}
	if (p->run_cmd) {
		if (test_cmd_run != NULL) {
			printf("\n>>> Running test-cmd...\n");
			ret |= test_cmd_run();
			collect_cmd_result();
		} else {
			printf("[test-cmd] module is not linked\n");
		}
	}
	if (p->run_workload) {
		printf("\n>>> Running typical workloads%s...\n",
		       p->quick_mode ? " (quick)" : "");
		collect_workload_results(p->quick_mode);
	}

	rtbench_result_end();
	if (!p->export_results) {
		printf("\n=============================================================\n");
		printf("[RTOS-Bench] Result export skipped (--no-export)\n");
		printf("=============================================================\n");
	} else if (rtbench_result_export_json(p->output_path) != 0) {
		ret = -1;
	} else {
		printf("\n=============================================================\n");
		printf("[RTOS-Bench] Results saved to: %s\n", p->output_path);
		printf("=============================================================\n");
	}
	if (p->xml_output_path != NULL) {
		if (rtbench_result_export_xml(p->xml_output_path) != 0) {
			ret = -1;
		} else {
			printf("[RTOS-Bench] XML results saved to: %s\n",
			       p->xml_output_path);
		}
	}
	rtbench_result_cleanup();
	return ret;
}

static int run_test_all(struct test_all_params *p)
{
	return rtbench_platform_run_test_all(run_test_all_impl, p);
}

static int run_workload_command(int argc, char **argv)
{
	struct execution_options opts;
	int parse_ret;

	set_default_exec_opts(&opts);
	parse_ret = parse_workload_args(argc, argv, &opts);
	if (parse_ret == RTBENCH_PARSE_HELP) return 0;
	if (parse_ret != 0) return -1;

	if (opts.list_only) {
		printf("Available workloads (%d):\n", rtosbench_workload_count());
		for (int i = 0; i < rtosbench_workload_count(); i++) {
			const struct rtosbench_workload *wl = rtosbench_get_workload(i);
			printf("  %s [%s] - %s\n",
			       (wl && wl->name) ? wl->name : "(null)",
			       (wl && wl->category) ? wl->category : "-",
			       (wl && wl->description) ? wl->description : "");
		}
		return 0;
	}
	if (opts.workload_name == NULL) {
		const struct rtosbench_workload *wl = rtosbench_get_workload(0);
		opts.workload_name = wl ? wl->name : NULL;
	}
	if (opts.workload_name == NULL ||
	    rtosbench_select_workload(opts.workload_name) != 0) {
		printf("No workload selected.\n");
		return -1;
	}
	printf("[rtbench] workload=%s period=%ld.%09ld tasks=%llu\n",
	       rtosbench_current_workload(), opts.period_sec, opts.period_nsec,
	       (unsigned long long)opts.tasks_to_launch);
#if defined(RUIHUA_PLATFORM)
	{
		unsigned long long count = opts.tasks_to_launch ? opts.tasks_to_launch : 1;
		long double start;
		long double end;
		uint64_t period_us = (uint64_t)opts.period_sec * 1000000ULL +
				     (uint64_t)opts.period_nsec / 1000ULL;

		printf("[rtbench] Ruihua direct periodic mode, activations=%llu period=%llu us\n",
		       count, (unsigned long long)period_us);
		if (workload_init(0, NULL) != 0) {
			printf("[rtbench] workload init failed\n");
			return -1;
		}
		start = rtbench_get_timestamp();
		for (unsigned long long i = 0; i < count; i++) {
			long double iter_start = rtbench_get_timestamp();
			workload_exec(0, NULL);
			if (i + 1 < count && period_us > 0) {
				long double elapsed = rtbench_get_timestamp() - iter_start;
				uint64_t elapsed_us = (uint64_t)(elapsed * 1000000.0L);
				if (elapsed_us < period_us) {
					usleep((useconds_t)(period_us - elapsed_us));
				}
			}
		}
		end = rtbench_get_timestamp();
		workload_teardown(0, NULL);
		printf("[rtbench] workload complete, elapsed=%.6Lf sec\n", end - start);
		return 0;
	}
#else
	return periodic_benchmark(&opts);
#endif
}

static int run_test_stress_command(int argc, char **argv)
{
	const char *job_name = "all";
	const char *stressor_name = NULL;
	int list_jobs = 0, quick = 0, single_mode = 0;
	int duration_sec = 10, num_workers = 1;
	uint64_t max_ops = 0;
	const char *method_name = NULL;
	const char *extra_opts = NULL;

	for (int i = 2; i < argc; i++) {
		const char *val;
		if (is_help_arg(argv[i])) return 0;
		if (!strcmp(argv[i], "--job") && i + 1 < argc) job_name = argv[++i];
		else if ((val = stress_opt_arg(argv[i], "--job=")) != NULL) job_name = val;
		else if (!strcmp(argv[i], "-s") && i + 1 < argc) { single_mode = 1; stressor_name = argv[++i]; }
		else if ((val = stress_opt_arg(argv[i], "-s=")) != NULL) { single_mode = 1; stressor_name = val; }
		else if (!strcmp(argv[i], "-t") && i + 1 < argc) duration_sec = atoi(argv[++i]);
		else if ((val = stress_opt_arg(argv[i], "-t=")) != NULL) duration_sec = atoi(val);
		else if (!strcmp(argv[i], "-c") && i + 1 < argc) num_workers = atoi(argv[++i]);
		else if ((val = stress_opt_arg(argv[i], "-c=")) != NULL) num_workers = atoi(val);
		else if (!strcmp(argv[i], "--ops") && i + 1 < argc) max_ops = strtoull(argv[++i], NULL, 10);
		else if ((val = stress_opt_arg(argv[i], "--ops=")) != NULL) max_ops = strtoull(val, NULL, 10);
		else if (!strcmp(argv[i], "--method") && i + 1 < argc) method_name = argv[++i];
		else if ((val = stress_opt_arg(argv[i], "--method=")) != NULL) method_name = val;
		else if (!strcmp(argv[i], "--opts") && i + 1 < argc) extra_opts = argv[++i];
		else if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--list")) list_jobs = 1;
		else if (!strcmp(argv[i], "--quick")) quick = 1;
		else if (!strcmp(argv[i], "-q")) benchmark_verbosity = LOG_LEVEL_INFO;
		else {
			printf("[test-stress] Unknown option: %s\n", argv[i]);
			return -1;
		}
	}
	if (list_jobs) {
		if (test_stress_list_jobs != NULL) test_stress_list_jobs();
		if (test_stress_list_stressors != NULL) test_stress_list_stressors();
		return 0;
	}
	if (single_mode) {
		if (test_stress_run_single == NULL || stressor_name == NULL) {
			printf("[test-stress] module is not linked or -s is missing\n");
			return -1;
		}
		return test_stress_run_single(stressor_name, duration_sec,
					      num_workers, max_ops,
					      method_name, extra_opts);
	}
	if (quick && strstr(job_name, "-quick") == NULL) {
		static char quick_job[64];
		snprintf(quick_job, sizeof(quick_job), "%s-quick", job_name);
		job_name = quick_job;
	}
	if (test_stress_run_job == NULL) {
		printf("[test-stress] module is not linked\n");
		return -1;
	}
	return test_stress_run_job(job_name);
}

int rtbench_command_main(int argc, char **argv)
{
	if (argc < 1 || argv == NULL) {
		return -1;
	}
	ensure_workloads_registered();
	if (argc < 2 || is_help_arg(argv[1])) {
		rtbench_command_print_usage();
		return 0;
	}
	if (!strcmp(argv[1], "test-all")) {
		struct test_all_params params;
		int parse_ret;
		memset(&params, 0, sizeof(params));
		params.run_realtime = params.run_schedule = params.run_stress = 1;
		params.run_cmd = params.run_workload = 1;
		params.export_results = 1;
		params.schedule_cycles = TEST_SCHEDULE_CYCLES;
		params.schedule_util_start = TEST_SCHEDULE_UTIL_START;
		params.schedule_util_end = TEST_SCHEDULE_UTIL_END;
		params.schedule_util_step = TEST_SCHEDULE_UTIL_STEP;
#ifdef DONGTU_PLATFORM
		params.quick_mode = 1;
		params.schedule_cycles = TEST_SCHEDULE_QUICK_CYCLES;
		params.schedule_util_start = TEST_SCHEDULE_QUICK_UTIL_START;
		params.schedule_util_end = TEST_SCHEDULE_QUICK_UTIL_END;
		params.schedule_util_step = TEST_SCHEDULE_QUICK_UTIL_STEP;
#endif
		params.output_path = default_output_path();
		parse_ret = parse_test_all_args(argc, argv, &params);
		if (parse_ret == RTBENCH_PARSE_HELP) return 0;
		if (parse_ret != 0) return -1;
		return run_test_all(&params);
	}
	if (!strcmp(argv[1], "test-schedule")) {
		int ret = run_test_schedule_with_args(argc, argv);
		return ret == RTBENCH_PARSE_HELP ? 0 : ret;
	}
	if (!strcmp(argv[1], "test-realtime")) {
		int run_multicore = 0, run_verify = 0;
		for (int i = 2; i < argc; i++) {
			if (is_help_arg(argv[i])) return 0;
			if (!strcmp(argv[i], "--verify") || !strcmp(argv[i], "-v")) run_verify = 1;
			else if (!strcmp(argv[i], "--multicore") || !strcmp(argv[i], "-m")) run_multicore = 1;
			else if (!strcmp(argv[i], "-q")) benchmark_verbosity = LOG_LEVEL_INFO;
			else return -1;
		}
		if (run_verify && test_realtime_verify != NULL) test_realtime_verify();
		if (test_realtime_run == NULL) {
			printf("[test-realtime] module is not linked\n");
			return -1;
		}
		return test_realtime_run(run_multicore);
	}
	if (!strcmp(argv[1], "test-stress")) return run_test_stress_command(argc, argv);
	if (!strcmp(argv[1], "test-cmd")) {
		if (test_cmd_run == NULL) {
			printf("[test-cmd] module is not linked\n");
			return -1;
		}
		return test_cmd_run();
	}
	if (!strcmp(argv[1], "export-result")) {
		const char *output_path = default_output_path();
		const char *xml_output_path = NULL;
		int ret;
		for (int i = 2; i < argc; i++) {
			const char *value;
			int matched;
			if (is_help_arg(argv[i])) {
				print_export_usage();
				return 0;
			}
			matched = match_value_option(argc, argv, &i, "-o", "--output", &value);
			if (matched < 0) {
				print_export_usage();
				return -1;
			} else if (matched) {
				output_path = value;
				continue;
			}
			matched = match_value_option(argc, argv, &i, NULL, "--xml-output", &value);
			if (matched < 0) {
				print_export_usage();
				return -1;
			} else if (matched) {
				xml_output_path = value;
				continue;
			} else {
				print_export_usage();
				return -1;
			}
		}
		ret = rtbench_result_export_json(output_path);
		if (ret == 0 && xml_output_path != NULL) {
			ret = rtbench_result_export_xml(xml_output_path);
		}
		return ret;
	}
	if (argv[1][0] != '-') {
		printf("[rtbench] Unknown command: %s\n", argv[1]);
		rtbench_command_print_usage();
		return -1;
	}
	return run_workload_command(argc, argv);
}

int rtbench_command_run_benchmark(const char *workload_name,
				  double period_sec, int num_tasks)
{
	char *argv[8];
	char period_buf[32];
	char tasks_buf[32];

	if (workload_name == NULL || workload_name[0] == '\0') {
		workload_name = "stub";
	}
	if (period_sec <= 0.0) period_sec = 1.0;
	if (num_tasks <= 0) num_tasks = 1;
	snprintf(period_buf, sizeof(period_buf), "%.9g", period_sec);
	snprintf(tasks_buf, sizeof(tasks_buf), "%d", num_tasks);
	argv[0] = "rtbench";
	argv[1] = "-b";
	argv[2] = (char *)workload_name;
	argv[3] = "-p";
	argv[4] = period_buf;
	argv[5] = "-t";
	argv[6] = tasks_buf;
	return rtbench_command_main(7, argv);
}

