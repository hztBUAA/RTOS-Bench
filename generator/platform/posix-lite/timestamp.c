/**
 * @file timestamp.c
 * @brief POSIX-lite timestamp helpers for OneOS / Dongtu / Ruihua.
 */

#include "platform_abstraction.h"

#if defined(RTBENCH_PLATFORM_ONEOS) || defined(RTBENCH_PLATFORM_DONGTU) ||     \
	defined(RTBENCH_PLATFORM_RUIHUA)

#include <sys/time.h>
#include <time.h>

unsigned long long rtbench_get_rdtsc(void)
{
	struct timespec ts;
#ifdef CLOCK_MONOTONIC
	clock_gettime(CLOCK_MONOTONIC, &ts);
#else
	clock_gettime(CLOCK_REALTIME, &ts);
#endif
	return (unsigned long long)ts.tv_sec * 1000000000ULL +
	       (unsigned long long)ts.tv_nsec;
}

long double rtbench_get_timestamp(void)
{
	struct timespec ts;
#ifdef CLOCK_MONOTONIC
	if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
		return (long double)ts.tv_sec +
		       (long double)ts.tv_nsec / 1000000000.0L;
	}
#endif
	struct timeval tv;
	if (gettimeofday(&tv, NULL) == 0) {
		return (long double)tv.tv_sec +
		       (long double)tv.tv_usec / 1000000.0L;
	}
	return 0;
}

#endif /* RTBENCH_PLATFORM_ONEOS || RTBENCH_PLATFORM_DONGTU || RTBENCH_PLATFORM_RUIHUA */
