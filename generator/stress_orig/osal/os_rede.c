/* osal/os_rede.c
 * ReWorks OSAL implementation for rtosbench/stress.
 *
 * This file intentionally uses ReWorks-native pthread extension APIs instead
 * of the SylixOS-style pthread_attr path:
 *   - pthread_create2(): create task with ReWorks name/priority/options/stack
 *   - pthread_cancelforce(): force task cancellation when OSAL delete is asked
 *   - pthread_verifyid(): wait until a cancelled detached task has disappeared
 *   - pthread_delay()/tick_get()/sys_clk_rate_get(): ReWorks tick-based timing
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <limits.h>

#include <pthread.h>
#include <semaphore.h>
#include <clock.h>

#include "stress_osal.h"

/* Some ReWorks headers provide these via <pthread.h>/<reworks/thread.h> and
 * <reworks/types.h>. Keep local fallbacks so this file is robust against
 * trimmed BSP headers.
 */
#ifndef RE_FP_TASK
#define RE_FP_TASK          0x0008
#endif
#ifndef RE_NO_TIMESLICE
#define RE_NO_TIMESLICE     0x1000
#endif
#ifndef PTHREAD_PRI_MIN
#define PTHREAD_PRI_MIN     0
#endif
#ifndef PTHREAD_PRI_MAX
#define PTHREAD_PRI_MAX     255
#endif
#ifndef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN   4096
#endif
#ifndef WAIT
#define WAIT                1
#endif
#ifndef NO_WAIT
#define NO_WAIT             0
#endif
#ifndef NO_TIMEOUT
#define NO_TIMEOUT          0
#endif

#ifndef STRESS_OSAL_REDE_DEFAULT_STACK
#define STRESS_OSAL_REDE_DEFAULT_STACK  (16U * 1024U)
#endif

#ifndef STRESS_OSAL_REDE_DELETE_WAIT_MS
#define STRESS_OSAL_REDE_DELETE_WAIT_MS 1000U
#endif

/* ReWorks provides sem_wait2() in semaphore.h. If an older header lacks the
 * prototype, this compatible declaration prevents implicit-int builds.
 */
extern int sem_wait2(sem_t *sem, int is_wait, unsigned int timeout);

/* =========================================================================
 * 1. System and logging
 * ========================================================================= */

void stress_osal_print(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fflush(stdout);
}

int stress_osal_snprintf(char *str, size_t size, const char *format, ...)
{
    va_list args;
    int rc;

    va_start(args, format);
    rc = vsnprintf(str, size, format, args);
    va_end(args);
    return rc;
}

/* =========================================================================
 * 2. Memory management
 * ========================================================================= */

void *stress_osal_malloc(size_t size) { return malloc(size); }
void stress_osal_free(void *ptr) { free(ptr); }
void *stress_osal_realloc(void *ptr, size_t size) { return realloc(ptr, size); }
void *stress_osal_calloc(size_t count, size_t size) { return calloc(count, size); }
char *stress_osal_strdup(const char *str) { return str ? strdup(str) : NULL; }

/* =========================================================================
 * 3. Thread management, ReWorks native pthread extension implementation
 * ========================================================================= */

typedef struct stress_rede_thread_wrapper {
    stress_entry_t entry;
    void *parameter;
} stress_rede_thread_wrapper_t;

static void stress_rede_thread_wrapper_free(void *arg)
{
    free(arg);
}

static void *stress_rede_thread_entry(void *arg)
{
    stress_rede_thread_wrapper_t *wrapper = (stress_rede_thread_wrapper_t *)arg;

    pthread_cleanup_push(stress_rede_thread_wrapper_free, wrapper);

    /* Detach as early as possible. pthread_create2() creates joinable tasks,
     * while the stress framework waits through semaphores rather than join().
     * The parent also calls pthread_detach() right after create to cover the
     * case where this new task is created but not scheduled immediately.
     */
    (void)pthread_detach(pthread_self());

    /* ReWorks defaults to cancellation enabled + deferred. For stress cleanup
     * paths we want a delete request to take effect even if the worker is in a
     * CPU loop and does not hit a cancellation point. pthread_cancelforce()
     * also ignores state/type, but setting this makes ordinary cancellation
     * semantics sane as well.
     */
    (void)pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    (void)pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    if (wrapper && wrapper->entry) {
        wrapper->entry(wrapper->parameter);
    }

    pthread_cleanup_pop(1);
    return NULL;
}

