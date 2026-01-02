/**
 * @file timer.c
 * @brief RT-Thread timer implementation for rt-bench
 */

#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_RTTHREAD

#include <rtthread.h>
#include <stdlib.h>

struct rtbench_timer_internal {
	rt_timer_t timer;
	rtbench_timer_callback_t callback;
	void *user_data;
	rtbench_timer_type_t type;
};

static void rtbench_timer_timeout(void *parameter)
{
	struct rtbench_timer_internal *timer =
		(struct rtbench_timer_internal *)parameter;

	if (timer != NULL && timer->callback != NULL) {
		timer->callback(timer->user_data);
	}
}

rtbench_timer_t rtbench_timer_create(rtbench_timer_type_t timer_type,
				      rtbench_timer_callback_t callback,
				      void *user_data)
{
	struct rtbench_timer_internal *timer;
	char timer_name[RT_NAME_MAX];

	if (callback == NULL) {
		return NULL;
	}

	timer = (struct rtbench_timer_internal *)rt_malloc(sizeof(*timer));
	if (timer == NULL) {
		return NULL;
	}

	timer->callback = callback;
	timer->user_data = user_data;
	timer->type = timer_type;

	rt_snprintf(timer_name, RT_NAME_MAX, "rtbench_t%d", (int)timer_type);
	timer->timer = rt_timer_create(timer_name, rtbench_timer_timeout, timer,
				      RT_TICK_PER_SECOND,
				      RT_TIMER_FLAG_ONE_SHOT |
				      RT_TIMER_FLAG_HARD_TIMER);

	if (timer->timer == RT_NULL) {
		rt_free(timer);
		return NULL;
	}

	return (rtbench_timer_t)timer;
}

int rtbench_timer_settime(rtbench_timer_t timer, long sec, long nsec)
{
	struct rtbench_timer_internal *t =
		(struct rtbench_timer_internal *)timer;
	rt_tick_t timeout;

	if (t == NULL || t->timer == RT_NULL) {
		return -1;
	}

	timeout = (rt_tick_t)((sec * RT_TICK_PER_SECOND) +
			     (nsec * RT_TICK_PER_SECOND / 1000000000L));

	if (timeout == 0) {
		rt_timer_stop(t->timer);
		return 0;
	}

	rt_timer_control(t->timer, RT_TIMER_CTRL_SET_TIME, &timeout);
	if (t->type == RTBENCH_TIMER_PERIOD) {
		rt_timer_control(t->timer, RT_TIMER_CTRL_SET_PERIODIC, RT_NULL);
	} else {
		rt_timer_control(t->timer, RT_TIMER_CTRL_SET_ONESHOT, RT_NULL);
	}

	rt_timer_start(t->timer);
	return 0;
}

int rtbench_timer_delete(rtbench_timer_t timer)
{
	struct rtbench_timer_internal *t =
		(struct rtbench_timer_internal *)timer;

	if (t == NULL) {
		return -1;
	}

	if (t->timer != RT_NULL) {
		rt_timer_stop(t->timer);
		rt_timer_delete(t->timer);
		t->timer = RT_NULL;
	}

	rt_free(t);
	return 0;
}

#endif /* RTBENCH_PLATFORM_RTTHREAD */
