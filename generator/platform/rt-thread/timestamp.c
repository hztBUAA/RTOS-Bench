/**
 * @file timestamp.c
 * @brief RT-Thread timestamp implementation for rt-bench
 */

#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_RTTHREAD

#include <rtthread.h>

unsigned long long rtbench_get_rdtsc(void)
{
	rt_tick_t tick = rt_tick_get();
	unsigned long long cycles =
		(unsigned long long)tick *
		(100000000ULL / RT_TICK_PER_SECOND);

	return cycles;
}

long double rtbench_get_timestamp(void)
{
	rt_tick_t tick = rt_tick_get();
	return (long double)tick / (long double)RT_TICK_PER_SECOND;
}

#endif /* RTBENCH_PLATFORM_RTTHREAD */
