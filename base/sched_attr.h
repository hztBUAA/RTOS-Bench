#ifndef RTBENCH_SCHED_ATTR_H
#define RTBENCH_SCHED_ATTR_H
/*
 * sched_{set,get}attr compat layer, inspired by schedutils
 */
#include <inttypes.h>
#include <sched.h>
#include <unistd.h>
#include <sys/syscall.h>

/* Use our own sched_attr structure instead of the one in sched.h to
 * allow later setting further parameters at the end (e.g., criticality).
 */
struct rtbench_sched_attr {
	uint32_t size;

	uint32_t sched_policy;
	uint64_t sched_flags;

	/* SCHED_NORMAL, SCHED_BATCH */
	int32_t sched_nice;

	/* SCHED_FIFO, SCHED_RR */
	uint32_t sched_priority;

	/* SCHED_DEADLINE */
	uint64_t sched_runtime;
	uint64_t sched_deadline;
	uint64_t sched_period;

	/* Utilization hints */
	uint32_t sched_util_min;
	uint32_t sched_util_max;

	/* Criticality */
	uint32_t sched_criticality;
	uint32_t padding;
};

static inline int sched_setattr(pid_t pid, const struct rtbench_sched_attr *attr, unsigned int flags)
{
	return syscall(SYS_sched_setattr, pid, attr, flags);
}

static inline int sched_getattr(pid_t pid, struct rtbench_sched_attr *attr, unsigned int size, unsigned int flags)
{
	return syscall(SYS_sched_getattr, pid, attr, size, flags);
}

#endif
