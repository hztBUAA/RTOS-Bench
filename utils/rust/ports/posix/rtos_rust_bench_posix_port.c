#define _POSIX_C_SOURCE 200809L

#include "rtos_rust_bench.h"

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifndef RTOS_RUST_BENCH_POSIX_ENABLE_PRIORITY
#define RTOS_RUST_BENCH_POSIX_ENABLE_PRIORITY 0
#endif

struct rtos_bench_posix_thread_start {
    rtos_bench_thread_entry_t entry;
    void *arg;
};

struct rtos_bench_posix_sem {
    sem_t sem;
    pthread_mutex_t lock;
    uint32_t count;
    uint32_t max;
};

static void *rtos_bench_posix_thread_trampoline(void *arg)
{
    struct rtos_bench_posix_thread_start *start =
        (struct rtos_bench_posix_thread_start *)arg;
    rtos_bench_thread_entry_t entry = start->entry;
    void *entry_arg = start->arg;

    free(start);
    entry(entry_arg);
    return NULL;
}

static int rtos_bench_posix_wait_sem(sem_t *sem)
{
    int ret;

    do {
        ret = sem_wait(sem);
    } while (ret != 0 && errno == EINTR);

    return ret == 0 ? 0 : -1;
}

static int rtos_bench_posix_make_abs_time(uint64_t timeout_us, struct timespec *out)
{
    if (clock_gettime(CLOCK_REALTIME, out) != 0) {
        return -1;
    }

    uint64_t sec = timeout_us / 1000000ULL;
    uint64_t nsec = (timeout_us % 1000000ULL) * 1000ULL;

    out->tv_sec += (time_t)sec;
    out->tv_nsec += (long)nsec;
    if (out->tv_nsec >= 1000000000L) {
        out->tv_sec += 1;
        out->tv_nsec -= 1000000000L;
    }

    return 0;
}

int32_t rtos_bench_thread_spawn(
    rtos_bench_thread_entry_t entry,
    void *arg,
    uint32_t priority,
    uint32_t stack_bytes,
    const uint8_t *name,
    size_t name_len)
{
    (void)name;
    (void)name_len;

    if (entry == NULL) {
        return -1;
    }

    struct rtos_bench_posix_thread_start *start =
        (struct rtos_bench_posix_thread_start *)malloc(sizeof(*start));
    if (start == NULL) {
        return -1;
    }

    start->entry = entry;
    start->arg = arg;

    pthread_attr_t attr;
    if (pthread_attr_init(&attr) != 0) {
        free(start);
        return -1;
    }

    if (stack_bytes > 0U) {
        (void)pthread_attr_setstacksize(&attr, (size_t)stack_bytes);
    }

#if RTOS_RUST_BENCH_POSIX_ENABLE_PRIORITY
    struct sched_param sched_param;
    memset(&sched_param, 0, sizeof(sched_param));
    sched_param.sched_priority = (int)priority;
    (void)pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    (void)pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    (void)pthread_attr_setschedparam(&attr, &sched_param);
#else
    (void)priority;
#endif

    pthread_t thread;
    int ret = pthread_create(&thread, &attr, rtos_bench_posix_thread_trampoline, start);
    (void)pthread_attr_destroy(&attr);

    if (ret != 0) {
        free(start);
        return -1;
    }

    (void)pthread_detach(thread);
    return 0;
}

void rtos_bench_thread_exit(void)
{
    pthread_exit(NULL);
}

int32_t rtos_bench_sem_create(uint32_t initial, uint32_t max, void **out_sem)
{
    if (out_sem == NULL || max == 0U || initial > max) {
        return -1;
    }

    struct rtos_bench_posix_sem *wrapper =
        (struct rtos_bench_posix_sem *)malloc(sizeof(*wrapper));
    if (wrapper == NULL) {
        return -1;
    }

    if (sem_init(&wrapper->sem, 0, initial) != 0) {
        free(wrapper);
        return -1;
    }

    if (pthread_mutex_init(&wrapper->lock, NULL) != 0) {
        (void)sem_destroy(&wrapper->sem);
        free(wrapper);
        return -1;
    }

    wrapper->count = initial;
    wrapper->max = max;
    *out_sem = wrapper;
    return 0;
}

int32_t rtos_bench_sem_take(void *sem, uint64_t timeout_us)
{
    if (sem == NULL) {
        return -1;
    }

    struct rtos_bench_posix_sem *wrapper = (struct rtos_bench_posix_sem *)sem;
    int ret;

    if (timeout_us == UINT64_MAX) {
        ret = rtos_bench_posix_wait_sem(&wrapper->sem);
    } else {
        struct timespec abs_time;
        if (rtos_bench_posix_make_abs_time(timeout_us, &abs_time) != 0) {
            return -1;
        }

        do {
            ret = sem_timedwait(&wrapper->sem, &abs_time);
        } while (ret != 0 && errno == EINTR);

        ret = ret == 0 ? 0 : -1;
    }

    if (ret != 0) {
        return -1;
    }

    if (pthread_mutex_lock(&wrapper->lock) == 0) {
        if (wrapper->count > 0U) {
            wrapper->count -= 1U;
        }
        (void)pthread_mutex_unlock(&wrapper->lock);
    }

    return 0;
}

int32_t rtos_bench_sem_give(void *sem)
{
    if (sem == NULL) {
        return -1;
    }

    struct rtos_bench_posix_sem *wrapper = (struct rtos_bench_posix_sem *)sem;
    int should_post = 0;

    if (pthread_mutex_lock(&wrapper->lock) != 0) {
        return -1;
    }

    if (wrapper->count < wrapper->max) {
        wrapper->count += 1U;
        should_post = 1;
    }

    (void)pthread_mutex_unlock(&wrapper->lock);

    if (should_post && sem_post(&wrapper->sem) != 0) {
        return -1;
    }

    return 0;
}

void rtos_bench_sem_destroy(void *sem)
{
    if (sem == NULL) {
        return;
    }

    struct rtos_bench_posix_sem *wrapper = (struct rtos_bench_posix_sem *)sem;
    (void)pthread_mutex_destroy(&wrapper->lock);
    (void)sem_destroy(&wrapper->sem);
    free(wrapper);
}

uint64_t rtos_bench_now_us(void)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
        return 0;
    }

    return ((uint64_t)now.tv_sec * 1000000ULL) + ((uint64_t)now.tv_nsec / 1000ULL);
}

void rtos_bench_write(const uint8_t *ptr, size_t len)
{
    if (ptr == NULL || len == 0U) {
        return;
    }

    const uint8_t *cursor = ptr;
    size_t remaining = len;

    while (remaining > 0U) {
        ssize_t written = write(STDOUT_FILENO, cursor, remaining);
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            return;
        }

        cursor += (size_t)written;
        remaining -= (size_t)written;
    }
}

void *rtos_bench_alloc(size_t size)
{
    return malloc(size);
}

void rtos_bench_free(void *ptr)
{
    free(ptr);
}

void rtos_bench_yield(void)
{
    sched_yield();
}

void rtos_bench_busy_hint(void)
{
    sched_yield();
}
