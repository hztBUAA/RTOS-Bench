/**
 * @file test_stress.h
 * @brief Stress/Power consumption test for RTOS-Bench
 * @details Integrates rtos_stress (stress-ng port) for CPU/memory stress testing
 *          to support power consumption measurements.
 *
 * Usage: rtbench test-stress [OPTIONS]
 *
 * The test provides various stressors:
 * - CPU intensive workloads (cpu, matrix, prime, trig)
 * - Memory workloads (vm, malloc, memcpy)
 * - I/O workloads (hdd, open, pipe)
 */

#ifndef TEST_STRESS_H
#define TEST_STRESS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Available stress test types
 */
typedef enum {
    STRESS_TYPE_CPU,        /**< CPU compute stress */
    STRESS_TYPE_MATRIX,     /**< Matrix operations */
    STRESS_TYPE_VM,         /**< Virtual memory stress */
    STRESS_TYPE_MALLOC,     /**< Memory allocation stress */
    STRESS_TYPE_MEMCPY,     /**< Memory copy stress */
    STRESS_TYPE_PRIME,      /**< Prime number calculation */
    STRESS_TYPE_TRIG,       /**< Trigonometric functions */
    STRESS_TYPE_FP,         /**< Floating point operations */
    STRESS_TYPE_ALL,        /**< Run all stressors sequentially */
} stress_type_t;

/**
 * @brief Stress test configuration
 */
struct test_stress_config {
    stress_type_t type;     /**< Stressor type */
    int duration_sec;       /**< Duration per stressor in seconds */
    int num_workers;        /**< Number of parallel workers (0 = auto) */
    int quiet;              /**< Quiet mode (reduce output) */
};

/**
 * @brief Stress test result for one stressor
 */
struct test_stress_result {
    const char *name;       /**< Stressor name */
    double duration;        /**< Actual duration in seconds */
    uint64_t bogo_ops;      /**< Bogo operations completed */
    double bogo_ops_per_sec;/**< Operations per second */
    int success;            /**< 0 = success, non-zero = error */
};

/**
 * @brief Run stress test with default configuration
 * @return 0 on success, negative on error
 *
 * Default: CPU stress for 10 seconds
 */
int test_stress_run(void);

/**
 * @brief Run stress test with custom configuration
 * @param config Test configuration
 * @return 0 on success, negative on error
 */
int test_stress_run_config(const struct test_stress_config *config);

/**
 * @brief Run a specific stressor by name
 * @param name Stressor name (e.g., "cpu", "matrix", "vm")
 * @param duration_sec Duration in seconds
 * @return 0 on success, negative on error
 */
int test_stress_run_stressor(const char *name, int duration_sec);

/**
 * @brief List all available stressors
 */
void test_stress_list_stressors(void);

/**
 * @brief Stop running stress test
 */
void test_stress_stop(void);

/**
 * @brief Get bogo_ops from the last stress test run
 * @return Number of bogo operations completed in the last run
 */
uint64_t test_stress_get_last_bogo_ops(void);

#ifdef __cplusplus
}
#endif

#endif /* TEST_STRESS_H */
