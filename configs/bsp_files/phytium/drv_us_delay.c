/*
 * SPDX-License-Identifier: Apache-2.0
 * Microsecond busy wait using the AArch64 physical system counter.
 */
#include <rtthread.h>
#include <gtimer.h>
#include <cpuport.h>

void rt_hw_us_delay(rt_uint32_t us)
{
    rt_uint64_t freq, ticks, start, now;

    if (us == 0)
        return;

    freq = rt_hw_get_gtimer_frq();
    RT_ASSERT(freq != 0);
    ticks = (freq * (rt_uint64_t)us + 999999ULL) / 1000000ULL;

    rt_hw_isb();
    start = rt_hw_get_cntpct_val();
    do
    {
        rt_hw_isb();
        now = rt_hw_get_cntpct_val();
    } while ((now - start) < ticks);
}
