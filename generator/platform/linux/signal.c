/**
 * @file signal.c
 * @brief Linux signal abstraction for rt-bench.
 */

#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_LINUX

#include <signal.h>
#include <string.h>

static rtbench_signal_handler_t signal_handlers[RTBENCH_SIG_QUIT + 1];

static int rtbench_signal_to_signo(rtbench_signal_t signal)
{
	switch (signal) {
	case RTBENCH_SIG_DEADLINE:
		return SIGRTMIN;
	case RTBENCH_SIG_PERIOD:
		return SIGRTMIN + 1;
	case RTBENCH_SIG_QUIT:
		return SIGINT;
	default:
		return -1;
	}
}

static rtbench_signal_t rtbench_signo_to_signal(int signo)
{
	if (signo == SIGRTMIN) {
		return RTBENCH_SIG_DEADLINE;
	}
	if (signo == SIGRTMIN + 1) {
		return RTBENCH_SIG_PERIOD;
	}
	return RTBENCH_SIG_QUIT;
}

static void rtbench_signal_dispatch(int signo, siginfo_t *info, void *context)
{
	rtbench_signal_t sig = rtbench_signo_to_signal(signo);

	if (sig <= RTBENCH_SIG_QUIT && signal_handlers[sig] != NULL) {
		signal_handlers[sig](sig, context);
	}
}

int rtbench_signal_register(rtbench_signal_t signal,
			    rtbench_signal_handler_t handler)
{
	struct sigaction sa;
	int signo = rtbench_signal_to_signo(signal);

	if (signo < 0) {
		return -1;
	}

	signal_handlers[signal] = handler;
	memset(&sa, 0, sizeof(sa));
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = rtbench_signal_dispatch;
	if (sigemptyset(&sa.sa_mask) == -1) {
		return -1;
	}
	if (sigaction(signo, &sa, NULL) == -1) {
		return -1;
	}
	return 0;
}

#endif /* RTBENCH_PLATFORM_LINUX */
