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

#include "platform_abstraction.h"
#include "periodic_benchmark.h"
#include "logging.h"
#include "workload_registry.h"
#include "test_schedule.h"
#include "test_realtime.h"
#include "test_stress.h"
#include "test_cmd.h"
#include "result_export.h"

#if defined(SYLIXOS_PLATFORM) || defined(RTBENCH_PLATFORM_SYLIXOS)

/* Forward declarations */
extern void rtosbench_register_rtos_workloads(void);
__attribute__((weak)) int __fdlib_version = -1;

#define SYLIXOS_DEFAULT_OUTPUT_PATH "./rtbench_result.json"

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
	static int printed_entry = 0;
	if (!printed_entry) {
		printf("[rtbench][entry] sylixos\n");
		printed_entry = 1;
	}
	printf("[rtbench] workload=%s period=%ld.%09ld tasks=%llu\n",
	       rtosbench_current_workload(),
	       opts->period_sec, opts->period_nsec,
	       (unsigned long long)opts->tasks_to_launch);
}

extern int run_all_workloads(void);

/* External references to realtime benchmark results (bench_init.c) */
extern uint64_t *get_realtime_service_cost(void);
extern uint64_t *get_realtime_interrupt(void);
extern uint64_t get_realtime_context_switch(void);
extern uint64_t *get_realtime_syscall(void);
extern uint64_t *get_multicore_memory_bandwidth(void);
extern uint64_t *get_multicore_ipc_bandwidth(void);
extern uint64_t *get_multicore_intra_inter_bandwidth(void);
extern uint64_t *get_multicore_init_dlt_latency(void);

#define NS_TO_US(ns) ((double)(ns) / 1000.0)
#define RAW_TO_GBS(v) ((double)(v) / 1000.0)

