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

#include "rtbench_command.h"

#include <rtthread.h>

#define _RTBENCH_STR(x) #x
#define _RTBENCH_XSTR(x) _RTBENCH_STR(x)
#define RT_VERSION_STRING \
	_RTBENCH_XSTR(RT_VERSION_MAJOR) "." \
	_RTBENCH_XSTR(RT_VERSION_MINOR) "." \
	_RTBENCH_XSTR(RT_VERSION_PATCH)

#define RTBENCH_TEST_ALL_STACK_SIZE (32 * 1024)

void rtosbench_register_rtos_workloads(void);

struct rtthread_test_all_job {
	rtbench_command_runner_fn runner;
	void *ctx;
	int result;
	struct rt_semaphore done_sem;
};

static void rtthread_test_all_thread(void *parameter)
{
	struct rtthread_test_all_job *job =
		(struct rtthread_test_all_job *)parameter;

	job->result = job->runner ? job->runner(job->ctx) : -1;
	rt_sem_release(&job->done_sem);
}

const char *rtbench_platform_default_output_path(void)
{
	return "/rtbench_result.json";
}

void rtbench_platform_get_env(struct rtbench_platform_env *env)
{
	if (env == RT_NULL) {
		return;
	}
	env->os_name = "RT-Thread";
	env->os_version = RT_VERSION_STRING;
	env->board = "RT-Thread-Board";
	env->cpu_type = "Unknown";
	env->cpu_freq_mhz = 0;
	env->cpu_core_num = 1;
}

int rtbench_platform_run_test_all(rtbench_command_runner_fn runner, void *ctx)
{
	struct rtthread_test_all_job job;
	rt_thread_t thread;

	if (runner == RT_NULL) {
		return -1;
	}
	job.runner = runner;
	job.ctx = ctx;
	job.result = -1;
	rt_sem_init(&job.done_sem, "ta_done", 0, RT_IPC_FLAG_PRIO);

	thread = rt_thread_create("rtbench",
				  rtthread_test_all_thread,
				  &job,
				  RTBENCH_TEST_ALL_STACK_SIZE,
				  20,
				  10);
	if (thread == RT_NULL) {
		rt_kprintf("[RTOS-Bench] Failed to create test-all worker thread\n");
		rt_sem_detach(&job.done_sem);
		return -1;
	}
	rt_thread_startup(thread);
	rt_sem_take(&job.done_sem, RT_WAITING_FOREVER);
	rt_sem_detach(&job.done_sem);
	return job.result;
}

int rtosbench_rtthread_entry(int argc, char **argv)
{
	return rtbench_command_main(argc, argv);
}

int rtbench_rtthread_entry(int argc, char **argv)
{
	return rtosbench_rtthread_entry(argc, argv);
}

#endif /* RT_THREAD_PLATFORM */
