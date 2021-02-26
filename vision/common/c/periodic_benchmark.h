/**
 * A general implementation of a periodic benchmark using a real time timer, where expiration triggers a SIGRTMIN and SIGINT is used to stop and destroy the timer.
*/
#ifndef PERIODIC_BENCHMARK_H
#define PERIODIC_BENCHMARK_H

///Struct used to hold the parsed arguments and options
struct execution_options {
	int args_num; //< The length of the args array.
	char **args; //< The given arguments.
	long deadline_sec; //< The deadline in seconds.
	long deadline_nsec; //< The deadline in nanoseconds.
	char *output_path; //< Path where the execution info will be written.
};

/** The function that will handle the timer creation and setup.
 * @param[in] parameters_num The lenght of the parameters array.
 * @param[in] The array containing the parameters given to the benchmark.
 * @param[in] deadline_sec The timer deadline in seconds.
 * @param[in] deadline_nsec The timer deadline in nanoseconds.
 * @return 0 in case of success and an error code otherwise.
 */
int periodic_benchmark(struct execution_options *exec_opts);

#endif
