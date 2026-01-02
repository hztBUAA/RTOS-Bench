#include "benchmark_registry.h"
#include "platform_abstraction.h"

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
	/* Busy-wait for a small interval to simulate work */
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

const struct rtbench_ops rtbench_busywait_ops = {
	.name = "busywait",
	.init = busywait_init,
	.exec = busywait_exec,
	.teardown = busywait_teardown,
};
