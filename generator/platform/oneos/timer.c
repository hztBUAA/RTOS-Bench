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

#if defined(ONEOS_V2_ARM64)
/* V2.0 ARM64: use os_timer_id and static control block */
struct rtbench_timer_internal {
    os_timer_id timer;
    os_timer_dummy_t timer_cb;  /* Static control block for V2.0 */
    rtbench_timer_callback_t callback;
    void *user_data;
    rtbench_timer_type_t type;
};
#else
/* V1.x ARM32: use os_timer_t pointer */
struct rtbench_timer_internal {
    os_timer_t *timer;
    rtbench_timer_callback_t callback;
    void *user_data;
    rtbench_timer_type_t type;
};
#endif

static void rtbench_timer_dispatch(void *parameter)
{
    struct rtbench_timer_internal *t = (struct rtbench_timer_internal *)parameter;
    if (t != NULL && t->callback != NULL) {
        t->callback(t->user_data);
    }
}

rtbench_timer_t rtbench_timer_create(rtbench_timer_type_t timer_type,
                                      rtbench_timer_callback_t callback,
                                      void *user_data)
{
    struct rtbench_timer_internal *t;
    uint8_t flag;

    if (callback == NULL) {
        return NULL;
    }

    t = (struct rtbench_timer_internal *)os_malloc(sizeof(*t));
    if (t == NULL) {
        return NULL;
    }

    flag = (timer_type == RTBENCH_TIMER_PERIOD) ?
           OS_TIMER_FLAG_PERIODIC : OS_TIMER_FLAG_ONE_SHOT;

#if defined(ONEOS_V2_ARM64)
    /* V2.0 ARM64: pass static control block as first parameter */
    t->timer = os_timer_create(&t->timer_cb, "rtbench", rtbench_timer_dispatch, t, 1, flag);
    if (t->timer == OS_NULL) {
        os_free(t);
        return NULL;
    }
#else
    /* V1.x ARM32: dynamic allocation, no static control block */
    t->timer = os_timer_create("rtbench", rtbench_timer_dispatch, t, 1, flag);
    if (t->timer == NULL) {
        os_free(t);
        return NULL;
    }
#endif

    t->callback = callback;
    t->user_data = user_data;
    t->type = timer_type;

    return (rtbench_timer_t)t;
}

int rtbench_timer_settime(rtbench_timer_t timer, long sec, long nsec)
{
    struct rtbench_timer_internal *t = (struct rtbench_timer_internal *)timer;
    os_tick_t ticks;
    uint32_t ms;

#if defined(ONEOS_V2_ARM64)
    if (t == NULL || t->timer == OS_NULL) {
        return -1;
    }
#else
    if (t == NULL || t->timer == NULL) {
        return -1;
    }
#endif

    /* Convert to milliseconds, then to ticks */
    ms = (uint32_t)(sec * 1000 + nsec / 1000000);
    if (ms == 0) {
        ms = 1;  /* Minimum 1ms */
    }
    ticks = os_tick_from_ms(ms);
    if (ticks == 0) {
        ticks = 1;
    }

    os_timer_stop(t->timer);
    os_timer_set_timeout_ticks(t->timer, ticks);
    os_timer_start(t->timer);

    return 0;
}

int rtbench_timer_delete(rtbench_timer_t timer)
{
    struct rtbench_timer_internal *t = (struct rtbench_timer_internal *)timer;

    if (t == NULL) {
        return -1;
    }

#if defined(ONEOS_V2_ARM64)
    if (t->timer != OS_NULL) {
        os_timer_stop(t->timer);
        os_timer_destroy(t->timer);
    }
#else
    if (t->timer != NULL) {
        os_timer_stop(t->timer);
        os_timer_destroy(t->timer);
    }
#endif

    os_free(t);
    return 0;
}

#endif /* RTBENCH_PLATFORM_ONEOS */
