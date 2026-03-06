/**
 * @file test_stress.h
 * @brief Stress/Power consumption test for RTOS-Bench
 * @details Integrates rtos_stress (stress-ng port) for CPU/memory/file stress testing
 *          using built-in jobfiles with 5-stage graduated load levels.
 *
 * Usage: rtbench test-stress [--job cpu|memory|file|all] [-l] [-q]
 */

#ifndef TEST_STRESS_H
#define TEST_STRESS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum number of results from a single job run (all = 135) */
#define TEST_STRESS_MAX_RESULTS 160

/**
 * @brief Single stressor result from a job run
 */
struct test_stress_job_result {
    char name[32];          /**< Stressor name (e.g., "cpu", "matrix") */
    char type[16];          /**< Job type: "cpu"/"memory"/"file" */
    int stage;              /**< Stage level 1-5 */
    uint64_t bogo_ops;      /**< Bogo operations completed */
    double duration_sec;    /**< Actual duration in seconds */
    double metric_value;    /**< Optional metric (throughput, etc.) */
    char metric_unit[32];   /**< Optional metric unit */
    int success;            /**< 1 = success, 0 = failure */
};

/**
 * @brief Run a stress job (built-in jobfile)
 * @param job_name Job name: "cpu", "memory", "file", or "all"
 * @return 0 on success, negative on error
 */
int test_stress_run_job(const char *job_name);

/**
 * @brief Get results from the last job run
 * @param count_out Pointer to receive the number of results
 * @return Pointer to static results array (valid until next run)
 */
const struct test_stress_job_result *test_stress_get_job_results(int *count_out);

/**
 * @brief List available jobs
 */
void test_stress_list_jobs(void);

/**
 * @brief Stop running stress test
 */
void test_stress_stop(void);

/* Legacy API (preserved for backward compatibility) */

/**
 * @brief Available stress test types (legacy)
 */
typedef enum {
    STRESS_TYPE_CPU,
    STRESS_TYPE_MATRIX,
    STRESS_TYPE_VM,
    STRESS_TYPE_MALLOC,
    STRESS_TYPE_MEMCPY,
    STRESS_TYPE_PRIME,
    STRESS_TYPE_TRIG,
    STRESS_TYPE_FP,
    STRESS_TYPE_ALL,
} stress_type_t;

struct test_stress_config {
    stress_type_t type;
    int duration_sec;
    int num_workers;
    int quiet;
};

int test_stress_run_stressor(const char *name, int duration_sec);
int test_stress_run_config(const struct test_stress_config *config);
uint64_t test_stress_get_last_bogo_ops(void);
void test_stress_list_stressors(void);

#ifdef __cplusplus
}
#endif

#endif /* TEST_STRESS_H */
