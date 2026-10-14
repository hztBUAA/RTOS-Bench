#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>
#include "safe_sleep.h"

#if defined(RT_THREAD_PLATFORM)
#include <rtthread.h>
#endif

#if defined(RUIHUA_PLATFORM) || defined(DONGTU_PLATFORM)
void safe_usleep(uint64_t us) __attribute__((weak));
#endif

/* 安全睡眠 */
void safe_usleep(uint64_t us) {
#if defined(RT_THREAD_PLATFORM)
    /* RT-Thread libc splits usleep()/nanosleep() into whole-tick
     * rt_thread_delay() plus a sub-tick remainder served by rt_hw_us_delay().
     * Boards whose BSP omits rt_hw_us_delay() (e.g. Phytium) only ship the
     * weak stub, which prints a warning and returns without delaying -- the
     * 10/100 us settle calls here then flood the console on every multicore
     * bind.  Round up to whole ticks and use rt_thread_mdelay, which never
     * touches rt_hw_us_delay. */
    if (us == 0) {
        return;
    }
    {
        uint64_t ms = (us + 999u) / 1000u; /* ceil to 1 ms (>= 1 tick) */
        rt_thread_mdelay((rt_int32_t)ms);
    }
#else
    struct timespec req, rem;

    // 转换微秒到秒和纳秒
    req.tv_sec = us / 1000000;
    req.tv_nsec = (us % 1000000) * 1000L;

    while (nanosleep(&req, &rem) == -1 && errno == EINTR) {
        req = rem;
    }
#endif
}
