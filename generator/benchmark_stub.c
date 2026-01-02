#include "benchmark_registry.h"

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

const struct rtbench_ops rtbench_stub_ops = {
	.name = "stub",
	.init = stub_init,
	.exec = stub_exec,
	.teardown = stub_teardown,
};
