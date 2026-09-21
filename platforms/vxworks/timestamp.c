/**
 * @file timestamp.c
 * @brief VxWorks 时间戳抽象（RTOS-Bench 参考实现脚手架）
 *
 * 契约（见 generator/platform_abstraction.h）：
 *   rtbench_get_rdtsc()     -> 单调递增的"周期数"（本实现用纳秒近似）
 *   rtbench_get_timestamp() -> 秒（long double）
 *
 * 两条路径：
 *   1. tickGet64() 按 sysClkRateGet() 换算 —— 默认路径，任何 VxWorks 版本都可用；
 *      精度 = 1/sysClkRateGet() 秒（典型 1~16 ms，对可调度性测试偏粗）。
 *   2. clock_gettime(CLOCK_MONOTONIC, ...) —— 高精度路径，需显式定义
 *      RTBENCH_VXWORKS_USE_POSIX_CLOCK 启用（VxWorks 7 的 POSIX 层提供；
 *      6.9 内核若没有该符号则不要启用，编译会失败）。
 * 若目标板有独立 timebase 寄存器（vxTimeBaseGet()/vxTimeBaseFreq()），
 * 可达亚 tick 精度，建议自行插入为第三条路径。
 *
 * 注意：本文件在仓库的现有构建中不会被编译（见 platforms/vxworks/README.md）。
 */

#include "platform_abstraction.h"

#if defined(RTBENCH_PLATFORM_VXWORKS)

#include <vxWorks.h>
#include <sysLib.h>
#include <tickLib.h>
#include <time.h>

#if defined(RTBENCH_VXWORKS_USE_POSIX_CLOCK)
#define RTBENCH_VXWORKS_HAS_CLOCK 1
#else
#define RTBENCH_VXWORKS_HAS_CLOCK 0
#endif

unsigned long long rtbench_get_rdtsc(void)
{
#if RTBENCH_VXWORKS_HAS_CLOCK
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return (unsigned long long)ts.tv_sec * 1000000000ULL
               + (unsigned long long)ts.tv_nsec;
    }
#endif
    {
        int rate = sysClkRateGet();
        if (rate <= 0) {
            rate = 1;
        }
        return (unsigned long long)tickGet64() * (1000000000ULL / (unsigned long long)rate);
    }
}

long double rtbench_get_timestamp(void)
{
#if RTBENCH_VXWORKS_HAS_CLOCK
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return (long double)ts.tv_sec + (long double)ts.tv_nsec / 1000000000.0L;
    }
#endif
    {
        int rate = sysClkRateGet();
        if (rate <= 0) {
            rate = 1;
        }
        return (long double)tickGet64() / (long double)rate;
    }
}

#endif /* RTBENCH_PLATFORM_VXWORKS */
