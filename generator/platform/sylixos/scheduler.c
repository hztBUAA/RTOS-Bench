/**
 * @file scheduler.c
 * @brief SylixOS scheduler abstraction for rt-bench.
 * @details SylixOS supports POSIX sched_setscheduler but not SCHED_DEADLINE.
 */

#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_SYLIXOS

#include <sched.h>
#include <pthread.h>
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
	/* SylixOS does not support SCHED_DEADLINE */
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
	/* CPU affinity not supported */
	(void)cpu_mask;
	return -1;
#endif
}

#endif /* RTBENCH_PLATFORM_SYLIXOS */
