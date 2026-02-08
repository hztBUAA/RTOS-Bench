/**
 * @file time.h
 * @brief Shadow header for time.h that provides CLOCK_MONOTONIC support
 *
 * OneOS's clock_gettime() only supports CLOCK_REALTIME. This wrapper:
 * 1. Includes the real system time.h
 * 2. Provides clock_gettime declaration
 * 3. Maps CLOCK_MONOTONIC to CLOCK_REALTIME so workloads using monotonic
 *    time can work (with the caveat that time may be affected by changes).
 */

#ifndef _ONEOS_TIME_WRAPPER_H
#define _ONEOS_TIME_WRAPPER_H

/* Include the real system time.h */
#if defined(__GNUC__)
#include_next <time.h>
#else
#include <time.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Clock ID definitions */
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME      1
#endif

/* Undefine any existing CLOCK_MONOTONIC (OneOS defines it as 4 but doesn't support it) */
#undef CLOCK_MONOTONIC
/* Map CLOCK_MONOTONIC to CLOCK_REALTIME so workloads work */
#define CLOCK_MONOTONIC CLOCK_REALTIME

/* clockid_t type */
#ifndef __clockid_t_defined
typedef int clockid_t;
#define __clockid_t_defined
#endif

/* clock_gettime declaration (provided by OneOS POSIX layer) */
int clock_gettime(clockid_t clockid, struct timespec *tp);
int clock_getres(clockid_t clockid, struct timespec *res);
int clock_settime(clockid_t clockid, const struct timespec *tp);

#ifdef __cplusplus
}
#endif

#endif /* _ONEOS_TIME_WRAPPER_H */
