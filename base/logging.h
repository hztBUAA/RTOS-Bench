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

void print_statistics(FILE *file, unsigned long long period_start_clocks,
                        unsigned long long period_end_clocks, unsigned long long job_end_clocks,
                        unsigned long long deadline_clocks, long double period_start,
                        long double period_end, long double job_end,
                        long double deadline, long unsigned l1_ref_start, long unsigned l1_miss_start,
                        long unsigned l2_ref_start, long unsigned l2_miss_start, long unsigned l1_ref_end,
                        long unsigned l1_miss_end, long unsigned l2_ref_end, long unsigned l2_miss_end);

#endif
