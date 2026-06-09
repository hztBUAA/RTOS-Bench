#include "test_schedule/sched_workloads.h"

const struct sched_workload_wrapper *sched_get_wrapper(const char *name)
{
	(void)name;
	return 0;
}

int sched_wrapper_count(void)
{
	return 0;
}

const struct sched_workload_wrapper *sched_get_wrapper_by_index(int idx)
{
	(void)idx;
	return 0;
}
