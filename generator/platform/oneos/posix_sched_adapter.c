/**
 * @file posix_sched_adapter.c
 * @brief POSIX scheduling functions adapter for OneOS
 *
 * Provides implementations of pthread_attr_setinheritsched and
 * pthread_attr_getinheritsched that are missing from OneOS's POSIX layer.
 *
 * These functions work with OneOS's pthread_attr_t structure which already
 * has an inheritsched field.
 */

#include <pthread.h>
#include <errno.h>

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
