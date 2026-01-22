/**
 * @file timer.c
 * @brief RT-Thread timer implementation for rt-bench
 */

#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_RTTHREAD

#include <rtthread.h>
#include <stdlib.h>
#include "logging.h"

struct rtbench_timer_internal {
	rt_timer_t timer;
	rtbench_timer_callback_t callback;
	void *user_data;
	rtbench_timer_type_t type;
	rt_tick_t timeout_ticks;
};

static void rtbench_timer_shim(void *parameter)
{
	struct rtbench_timer_internal *t =
		(struct rtbench_timer_internal *)parameter;

	if (t == RT_NULL || t->callback == RT_NULL) {
		elogf(LOG_LEVEL_ERR, "[rtbench][timer] null shim context\n");
		return;
	}
	elogf(LOG_LEVEL_TRACE, "[rtbench][timer] fire type=%d ticks=%lu\n",
	      t->type, (unsigned long)t->timeout_ticks);
	t->callback(t->user_data);
}

rtbench_timer_t rtbench_timer_create(rtbench_timer_type_t timer_type,
				     rtbench_timer_callback_t callback,
				     void *user_data)
{
	struct rtbench_timer_internal *timer;
	rt_uint8_t flags;

	if (callback == NULL) {
		return NULL;
	}

	timer = (struct rtbench_timer_internal *)rt_malloc(sizeof(*timer));
	if (timer == NULL) {
		return NULL;
	}
	rt_memset(timer, 0, sizeof(*timer));
	timer->callback = callback;
	timer->user_data = user_data;
	timer->type = timer_type;
	timer->timeout_ticks = 1;

	flags = RT_TIMER_FLAG_SOFT_TIMER;
	if (timer_type == RTBENCH_TIMER_PERIOD) {
		flags |= RT_TIMER_FLAG_PERIODIC;
	} else {
		flags |= RT_TIMER_FLAG_ONE_SHOT;
	}

	timer->timer = rt_timer_create("rtb_tim", rtbench_timer_shim, timer,
				       timer->timeout_ticks, flags);
	if (timer->timer == RT_NULL) {
		rt_free(timer);
		return NULL;
	}

	elogf(LOG_LEVEL_TRACE, "[rtbench][timer] create type=%d\n",
	      timer_type);
	return (rtbench_timer_t)timer;
}

int rtbench_timer_settime(rtbench_timer_t timer, long sec, long nsec)
{
	struct rtbench_timer_internal *t =
		(struct rtbench_timer_internal *)timer;
	rt_tick_t ticks;

	if (t == NULL || t->timer == RT_NULL) {
		return -1;
	}

	ticks = (rt_tick_t)((sec * RT_TICK_PER_SECOND) +
			    (nsec * RT_TICK_PER_SECOND / 1000000000L));
	if (ticks == 0) {
		return -1;
	}

	t->timeout_ticks = ticks;
	rt_timer_control(t->timer, RT_TIMER_CTRL_SET_TIME, &ticks);
	elogf(LOG_LEVEL_TRACE,
	      "[rtbench][timer] settime type=%d ticks=%lu (sec=%ld nsec=%ld)\n",
	      t->type, (unsigned long)ticks, sec, nsec);
	return rt_timer_start(t->timer);
}

int rtbench_timer_delete(rtbench_timer_t timer)
{
	struct rtbench_timer_internal *t =
		(struct rtbench_timer_internal *)timer;

	if (t == NULL || t->timer == RT_NULL) {
		return -1;
	}

	rt_timer_stop(t->timer);
	rt_timer_delete(t->timer);
	t->timer = RT_NULL;
	elogf(LOG_LEVEL_TRACE, "[rtbench][timer] delete\n");
	rt_free(t);
	return 0;
}

#endif /* RTBENCH_PLATFORM_RTTHREAD */
