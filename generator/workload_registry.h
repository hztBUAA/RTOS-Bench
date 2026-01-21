/**
 * @file workload_registry.h
 * @brief Unified workload registry for RTOS-Bench
 * @details Provides a mechanism to register multiple workloads in a single binary.
 *          Works across all platforms (Linux, RT-Thread, SylixOS).
 *
 * Usage:
 *   1. Define your workload ops:
 *      static const struct rtosbench_workload my_workload = {
 *          .name = "my_workload",
 *          .init = my_init,
 *          .exec = my_exec,
 *          .teardown = my_teardown,
 *      };
 *
 *   2. Register using the macro:
 *      RTOSBENCH_REGISTER_WORKLOAD(my_workload);
 *
 *   3. At runtime, select workload:
 *      rtosbench_select_workload("my_workload");
 */

#ifndef RTOSBENCH_WORKLOAD_REGISTRY_H
#define RTOSBENCH_WORKLOAD_REGISTRY_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Workload operations structure
 */
struct rtosbench_workload {
	const char *name;        /**< Workload name (used for selection) */
	const char *description; /**< Human-readable description */

	/** Initialize the workload (called once before execution loop) */
	int (*init)(int parameters_num, void **parameters);

	/** Execute one job iteration */
	void (*exec)(int parameters_num, void **parameters);

	/** Cleanup the workload (called after all iterations complete) */
	void (*teardown)(int parameters_num, void **parameters);

#ifdef EXTENDED_REPORT
	/** Return CSV header for extended metrics */
	const char *(*log_header)(void);

	/** Return current metric value for CSV output */
	float (*log_data)(void);
#endif
};

/**
 * @brief Maximum number of workloads that can be registered
 */
#define RTOSBENCH_MAX_WORKLOADS 32

/**
 * @brief Register a workload with the registry
 * @param workload Pointer to workload structure (must have static lifetime)
 * @return 0 on success, -1 if registry is full
 */
int rtosbench_register_workload(const struct rtosbench_workload *workload);

/**
 * @brief Select a workload by name
 * @param name Workload name
 * @return 0 on success, -1 if not found
 */
int rtosbench_select_workload(const char *name);

/**
 * @brief Get currently selected workload name
 * @return Workload name or empty string if none selected
 */
const char *rtosbench_current_workload(void);

/**
 * @brief List all registered workloads
 * @param cb Callback function called for each workload
 */
void rtosbench_list_workloads(void (*cb)(const char *name, const char *desc));

/**
 * @brief Get count of registered workloads
 * @return Number of registered workloads
 */
int rtosbench_workload_count(void);

/* ============================================================================
 * Workload Registration Macros
 * ============================================================================ */

/**
 * @brief Register a workload at runtime (call from main or init)
 *
 * For platforms without constructor support (some RTOS), call this explicitly.
 */
#define RTOSBENCH_REGISTER_WORKLOAD_MANUAL(wl) \
	rtosbench_register_workload(&(wl))

/**
 * @brief Auto-register workload using constructor attribute
 *
 * This works on Linux, SylixOS, and most GCC-based toolchains.
 * For RT-Thread, use RTOSBENCH_REGISTER_WORKLOAD_MANUAL instead.
 */
#if defined(__GNUC__) && !defined(RT_THREAD_PLATFORM)
#define RTOSBENCH_REGISTER_WORKLOAD(wl) \
	__attribute__((constructor)) \
	static void __rtosbench_register_##wl(void) { \
		rtosbench_register_workload(&(wl)); \
	}
#else
/* For RT-Thread, registration happens explicitly in rtthread_workloads_init() */
#define RTOSBENCH_REGISTER_WORKLOAD(wl) /* no-op, use manual registration */
#endif

/* ============================================================================
 * Workload Execution Interface (called by periodic_benchmark)
 * ============================================================================ */

/**
 * @brief Initialize current workload
 */
int workload_init(int parameters_num, void **parameters);

/**
 * @brief Execute one iteration of current workload
 */
void workload_exec(int parameters_num, void **parameters);

/**
 * @brief Teardown current workload
 */
void workload_teardown(int parameters_num, void **parameters);

#ifdef EXTENDED_REPORT
/**
 * @brief Get log header from current workload
 */
const char *workload_log_header(void);

/**
 * @brief Get log data from current workload
 */
float workload_log_data(void);
#endif

/* ============================================================================
 * Legacy API Compatibility (maps to new workload API)
 * ============================================================================ */

/* These are kept for backward compatibility with existing benchmarks */
#define rtbench_ops rtosbench_workload
#define rtbench_select_benchmark rtosbench_select_workload
#define rtbench_current_benchmark rtosbench_current_workload
#define rtbench_list_benchmarks(cb) rtosbench_list_workloads((void(*)(const char*,const char*))(cb))

/* Legacy function names (will call workload_* internally) */
int benchmark_init(int parameters_num, void **parameters);
void benchmark_execution(int parameters_num, void **parameters);
void benchmark_teardown(int parameters_num, void **parameters);
#ifdef EXTENDED_REPORT
const char *benchmark_log_header(void);
float benchmark_log_data(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* RTOSBENCH_WORKLOAD_REGISTRY_H */
