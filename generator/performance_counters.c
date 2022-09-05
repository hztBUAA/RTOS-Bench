/** @file performance_counters.c
 * @ingroup generator
 * @brief Implementation of a architecture independent and highly abstrcat way access to performance counters.
 * @details Uses interface provided by the Linux kernel to access teh performance counters value.
 * @author Denis Hoornaert
 *
 * @copyright (C) 2021 - 2022, Denis Hoornaert <denis.hoornaert@tum.de> and the rt-bench contributors.
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "performance_counters.h"
#include "logging.h"

/// System-call number to open performance counter event.
#ifdef AARCH64
#define __NR_perf_event_open 241
#elif X86_64
#define __NR_perf_event_open 298
//#elif X86_64
//#define __NR_perf_event_open 336
#endif

/// Core model specific performance counter event IDs
#ifdef CORTEX_A53
#define L1_REFERENCES 0x04
#define L1_REFILLS 0x03
#define L2_REFERENCES 0x16
#define L2_REFILLS 0x17
#define INST_RETIRED 0x08
#define CPU_CYCLES 0x11
#elif CORE_I7
#define L1_REFERENCES (PERF_COUNT_HW_CACHE_L1D)|(PERF_COUNT_HW_CACHE_OP_READ<<8)|(PERF_COUNT_HW_CACHE_RESULT_ACCESS<<16)
#define L1_REFILLS (PERF_COUNT_HW_CACHE_L1D)|(PERF_COUNT_HW_CACHE_OP_READ<<8)|(PERF_COUNT_HW_CACHE_RESULT_MISS<<16)
#define L2_REFERENCES (PERF_COUNT_HW_CACHE_LL)|(PERF_COUNT_HW_CACHE_OP_READ<<8)|(PERF_COUNT_HW_CACHE_RESULT_ACCESS<<16)
#define L2_REFILLS (PERF_COUNT_HW_CACHE_LL)|(PERF_COUNT_HW_CACHE_OP_READ<<8)|(PERF_COUNT_HW_CACHE_RESULT_MISS<<16)
#define INST_RETIRED PERF_COUNT_HW_INSTRUCTIONS
#define CPU_CYCLES PERF_COUNT_HW_REF_CPU_CYCLES
#else
#define L1_REFERENCES 0x0
#define L1_REFILLS 0x0
#define L2_REFERENCES 0x0
#define L2_REFILLS 0x0
#define INST_RETIRED 0x0
#endif

/// Indicates which thread/process performance counters to follow.
#define this_thread 0

/// Enables monitoring of the task performance events on any cores
#define any_core -1

/** @brief Struct holding raw measurement and ID of a performance counter.
 *
 */
struct event {
	long unsigned value; /* The value of the event */
	long unsigned id; /* if PERF_FORMAT_ID */
};

/** @brief Struct returned by the kernel upon reading the file descriptor of the performance counters.
 *  @details The struct holds values for l1-D refills and misses and l2 refills and misses.
 */
struct read_format {
	long unsigned nr; /* The number of events */
	long unsigned time_enabled; /* if PERF_FORMAT_TOTAL_TIME_ENABLED */
	long unsigned time_running; /* if PERF_FORMAT_TOTAL_TIME_RUNNING */
	struct event l1_references;
	struct event l1_refills;
	struct event l2_references;
	struct event l2_refills;
	struct event inst_retired;
	struct event clock_count;
};

/// File descriptor for L1-D references (also, group-fd head)
static int l1_references_fd;

/// File descriptor for L1-D missess
static int l1_refills_fd;

/// File descriptor for L2 references
static int l2_references_fd;

/// File descriptor for L2 misses
static int l2_refills_fd;

/// File descriptor for instruction retired
static int inst_retired_fd;

/// File descriptor for clock cycles count
static int clock_count_fd;

#if defined(CORTEX_A53) || defined(CORE_I7)
/**
 * @brief Open a file descriptor for the performance counter specified.
 * @param[in] pmc_type Specify the event type.
 * @param[in] pmc_config The platform specific ID of the performance counter.
 * @param[in] group_fd The file descriptor group to which the performance counter belongs.
 * @return The file directory opened, -1 on failures.
 */
static int open_pmc_fd(unsigned int pmc_type, unsigned int pmc_config, int group_fd)
{
	static struct perf_event_attr attr;
	attr.type = pmc_type;
	attr.config = pmc_config;
	attr.size = sizeof(struct perf_event_attr);
	attr.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID |
			   PERF_FORMAT_TOTAL_TIME_ENABLED |
			   PERF_FORMAT_TOTAL_TIME_RUNNING;

	int fd = syscall(__NR_perf_event_open, &attr, this_thread, any_core,
			 group_fd, 0);

	if (fd == -1) {
		perror("Could not open fd for performance counter\n");
	}

	return fd;
}
#endif

