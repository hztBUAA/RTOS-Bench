/**
 * @file test_realtime.h
 * @brief Realtime performance test for RTOS-Bench
 * @details Wrapper for the original realtime benchmark implementation
 *          located in generator/realtime_orig/
 *
 * Usage: rtbench test-realtime [--multicore]
 *
 * The test measures:
 * - Context switch latency
 * - Interrupt latency (requires kernel instrumentation)
 * - System call latency
 * - IPC primitives (semaphore, mutex, message queue, memory pool)
 * - Multicore performance (optional): memory bandwidth, IPC, task latency
 *
 * Reference: Industrial RTOS Benchmark Standard v1.5
 */

#ifndef TEST_REALTIME_H
#define TEST_REALTIME_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Run realtime performance test
 * @param run_multicore 1 to include multicore tests, 0 for single-core only
 * @return 0 on success, negative on error
 *
 * This function wraps the original realtime benchmark implementation.
 * Results are printed directly to console.
 */
int test_realtime_run(int run_multicore);

/**
 * @brief Section selector for test_realtime_run_ex()
 *
 * Section split used by the dedicated test-realtime commands:
 *   --delay          RTBENCH_REALTIME_SECTION_DELAY         test1 + test2
 *   --cost           RTBENCH_REALTIME_SECTION_COST          test4 - test10
 *                                                           (8 operations x 4 scenarios,
 *                                                            test3 is not included)
 *   --multi-access   RTBENCH_REALTIME_SECTION_MULTI_ACCESS  multicore memory access
 *                                                           bandwidth (8 modes x 1/2/4/8)
 *   --multi-service  RTBENCH_REALTIME_SECTION_MULTI_SERVICE multicore ipc bandwidth,
 *                                                           task create/delete latency,
 *                                                           intra/inter core communication
 */
#define RTBENCH_REALTIME_SECTION_DELAY		0x01u
#define RTBENCH_REALTIME_SECTION_COST		0x02u
#define RTBENCH_REALTIME_SECTION_MULTI_ACCESS	0x04u
#define RTBENCH_REALTIME_SECTION_MULTI_SERVICE	0x08u
#define RTBENCH_REALTIME_SECTION_ALL		0x0fu

/**
 * @brief Run the selected sections of the realtime performance test
 * @param sections bitwise OR of RTBENCH_REALTIME_SECTION_* values
 * @return 0 on success, negative on error
 *
 * Only the requested sections are executed and printed. The legacy
 * test_realtime_run(run_multicore) entry keeps its original behaviour.
 */
int test_realtime_run_ex(unsigned int sections);

/**
 * @brief Run realtime verify program
 */
void test_realtime_verify(void);

#ifdef __cplusplus
}
#endif

#endif /* TEST_REALTIME_H */
