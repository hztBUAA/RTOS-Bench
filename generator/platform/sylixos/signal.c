/**
 * @file signal.c
 * @brief SylixOS signal abstraction for rt-bench.
 * @details SylixOS supports POSIX signal API.
 */

#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_SYLIXOS

#include <signal.h>
#include <stddef.h>

static rtbench_signal_handler_t signal_handlers[3] = { NULL, NULL, NULL };

static void signal_dispatch(int signo)
{
	rtbench_signal_t sig;

	switch (signo) {
	case SIGALRM:
		sig = RTBENCH_SIG_DEADLINE;
		break;
	case SIGUSR1:
		sig = RTBENCH_SIG_PERIOD;
		break;
	case SIGINT:
	case SIGTERM:
		sig = RTBENCH_SIG_QUIT;
		break;
	default:
		return;
	}

	if (signal_handlers[sig] != NULL) {
		signal_handlers[sig](sig, NULL);
	}
}

int rtbench_signal_register(rtbench_signal_t signal,
			    rtbench_signal_handler_t handler)
{
	struct sigaction sa;
	int signo;

	if ((int)signal < 0 || (int)signal > 2) {
		return -1;
	}

	signal_handlers[signal] = handler;

	switch (signal) {
	case RTBENCH_SIG_DEADLINE:
		signo = SIGALRM;
		break;
	case RTBENCH_SIG_PERIOD:
		signo = SIGUSR1;
		break;
	case RTBENCH_SIG_QUIT:
		signo = SIGINT;
		break;
	default:
		return -1;
	}

	sa.sa_handler = signal_dispatch;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	return sigaction(signo, &sa, NULL);
}

#endif /* RTBENCH_PLATFORM_SYLIXOS */
