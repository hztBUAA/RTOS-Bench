/**
 * @file timestamp.c
 * @brief SylixOS timestamp implementation for rt-bench platform abstraction.
 * @details SylixOS supports POSIX clock_gettime API.
 */

#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_SYLIXOS

#include <time.h>

unsigned long long rtbench_get_rdtsc(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (unsigned long long)ts.tv_sec * 1000000000ULL +
	       (unsigned long long)ts.tv_nsec;
}

long double rtbench_get_timestamp(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (long double)ts.tv_sec +
	       (long double)ts.tv_nsec / 1000000000.0L;
}

#endif /* RTBENCH_PLATFORM_SYLIXOS */
