/**
 * @file workload_busywait.c
 * @brief Busy-wait workload for RTOS-Bench
 */

#include "workload_registry.h"
#include "platform_abstraction.h"
#include <stddef.h>

static int busywait_init(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	return 0;
}

static void busywait_exec(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;

	/* Busy-wait for a small interval to simulate CPU-bound work */
	unsigned long long start = rtbench_get_rdtsc();
	while ((rtbench_get_rdtsc() - start) < 100000ULL) {
		/* spin */
	}
}

static void busywait_teardown(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
}

const struct rtosbench_workload rtosbench_busywait_workload = {
	.name = "busywait",
	.description = "CPU busy-wait workload",
	.init = busywait_init,
	.exec = busywait_exec,
	.teardown = busywait_teardown,
#ifdef EXTENDED_REPORT
	.log_header = NULL,
	.log_data = NULL,
#endif
};
