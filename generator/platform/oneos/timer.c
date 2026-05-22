/**
 * @file timer.c
 * @brief OneOS native timer implementation for RTOS-Bench.
 */

#include "platform_abstraction.h"

#if defined(RTBENCH_PLATFORM_ONEOS)

#include <os_timer.h>
#include <os_clock.h>
#include <os_memory.h>
#include <os_task.h>
#include <string.h>
#include <stdint.h>

/*
 * OneOS kernel timers do not reliably invoke callbacks from dynamically loaded
 * .out modules on the Phytium Pi V1.5 image. Use a small task-based timer
 * instead; it is less precise, but gives RTOS-Bench's period semaphores a
 * stable callback context for board acceptance.
 */
struct rtbench_timer_internal {
    os_task_id task;
    os_task_dummy_t task_cb;
    rtbench_timer_callback_t callback;
    void *user_data;
    rtbench_timer_type_t type;
    uint32_t interval_ms;
    volatile int running;
    volatile int started;
};

#define RTBENCH_TIMER_STACK_SIZE (4096U)
#define RTBENCH_TIMER_PRIORITY   (OS_TASK_PRIORITY_MAX / 2)

static void rtbench_timer_task(void *parameter)
{
    struct rtbench_timer_internal *t = (struct rtbench_timer_internal *)parameter;

    while (t != NULL && t->running) {
        os_task_msleep(t->interval_ms > 0 ? t->interval_ms : 1);
        if (!t->running) {
            break;
        }
        if (t->callback != NULL) {
            t->callback(t->user_data);
        }
        if (t->type == RTBENCH_TIMER_DEADLINE) {
            break;
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

    t = (struct rtbench_timer_internal *)os_malloc(sizeof(*t));
    if (t == NULL) {
        return NULL;
    }

    memset(t, 0, sizeof(*t));
    t->callback = callback;
    t->user_data = user_data;
    t->type = timer_type;
    t->interval_ms = 1;

    return (rtbench_timer_t)t;
}

int rtbench_timer_settime(rtbench_timer_t timer, long sec, long nsec)
{
    struct rtbench_timer_internal *t = (struct rtbench_timer_internal *)timer;
    uint32_t ms;

    if (t == NULL) {
        return -1;
    }

    /* Convert to milliseconds, then to ticks */
    ms = (uint32_t)(sec * 1000 + nsec / 1000000);
    if (ms == 0) {
        ms = 1;  /* Minimum 1ms */
    }
    t->interval_ms = ms;

    if (t->started) {
        return 0;
    }

    t->running = 1;
    t->task = os_task_create(&t->task_cb,
                             OS_NULL,
                             RTBENCH_TIMER_STACK_SIZE,
                             "rtb_timer",
                             rtbench_timer_task,
                             t,
                             RTBENCH_TIMER_PRIORITY);
    if (t->task < 0) {
        t->running = 0;
        return -1;
    }
    t->started = 1;
    os_task_startup(t->task);

    return 0;
}

int rtbench_timer_delete(rtbench_timer_t timer)
{
    struct rtbench_timer_internal *t = (struct rtbench_timer_internal *)timer;

    if (t == NULL) {
        return -1;
    }

    t->running = 0;
    if (t->started && t->task >= 0) {
        os_task_destroy(t->task);
    }

    os_free(t);
    return 0;
}

#endif /* RTBENCH_PLATFORM_ONEOS */
