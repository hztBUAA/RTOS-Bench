#ifndef RTBENCH_SCHED_ATTR_H
#define RTBENCH_SCHED_ATTR_H
/** @file sched_attr.h
 * @ingroup generator
 * @brief sched_{set,get}attr compat layer, inspired by schedutils.
 * @author Andrea Bastoni
 *
 * @copyright (C) 2021 - 2022, Andrea Bastoni <andrea.bastoni@tum.de> and the rt-bench contributors.
 * SPDX-License-Identifier: MIT
 */
#include <inttypes.h>
#include <sched.h>
#include <unistd.h>
#include <sys/syscall.h>

/** @brief The struct which will contain the schedulability properties for this benchmarks.
 * @details
 * Use our own `sched_attr` structure instead of the one in `sched.h` to
 * allow later setting further parameters at the end (e.g., criticality).
 */
struct rtbench_sched_attr {
	uint32_t size; ///< The size of the structure.

	uint32_t sched_policy; ///< The scheduling policy.
	uint64_t sched_flags; ///< The nice value used only in `SCHED_NORMAL` and `SCHED_BATCH`

	/* SCHED_NORMAL, SCHED_BATCH */
	int32_t sched_nice;

	/* SCHED_FIFO, SCHED_RR */
	uint32_t sched_priority; ///< Priority value, used only in `SCHED_FIFO` and `SCHED_RR`.

	/* SCHED_DEADLINE */
	uint64_t sched_runtime; ///< `SCHED_DEADLINE` runtime value.
	uint64_t sched_deadline; ///< `SCHED_DEADLINE` deadline value.
	uint64_t sched_period; ///< `SCHED_DEADLINE` period value.

	/* Utilization hints */
	uint32_t sched_util_min; ///< Utilization hint.
	uint32_t sched_util_max; ///< Utilization hint.
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
