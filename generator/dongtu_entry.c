/**
 * @file dongtu_entry.c
 * @brief Thin Dongtu/Intewell entry for the shared RTOS-Bench dispatcher.
 */

#include "rtbench_command.h"

#include <stdio.h>

extern void rtosbench_register_rtos_workloads(void);
extern int test_schedule_run_custom(int cycles, int util_start,
				    int util_end, int util_step);
extern int test_realtime_run(int run_multicore);
extern int test_stress_run_job(const char *job_name);
extern int test_cmd_run(void);

#if defined(__GNUC__)
#define RTBENCH_USED __attribute__((used))
#else
#define RTBENCH_USED
#endif

/*
 * Dongtu links RTOS-Bench objects through the IDE-generated static library.
 * Keep strong references here so the linker extracts the full framework
 * modules while rtbench_command.c can still use weak references for minimal
 * platform builds.
 */
static void * const g_dongtu_required_modules[] RTBENCH_USED = {
	(void *)rtosbench_register_rtos_workloads,
	(void *)test_schedule_run_custom,
	(void *)test_realtime_run,
	(void *)test_stress_run_job,
	(void *)test_cmd_run,
};

const char *rtbench_platform_default_output_path(void)
{
	return "/rtbench_result.json";
}

void rtbench_platform_get_env(struct rtbench_platform_env *env)
{
	if (env == NULL) {
		return;
	}
	env->os_name = "Dongtu";
	env->os_version = "Intewell";
	env->board = "Dongtu-OrangePi";
	env->cpu_type = "aarch64";
	env->cpu_freq_mhz = 0;
	env->cpu_core_num = 8;
}

int rtbench_platform_run_test_all(rtbench_command_runner_fn runner, void *ctx)
{
	return runner ? runner(ctx) : -1;
}

/* ============================================================================
 * test-all worker thread (uses pthread to get large stack)
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
	sem_t done_sem;
};

static void *test_all_thread_entry(void *parameter)
{
	struct test_all_params *p = (struct test_all_params *)parameter;

	/* Initialize result collection */
	rtbench_result_init();
	rtbench_result_set_env("Dongtu", "Intewell", "Dongtu-Board", "x86_64", 0, 1);
	rtbench_result_start();

	/* Register workloads */
	rtosbench_register_rtos_workloads();

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
	sem_post(&p->done_sem);
	return NULL;
}

extern int run_all_workloads(void);
/**
 * @brief Shell command entry point for Dongtu RTOS
 *
 * This function can be registered as a shell command in Dongtu's shell system.
 * The exact registration method depends on the Dongtu version and configuration.
 */
int rtbench_dongtu_entry(int argc, char **argv)
{
	return rtbench_command_main(argc, argv);
}

int main(int argc, char **argv)
{
	return rtbench_dongtu_entry(argc, argv);
}
