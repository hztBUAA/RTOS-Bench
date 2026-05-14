/**
 * @file periodic_benchmark.h
 * @ingroup generator
 * @brief A general periodic benchmark using a real time timer.
 * @author Mattia Nicolella
 *
 * @copyright (C) 2021 - 2022, Mattia Nicolella <mnico@bu.edu> and the rt-bench contributors.
 * SPDX-License-Identifier: MIT
*/
#ifndef PERIODIC_BENCHMARK_H
#define PERIODIC_BENCHMARK_H
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <inttypes.h>
#include <stdlib.h>

#if defined(RT_THREAD_PLATFORM) || defined(RUIHUA_PLATFORM) || defined(DONGTU_PLATFORM) || defined(ONEOS_PLATFORM)
#include <stdint.h>
typedef uint32_t cpu_set_t;
#define CPU_ZERO(set) (*(set) = 0)
#define CPU_SET(cpu, set) (*(set) |= (1u << (cpu)))
#define CPU_ISSET(cpu, set) ((*(set) & (1u << (cpu))) != 0)
#define CPU_COUNT(set) __builtin_popcount(*(set))
#else
#include <sched.h>
#ifdef SYLIXOS_PLATFORM
/* SylixOS sched.h 没有 CPU_COUNT 宏，需要补充定义 */
#ifndef CPU_COUNT
static inline int __sylixos_cpu_count(const cpu_set_t *set) {
    int count = 0;
    for (int i = 0; i < CPU_SETSIZE; i++) {
        if (CPU_ISSET(i, set)) count++;
    }
    return count;
}
#define CPU_COUNT(set) __sylixos_cpu_count(set)
#endif
#endif
#endif

/** @brief Struct used to hold the parsed arguments and options.
 * @details
 * Will determine the how periodic_benchmark() behaves by influencing the passed
 * parameters to the benchmark and the timers expiration.
 */
struct execution_options {
	int args_num; ///< The length of the args array.
	char **args; ///< The given arguments.
	double parsed_deadline; ///< The deadline specification as parsed.
	double parsed_period; ///< The period specification as parsed.
	long deadline_sec; ///< The deadline in seconds.
	long deadline_nsec; ///< The deadline in nanoseconds.
	long period_sec; ///< The period in seconds.
	long period_nsec; ///<  The period in nanoseconds.
	char *output_path; ///< Path where the execution info will be written.
	size_t bytes_to_preallocate; ///< The heap memory that will be preallocated and will act as a limit for dynamic memory requested during the benchmark execution.
	cpu_set_t core_affinity; ///< The core mask which will be used during the benchmark to set the core affinity. This mask can represent at most 1024, if more are needed the mask should allocated dynamically via `CPU_ALLOC`.
	unsigned long long tasks_to_launch; ///< Number of tasks to launch before exiting, if 0 the program will run until `SIGINT` is received.
	/** Sched FIFO period */
	uint32_t prio; ///< Priority of the tasks when using SCHED-FIFO
	/** MCMG criticality of the task */
	uint32_t criticality; ///< Criticality associated with the tasks
	/** Sched Deadline parameters */
	uint64_t runtime; ///< Allocated CPU runtime in nano-seconds when using SCHED-DEADLINE
	uint64_t period; ///< The SCHED_DEADLINE period in nano-seconds
	uint64_t deadline; ///< The SCHED-DEADLINE deadline in nano-seconds
	/** Runtime memory profiling parameters */
	unsigned memory_profiling_enable;
	cpu_set_t memory_profiling_core_affinity;
	long unsigned memory_profiling_time_bucket;
	/* Workload selection helpers (multi-run orchestrated in entry points) */
	const char *workload_name;   ///< single workload name (optional)
	const char *category_filter; ///< comma-separated categories (optional)
	unsigned run_all_workloads;  ///< run every registered workload
	unsigned list_only;          ///< only list workloads and exit
	long duration_sec;           ///< Maximum wall-clock duration in seconds (0 = unlimited)
	unsigned run_workload_suite; ///< run all workloads in simple sequential mode and exit
};

/**
 * @brief Handles the timer creation, setup and the periodic execution of a generic benchmark.
 * @param[in] exec_opts Benchmark execution options.
 * @return 0 in case of success and an error code otherwise.
 */
int periodic_benchmark(struct execution_options *exec_opts);


/** @cond SKIP
 * Documentation of the following prototypes is delegated to the benchmark that implements them.
 */
#ifdef EXTENDED_REPORT
extern const char* benchmark_log_header();

extern float benchmark_log_data();
#endif

//The benchmark init function, which will be defined by the benchmark itself
extern int benchmark_init(int parameters_num, void **parameters);

//The benchmark execution function, which will be defined by the benchmark itself
extern void benchmark_execution(int parameters_num, void **parameters);

//The benchmark teardown function, which will be defined by the benchmark itself
extern void benchmark_teardown(int parameters_num, void **parameters);

/// @endcond

#endif
