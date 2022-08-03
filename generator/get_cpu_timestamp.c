#include "get_cpu_timestamp.h"
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

/** @file get_cpu_timestamp.c
 * @ingroup generator
 * @author Mattia Nicolella
 * @brief Implementation of the API to get the CPU timestamp value.
 *
 * @copyright (C) 2021 - 2022, Mattia Nicolella <mnico@bu.edu> and the rt-bench contributors.
 * SPDX-License-Identifier: MIT
 */

/** @details
 * Uses the `rdtsc` assembly primitive via the magic_timing_begin() macro.
 */
unsigned long long get_rdtsc() {
  unsigned long long timing = 0;
  unsigned long timeHigh = 0, timeLow = 0;
  magic_timing_begin(timeLow, timeHigh);
  timing = (((unsigned long long)0x0) | timeHigh) << 32 | timeLow;
  return timing;
}

/** @details
 * The monotonic clock is always preferred when present and it is accessed by
 * `clock_gettime()`, if this call gives an error or if the clock is not present
 * then the timestamp is get using `gettimeofday()` which is less precise. If
 * this also fails then 0 is returned.
 */
long double get_timestamp() {
#ifdef _SC_MONOTONIC_CLOCK
  if (sysconf(_SC_MONOTONIC_CLOCK) > 0) {
    /* monotonic clock present */
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
      return (long double)(ts.tv_sec + ts.tv_nsec / 1000000000.0);
    }
  }
#endif
  struct timeval tv;
  if (gettimeofday(&tv, NULL) == 0) {
    return (long double)(tv.tv_sec + tv.tv_usec / 1000000.0);
  } else {
    return 0;
  }
}
