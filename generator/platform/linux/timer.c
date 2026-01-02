/**
 * @file timer.c
 * @brief Linux timer implementation for rt-bench platform abstraction.
 */

#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_LINUX

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct rtbench_timer_internal {
	timer_t timer;
	rtbench_timer_callback_t callback;
	void *user_data;
	rtbench_timer_type_t type;
};

static void rtbench_timer_dispatch(union sigval value)
{
	struct rtbench_timer_internal *timer =
		(struct rtbench_timer_internal *)value.sival_ptr;

	if (timer != NULL && timer->callback != NULL) {
		timer->callback(timer->user_data);
	}
}

rtbench_timer_t rtbench_timer_create(rtbench_timer_type_t timer_type,
				      rtbench_timer_callback_t callback,
				      void *user_data)
{
	struct rtbench_timer_internal *timer = NULL;
	struct sigevent event;
	int res;

	if (callback == NULL) {
		return NULL;
	}

	timer = malloc(sizeof(*timer));
	if (timer == NULL) {
		return NULL;
	}

	memset(&event, 0, sizeof(event));
	event.sigev_notify = SIGEV_THREAD;
	event.sigev_notify_function = rtbench_timer_dispatch;
	event.sigev_value.sival_ptr = timer;

	res = timer_create(CLOCK_REALTIME, &event, &timer->timer);
	if (res != 0) {
		free(timer);
		return NULL;
	}

	timer->callback = callback;
	timer->user_data = user_data;
	timer->type = timer_type;

	return (rtbench_timer_t)timer;
}

int rtbench_timer_settime(rtbench_timer_t timer, long sec, long nsec)
{
	struct rtbench_timer_internal *t =
		(struct rtbench_timer_internal *)timer;
	struct itimerspec timer_spec;
	int res;

	if (t == NULL) {
		return -1;
	}

	memset(&timer_spec, 0, sizeof(timer_spec));
	timer_spec.it_value.tv_sec = sec;
	timer_spec.it_value.tv_nsec = nsec;
	if (t->type == RTBENCH_TIMER_PERIOD) {
		timer_spec.it_interval.tv_sec = sec;
		timer_spec.it_interval.tv_nsec = nsec;
	}

	res = timer_settime(t->timer, 0, &timer_spec, NULL);
	if (res < 0) {
		return -1;
	}

	return 0;
}

int rtbench_timer_delete(rtbench_timer_t timer)
{
	struct rtbench_timer_internal *t =
		(struct rtbench_timer_internal *)timer;
	int res;

	if (t == NULL) {
		return -1;
	}

	res = timer_delete(t->timer);
	free(t);
	return (res == 0) ? 0 : -1;
}

#endif /* RTBENCH_PLATFORM_LINUX */
