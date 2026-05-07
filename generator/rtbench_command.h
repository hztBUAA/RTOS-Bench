/**
 * @file rtbench_command.h
 * @brief Common RTOS-Bench command dispatcher used by platform entries.
 */

#ifndef RTBENCH_COMMAND_H
#define RTBENCH_COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

int rtbench_command_main(int argc, char **argv);
int rtbench_command_run_benchmark(const char *workload_name,
				  double period_sec, int num_tasks);
void rtbench_command_print_usage(void);

#ifdef __cplusplus
}
#endif

#endif /* RTBENCH_COMMAND_H */
