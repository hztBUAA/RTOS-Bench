/**
 * @file signal.c
 * @brief OneOS native signal/event abstraction for RTOS-Bench.
 */

#include "platform_abstraction.h"

#if defined(RTBENCH_PLATFORM_ONEOS)

#include <stddef.h>

static rtbench_signal_handler_t g_handlers[3] = {NULL, NULL, NULL};

int rtbench_signal_register(rtbench_signal_t signal,
                            rtbench_signal_handler_t handler)
{
    if (signal > RTBENCH_SIG_QUIT) {
        return -1;
    }
    g_handlers[signal] = handler;
    return 0;
}

/* Internal function to trigger signal handlers (called by timer callbacks) */
void rtbench_signal_trigger(rtbench_signal_t signal, void *context)
{
    if (signal <= RTBENCH_SIG_QUIT && g_handlers[signal] != NULL) {
        g_handlers[signal](signal, context);
    }
}

#endif /* RTBENCH_PLATFORM_ONEOS */