static void collect_realtime_result(int run_multicore)
{
	struct rtbench_result *r = rtbench_result_get();
	struct rtbench_realtime_result *rt = &r->realtime;
	uint64_t ctx_sw = get_realtime_context_switch();
	uint64_t *interrupt = get_realtime_interrupt();
	uint64_t *syscall = get_realtime_syscall();
	uint64_t *svc = get_realtime_service_cost();

	if (!interrupt || !syscall || !svc) {
		rt->valid = 0;
		return;
	}

	rt->valid = 1;
	rt->multicore_valid = run_multicore ? 1 : 0;

	rt->context_switch_avg_us = NS_TO_US(ctx_sw);
	rt->interrupt_min_us = NS_TO_US(interrupt[0]);
	rt->interrupt_max_us = NS_TO_US(interrupt[1]);
	rt->interrupt_avg_us = NS_TO_US(interrupt[2]);
	rt->syscall_min_us = NS_TO_US(syscall[0]);
	rt->syscall_max_us = NS_TO_US(syscall[1]);
	rt->syscall_avg_us = NS_TO_US(syscall[2]);

	static const char *svc_ops[] = {
		"sem_take", "sem_release",
		"mq_send", "mq_recv",
		"mutex_take", "mutex_release",
		"mempool_alloc", "mempool_free"
	};
	for (int i = 0; i < 8; i++) {
		rtbench_realtime_add_service_cost(
			rt, svc_ops[i],
			NS_TO_US(svc[i * 4 + 0]),
			NS_TO_US(svc[i * 4 + 1]),
			NS_TO_US(svc[i * 4 + 2]),
			NS_TO_US(svc[i * 4 + 3]));
	}

	if (run_multicore) {
		uint64_t *mem_bw = get_multicore_memory_bandwidth();
		uint64_t *ipc_bw = get_multicore_ipc_bandwidth();
		uint64_t *intra_inter = get_multicore_intra_inter_bandwidth();
		uint64_t *task_lat = get_multicore_init_dlt_latency();

		if (!mem_bw || !ipc_bw || !intra_inter || !task_lat) {
			rt->multicore_valid = 0;
			return;
		}

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

static int collect_schedule_result(void)
{
	struct rtbench_result *r = rtbench_result_get();
	struct rtbench_schedule_result *sched = &r->schedule;
	const struct test_schedule_result *src = test_schedule_get_result();

	if (src == NULL) {
		sched->valid = 0;
		return -1;
	}

	sched->valid = 1;
	sched->cycles = TEST_SCHEDULE_CYCLES;
	sched->util_start = TEST_SCHEDULE_UTIL_START;
	sched->util_end = TEST_SCHEDULE_UTIL_END;
	sched->util_step = TEST_SCHEDULE_UTIL_STEP;
	sched->average_miss_rate = src->average_miss_rate;
	sched->final_score = src->final_score;

	sched->gradient_count = src->num_gradients;
	if (sched->gradient_count > RTBENCH_MAX_GRADIENTS) {
		sched->gradient_count = RTBENCH_MAX_GRADIENTS;
	}

	for (int i = 0; i < sched->gradient_count; i++) {
		const struct schedule_gradient_result *g = &src->gradients[i];
		struct rtbench_gradient_result *out = &sched->gradients[i];

		out->utilization_percent = g->utilization_percent;
		out->actual_utilization = g->actual_utilization;
		out->total_jobs = g->total_jobs;
		out->deadline_misses = g->total_misses;
		out->miss_rate = g->miss_rate;
		out->task_count = g->num_tasks;
		if (out->task_count > RTBENCH_MAX_WORKLOADS) {
			out->task_count = RTBENCH_MAX_WORKLOADS;
		}

		for (int j = 0; j < out->task_count; j++) {
			const struct schedule_task_stats *src_task = &g->task_stats[j];
			struct rtbench_task_stat *dst_task = &out->task_stats[j];

			if (src_task->name) {
				strncpy(dst_task->name, src_task->name,
					sizeof(dst_task->name) - 1);
			}
			dst_task->jobs = src_task->total_jobs;
			dst_task->misses = src_task->deadline_misses;
			dst_task->max_response_ms =
				(double)src_task->max_response_ns / 1000000.0;
		}
	}
	return 0;
}

static int collect_stress_result(int module_ret)
{
	struct rtbench_result *r = rtbench_result_get();
	struct rtbench_stress_result *stress = &r->stress;
	int count = 0;
	int has_failure = (module_ret != 0) ? 1 : 0;
	double total_duration = 0.0;
	const struct test_stress_job_result *results =
		test_stress_get_job_results(&count);

	if (!results || count <= 0) {
		stress->valid = 0;
		return -1;
	}

	stress->valid = 1;

	for (int i = 0; i < count; i++) {
		const struct test_stress_job_result *jr = &results[i];

		total_duration += jr->duration_sec;
		rtbench_stress_add_stressor(stress, jr->name, jr->type, jr->stage,
					    jr->bogo_ops, jr->duration_sec,
					    jr->metric_value, jr->metric_unit);
		if (!jr->success) {
			has_failure = 1;
		}
	}

	stress->duration_sec = total_duration;
	return has_failure ? -1 : 0;
}

static int collect_cmd_result(int module_ret)
{
	struct rtbench_result *r = rtbench_result_get();
	struct rtbench_cmd_module_result *dst = &r->cmd;
	const struct test_cmd_module_result *src = test_cmd_get_result();

	if (!src || !src->valid) {
		dst->valid = 0;
		return -1;
	}

	dst->valid = 1;
	dst->cmd_count = src->cmd_count;
	dst->pass_count = src->pass_count;

	for (int i = 0; i < src->cmd_count && i < RTBENCH_MAX_CMD_COMMANDS; i++) {
		if (src->results[i].command) {
			strncpy(dst->results[i].command, src->results[i].command,
				sizeof(dst->results[i].command) - 1);
		}
		if (src->results[i].name) {
			strncpy(dst->results[i].name, src->results[i].name,
				sizeof(dst->results[i].name) - 1);
		}
		dst->results[i].supported = src->results[i].supported;
	}

	if (module_ret != 0 || dst->pass_count < dst->cmd_count) {
		return -1;
	}
	return 0;
}

static int collect_workload_results(void)
{
	struct rtbench_result *r = rtbench_result_get();
	struct rtbench_workload_module_result *wl = &r->workload;
	int has_failure = 0;
	int executed = 0;
	long double start = rtbench_get_timestamp();

	wl->valid = 1;

	for (int i = 0; i < rtosbench_workload_count(); i++) {
		const struct rtosbench_workload *w = rtosbench_get_workload(i);
		long double run_start, run_end;
		double total_ms;

		if (!w || !w->name || !w->exec) {
			continue;
		}
		if (w->category && strcmp(w->category, "synthetic") == 0) {
			continue;
		}

		printf("  Running workload: %s\n", w->name);

		if (w->init && w->init(0, NULL) != 0) {
			rtbench_workload_add_result(wl, w->name,
						    w->category ? w->category : "",
						    0, 0, 0.0, 0.0);
			has_failure = 1;
			continue;
		}

		run_start = rtbench_get_timestamp();
		w->exec(0, NULL);
		run_end = rtbench_get_timestamp();
		total_ms = (double)(run_end - run_start) * 1000.0;
		executed++;

		if (w->teardown) {
			w->teardown(0, NULL);
		}

		rtbench_workload_add_result(wl, w->name,
					    w->category ? w->category : "",
					    1, 1, total_ms, total_ms);
		printf("    %s: %.3f ms\n", w->name, total_ms);
	}

	wl->duration_sec = (double)(rtbench_get_timestamp() - start);

	if (executed == 0) {
		wl->valid = 0;
		return -1;
	}
	if (has_failure) {
		return -1;
	}
	return 0;
}

/**
 * @brief Main entry point for SylixOS
 */
int main(int argc, char **argv)
{
	struct execution_options opts;
	const char *output_path = SYLIXOS_DEFAULT_OUTPUT_PATH;

	if (argc == 2 && strcmp(argv[1], "-s") == 0) {
        run_all_workloads();
        return 0; // 执行完毕后直接退出，不再走后续的 argp 解析和 benchmark 流程
    }
	
	/* Register workloads */
	rtosbench_register_rtos_workloads();

	/* Handle export-result subcommand */
	if (argc >= 2 && strcmp(argv[1], "export-result") == 0) {
		for (int i = 2; i < argc; i++) {
			if ((strcmp(argv[i], "-o") == 0 ||
			     strcmp(argv[i], "--output") == 0) &&
			    (i + 1 < argc)) {
				output_path = argv[++i];
			}
		}
		int ret = rtbench_result_export_json(output_path);
		if (ret == 0) {
			printf("[RTOS-Bench] Results exported to: %s\n",
			       output_path);
		}
		return ret;
	}

	/* Handle test-all subcommand */
	if (argc >= 2 && strcmp(argv[1], "test-all") == 0) {
		int run_realtime = 1;
		int run_schedule = 1;
		int run_stress = 1;
		int run_cmd = 1;
		int run_workload = 1;
		int run_multicore = 0;
		int overall_fail = 0;
		int mod_ret = 0;

		for (int i = 2; i < argc; i++) {
			if ((strcmp(argv[i], "-o") == 0 ||
			     strcmp(argv[i], "--output") == 0) &&
			    (i + 1 < argc)) {
				output_path = argv[++i];
			} else if (strcmp(argv[i], "--no-realtime") == 0) {
				run_realtime = 0;
			} else if (strcmp(argv[i], "--no-schedule") == 0) {
				run_schedule = 0;
			} else if (strcmp(argv[i], "--no-stress") == 0) {
				run_stress = 0;
			} else if (strcmp(argv[i], "--no-cmd") == 0) {
				run_cmd = 0;
			} else if (strcmp(argv[i], "--no-workload") == 0) {
				run_workload = 0;
			} else if (strcmp(argv[i], "--multicore") == 0 ||
				   strcmp(argv[i], "-m") == 0) {
				run_multicore = 1;
			} else if (strcmp(argv[i], "-q") == 0) {
				benchmark_verbosity = LOG_LEVEL_INFO;
			}
		}

		rtbench_result_init();
		rtbench_result_set_env("SylixOS", "unknown", "unknown", "unknown",
				       0, 0);
		rtbench_result_start();

		printf("\n=============================================================\n");
		printf("[RTOS-Bench] Comprehensive Test Suite (SylixOS)\n");
		printf("=============================================================\n");
		printf("Output: %s\n", output_path);
		printf("Modules: realtime=%s schedule=%s stress=%s cmd=%s workload=%s\n",
		       run_realtime ? "yes" : "no",
		       run_schedule ? "yes" : "no",
		       run_stress ? "yes" : "no",
		       run_cmd ? "yes" : "no",
		       run_workload ? "yes" : "no");

		if (run_realtime) {
			printf("\n>>> Running test-realtime...\n");
			mod_ret = test_realtime_run(run_multicore);
			if (mod_ret == 0) {
				collect_realtime_result(run_multicore);
			} else {
				overall_fail = 1;
			}
		}

		if (run_schedule) {
			printf("\n>>> Running test-schedule...\n");
			mod_ret = test_schedule_run();
			if (collect_schedule_result() != 0 || mod_ret != 0) {
				overall_fail = 1;
			}
		}

		if (run_stress) {
			printf("\n>>> Running test-stress (job: all)...\n");
			mod_ret = test_stress_run_job("all");
			if (collect_stress_result(mod_ret) != 0) {
				overall_fail = 1;
			}
		}

		if (run_cmd) {
			printf("\n>>> Running test-cmd...\n");
			mod_ret = test_cmd_run();
			if (collect_cmd_result(mod_ret) != 0) {
				overall_fail = 1;
			}
		}

		if (run_workload) {
			printf("\n>>> Running typical workloads...\n");
			mod_ret = collect_workload_results();
			if (mod_ret != 0) {
				overall_fail = 1;
			}
		}

		rtbench_result_end();
		mod_ret = rtbench_result_export_json(output_path);
		if (mod_ret == 0) {
			printf("\n=============================================================\n");
			printf("[RTOS-Bench] Results saved to: %s\n", output_path);
			if (overall_fail) {
				printf("[RTOS-Bench] One or more modules failed\n");
			}
			printf("=============================================================\n");
		} else {
			printf("\n[RTOS-Bench] Failed to save results: %d\n", mod_ret);
		}
		rtbench_result_cleanup();

		if (mod_ret != 0) {
			return mod_ret;
		}
		return overall_fail ? -1 : 0;
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
			} else if (strcmp(argv[i], "-l") == 0 ||
			           strcmp(argv[i], "--list") == 0) {
				list_jobs = 1;
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
		printf("[test-cmd] Starting shell command support test on SylixOS\n");
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
#endif /* SYLIXOS_PLATFORM || RTBENCH_PLATFORM_SYLIXOS */
