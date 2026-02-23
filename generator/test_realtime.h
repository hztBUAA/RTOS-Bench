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

#ifdef __cplusplus
}
#endif

#endif /* TEST_REALTIME_H */
