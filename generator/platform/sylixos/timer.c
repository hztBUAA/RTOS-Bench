/**
 * @file timer.c
 * @brief SylixOS timer implementation for rt-bench platform abstraction.
 * @details SylixOS does NOT support SIGEV_THREAD, so we use SIGEV_SIGNAL
 *          with a dedicated signal (SIGRTMIN) and a global timer registry.
 */

#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_SYLIXOS

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Maximum concurrent timers supported */
#define RTBENCH_MAX_TIMERS 64

/* Use SIGRTMIN for timer signals (real-time signal, won't conflict) */
#define RTBENCH_TIMER_SIGNAL SIGRTMIN

struct rtbench_timer_internal {
	timer_t timer;
	rtbench_timer_callback_t callback;
	void *user_data;
	rtbench_timer_type_t type;
	int active;
};

/* Global timer registry for signal handler lookup */
static struct rtbench_timer_internal *g_timer_registry[RTBENCH_MAX_TIMERS];
static pthread_mutex_t g_timer_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_signal_handler_installed = 0;

/**
 * @brief Signal handler for POSIX timer expiration
 */
static void rtbench_timer_signal_handler(int sig, siginfo_t *si, void *uc)
{
	struct rtbench_timer_internal *timer;
	(void)sig;
	(void)uc;

	if (si == NULL) {
		return;
	}

	/* Get timer pointer from signal value */
	timer = (struct rtbench_timer_internal *)si->si_value.sival_ptr;
	if (timer != NULL && timer->active && timer->callback != NULL) {
		timer->callback(timer->user_data);
	}
}

/**
 * @brief Install signal handler for timer signals (once)
 */
static int rtbench_timer_install_handler(void)
{
	struct sigaction sa;

	if (g_signal_handler_installed) {
		return 0;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = rtbench_timer_signal_handler;
	sigemptyset(&sa.sa_mask);

	if (sigaction(RTBENCH_TIMER_SIGNAL, &sa, NULL) < 0) {
		fprintf(stderr, "[timer] sigaction failed: errno=%d (%s)\n",
			errno, strerror(errno));
		return -1;
	}

	g_signal_handler_installed = 1;
	return 0;
}

/**
 * @brief Register timer in global registry
 */
static int rtbench_timer_register(struct rtbench_timer_internal *timer)
{
	int i;

	pthread_mutex_lock(&g_timer_mutex);
	for (i = 0; i < RTBENCH_MAX_TIMERS; i++) {
		if (g_timer_registry[i] == NULL) {
			g_timer_registry[i] = timer;
			pthread_mutex_unlock(&g_timer_mutex);
			return 0;
		}
	}
	pthread_mutex_unlock(&g_timer_mutex);
	return -1; /* Registry full */
}

/**
 * @brief Unregister timer from global registry
 */
static void rtbench_timer_unregister(struct rtbench_timer_internal *timer)
{
	int i;

	pthread_mutex_lock(&g_timer_mutex);
	for (i = 0; i < RTBENCH_MAX_TIMERS; i++) {
		if (g_timer_registry[i] == timer) {
			g_timer_registry[i] = NULL;
			break;
		}
	}
	pthread_mutex_unlock(&g_timer_mutex);
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

	/* Install signal handler if not already done */
	if (rtbench_timer_install_handler() < 0) {
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
	timer->active = 1;

	/* Register in global registry */
	if (rtbench_timer_register(timer) < 0) {
		fprintf(stderr, "[timer] timer registry full\n");
		free(timer);
		return NULL;
	}

	/* Use SIGEV_SIGNAL instead of SIGEV_THREAD */
	memset(&event, 0, sizeof(event));
	event.sigev_notify = SIGEV_SIGNAL;
	event.sigev_signo = RTBENCH_TIMER_SIGNAL;
	event.sigev_value.sival_ptr = timer;

	res = timer_create(CLOCK_REALTIME, &event, &timer->timer);
	if (res != 0) {
		fprintf(stderr, "[timer] timer_create failed: errno=%d (%s)\n",
			errno, strerror(errno));
		rtbench_timer_unregister(timer);
		free(timer);
		return NULL;
	}

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

	t->active = 0;
	res = timer_delete(t->timer);
	rtbench_timer_unregister(t);
	free(t);
	return (res == 0) ? 0 : -1;
}

#endif /* RTBENCH_PLATFORM_SYLIXOS */
