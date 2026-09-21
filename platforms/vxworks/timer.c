/**
 * @file timer.c
 * @brief VxWorks 定时器抽象（RTOS-Bench 参考实现脚手架）
 *
 * 契约（见 generator/platform_abstraction.h）：
 *   rtbench_timer_create(type, callback, user_data) -> rtbench_timer_t
 *   rtbench_timer_settime(timer, sec, nsec)         -> 0 / -1
 *   rtbench_timer_delete(timer)                     -> 0 / -1
 *
 * 实现方式：taskSpawn + taskDelay 轮询。
 * 之所以不用 wdCreate/wdStart（看门狗定时器）：watchdog 回调运行在中断上下文，
 * 而 rt-bench 的定时器回调会释放周期信号量（可能唤醒任务），在中断上下文里
 * 做这件事在多数 VxWorks 配置下是不合法的。若你的 BSP 明确允许，可自行换成
 * watchdog 版本以获得更高精度。
 *
 * 精度说明：taskDelay 以 tick 为单位，精度受 sysClkRateGet() 限制，
 * 典型为 60~1000 Hz。需要亚 tick 精度请改用心跳 + vxTimeBaseGet() 自旋补偿。
 *
 * 注意：本文件在仓库的现有构建中不会被编译（见 platforms/vxworks/README.md）。
 */

#include "platform_abstraction.h"

#if defined(RTBENCH_PLATFORM_VXWORKS)

#include <vxWorks.h>
#include <taskLib.h>
#include <sysLib.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct rtbench_timer_internal {
    int tid;                         /* taskSpawn 返回的任务 ID，ERROR 表示失败 */
    rtbench_timer_callback_t callback;
    void *user_data;
    rtbench_timer_type_t type;
    uint32_t interval_ticks;
    volatile int running;
    volatile int started;
};

#define RTBENCH_TIMER_STACK_SIZE 0x4000   /* 16 KB，可按需调整 */
#define RTBENCH_TIMER_PRIORITY   150      /* VxWorks 数值越大优先级越低 */

static uint32_t rtbench_ms_to_ticks(uint32_t ms)
{
    int rate = sysClkRateGet();           /* ticks per second */
    uint32_t ticks;

    if (rate <= 0) {
        return ms;
    }
    ticks = (uint32_t)(((uint64_t)ms * (uint32_t)rate) / 1000U);
    return (ticks == 0U) ? 1U : ticks;    /* taskDelay(0) 只让出时间片，不能当延时用 */
}

static void rtbench_timer_task(void *parameter)
{
    struct rtbench_timer_internal *t = (struct rtbench_timer_internal *)parameter;

    while (t != NULL && t->running) {
        taskDelay((int)t->interval_ticks);
        if (!t->running) {
            break;
        }
        if (t->callback != NULL) {
            t->callback(t->user_data);
        }
        if (t->type == RTBENCH_TIMER_DEADLINE) {
            break;                        /* 单次定时：触发一次即退出 */
        }
    }

    if (t != NULL) {
        t->running = 0;
    }
}

rtbench_timer_t rtbench_timer_create(rtbench_timer_type_t timer_type,
                                     rtbench_timer_callback_t callback,
                                     void *user_data)
{
    struct rtbench_timer_internal *t;

    if (callback == NULL) {
        return NULL;
    }

    t = (struct rtbench_timer_internal *)calloc(1, sizeof(*t));
    if (t == NULL) {
        return NULL;
    }

    t->tid = ERROR;
    t->callback = callback;
    t->user_data = user_data;
    t->type = timer_type;
    t->interval_ticks = 1;

    return (rtbench_timer_t)t;
}

int rtbench_timer_settime(rtbench_timer_t timer, long sec, long nsec)
{
    struct rtbench_timer_internal *t = (struct rtbench_timer_internal *)timer;
    uint32_t ms;

    if (t == NULL) {
        return -1;
    }

    ms = (uint32_t)(sec * 1000 + nsec / 1000000);
    if (ms == 0) {
        ms = 1;                           /* 最小 1 ms */
    }
    t->interval_ticks = rtbench_ms_to_ticks(ms);

    if (t->started) {
        return 0;                         /* 只更新周期，任务已在跑 */
    }

    t->running = 1;
    t->tid = taskSpawn("rtb_timer",
                       RTBENCH_TIMER_PRIORITY,
                       0,                     /* options：0 = 默认，可加 VX_FP_TASK */
                       RTBENCH_TIMER_STACK_SIZE,
                       (FUNCPTR)rtbench_timer_task,
                       (int)t, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    if (t->tid == ERROR) {
        t->running = 0;
        return -1;
    }
    t->started = 1;

    return 0;
}

int rtbench_timer_delete(rtbench_timer_t timer)
{
    struct rtbench_timer_internal *t = (struct rtbench_timer_internal *)timer;

    if (t == NULL) {
        return -1;
    }

    t->running = 0;
    if (t->started && t->tid != ERROR) {
        taskDelete(t->tid);
    }

    free(t);
    return 0;
}

#endif /* RTBENCH_PLATFORM_VXWORKS */
