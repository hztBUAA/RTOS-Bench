/** @file logging.h
 * @ingroup base
 * @brief Logging utilities.
 * @author Mattia Nicolella
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
	LOG_LEVEL_TRACE, ///< Print informative messages on stdout and debug messages on stderr.
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

/** @brief Reports the benchmark timing depending on the chosen logging level.
 * @param[in] file The file where the timing will be printed if the logging level is set to `::LOG_LEVEL_FILE`.
 * @param[in] period_start_clocks The timestamp, in clock cycles, when the period started.
 * @param[in] period_end_clocks The timestamp, in clock cycles, when the period completed.
 * @param[in] job_end_clocks The timestamp, in clock cycles,  when the job ended.
 * @param[in] deadline_clocks The timestamp, in clock cycles, of the first deadline since the job started, or the timestamp of the skipped deadline.
 * @param[in] period_start The timestamp, in seconds, when the period started.
 * @param[in] period_end The timestamp, in seconds, when the period completed.
 * @param[in] job_end The timestamp, in seconds,  when the job ended.
 * @param[in] deadline The timestamp, in seconds, of the first deadline since the job started, or the timestamp of the skipped deadline.
*/
void print_timing(FILE *file, unsigned long long period_start_clocks,
		  unsigned long long period_end_clocks,
		  unsigned long long job_end_clocks,
		  unsigned long long deadline_clocks, long double period_start,
		  long double period_end, long double job_end,
		  long double deadline);
#endif
