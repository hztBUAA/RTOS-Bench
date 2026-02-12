/**
 * @file test_realtime.h
 * @brief Realtime performance test suite for RTOS-Bench
 * @details Implements various realtime latency and throughput measurements
 *          including context switch, semaphore, mutex, message queue, and memory tests.
 *
 * Usage: rtbench test-realtime [OPTIONS]
 *
 * The test performs:
 * 1. Single-core realtime latency tests:
 *    - Context switch latency
 *    - Semaphore operations (take/give with various scenarios)
 *    - Message queue operations (send/receive)
 *    - Mutex operations (lock/unlock)
 *    - Memory allocation (malloc/free)
 * 2. (Optional) Multicore performance tests:
 *    - Memory bandwidth
 *    - IPC bandwidth
 *    - Task create/delete latency
 *
 * Reference: Industrial RTOS Benchmark Standard v1.5
 */

#ifndef TEST_REALTIME_H
#define TEST_REALTIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Configuration
 * ============================================================================ */

/* Priority levels for benchmark threads */
#ifndef REALTIME_HIGH_PRIO
#define REALTIME_HIGH_PRIO      14
#endif
#ifndef REALTIME_MIDDLE_PRIO
#define REALTIME_MIDDLE_PRIO    15
#endif
#ifndef REALTIME_LOW_PRIO
#define REALTIME_LOW_PRIO       16
#endif

/* Number of processors for multicore tests */
#ifndef REALTIME_USE_PROCESSORS
#define REALTIME_USE_PROCESSORS 2
#endif

/* Test iteration count */
#define REALTIME_TEST_ITERATIONS 1000

/* ============================================================================
 * Timer Interface (platform-specific)
 * ============================================================================ */

/**
 * @brief Start the high-resolution timer (if needed by platform)
 */
void realtime_timer_start(void);

/**
 * @brief Get timer frequency in Hz
 * @return Timer frequency
 */
uint64_t realtime_freq_get(void);

/**
 * @brief Get current timer value (cycles/ticks)
 * @return Current timer value
 */
uint64_t realtime_time_get(void);

/**
 * @brief Convert cycles to nanoseconds
 * @param cycles Number of timer cycles
 * @return Time in nanoseconds
 */
uint64_t realtime_cycles_to_ns(uint64_t cycles);

/* ============================================================================
 * Result Structures
 * ============================================================================ */

/**
 * @brief Single-core realtime test results
 *
 * All latency values are in nanoseconds.
 * Array indices for service costs:
 *   [0] = immediate execution (no blocking)
 *   [1] = suspend/sleep (blocking wait)
 *   [2] = low-priority ready (release without context switch)
 *   [3] = high-priority wakeup (release with context switch)
 */
struct realtime_singlecore_result {
	/* Context switch average latency (ns) */
	uint64_t context_switch_avg;

	/* Interrupt latency - requires kernel instrumentation */
	uint64_t interrupt_min;
	uint64_t interrupt_max;
	uint64_t interrupt_avg;

	/* Syscall latency - requires kernel instrumentation */
	uint64_t syscall_min;
	uint64_t syscall_max;
	uint64_t syscall_avg;

	/* System service costs [operation][scenario] */
	/* Rows: sem_take, sem_give, mq_send, mq_recv, mutex_lock, mutex_unlock, malloc, free */
	/* Cols: immediate, suspend, low-ready, high-wakeup */
	uint64_t service_cost[8][4];

	/* Message queue behavior: 0 = blocks when full, non-zero = returns error */
	int mq_blocks_when_full;
};

/**
 * @brief Multicore test results
 *
 * Memory bandwidth values are in MB/s.
 * IPC bandwidth values are in GB/s.
 * Task latency values are in microseconds.
 */
struct realtime_multicore_result {
	/* Memory bandwidth [mode][concurrency] */
	/* mode: 0=rd, 1=wr, 2=cp, 3=frd, 4=fwr, 5=fcp, 6=memset, 7=memcpy */
	/* concurrency index: 0=1, 1=2, 2=4, 3=8 workers */
	uint64_t mem_bandwidth[8][4];

	/* IPC bandwidth [concurrency] */
	uint64_t ipc_bandwidth[4];

	/* Same-core vs different-core IPC comparison */
	uint64_t ipc_same_core;
	uint64_t ipc_diff_core;

	/* Task create/delete latency [concurrency] in microseconds */
	uint64_t task_latency[4];
};

/**
 * @brief Complete test result
 */
struct test_realtime_result {
	struct realtime_singlecore_result singlecore;
	struct realtime_multicore_result multicore;
	int multicore_tested;  /* 1 if multicore tests were run */
};

/* ============================================================================
 * Test Entry Points
 * ============================================================================ */

/**
 * @brief Run all single-core realtime tests
 * @param[out] result Pointer to result structure
 * @return 0 on success, negative on error
 */
int test_realtime_singlecore(struct realtime_singlecore_result *result);

/**
 * @brief Run all multicore tests
 * @param[out] result Pointer to result structure
 * @return 0 on success, negative on error
 */
int test_realtime_multicore(struct realtime_multicore_result *result);

/**
 * @brief Run complete test-realtime benchmark
 * @param run_multicore Whether to run multicore tests (requires SMP)
 * @return 0 on success, negative on error
 */
int test_realtime_run(int run_multicore);

/**
 * @brief Print single-core test results
 * @param result Pointer to result structure
 */
void test_realtime_print_singlecore(const struct realtime_singlecore_result *result);

/**
 * @brief Print multicore test results
 * @param result Pointer to result structure
 */
void test_realtime_print_multicore(const struct realtime_multicore_result *result);

/**
 * @brief Get the last test result
 * @return Pointer to the result structure (valid until next test run)
 */
const struct test_realtime_result *test_realtime_get_result(void);

#ifdef __cplusplus
}
#endif

#endif /* TEST_REALTIME_H */
