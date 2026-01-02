/**
 * @file scheduler.c
 * @brief RT-Thread scheduler abstraction for rt-bench
 */

#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_RTTHREAD

#include <rtthread.h>

int rtbench_set_priority(unsigned int priority)
{
	rt_thread_t thread = rt_thread_self();
	rt_uint8_t prio = (rt_uint8_t)priority;

	if (thread == RT_NULL) {
		return -1;
	}

	return (rt_thread_control(thread, RT_THREAD_CTRL_CHANGE_PRIORITY,
				  &prio) == RT_EOK)
		       ? 0
		       : -1;
}

int rtbench_set_deadline(uint64_t runtime, uint64_t deadline, uint64_t period)
{
	(void)runtime;
	(void)deadline;
	(void)period;
	return -1;
}

int rtbench_set_affinity(uint32_t cpu_mask)
{
	rt_thread_t thread = rt_thread_self();
	rt_uint8_t cpu_id = 0;

	if (thread == RT_NULL) {
		return -1;
	}

	if (cpu_mask == 0) {
		return 0;
	}

	while (cpu_id < 32 && ((cpu_mask & (1u << cpu_id)) == 0)) {
		cpu_id++;
	}

#if defined(RT_THREAD_CTRL_BIND_CPU)
	return (rt_thread_control(thread, RT_THREAD_CTRL_BIND_CPU, &cpu_id) ==
		RT_EOK)
		       ? 0
		       : -1;
#else
	(void)cpu_id;
	return -1;
#endif
}

#endif /* RTBENCH_PLATFORM_RTTHREAD */
