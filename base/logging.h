/** @file logging.h
 * @ingroup base
 * @brief Logging utilities.
 * @details Logging interfaces, to support different verbosity levels.
 * These interfaces are implemented as function-like macros, since they will mainly rely on the fprintf() function.
 *
 * In the function-like macros `##__VA_ARGS__` is used to make the macro variadic, however it could be incompatible, with some compilers.
 * A simple work around is to delete the `##` before `__VA_ARGS__` and pass a blank string (`""`) when the macro is used without parameters
 * other than the format string and the log level.\n
 * Without the `##` it could happen that the compiler does not strip the `,` after the format string when is not necessary and the macro is not expanded properly.
 */

#ifndef LOGGING_H
#define LOGGING_H

#include<stdio.h>

/// Deadline missed status.
#define DEADLINE_MISSED 0
/// Deadline met status.
#define DEADLINE_MET 1

/** @brief An enum that identifies the available log levels.
 * @details The log level will determine if a particular message will be printed or not.
 */
enum log_level{
	LOG_LEVEL_ERR=1, ///< Only print error messages.
	LOG_LEVEL_FILE, ///< Print benchmark stats to file.
	LOG_LEVEL_INFO, ///< Print benchmark stats to stdout.
	LOG_LEVEL_TRACE, ///< Print informative and debug messages.
};

/// The benchmark verbosity.
extern enum log_level benchmark_verbosity;

/** @brief General logging utility.
 * @param[in] mesg_log_level The log level of the message.
 * @param[in] file The file where the message must be printed.
 * @param[in] format The message format string.
 * @param[in] ... Parameters referenced by the format string.
 * @details This simple function will leverage `fprintf()` to print the given message only if the `::benchmark_verbosity` is >= of the message log level.
 */
#define flogf(mesg_log_level,file,format,...) \
	if(mesg_log_level<=benchmark_verbosity){ \
		fprintf(file,format,##__VA_ARGS__); \
	}

/** @brief Logging interface for `stdout`.
 * @param[in] mesg_log_level The message log level.
 * @param[in] format The message format string.
 * @param[in] ... The format string parameters.
 * @details A `flogf()` wrapper to easily print messages on `stdout`.
 */
#define logf(mesg_log_level,format,...) \
	flogf(mesg_log_level,stdout,format,##__VA_ARGS__)

/** @brief Logging interface for `stderr`.
 * @param[in] mesg_log_level The message log level.
 * @param[in] format The message format string.
 * @param[in] ... The format string parameters.
 * @details A `flogf()` wrapper to easily print messages on `stderr`.
 */
#define elogf(mesg_log_level,format,...) \
	flogf(mesg_log_level,stderr,format,##__VA_ARGS__)

/** @brief A function-like macro that reports the benchmark timing depending on the chosen logging level.
 * @param[in] file The file where the timing will be printed if the logging level is set to `::LOG_LEVEL_FILE`.
 * @param[in] period_start The timestamp when the period started.
 * @param[in] period_end The timestamp when the period completed.
 * @param[in] job_end The timestamp  when the job ended.
 * @param[in] deadline The timestamp of the first deadline since the job started, or the timestamp of the skipped deadline.
 * @details
 * Depending on the chosen log level (`::benchmark_verbosity` value), the benchmark timing can be either be printed in a human-friendly or in a csv-like format.
 * The job is assumed to start when the period starts.\n
 * The verbose output is associated to `::LOG_LEVEL_TRACE`, while the csv-like format is associated to both `::LOG_LEVEL_INFO` and `::LOG_LEVEL_FILE`.
 * When `::LOG_LEVEL_ERR` is set no message will be printed.
 *
 * The csv-like format is composed by the following elements, which will be printed in the order they are described, separated by commas:
 * 1. period start timestamp (in clock cycles);
 * 2. period end timestamp (in clock cycles);
 * 3. job end timestamp (in clock cycles);
 * 4. timestamp of the first deadline since job start, called "job deadline";
 * 5. job deadline status:
 *   - `::DEADLINE_MET` if the job deadline was met.
 *   - `::DEADLINE_MISSED` if the job deadline was missed.
 * 6. job elapsed time (in clock cycles);
 * 7. job utilization (elapsed / (period end - period start));
 * 8. job density (elapsed / (job deadline - period start));
 *
 * Example for a completed job with missed deadline: `9784789712001,9787383674679,9784790003557,9784789812714,0,291556,0.000112,2.89`.
 *
 * Since jobs that exceed the deadline are not killed, there can be jobs that take multiple deadlines and periods to terminate.\n
 * The skipped deadlines and periods can be reported by setting all the elements described to `0`, excluding only the deadline timestamp or the period start timestamp.\n
 * This report is made by printing a line where all the elements but the deadline or the period start are set to 0.\n
 * Example for a deadline skip: `0,0,0,10039859937104,0,0,0,0`.
 */
#define print_benchmark_timing(file,period_start,period_end,job_end,deadline) \
	do { \
		int deadline_status= (job_end > 0 && job_end <=deadline) ? DEADLINE_MET : DEADLINE_MISSED; \
		unsigned long long elapsed=(job_end > period_start) ? job_end - period_start : 0; \
		double utilization=(period_end > period_start) ? (elapsed + 0.0) / (period_end - period_start) : 0; \
		double density= (deadline > period_start) ?  (elapsed + 0.0) / (deadline - period_start) : 0; \
		switch(benchmark_verbosity){ \
			case LOG_LEVEL_TRACE: \
				if(job_end!=0){ \
					printf("\nJob completed\n"); \
					printf("period start: %llu\t-\t period end: %llu\n",period_start,period_end); \
					printf("job end: %llu\n",job_end); \
					printf("job deadline: %llu\t-\tdeadline status:%d (%d=met)\n",deadline,deadline_status,DEADLINE_MET); \
					printf("job duration: %llu\t-\tutilization:%.3g\t-\tdensity:%.3g\n\n",elapsed,utilization,density); \
				} else { \
					if(deadline != 0){ \
						printf("\n\t Deadline %llu skipped\n\n",deadline); \
					} \
					if(period_start != 0){ \
						printf("\n\t Period %llu skipped\n\n",period_start); \
					} \
				} \
				break; \
			case LOG_LEVEL_FILE: \
				fprintf(file,"%llu,%llu,%llu,%llu,%d,%llu,%.3g,%.3g\n",period_start,period_end,job_end,deadline,deadline_status,elapsed,utilization,density); \
				break; \
			case LOG_LEVEL_INFO: \
				printf("%llu,%llu,%llu,%llu,%d,%llu,%.3g,%.3g\n",period_start,period_end,job_end,deadline,deadline_status,elapsed,utilization,density); \
				break; \
		} \
	} while(0);
#endif