static uint32_t stress_rede_normalize_stack(uint32_t stack_size)
{
    if (stack_size == 0U) {
        stack_size = STRESS_OSAL_REDE_DEFAULT_STACK;
    }

    if (stack_size < (uint32_t)PTHREAD_STACK_MIN) {
        stack_size = (uint32_t)PTHREAD_STACK_MIN;
    }

    /* pthread_create2() documents that stacksize should be even. */
    if (stack_size & 1U) {
        stack_size++;
    }

    return stack_size;
}

static int stress_rede_normalize_priority(uint8_t priority)
{
    int prio = (int)priority;

    if (prio < PTHREAD_PRI_MIN) {
        prio = PTHREAD_PRI_MIN;
    }
    if (prio > PTHREAD_PRI_MAX) {
        prio = PTHREAD_PRI_MAX;
    }

    return prio;
}

stress_tid_t stress_osal_thread_spawn(const char *name,
                                      stress_entry_t entry,
                                      void *parameter,
                                      uint32_t stack_size,
                                      uint8_t priority)
{
    pthread_t tid;
    stress_rede_thread_wrapper_t *wrapper;
    char thread_name[STRESS_OSAL_NAME_MAX];
    int ret;
    int prio;
    int options;

    if (!entry) {
        errno = EINVAL;
        return STRESS_INVALID_TID;
    }

    wrapper = (stress_rede_thread_wrapper_t *)malloc(sizeof(*wrapper));
    if (!wrapper) {
        return STRESS_INVALID_TID;
    }

    wrapper->entry = entry;
    wrapper->parameter = parameter;

    if (name && name[0]) {
        stress_osal_snprintf(thread_name, sizeof(thread_name), "%s", name);
    } else {
        stress_osal_snprintf(thread_name, sizeof(thread_name), "rtstress");
    }

    stack_size = stress_rede_normalize_stack(stack_size);
    prio = stress_rede_normalize_priority(priority);

    /* stressors use floating point heavily, so request FPU context save/restore.
     * Do not set RE_NO_TIMESLICE: the stress workload relies on same-priority
     * tasks sharing CPU fairly.
     */
    options = RE_FP_TASK;

    ret = pthread_create2(&tid,
                          thread_name,
                          prio,
                          options,
                          (int)stack_size,
                          stress_rede_thread_entry,
                          wrapper);
    if (ret != 0) {
        stress_osal_print("os_rede: pthread_create2('%s') failed ret=%d errno=%d prio=%d stack=%u\n",
                          thread_name, ret, errno, prio, (unsigned int)stack_size);
        free(wrapper);
        return STRESS_INVALID_TID;
    }

    /* The stress framework waits for completion through semaphores, not join().
     * Therefore make normal-returning workers detached so ReWorks can reclaim
     * their task resources automatically. Forced delete uses pthread_cancelforce()
     * and then polls pthread_verifyid() until the detached task disappears.
     */
    ret = pthread_detach(tid);
    if (ret != 0) {
        /* A fast-starting worker may already have detached itself. That is not
         * an error for this OSAL; the important property is that normal worker
         * termination is not left joinable.
         */
        (void)ret;
    }

    return (stress_tid_t)tid;
}

stress_tid_t stress_osal_thread_self(void)
{
    return (stress_tid_t)pthread_self();
}

void stress_osal_thread_yield(void)
{
    (void)pthread_delay(0);
}

