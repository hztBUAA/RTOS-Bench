/**
 * @file signal.c
 * @brief RT-Thread signal abstraction for rt-bench
 */

#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_RTTHREAD

int rtbench_signal_register(rtbench_signal_t signal,
			    rtbench_signal_handler_t handler)
{
	(void)signal;
	(void)handler;
	return -1;
}

#endif /* RTBENCH_PLATFORM_RTTHREAD */
