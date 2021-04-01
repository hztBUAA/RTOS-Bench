#include "timingUtils.h"

/** @file get_cpu_timestamp.c
 * @brief Implementation of the API to get the CPU timestamp value.
 */

/** @details
 * Uses the `rdtsc` assembly primitive via the magic_timing_begin() macro.
 */
unsigned long long get_cpu_timestamp()
{
	unsigned long long timing = 0;
	unsigned int timeHigh = 0, timeLow = 0;
	magic_timing_begin(timeLow, timeHigh);
	timing = (((unsigned long long)0x0) | timeHigh) << 32 | timeLow;
	return timing;
}
