/**
 * @file scheduler.c
 * @brief OneOS native scheduler abstraction for RTOS-Bench.
 */

#include "platform_abstraction.h"

#if defined(RTBENCH_PLATFORM_ONEOS)

#include <stddef.h>
#include <os_task.h>

int rtbench_set_priority(unsigned int priority)
{
    os_task_t *current = os_task_self();
    if (current == NULL) {
        return -1;
    }
    /* OneOS priority: lower value = higher priority */
    os_task_set_priority(current, (os_uint8_t)priority);
    return 0;
}

int rtbench_set_deadline(uint64_t runtime, uint64_t deadline, uint64_t period)
{
    /* OneOS does not have native deadline scheduling */
    (void)runtime;
    (void)deadline;
    (void)period;
    return -1;
}

int rtbench_set_affinity(uint32_t cpu_mask)
{
    /* OneOS single-core targets don't need affinity */
    (void)cpu_mask;
    return 0;
}

#endif /* RTBENCH_PLATFORM_ONEOS */
