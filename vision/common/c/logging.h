/** @file logging.h
 * @brief Logging utilities.
 * @details Logging interfaces, to support different verbosity levels.
 * These interfaces are implemented as function-like macros, since they will mainly rely on the fprintf() function.
 */

#ifndef LOGGING_H
#define LOGGING_H

#include<stdio.h>

/** @brief An enum that identifies the available log levels.
 * @details The log level will determine if a particular message will be printed or not.
 */
enum log_level{
	LOG_LEVEL_ERR=1, ///< Only print error messages.
	LOG_LEVEL_FILE, ///< Print benchmark stats only to file.
	LOG_LEVEL_INFO, ///< Print benchmark stats to stdout.
	LOG_LEVEL_TRACE, ///< Print also informative messages on the benchmark progress.
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

#endif
