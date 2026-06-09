/**
 * @file dongtu_entry.c
 * @brief Thin Dongtu/Intewell entry for the shared RTOS-Bench dispatcher.
 */

#include "rtbench_command.h"
#include "test_schedule.h"

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
	return "/nfsd/rtbench_result.json";
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
