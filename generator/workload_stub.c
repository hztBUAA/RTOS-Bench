/**
 * @file workload_stub.c
 * @brief Stub workload (no-op) for RTOS-Bench testing
 */

#include "workload_registry.h"
#include <stddef.h>

static int stub_init(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	return 0;
}

static void stub_exec(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	/* no-op */
}

static void stub_teardown(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
}

const struct rtosbench_workload rtosbench_stub_workload = {
	.name = "stub",
	.description = "No-op stub workload for testing",
	.init = stub_init,
	.exec = stub_exec,
	.teardown = stub_teardown,
#ifdef EXTENDED_REPORT
	.log_header = NULL,
	.log_data = NULL,
#endif
};