/** @brief Enable user-space access to performance counters.
 * @return Group_fd head's pid on sucess, -1 on error.
 */
int setup_pmcs(void)
{
	elogf(LOG_LEVEL_TRACE, "Opening performance counters fd\n");
#ifdef CORTEX_A53
	l1_references_fd = open_pmc_fd(PERF_TYPE_RAW, L1_REFERENCES, -1);
	if (l1_references_fd == -1)
		return -1;
	l1_refills_fd = open_pmc_fd(PERF_TYPE_RAW, L1_REFILLS, l1_references_fd);
	if (l1_refills_fd == -1)
		return -1;
	l2_references_fd = open_pmc_fd(PERF_TYPE_RAW, L2_REFERENCES, l1_references_fd);
	if (l2_references_fd == -1)
		return -1;
	l2_refills_fd = open_pmc_fd(PERF_TYPE_RAW, L2_REFILLS, l1_references_fd);
	if (l2_refills_fd == -1)
		return -1;
	inst_retired_fd = open_pmc_fd(PERF_TYPE_RAW, INST_RETIRED, l1_references_fd);
	if (inst_retired_fd == -1)
		return -1;
	clock_count_fd = open_pmc_fd(PERF_TYPE_RAW, CPU_CYCLES, l1_references_fd);
        if (clock_count_fd == -1)
                return -1;
#elif CORE_I7
	l1_references_fd = open_pmc_fd(PERF_TYPE_HW_CACHE, L1_REFERENCES, -1);
        if (l1_references_fd == -1)
                return -1;
        l1_refills_fd = open_pmc_fd(PERF_TYPE_HW_CACHE, L1_REFILLS, l1_references_fd);
        if (l1_refills_fd == -1)
                return -1;
        l2_references_fd = open_pmc_fd(PERF_TYPE_HW_CACHE, L2_REFERENCES, l1_references_fd);
        if (l2_references_fd == -1)
                return -1;
        l2_refills_fd = open_pmc_fd(PERF_TYPE_HW_CACHE, L2_REFILLS, l1_references_fd);
        if (l2_refills_fd == -1)
                return -1;
        inst_retired_fd = open_pmc_fd(PERF_TYPE_HARDWARE, INST_RETIRED, l1_references_fd);
        if (inst_retired_fd == -1)
                return -1;
	clock_count_fd = open_pmc_fd(PERF_TYPE_HARDWARE, CPU_CYCLES, l1_references_fd);
        if (clock_count_fd == -1)
                return -1;
#endif
	return l1_references_fd;
}

/**
 * @brief Close the file descriptor related to the performance counters.
 * @param[in] fd The file descriptor to close.
 * @return Returns file descriptor status upon closing, return -1 on failures.
 */
static inline int close_pmc_fd(int fd)
{
	int ret = close(fd);
	if (ret == -1) {
		perror("Could not close fd for performance counter\n");
	}
	return ret;
}

/** @brief Close access to performance counters.
 * @return 0 on sucess, -1 on error.
 */
int teardown_pmcs(void)
{
	elogf(LOG_LEVEL_TRACE, "Closing performance counters fd\n");
	int ret = 0;
	ret = close_pmc_fd(l1_references_fd);
	if (ret == -1)
		return ret;
	ret = close_pmc_fd(l1_refills_fd);
	if (ret == -1)
		return ret;
	ret = close_pmc_fd(l2_references_fd);
	if (ret == -1)
		return ret;
	ret = close_pmc_fd(l2_refills_fd);
	if (ret == -1)
		return ret;
	ret = close_pmc_fd(inst_retired_fd);
	if (ret == -1)
		return ret;
	ret = close_pmc_fd(clock_count_fd);
        if (ret == -1)
                return ret;
	return 0;
}

/** @brief Read performance counters value.
 * @return struct perf_countrers.
 */
struct perf_counters pmcs_get_value(void)
{
	struct read_format measurement;
	size_t size = read(l1_references_fd, &measurement,
			   sizeof(struct read_format));
	if (size != sizeof(struct read_format)) {
		perror("Error: Size read from performance counters differ from size expected.");
	}
	struct perf_counters res;
	res.l1_references = measurement.l1_references.value;
	res.l1_refills = measurement.l1_refills.value;
	res.l2_references = measurement.l2_references.value;
	res.l2_refills = measurement.l2_refills.value;
	res.inst_retired = measurement.inst_retired.value;
	res.clock_count = measurement.clock_count.value;
	return res;
}