void stress_osal_thread_delete(stress_tid_t tid)
{
    pthread_t pthread;
    int ret;

    if (tid == STRESS_INVALID_TID) {
        return;
    }

    pthread = (pthread_t)tid;

    if ((stress_tid_t)pthread_self() == tid) {
        pthread_exit(PTHREAD_CANCELED);
        return;
    }

    if (pthread_verifyid(pthread) != 0) {
        return;
    }

    /* ReWorks pthread_cancel() is deferred and depends on the target task's
     * cancel state/type. OSAL delete semantics are closer to forced task delete,
     * so use the documented ReWorks extension.
     */
    ret = pthread_cancelforce(pthread);
    if (ret != 0) {
        if (pthread_verifyid(pthread) == 0) {
            stress_osal_print("os_rede: pthread_cancelforce(%p) failed ret=%d errno=%d\n",
                              tid, ret, errno);
        }
        return;
    }

    {
        stress_tick_t hz = (stress_tick_t)stress_osal_tick_hz();
        stress_tick_t wait_ticks = (hz * (stress_tick_t)STRESS_OSAL_REDE_DELETE_WAIT_MS + 999U) / 1000U;
        stress_tick_t deadline = stress_osal_tick_get() + (wait_ticks ? wait_ticks : 1U);

        while (stress_osal_tick_get() < deadline) {
            if (pthread_verifyid(pthread) != 0) {
                return;
            }
            (void)pthread_delay(1);
        }
    }

    stress_osal_print("os_rede: warning: task %p still exists after pthread_cancelforce()\n", tid);
}

/* =========================================================================
 * 4. Synchronization, ReWorks POSIX semaphore implementation
 * ========================================================================= */

stress_sem_t stress_osal_sem_create(const char *name, int initial_value)
{
    sem_t *sem;

    (void)name;

    sem = (sem_t *)malloc(sizeof(*sem));
    if (!sem) {
        return NULL;
    }

    if (sem_init(sem, 0, (unsigned int)initial_value) != 0) {
        stress_osal_print("os_rede: sem_init failed errno=%d\n", errno);
        free(sem);
        return NULL;
    }

    return (stress_sem_t)sem;
}

void stress_osal_sem_delete(stress_sem_t sem)
{
    if (sem) {
        (void)sem_destroy((sem_t *)sem);
        free((void *)sem);
    }
}

int stress_osal_sem_take(stress_sem_t sem, stress_tick_t timeout_ticks)
{
    sem_t *psem = (sem_t *)sem;
    unsigned int timeout;
    int ret;

    if (!psem) {
        errno = EINVAL;
        return -1;
    }

    if (timeout_ticks == STRESS_WAIT_FOREVER) {
        ret = sem_wait2(psem, WAIT, NO_TIMEOUT);
    } else if (timeout_ticks == STRESS_WAIT_NO_WAIT) {
        ret = sem_wait2(psem, NO_WAIT, NO_TIMEOUT);
    } else {
        timeout = (timeout_ticks > (stress_tick_t)UINT_MAX) ? UINT_MAX : (unsigned int)timeout_ticks;
        ret = sem_wait2(psem, WAIT, timeout);
    }

    return (ret == 0) ? 0 : -1;
}

void stress_osal_sem_release(stress_sem_t sem)
{
    if (sem) {
        (void)sem_post((sem_t *)sem);
    }
}

/* =========================================================================
 * 5. Time and clocks
 * ========================================================================= */

stress_tick_t stress_osal_tick_get(void)
{
    return (stress_tick_t)tick_get();
}

uint32_t stress_osal_tick_hz(void)
{
    uint32_t hz = (uint32_t)sys_clk_rate_get();
    return hz ? hz : 1000U;
}

void stress_osal_sleep_ms(uint32_t ms)
{
    uint32_t hz = stress_osal_tick_hz();
    uint64_t ticks64;
    int ticks;

    if (ms == 0U) {
        (void)pthread_delay(0);
        return;
    }

    ticks64 = ((uint64_t)ms * (uint64_t)hz + 999ULL) / 1000ULL;
    if (ticks64 == 0ULL) {
        ticks64 = 1ULL;
    }
    if (ticks64 > (uint64_t)INT_MAX) {
        ticks64 = (uint64_t)INT_MAX;
    }

    ticks = (int)ticks64;
    (void)pthread_delay(ticks);
}

double stress_osal_time_now(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
    }

    return (double)stress_osal_tick_get() / (double)stress_osal_tick_hz();
}

/* =========================================================================
 * 6. Sorting and algorithms
 * ========================================================================= */

