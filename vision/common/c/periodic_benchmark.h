/**
 * @file periodic_benchmark.h
 * @brief A general periodic benchmark using a real time timer.
 * @details Timer expiration triggers a `SIGRTMIN` and `SIGINT` is used to stop and destroy the timer.
*/
#ifndef PERIODIC_BENCHMARK_H
#define PERIODIC_BENCHMARK_H

/** @brief Struct used to hold the parsed arguments and options.
 * @details
 * Will determine the how periodic_benchmark() behaves by influencing the passed
 * parameters and the timer deadline.
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

//The benchmark init function, which will be defined by the benchmark itself
extern int benchmark_init(int parameters_num, void **parameters);

//The benchmark execution function, which will be defined by the benchmark itself
extern void benchmark_execution(int parameters_num, void **parameters);

//The benchmark teardown function, which will be defined by the benchmark itself
extern void benchmark_teardown(int parameters_num, void **parameters);

/// @endcond

#endif
