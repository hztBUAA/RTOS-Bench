/**
 * @file timer.c
 * @brief RT-Thread timer implementation for rt-bench
 */

#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_RTTHREAD

#include <rtthread.h>
#include <stdlib.h>

struct rtbench_timer_internal {
	rt_thread_t thread;
	rtbench_timer_callback_t callback;
	void *user_data;
	rtbench_timer_type_t type;
	rt_tick_t timeout_ticks;
	rt_bool_t running;
};

static void rtbench_timer_thread(void *parameter)
{
	struct rtbench_timer_internal *timer =
		(struct rtbench_timer_internal *)parameter;

	if (timer == RT_NULL || timer->callback == RT_NULL) {
		return;
	}

	if (timer->type == RTBENCH_TIMER_PERIOD) {
		while (timer->running) {
			rt_thread_mdelay(timer->timeout_ticks);
			if (!timer->running) {
				break;
			}
			timer->callback(timer->user_data);
		}
	} else {
		rt_thread_mdelay(timer->timeout_ticks);
		if (timer->running) {
			timer->callback(timer->user_data);
		}
	}
}

rtbench_timer_t rtbench_timer_create(rtbench_timer_type_t timer_type,
				     rtbench_timer_callback_t callback,
				     void *user_data)
{
	struct rtbench_timer_internal *timer;
	char thread_name[RT_NAME_MAX];

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
	timer->timeout_ticks = 0;
	timer->running = RT_FALSE;

	rt_snprintf(thread_name, RT_NAME_MAX, "rtb_t%02d", (int)timer_type);
	/* Use a moderate priority and larger stack to avoid stack overflow. */
	timer->thread = rt_thread_create(thread_name, rtbench_timer_thread,
					timer, 2048,
					RT_THREAD_PRIORITY_MAX / 2,
					10);
	if (timer->thread == RT_NULL) {
		rt_free(timer);
		return NULL;
	}

	return (rtbench_timer_t)timer;
}

int rtbench_timer_settime(rtbench_timer_t timer, long sec, long nsec)
{
	struct rtbench_timer_internal *t =
		(struct rtbench_timer_internal *)timer;
	rt_tick_t ticks;

	if (t == NULL) {
		return -1;
	}

	ticks = (rt_tick_t)((sec * RT_TICK_PER_SECOND) +
			    (nsec * RT_TICK_PER_SECOND / 1000000000L));
	if (ticks == 0) {
		t->running = RT_FALSE;
		return 0;
	}

	t->timeout_ticks = ticks;
	t->running = RT_TRUE;

	if (t->thread->stat == RT_THREAD_INIT) {
		rt_thread_startup(t->thread);
	}
	return 0;
}

int rtbench_timer_delete(rtbench_timer_t timer)
{
	struct rtbench_timer_internal *t =
		(struct rtbench_timer_internal *)timer;

	if (t == NULL) {
		return -1;
	}

	t->running = RT_FALSE;
	if (t->thread != RT_NULL) {
		rt_thread_delete(t->thread);
		t->thread = RT_NULL;
	}
	rt_free(t);
	return 0;
}

#endif /* RTBENCH_PLATFORM_RTTHREAD */
