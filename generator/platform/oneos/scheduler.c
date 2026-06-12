/**
 * @file scheduler.c
 * @brief OneOS native scheduler abstraction for RTOS-Bench.
 */

#include "platform_abstraction.h"

#if defined(RTBENCH_PLATFORM_ONEOS)

#include <stddef.h>
#include <stdint.h>
#include <os_task.h>

int rtbench_set_priority(unsigned int priority)
{
#if defined(ONEOS_V2_ARM64) || defined(ONEOS_V2_LOONGARCH64)
    /* V2.0 ARM64/LoongArch64: os_task_id is int type, use os_get_current_task() */
    os_task_id current = os_get_current_task();
    if (current == OS_NULL) {
        return -1;
    }
    os_task_set_priority(current, (uint8_t)priority);
#else
    /* V1.x ARM32: os_task_t* is pointer type, use os_task_self() */
    os_task_t *current = os_task_self();
    if (current == NULL) {
        return -1;
    }
    /* OneOS priority: lower value = higher priority */
    os_task_set_priority(current, (os_uint8_t)priority);
#endif
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
