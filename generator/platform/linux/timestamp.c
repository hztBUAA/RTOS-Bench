/**
 * @file timestamp.c
 * @brief Linux timestamp implementation for rt-bench platform abstraction.
 */

#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_LINUX

#include "get_cpu_timestamp.h"

unsigned long long rtbench_get_rdtsc(void)
{
	return get_rdtsc();
}

long double rtbench_get_timestamp(void)
{
	return get_timestamp();
}

#endif /* RTBENCH_PLATFORM_LINUX */
