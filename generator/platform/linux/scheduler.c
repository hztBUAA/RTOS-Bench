/**
 * @file scheduler.c
 * @brief Linux scheduler abstraction for rt-bench.
 */

#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_LINUX

#include "logging.h"
#include "sched_attr.h"
#include <sched.h>
#include <string.h>

int rtbench_set_priority(unsigned int priority)
{
	int ret;
	struct rtbench_sched_attr attr;

	memset(&attr, 0, sizeof(attr));
	if (priority > sched_get_priority_max(SCHED_FIFO)) {
		priority = sched_get_priority_max(SCHED_FIFO);
	}

	attr.sched_policy = SCHED_FIFO;
	attr.sched_priority = priority;

	ret = sched_setattr(0, &attr, 0);
	if (ret != 0) {
		return ret;
	}

	memset(&attr, 0, sizeof(attr));
	attr.size = sizeof(attr);

	ret = sched_getattr(0, &attr, sizeof(attr), 0);
	if (ret != 0) {
		return ret;
	}

	elogf(LOG_LEVEL_INFO,
	      "\nsize: %u, policy: %u, flags: %lu, prio: %u"
	      "\nT: %lu, D: %lu, P: %lu\n",
	      attr.size, attr.sched_policy, attr.sched_flags,
	      attr.sched_priority, attr.sched_runtime, attr.sched_deadline,
	      attr.sched_period);

	return ret;
}

int rtbench_set_deadline(uint64_t runtime, uint64_t deadline, uint64_t period)
{
	int ret;
	struct rtbench_sched_attr attr;

	memset(&attr, 0, sizeof(attr));
	if (period == 0) {
		return -1;
	}

	if (deadline == 0) {
		deadline = period;
	}

	if (runtime == 0) {
		runtime = deadline;
	}

	attr.size = sizeof(struct rtbench_sched_attr);
	attr.sched_policy = SCHED_DEADLINE;
	attr.sched_runtime = runtime;
	attr.sched_deadline = deadline;
	attr.sched_period = period;

	ret = sched_setattr(0, &attr, 0);
	if (ret != 0) {
		return ret;
	}

	memset(&attr, 0, sizeof(attr));
	attr.size = sizeof(attr);

	ret = sched_getattr(0, &attr, sizeof(attr), 0);
	if (ret != 0) {
		return ret;
	}

	elogf(LOG_LEVEL_INFO,
	      "\nsize: %u, policy: %u, flags: %lu, prio: %u"
	      "\nT: %lu, D: %lu, P: %lu\n",
	      attr.size, attr.sched_policy, attr.sched_flags,
	      attr.sched_priority, attr.sched_runtime, attr.sched_deadline,
	      attr.sched_period);

	return ret;
}

int rtbench_set_affinity(uint32_t cpu_mask)
{
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

	return sched_setaffinity(0, sizeof(set), &set);
}

#endif /* RTBENCH_PLATFORM_LINUX */
