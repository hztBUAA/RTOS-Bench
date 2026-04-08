/**
 * @file posix_sched_adapter.c
 * @brief POSIX scheduling functions adapter for OneOS
 *
 * Provides implementations of pthread_attr_setinheritsched and
 * pthread_attr_getinheritsched that are missing from OneOS's POSIX layer.
 *
 * For V1.x ARM32: Works with pthread_attr_t's inheritsched field.
 * For V2.0 ARM64: Uses a static variable since pthread_attr_t lacks this field.
 */

#include "platform_abstraction.h"
#include <pthread.h>
#include <errno.h>

#if defined(ONEOS_V2_ARM64)
/*
 * OneOS V2.0 ARM64: pthread_attr_t doesn't have inheritsched member.
 * Use a static variable to track the setting (per-process, not per-attr).
 */
static int g_default_inheritsched = PTHREAD_INHERIT_SCHED;

/**
 * @brief Set thread scheduling inheritance (V2.0 ARM64 stub)
 */
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched)
{
    if (attr == NULL) {
        return EINVAL;
    }

    if (inheritsched != PTHREAD_INHERIT_SCHED &&
        inheritsched != PTHREAD_EXPLICIT_SCHED) {
        return EINVAL;
    }

    /* Store globally since attr lacks this field */
    g_default_inheritsched = inheritsched;
    (void)attr;

    return 0;
}

/**
 * @brief Get thread scheduling inheritance (V2.0 ARM64 stub)
 */
int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inheritsched)
{
    if (attr == NULL || inheritsched == NULL) {
        return EINVAL;
    }

    *inheritsched = g_default_inheritsched;

    return 0;
}

#else
/*
 * OneOS V1.x ARM32: pthread_attr_t has inheritsched member.
 * Use the original implementation.
 */

/**
 * @brief Set thread scheduling inheritance
 *
 * @param attr Thread attributes structure
 * @param inheritsched Either PTHREAD_INHERIT_SCHED or PTHREAD_EXPLICIT_SCHED
 * @return 0 on success, error code on failure
 *
 * When set to PTHREAD_INHERIT_SCHED, the thread inherits scheduling
 * attributes from the creating thread. When set to PTHREAD_EXPLICIT_SCHED,
 * attributes are taken from the attr structure.
 */
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched)
{
    if (attr == NULL) {
        return EINVAL;
    }

    if (inheritsched != PTHREAD_INHERIT_SCHED &&
        inheritsched != PTHREAD_EXPLICIT_SCHED) {
        return EINVAL;
    }

    attr->inheritsched = inheritsched;

    return 0;
}

/**
 * @brief Get thread scheduling inheritance
 *
 * @param attr Thread attributes structure
 * @param inheritsched Pointer to store the result
 * @return 0 on success, error code on failure
 */
int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inheritsched)
{
    if (attr == NULL || inheritsched == NULL) {
        return EINVAL;
    }

    *inheritsched = attr->inheritsched;

    return 0;
}
#endif /* ONEOS_V2_ARM64 */
