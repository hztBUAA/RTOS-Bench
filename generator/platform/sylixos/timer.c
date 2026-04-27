/**
 * @file timer.c
 * @brief SylixOS timer implementation for rt-bench platform abstraction.
 * @details SylixOS has incomplete SIGEV_THREAD support in timer_create().
 *          This implementation uses pthread-based periodic timer as fallback.
 */

#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_SYLIXOS

#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct rtbench_timer_internal {
	timer_t timer;                    /* POSIX timer (unused, kept for compatibility) */
	pthread_t thread;                 /* Pthread-based timer thread */
	rtbench_timer_callback_t callback;
	void *user_data;
	rtbench_timer_type_t type;
	volatile int running;             /* Thread control flag */
	volatile int started;             /* Timer started flag */
	long period_sec;                  /* Period for pthread timer */
	long period_nsec;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
};

/**
 * @brief Pthread-based timer thread function
 * @details Implements periodic timer using clock_nanosleep() for accurate timing
 */
static void *rtbench_timer_thread(void *arg)
{
	struct rtbench_timer_internal *timer = (struct rtbench_timer_internal *)arg;
	struct timespec next_time, now;

	/* Wait for timer to be started */
	pthread_mutex_lock(&timer->mutex);
	while (!timer->started && timer->running) {
		pthread_cond_wait(&timer->cond, &timer->mutex);
	}
	pthread_mutex_unlock(&timer->mutex);

	if (!timer->running) {
		return NULL;
	}

	/* Get initial time */
	clock_gettime(CLOCK_MONOTONIC, &next_time);

	while (timer->running) {
		/* Calculate next wakeup time */
		next_time.tv_sec += timer->period_sec;
		next_time.tv_nsec += timer->period_nsec;
		if (next_time.tv_nsec >= 1000000000) {
			next_time.tv_sec++;
			next_time.tv_nsec -= 1000000000;
		}

		/* Sleep until next period */
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_time, NULL);

		/* Check if still running before invoking callback */
		if (!timer->running) {
			break;
		}

		/* Invoke callback */
		if (timer->callback) {
			timer->callback(timer->user_data);
		}

		/* For one-shot timers, exit after first callback */
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
	pthread_attr_t attr;
	int res;

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
	timer->running = 1;
	timer->started = 0;

	/* Initialize mutex and condition variable */
	pthread_mutex_init(&timer->mutex, NULL);
	pthread_cond_init(&timer->cond, NULL);

	/* Create timer thread */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	res = pthread_create(&timer->thread, &attr, rtbench_timer_thread, timer);
	pthread_attr_destroy(&attr);

	if (res != 0) {
		pthread_mutex_destroy(&timer->mutex);
		pthread_cond_destroy(&timer->cond);
		free(timer);
		return NULL;
	}

	return (rtbench_timer_t)timer;
}

int rtbench_timer_settime(rtbench_timer_t timer, long sec, long nsec)
{
	struct rtbench_timer_internal *t =
		(struct rtbench_timer_internal *)timer;

	if (t == NULL) {
		return -1;
	}

	/* Store period for pthread timer */
	t->period_sec = sec;
	t->period_nsec = nsec;

	/* Signal the timer thread to start */
	pthread_mutex_lock(&t->mutex);
	t->started = 1;
	pthread_cond_signal(&t->cond);
	pthread_mutex_unlock(&t->mutex);

	return 0;
}

int rtbench_timer_delete(rtbench_timer_t timer)
{
	struct rtbench_timer_internal *t =
		(struct rtbench_timer_internal *)timer;

	if (t == NULL) {
		return -1;
	}

	/* Signal thread to stop */
	pthread_mutex_lock(&t->mutex);
	t->running = 0;
	t->started = 1;  /* Wake up thread if it's waiting */
	pthread_cond_signal(&t->cond);
	pthread_mutex_unlock(&t->mutex);

	/* Wait for thread to finish */
	pthread_join(t->thread, NULL);

	/* Cleanup */
	pthread_mutex_destroy(&t->mutex);
	pthread_cond_destroy(&t->cond);
	free(t);

	return 0;
}

#endif /* RTBENCH_PLATFORM_SYLIXOS */
