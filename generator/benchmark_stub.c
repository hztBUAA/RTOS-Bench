#ifdef RT_THREAD_PLATFORM
#include "periodic_benchmark.h"

int benchmark_init(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	return 0;
}

void benchmark_execution(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
}

void benchmark_teardown(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
}
#endif
