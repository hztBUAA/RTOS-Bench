/**
 * @file rtbench_command.h
 * @brief Common RTOS-Bench command dispatcher used by platform entries.
 */

#ifndef RTBENCH_COMMAND_H
#define RTBENCH_COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

struct rtbench_platform_env {
	const char *os_name;
	const char *os_version;
	const char *board;
	const char *cpu_type;
	unsigned int cpu_freq_mhz;
	unsigned int cpu_core_num;
};

typedef int (*rtbench_command_runner_fn)(void *ctx);

int rtbench_command_main(int argc, char **argv);
int rtbench_command_run_benchmark(const char *workload_name,
				  double period_sec, int num_tasks);
void rtbench_command_print_usage(void);

const char *rtbench_platform_default_output_path(void);
void rtbench_platform_get_env(struct rtbench_platform_env *env);
int rtbench_platform_run_test_all(rtbench_command_runner_fn runner, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* RTBENCH_COMMAND_H */
