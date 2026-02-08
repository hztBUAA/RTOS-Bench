/**
 * @file timestamp.c
 * @brief OneOS native timestamp helpers for RTOS-Bench.
 */

#include "platform_abstraction.h"

#if defined(RTBENCH_PLATFORM_ONEOS)

#include <os_clock.h>
#include <oneos_config.h>

unsigned long long rtbench_get_rdtsc(void)
{
    os_tick_t ticks = os_tick_get();
    /* Convert ticks to nanoseconds */
    return (unsigned long long)ticks * (1000000000ULL / OS_TICK_PER_SECOND);
}

long double rtbench_get_timestamp(void)
{
    os_tick_t ticks = os_tick_get();
    return (long double)ticks / (long double)OS_TICK_PER_SECOND;
}

#endif /* RTBENCH_PLATFORM_ONEOS */
