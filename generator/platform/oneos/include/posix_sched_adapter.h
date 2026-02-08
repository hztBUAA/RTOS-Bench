/**
 * @file posix_sched_adapter.h
 * @brief POSIX scheduling functions adapter for OneOS
 *
 * Provides pthread_attr_setinheritsched() and pthread_attr_getinheritsched()
 * declarations that are missing from OneOS's POSIX layer.
 */

#ifndef POSIX_SCHED_ADAPTER_H
#define POSIX_SCHED_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration only - actual pthread_attr_t is defined in pthread.h */
struct pthread_attr;
typedef struct pthread_attr pthread_attr_t;

/**
 * @brief Set thread scheduling inheritance
 * @param attr Thread attributes structure
 * @param inheritsched Either PTHREAD_INHERIT_SCHED or PTHREAD_EXPLICIT_SCHED
 * @return 0 on success, error code on failure
 */
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched);

/**
 * @brief Get thread scheduling inheritance
 * @param attr Thread attributes structure
 * @param inheritsched Pointer to store the result
 * @return 0 on success, error code on failure
 */
int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inheritsched);

#ifdef __cplusplus
}
#endif

#endif /* POSIX_SCHED_ADAPTER_H */
