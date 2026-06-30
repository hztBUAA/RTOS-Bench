/**
 * @file timestamp.c
 * @brief OneOS native timestamp helpers for RTOS-Bench.
 */

#include "platform_abstraction.h"

#if defined(RTBENCH_PLATFORM_ONEOS)

#include <os_clock.h>
#include <oneos_config.h>

/* API compatibility: V2 style targets use os_tick_get_value(), V1.x uses os_tick_get() */
#if defined(ONEOS_V2_MUSL_LIBC) || defined(ONEOS_V2_ARM64) || defined(ONEOS_V2_LOONGARCH64)
    #define RTBENCH_GET_TICK()  os_tick_get_value()
#else
    #define RTBENCH_GET_TICK()  os_tick_get()
#endif

unsigned long long rtbench_get_rdtsc(void)
{
    os_tick_t ticks = RTBENCH_GET_TICK();
    /* Convert ticks to nanoseconds */
    return (unsigned long long)ticks * (1000000000ULL / OS_TICK_PER_SECOND);
}

long double rtbench_get_timestamp(void)
{
    os_tick_t ticks = RTBENCH_GET_TICK();
    return (long double)ticks / (long double)OS_TICK_PER_SECOND;
}

#endif /* RTBENCH_PLATFORM_ONEOS */
