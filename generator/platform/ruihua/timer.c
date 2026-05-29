/**
 * @file timer.c
 * @brief POSIX-lite timer implementation for OneOS / Dongtu / Ruihua.
 * @details Ruihua/ReWorks does not provide SIGEV_THREAD timers in the tested
 *          BSP, so the platform layer uses a small pthread-backed timer.
 */

#include "platform_abstraction.h"

#if defined(RTBENCH_PLATFORM_ONEOS) || defined(RTBENCH_PLATFORM_DONGTU) ||     \
	defined(RTBENCH_PLATFORM_RUIHUA)

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct rtbench_timer_internal {
	pthread_t thread;
	rtbench_timer_callback_t callback;
	void *user_data;
	rtbench_timer_type_t type;
	volatile int active;
	int thread_started;
	long sec;
	long nsec;
};

static void rtbench_timer_sleep(long sec, long nsec)
{
	uint64_t usec = (uint64_t)sec * 1000000ULL;
	usec += (uint64_t)nsec / 1000ULL;
	if (usec == 0) {
		usec = 1;
	}
	usleep((useconds_t)usec);
}

static void *rtbench_timer_thread(void *arg)
{
	struct rtbench_timer_internal *timer =
		(struct rtbench_timer_internal *)arg;

	while (timer != NULL && timer->active) {
		rtbench_timer_sleep(timer->sec, timer->nsec);
		if (!timer->active) {
			break;
		}
		if (timer->callback != NULL) {
			timer->callback(timer->user_data);
		}
		if (timer->type != RTBENCH_TIMER_PERIOD) {
			break;
		}
	}
	return NULL;
}

rtbench_timer_t rtbench_timer_create(rtbench_timer_type_t timer_type,
				      rtbench_timer_callback_t callback,
				      void *user_data)
{
	struct rtbench_timer_internal *timer = NULL;

	if (callback == NULL) {
		return NULL;
	}

	timer = malloc(sizeof(*timer));
	if (timer == NULL) {
		return NULL;
	}

	memset(timer, 0, sizeof(*timer));
	timer->callback = callback;
	timer->user_data = user_data;
	timer->type = timer_type;

	return (rtbench_timer_t)timer;
}

int rtbench_timer_settime(rtbench_timer_t timer, long sec, long nsec)
{
	struct rtbench_timer_internal *t =
		(struct rtbench_timer_internal *)timer;
	int res;

	if (t == NULL) {
		return -1;
	}

	if (t->thread_started) {
		t->active = 0;
		pthread_join(t->thread, NULL);
		t->thread_started = 0;
	}

	t->sec = sec;
	t->nsec = nsec;
	t->active = 1;
	res = pthread_create(&t->thread, NULL, rtbench_timer_thread, t);
	if (res != 0) {
		t->active = 0;
		return -1;
	}
	t->thread_started = 1;

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

	t->active = 0;
	res = 0;
	if (t->thread_started && !pthread_equal(pthread_self(), t->thread)) {
		res = pthread_join(t->thread, NULL);
	}
	free(t);
	return (res == 0) ? 0 : -1;
}

#endif /* RTBENCH_PLATFORM_ONEOS || RTBENCH_PLATFORM_DONGTU || RTBENCH_PLATFORM_RUIHUA */
