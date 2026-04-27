/**
 * @file sched_workloads.h
 * @brief Wrapper layer for test-schedule specific workload execution
 * @details Provides quick-execution wrappers for workloads that need modified
 *          parameters for schedulability testing without modifying original code.
 *
 * Purpose:
 * - Keep original workload code unchanged (full functionality for standalone use)
 * - Provide test-schedule specific "quick" versions with reduced iterations/delays
 * - Maintain code isolation: all test-schedule specific logic stays in this directory
 */

#ifndef SCHED_WORKLOADS_H
#define SCHED_WORKLOADS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Wrapper structure for test-schedule specific workload execution
 */
struct sched_workload_wrapper {
	const char *name;            /**< Must match original workload name exactly */
	int (*init)(void);           /**< Optional init for wrapper (NULL = use original) */
	int (*quick_exec)(void);     /**< Quick execution function for test-schedule */
	void (*teardown)(void);      /**< Optional teardown for wrapper (NULL = use original) */
	int max_wcet_ms;             /**< Expected max WCET in ms (for skip threshold) */
	int needs_state_reset;       /**< Whether to reset state between iterations */
};

/**
 * @brief Get the test-schedule wrapper for a workload
 * @param name Workload name to look up
 * @return Pointer to wrapper if found, NULL otherwise
 *
 * When a wrapper exists, test-schedule should use wrapper->quick_exec()
 * instead of the original workload's exec() function.
 */
const struct sched_workload_wrapper *sched_get_wrapper(const char *name);

/**
 * @brief Get total number of registered wrappers
 * @return Number of wrappers
 */
int sched_wrapper_count(void);

/**
 * @brief Get wrapper by index
 * @param idx Index (0 to sched_wrapper_count()-1)
 * @return Pointer to wrapper or NULL if out of range
 */
const struct sched_workload_wrapper *sched_get_wrapper_by_index(int idx);

/* ============================================================================
 * Wrapper declarations (implemented in separate files)
 * ============================================================================ */

/* MQTT quick wrapper - limits message count for fast execution */
extern int sched_mqtt_init(void);
extern int sched_mqtt_quick_exec(void);
extern void sched_mqtt_teardown(void);

/* MODBUS quick wrapper - reduces sleep/timeout values */
extern int sched_modbus_init(void);
extern int sched_modbus_quick_exec(void);
extern void sched_modbus_teardown(void);

#ifdef __cplusplus
}
#endif

#endif /* SCHED_WORKLOADS_H */
