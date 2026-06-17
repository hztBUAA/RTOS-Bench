/**
 * @file dongtu_entry.c
 * @brief Dongtu/Intewell entry for RTOS-Bench.
 */

#include "rtbench_command.h"

#include <stddef.h>
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

#if defined(BOARD_NAME)
	env->board = BOARD_NAME;
#elif defined(_X86_) || defined(_X86_32_) || defined(_I386_) || defined(__i386__) || defined(__x86_64__)
	env->board = "Dongtu-x86";
#elif defined(__aarch64__) || defined(__ARM64__) || defined(_AARCH64_)
	env->board = "Dongtu-OrangePi";
#else
	env->board = "Dongtu-Board";
#endif

#if defined(_X86_) || defined(_X86_32_) || defined(_I386_) || defined(__i386__)
	env->cpu_type = "x86";
#elif defined(__x86_64__)
	env->cpu_type = "x86_64";
#elif defined(__aarch64__) || defined(__ARM64__) || defined(_AARCH64_)
	env->cpu_type = "aarch64";
#else
	env->cpu_type = "Unknown";
#endif

	env->cpu_freq_mhz = 0;
	env->cpu_core_num = 0;
}

int rtbench_platform_run_test_all(rtbench_command_runner_fn runner, void *ctx)
{
	return runner ? runner(ctx) : -1;
}

int rtbench_dongtu_entry(int argc, char **argv)
{
	return rtbench_command_main(argc, argv);
}

int main(int argc, char **argv)
{
	return rtbench_dongtu_entry(argc, argv);
}
