/**
 * @file dongtu_entry.c
 * @brief Thin Dongtu/Intewell entry for the shared RTOS-Bench dispatcher.
 */

#include "rtbench_command.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

#define RTBENCH_TEST_ALL_STACK_SIZE (32 * 1024)

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

struct dongtu_test_all_job {
	rtbench_command_runner_fn runner;
	void *ctx;
	int result;
	sem_t done;
};

static void *dongtu_test_all_thread(void *arg)
{
	struct dongtu_test_all_job *job = (struct dongtu_test_all_job *)arg;

	job->result = job->runner ? job->runner(job->ctx) : -1;
	sem_post(&job->done);
	return NULL;
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
	env->os_name = "Dongtu";
	env->os_version = "Intewell";
	env->board = "Dongtu-OrangePi";
	env->cpu_type = "aarch64";
	env->cpu_freq_mhz = 0;
	env->cpu_core_num = 8;
}

int rtbench_platform_run_test_all(rtbench_command_runner_fn runner, void *ctx)
{
	struct dongtu_test_all_job job;
	pthread_t tid;
	pthread_attr_t attr;
	int ret;

	if (runner == NULL) {
		return -1;
	}
	job.runner = runner;
	job.ctx = ctx;
	job.result = -1;
	if (sem_init(&job.done, 0, 0) != 0) {
		return -1;
	}

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, RTBENCH_TEST_ALL_STACK_SIZE);
	ret = pthread_create(&tid, &attr, dongtu_test_all_thread, &job);
	pthread_attr_destroy(&attr);
	if (ret != 0) {
		printf("[RTOS-Bench] Failed to create test-all worker thread\n");
		sem_destroy(&job.done);
		return -1;
	}

	sem_wait(&job.done);
	pthread_join(tid, NULL);
	sem_destroy(&job.done);
	return job.result;
}

int rtbench_dongtu_entry(int argc, char **argv)
{
	return rtbench_command_main(argc, argv);
}

int main(int argc, char **argv)
{
	return rtbench_dongtu_entry(argc, argv);
}
