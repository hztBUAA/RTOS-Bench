/** @file logging.h
 * @brief Logging utilities.
 * @details Logging interfaces, to support different verbosity levels.
 * These interfaces are implemented as function-like macros, since they will mainly rely on the fprintf() function.
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
 * @details This simple function will leverage `fprintf()` to print the given message only if the ::benchmark_verbosity is >= of the message log level.
 */
#define flogf(mesg_log_level,file,format,...) if(mesg_log_level<=benchmark_verbosity){ \
	fprintf(file,format,##__VA_ARGS__); \
}

/** @brief Logging interface for `stdout`.
 * @param[in] mesg_log_level The message log level.
 * @param[in] format The message format string.
 * @param[in] ... The format string parameters.
 * @details A flogf() wrapper to easily print messages on `stdout`.
 */
#define logf(mesg_log_level,format,...) flogf(mesg_log_level,stdout,format,##__VA_ARGS__)

/** @brief Logging interface for `stderr`.
 * @param[in] mesg_log_level The message log level.
 * @param[in] format The message format string.
 * @param[in] ... The format string parameters.
 * @details A flogf() wrapper to easily print messages on `stderr`.
 */
#define elogf(mesg_log_level,format,...) flogf(mesg_log_level,stderr,format,##__VA_ARGS__)

/** @brief A function-like macro that reports the benchmark timing depending on the chosen logging level.
 * @param[in] file The file where the timing will be printed if the logging level is set to `::LOG_LEVEL_FILE`.
 * @param[in] period_start The timestamp when the last period started.
 * @param[in] period_end The timestamp when the last period completed.
 * @param[in] job_end The timestamp  when the last job ended.
 * @param[in] deadline The timestamp of the first deadline since the current job started, or the timestamp of the skipped deadline.
 * @details
 * Depending on the chosen log level, the benchmark timing can be either be printed in a human-friendly or in a csv-like format.
 * The job is assumed to start when the period starts.\n
 * The verbose output is associated to `::LOG_LEVEL_TRACE`, while the csv-like format is associated to both `::LOG_LEVEL_INFO` and `::LOG_LEVEL_FILE`.
 * When `::LOG_LEVEL_ERR` is set no message will be printed.
 *
 * The csv-like format is composed by the following elements, which will be printed in the order the are described and separated by commas:
 * 1. period start timestamp;
 * 2. period end timestamp;
 * 3. job end timestamp
 * 4. timestamp of the first deadline since job start, called "job deadline";
 * 5. job deadline status:
 *   - `1` if the job deadline was met.
 *   - `0` if the job deadline was missed.
 * 6. job elapsed time (in clock cycles);
 * 7. job utilization (elapsed / (period end - period start));
 * 8. job density (elapsed / (job deadline - period start));
 *
 * Example for a completed job with missed deadline: `29191750731621,29191750836938,105317,29191750746215,0`.
 *
 * Since job that exceed the deadline are not killed, there can be jobs that take multiple deadlines and period to terminate.
 * The skipped deadlines and periods can be reported by setting all the elements described to `0`, excluding only the deadline timestamp or the period start timestamp.
 * This report is made by printing a line where all the elements but the deadline or the period start are set to 0.\n
 * Example for a deadline skip: ``.
 * Example for a period skip: ``.
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
					printf("job duration: %llu\t-\tutilization:%f\t-\tdensity:%f\n\n",elapsed,utilization,density); \
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
				fprintf(file,"%llu,%llu,%llu,%llu,%d,%llu,%f,%f\n",period_start,period_end,job_end,deadline,deadline_status,elapsed,utilization,density); \
				break; \
			case LOG_LEVEL_INFO: \
				printf("%llu,%llu,%llu,%llu,%d,%llu,%.3g,%.3g\n",period_start,period_end,job_end,deadline,deadline_status,elapsed,utilization,density); \
				break; \
		} \
	} while(0);
#endif
