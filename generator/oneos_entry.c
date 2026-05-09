/**
 * @file oneos_entry.c
 * @brief Thin OneOS shell entry for the shared RTOS-Bench dispatcher.
 */

#include "rtbench_command.h"
#include "workload_registry.h"

#include <os_task.h>
#include <os_sem.h>
#include <oneos_config.h>
#include <shell.h>
#include <stdio.h>

#define RTBENCH_TEST_ALL_STACK_SIZE (32 * 1024)

extern int cusum_bench_run(void);
extern int ewma_bench_run(void);

static int cusum_init(int n, void **p) { (void)n; (void)p; return 0; }
static void cusum_exec(int n, void **p) { (void)n; (void)p; cusum_bench_run(); }
static void cusum_teardown(int n, void **p) { (void)n; (void)p; }

static const struct rtosbench_workload rtosbench_cusum_workload = {
	.name = "cusum",
	.description = "CUSUM mean-shift detector",
	.category = "detection",
	.init = cusum_init,
	.exec = cusum_exec,
	.teardown = cusum_teardown,
};

static int ewma_init(int n, void **p) { (void)n; (void)p; return 0; }
static void ewma_exec(int n, void **p) { (void)n; (void)p; ewma_bench_run(); }
static void ewma_teardown(int n, void **p) { (void)n; (void)p; }

static const struct rtosbench_workload rtosbench_ewma_workload = {
	.name = "ewma",
	.description = "EWMA residual thresholding",
	.category = "detection",
	.init = ewma_init,
	.exec = ewma_exec,
	.teardown = ewma_teardown,
};

static int g_oneos_workloads_registered;

void __attribute__((weak)) rtosbench_register_rtos_workloads(void)
{
	if (g_oneos_workloads_registered) {
		return;
	}
	g_oneos_workloads_registered = 1;
	rtosbench_register_workload(&rtosbench_cusum_workload);
	rtosbench_register_workload(&rtosbench_ewma_workload);
}

struct oneos_test_all_job {
	rtbench_command_runner_fn runner;
	void *ctx;
	int result;
	os_semaphore_id done_sem;
};

static void oneos_test_all_thread(void *parameter)
{
	struct oneos_test_all_job *job = (struct oneos_test_all_job *)parameter;

	job->result = job->runner ? job->runner(job->ctx) : -1;
	os_semaphore_post(job->done_sem);
}

const char *rtbench_platform_default_output_path(void)
{
	return "/rtbench_result.json";
}

void rtbench_platform_get_env(struct rtbench_platform_env *env)
{
	if (env == NULL) {
		return;
	}
	env->os_name = "OneOS";
	env->os_version = "3.x";
	env->board = "OneOS-Board";
	env->cpu_type = "ARM";
	env->cpu_freq_mhz = 0;
	env->cpu_core_num = 1;
}

int rtbench_platform_run_test_all(rtbench_command_runner_fn runner, void *ctx)
{
	struct oneos_test_all_job job;
	static os_task_dummy_t test_all_task_cb;
	os_task_id task;

	if (runner == NULL) {
		return -1;
	}
	job.runner = runner;
	job.ctx = ctx;
	job.result = -1;
	job.done_sem = os_semaphore_create(NULL, "ta_done", 0, OS_SEM_MAX_VALUE);
	if (job.done_sem == NULL) {
		printf("[RTOS-Bench] Failed to create semaphore\n");
		return -1;
	}

	task = os_task_create(&test_all_task_cb,
			      RTBENCH_TEST_ALL_STACK_SIZE,
			      "rtbench",
			      oneos_test_all_thread,
			      &job,
			      OS_TASK_PRIORITY_MAX / 2,
			      10);
	if (task < 0) {
		printf("[RTOS-Bench] Failed to create test-all worker task\n");
		os_semaphore_destroy(job.done_sem);
		return -1;
	}
	os_task_startup(task);
	os_semaphore_wait(job.done_sem, OS_WAIT_FOREVER);
	os_semaphore_destroy(job.done_sem);
	return job.result;
}

int cmd_rtbench_stub(int argc, char **argv)
{
	return rtbench_command_main(argc, argv);
}

static int cmd_rtbench(int argc, char **argv)
{
	return cmd_rtbench_stub(argc, argv);
}

SH_CMD_EXPORT(rtbench, cmd_rtbench, "RTOS-Bench workload runner");

static int rtbench_auto_init(void)
{
	rtosbench_register_rtos_workloads();
	return 0;
}

OS_APP_INIT(rtbench_auto_init);
