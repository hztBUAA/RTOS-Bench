/** @file performance_counters.c
 * @ingroup base
 * @brief Implementation of a architecture independent and highly abstrcat way access to performance counters.
 * @details Uses interface provided by the Linux kernel to access teh performance counters value.
 * @author Denis Hoornaert
 */

#include <linux/perf_event.h>
#include "performance_counters.h"

/// System-call number to open performance counter event.
#if ARCH == AARCH64
#define __NR_perf_event_open 241

/// Core model specific performance counter event IDs
#if CORE == CORTEX_A53
#define        L1_REFERENCES 0x04
#define           L1_REFILLS 0x03
#define        L2_REFERENCES 0x16
#define           L2_REFILLS 0x17

#endif
#endif

/// Indicates which thread/process performance counters to follow.
#define          this_thread 0

/** @brief Struct holding raw measurement and ID of a performance counter.
 *
 */
struct event {
        long unsigned value;         /* The value of the event */
        long unsigned id;            /* if PERF_FORMAT_ID */
};

/** @brief Struct returned by the kernel upon reading the file descriptor of the performance counters.
 *  @detail The struct holds values for l1-D refills and misses and l2 refills and misses.
 */
struct read_format {
        long unsigned nr;            /* The number of events */
        long unsigned time_enabled;  /* if PERF_FORMAT_TOTAL_TIME_ENABLED */
        long unsigned time_running;  /* if PERF_FORMAT_TOTAL_TIME_RUNNING */
        struct event l1_references;
        struct event l1_refills;
        struct event l2_references;
        struct event l2_refills;
};

/// File descriptor for L1-D references (also, group-fd head)
static int l1_references_fd;

/// File descriptor for L1-D missess
static int l1_refills_fd;

/// File descriptor for L2 references
static int l2_references_fd;

// File descriptor for L2 misses
static int l2_refills_fd;

/**
 * @brief Open a file descriptor for the performance counter specified.
 * @param[in] pmc_type The platform specific ID of the performance counter.
 * @param[in] group_fd The file descriptor group to which the performance counter belongs.
 * @param[in] this_cpu The CPU to which the core is attached.
 * @return The file directory opened, -1 on failures.
 */
static int open_pmc_fd(unsigned int pmc_type, int group_fd, int this_cpu) {
	static struct perf_event_attr attr;
	attr.type = PERF_TYPE_RAW;
	attr.config = pmc_type;
	attr.size = sizeof(struct perf_event_attr);
	attr.read_format = PERF_FORMAT_GROUP|PERF_FORMAT_ID|PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_TOTAL_TIME_RUNNING;

	int fd = syscall(__NR_perf_event_open, &attr, this_thread, this_cpu, group_fd, 0);

	if (fd == -1)
		perror("Could not open fd for performance counter %x\n", pmc_type);

	return fd;
}

/** @brief Enable user-space access to performance counters.
 * @return Group_fd head's pid on sucess, -1 on error.
 */
int setup_pmcs(void) {
	elogf(LOG_LEVEL_TRACE, "Openning performance counters fd\n");
	l1_references_fd = open_pmc_fd(L1_REFERENCES, -1);
        if (l1_references_fd == -1)
                return -1;
	l1_refills_fd = open_pmc_fd(L1_REFILLS, l1_references_fd);
        if (l1_refills_fd == -1)
                return -1;
	l2_references_fd = open_pmc_fd(L2_REFERENCES, l1_references_fd);
        if (l2_references_fd == -1)
                return -1;
	l2_refills_fd = open_pmc_fd(L2_REFILL, l1_references_fd);
        if (l2_refills_fd == -1)
                return -1;
	return l1_references_fd;
}

/**
 * @brief Close the file descriptor related to the performance counters.
 * @param[in] fd The file descriptor to close.
 * @param[in] pmc_type The platform specific ID of the performance counter to close.
 * @return Returns file descriptor status upon closing, return -1 on failures.
 */
static inline int close_pmc_fd(int fd, unsigned int pmc_type) {
	int ret = close(fd);
	if (ret == -1)
		perror("Could not close fd for performance counter %x\n", pmc_type);
	return ret;
}

/** @brief Close access to performance counters.
 * @return 0 on sucess, -1 on error.
 */
int teardown_pmcs(void) {
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
	return 0;
}

/** @brief Read performance counters value.
 * @return struct perf_countrers.
 */
inline struct perf_counters pmcs_get_value(void) {
	struct read_format measurement;
	read(l1_refills_fd, &measurement, sizeof(struct read_format));
	struct perf_counters res;
	res.l1_references = measurement.l1_accesses.value;
	res.l1_refills = measurement.l1_misses.value;
	res.l2_references = measurement.l2_accesses.value;
	res.l1_refills = measurement.l1_misses.value;
	return res;
}
