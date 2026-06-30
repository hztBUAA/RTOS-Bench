/**
 * @file sync.c
 * @brief OneOS native semaphore wrapper for RTOS-Bench.
 */

#include "platform_abstraction.h"

#if defined(RTBENCH_PLATFORM_ONEOS)

#include <stddef.h>
#include <os_sem.h>
#include <os_memory.h>

/* Header compatibility for OneOS V2 style targets */
#if defined(ONEOS_V2_MUSL_LIBC) || defined(ONEOS_V2_ARM64) || defined(ONEOS_V2_LOONGARCH64)
    /* V2 style targets may not have these headers; define fallback constants. */
    #ifndef OS_EOK
    #define OS_EOK 0
    #endif
    #ifndef OS_WAIT_FOREVER
    #define OS_WAIT_FOREVER ((os_tick_t)-1)
    #endif
    #ifndef OS_SEM_MAX_VALUE
    #define OS_SEM_MAX_VALUE 0xFFFFFFFF
    #endif
#else
    /* V1.x ARM32: use original headers */
    #include <os_errno.h>
    #include <os_stddef.h>
#endif

struct rtbench_sem_internal {
    os_semaphore_id sem;
};

rtbench_sem_t rtbench_sem_create(unsigned int initial_value)
{
    struct rtbench_sem_internal *s;

    s = (struct rtbench_sem_internal *)os_malloc(sizeof(*s));
    if (s == NULL) {
        return NULL;
    }

    s->sem = os_semaphore_create(NULL, "rtbench_sem", initial_value, OS_SEM_MAX_VALUE);
    if (s->sem == NULL) {
        os_free(s);
        return NULL;
    }

    return (rtbench_sem_t)s;
}

int rtbench_sem_wait(rtbench_sem_t sem)
{
    struct rtbench_sem_internal *s = (struct rtbench_sem_internal *)sem;

    if (s == NULL || s->sem == NULL) {
        return -1;
    }

    return (os_semaphore_wait(s->sem, OS_WAIT_FOREVER) == OS_EOK) ? 0 : -1;
}

int rtbench_sem_post(rtbench_sem_t sem)
{
    struct rtbench_sem_internal *s = (struct rtbench_sem_internal *)sem;

    if (s == NULL || s->sem == NULL) {
        return -1;
    }

    return (os_semaphore_post(s->sem) == OS_EOK) ? 0 : -1;
}

int rtbench_sem_destroy(rtbench_sem_t sem)
{
    struct rtbench_sem_internal *s = (struct rtbench_sem_internal *)sem;

    if (s == NULL) {
        return -1;
    }

    if (s->sem != NULL) {
        os_semaphore_destroy(s->sem);
    }

    os_free(s);
    return 0;
}

#endif /* RTBENCH_PLATFORM_ONEOS */
