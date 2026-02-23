/**
 * @file scheduler.c
 * @brief POSIX-lite scheduler abstraction for OneOS / Dongtu / Ruihua.
 * @details Provides FIFO priority + optional affinity. Deadline is not
 *          supported on these targets and returns -1.
 */

#include "platform_abstraction.h"

#if defined(RTBENCH_PLATFORM_ONEOS) || defined(RTBENCH_PLATFORM_DONGTU) ||     \
	defined(RTBENCH_PLATFORM_RUIHUA)

#include <pthread.h>
#include <sched.h>
#include <string.h>

int rtbench_set_priority(unsigned int priority)
{
	struct sched_param param;
	int max_prio;

	max_prio = sched_get_priority_max(SCHED_FIFO);
	if ((int)priority > max_prio) {
		priority = (unsigned int)max_prio;
	}

	memset(&param, 0, sizeof(param));
	param.sched_priority = (int)priority;

	return pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
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
#ifdef CPU_SETSIZE
	cpu_set_t set;
	int cpu;

	CPU_ZERO(&set);
	for (cpu = 0; cpu < 32; cpu++) {
		if (cpu_mask & (1u << cpu)) {
			CPU_SET(cpu, &set);
		}
	}

	if (cpu_mask == 0) {
		return 0;
	}

	return pthread_setaffinity_np(pthread_self(), sizeof(set), &set);
#else
	(void)cpu_mask;
	return -1;
#endif
}

#endif /* RTBENCH_PLATFORM_ONEOS || RTBENCH_PLATFORM_DONGTU || RTBENCH_PLATFORM_RUIHUA */
