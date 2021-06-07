/** @file logging.c
 * @ingroup base
 * @brief Implementation of the logging facilities in logging.h.
 */

#include "logging.h"

/** @details
 * The benchmark verbosity should be initialized only in during the benchmark startup.
 * It is made available as a global variable since every time the logging macro is invoked, this variable
 * must be checked to determine if the message has to be printed.
 * The default log level is ::LOG_LEVEL_INFO.
 */
enum log_level benchmark_verbosity = LOG_LEVEL_INFO;