void stress_osal_qsort(void *base, size_t nmemb, size_t size,
                       int (*compar)(const void *, const void *))
{
    qsort(base, nmemb, size, compar);
}

/* =========================================================================
 * 7. File system
 * ========================================================================= */

int stress_osal_mkdir(const char *path, int mode) { return mkdir(path, (mode_t)mode); }
int stress_osal_rmdir(const char *path) { return rmdir(path); }
int stress_osal_open(const char *path, int flags, int mode) { return open(path, flags, mode); }
int stress_osal_close(int fd) { return close(fd); }
int stress_osal_read(int fd, void *buf, size_t count) { return (int)read(fd, buf, count); }
int stress_osal_write(int fd, const void *buf, size_t count) { return (int)write(fd, buf, count); }
int stress_osal_rename(const char *oldpath, const char *newpath) { return rename(oldpath, newpath); }
int stress_osal_unlink(const char *path) { return unlink(path); }
int stress_osal_fsync(int fd) { return fsync(fd); }
int stress_osal_ftruncate(int fd, off_t length) { return ftruncate(fd, length); }

int stress_osal_stat(const char *path, struct stat *buf)
{
    if (!path || !buf) {
        errno = EINVAL;
        return -1;
    }
    return stat(path, buf);
}

int stress_osal_fstat(int fd, struct stat *buf)
{
    if (fd < 0 || !buf) {
        errno = EINVAL;
        return -1;
    }
    return fstat(fd, buf);
}

int stress_osal_pipe(int fd[2])
{
    if (fd) {
        fd[0] = -1;
        fd[1] = -1;
    }
    errno = ENOSYS;
    return -1;
}

/* =========================================================================
 * 8. String and character operations
 * ========================================================================= */

int stress_osal_strcmp(const char *s1, const char *s2)
{
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;

    if (p1 == p2) return 0;
    if (!p1) return -1;
    if (!p2) return 1;

    while (*p1 && *p1 == *p2) {
        p1++;
        p2++;
    }
    return (int)*p1 - (int)*p2;
}

int stress_osal_strncmp(const char *s1, const char *s2, size_t n)
{
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;

    if (p1 == p2 || n == 0) return 0;
    if (!p1) return -1;
    if (!p2) return 1;

    while (n > 0 && *p1 && *p1 == *p2) {
        p1++;
        p2++;
        n--;
    }
    if (n == 0) return 0;
    return (int)*p1 - (int)*p2;
}

size_t stress_osal_strlen(const char *s) { return strlen(s); }
char *stress_osal_strcpy(char *dest, const char *src) { return strcpy(dest, src); }
char *stress_osal_strcat(char *dest, const char *src) { return strcat(dest, src); }
char *stress_osal_strchr(const char *s, int c) { return strchr(s, c); }
int stress_osal_tolower(int c) { return tolower(c); }

/* =========================================================================
 * 9. Common algorithms and hardware abstraction
 * ========================================================================= */

void *stress_osal_memset(void *s, int c, size_t n) { return memset(s, c, n); }
void stress_osal_srand(unsigned int seed) { srand(seed); }
int stress_osal_rand(void) { return rand(); }

void stress_osal_mb(void)
{
    __sync_synchronize();
}

void stress_osal_cache_flush(void *addr, size_t len)
{
    (void)addr;
    (void)len;
    __sync_synchronize();
}

/* =========================================================================
 * 10. Math operations
 * ========================================================================= */

double stress_osal_cos(double x) { return cos(x); }
float stress_osal_cosf(float x) { return cosf(x); }
long double stress_osal_cosl(long double x) { return (long double)cos((double)x); }

double stress_osal_sin(double x) { return sin(x); }
float stress_osal_sinf(float x) { return sinf(x); }
long double stress_osal_sinl(long double x) { return (long double)sin((double)x); }

double stress_osal_tan(double x) { return tan(x); }
float stress_osal_tanf(float x) { return tanf(x); }
long double stress_osal_tanl(long double x) { return (long double)tan((double)x); }

double stress_osal_fabs(double x) { return fabs(x); }
long double stress_osal_fabsl(long double x) { return (long double)fabs((double)x); }
