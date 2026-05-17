/* osal/os_intewell.c */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <limits.h>

#include <pthread.h>
#include <semaphore.h>
#include <sched.h>

#include "stress_osal.h"

#ifndef STRESS_OSAL_DEFAULT_STACK_SIZE
#define STRESS_OSAL_DEFAULT_STACK_SIZE (16U * 1024U)
#endif

#ifndef STRESS_OSAL_TICK_HZ
#define STRESS_OSAL_TICK_HZ 1000U
#endif

typedef struct {
    stress_entry_t entry;
    void *parameter;
} stress_thread_wrapper_t;

static stress_tid_t stress_tid_from_pthread(pthread_t tid)
{
    return (stress_tid_t)(uintptr_t)tid;
}

static pthread_t stress_tid_to_pthread(stress_tid_t tid)
{
    return (pthread_t)(uintptr_t)tid;
}

static clockid_t stress_osal_monotonic_clock(void)
{
#ifdef CLOCK_MONOTONIC
    return CLOCK_MONOTONIC;
#else
    return CLOCK_REALTIME;
#endif
}

static int stress_osal_make_abs_timeout(stress_tick_t timeout_ticks,
                                        struct timespec *abs_ts)
{
    uint64_t add_ns;
    uint64_t total_ns;

    if (abs_ts == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (clock_gettime(CLOCK_REALTIME, abs_ts) != 0) {
        return -1;
    }

    add_ns = (uint64_t)timeout_ticks * (1000000000ULL / (uint64_t)STRESS_OSAL_TICK_HZ);
    total_ns = (uint64_t)abs_ts->tv_nsec + add_ns;

    abs_ts->tv_sec += (time_t)(total_ns / 1000000000ULL);
    abs_ts->tv_nsec = (long)(total_ns % 1000000000ULL);

    return 0;
}

static void *stress_posix_thread_entry(void *arg)
{
    stress_thread_wrapper_t *wrapper = (stress_thread_wrapper_t *)arg;

    (void)pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    (void)pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    if ((wrapper != NULL) && (wrapper->entry != NULL)) {
        wrapper->entry(wrapper->parameter);
    }

    free(wrapper);
    return NULL;
}

/* =========================================================================
 * 1. 缂侇垵宕电划鐑樼▔鎼淬垺锛夐煫鍥锋嫹
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
    int ret;
    va_start(args, format);
    ret = vsnprintf(str, size, format, args);
    va_end(args);
    return ret;
}

/* =========================================================================
 * 2. 闁告劕鎳庨悺銊х不閿涘嫭鍊�
 * ========================================================================= */

void *stress_osal_malloc(size_t size) { return malloc(size); }
void stress_osal_free(void *ptr) { free(ptr); }
void *stress_osal_realloc(void *ptr, size_t size) { return realloc(ptr, size); }
void *stress_osal_calloc(size_t count, size_t size) { return calloc(count, size); }
char *stress_osal_strdup(const char *str)
{
    if (str == NULL) {
        return NULL;
    }
    return strdup(str);
}

/* =========================================================================
 * 3. 缂佹崘娉曢埢鑲╃不閿涘嫭鍊�
 * ========================================================================= */

stress_tid_t stress_osal_thread_spawn(const char *name,
                                      stress_entry_t entry,
                                      void *parameter,
                                      uint32_t stack_size,
                                      uint8_t priority)
{
    pthread_t tid;
    pthread_attr_t attr;
    struct sched_param param;
    stress_thread_wrapper_t *wrapper;
    int ret;
    int policy = SCHED_RR;
    int min_prio;
    int max_prio;
    int req_prio;
    size_t final_stack;

    (void)name;

    if (entry == NULL) {
        errno = EINVAL;
        return STRESS_INVALID_TID;
    }

    wrapper = (stress_thread_wrapper_t *)malloc(sizeof(*wrapper));
    if (wrapper == NULL) {
        return STRESS_INVALID_TID;
    }

    wrapper->entry = entry;
    wrapper->parameter = parameter;

    ret = pthread_attr_init(&attr);
    if (ret != 0) {
        free(wrapper);
        errno = ret;
        return STRESS_INVALID_TID;
    }

    final_stack = (stack_size == 0U) ? (size_t)STRESS_OSAL_DEFAULT_STACK_SIZE
                                     : (size_t)stack_size;

#ifdef PTHREAD_STACK_MIN
    if (final_stack < (size_t)PTHREAD_STACK_MIN) {
        final_stack = (size_t)PTHREAD_STACK_MIN;
    }
#else
    if (final_stack < 4096U) {
        final_stack = 4096U;
    }
#endif

    (void)pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    (void)pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    (void)pthread_attr_setstacksize(&attr, final_stack);

    min_prio = sched_get_priority_min(policy);
    max_prio = sched_get_priority_max(policy);
    req_prio = (int)priority;

    if ((min_prio != -1) && (max_prio != -1)) {
        if (req_prio < min_prio) {
            req_prio = min_prio;
        }
        if (req_prio > max_prio) {
            req_prio = max_prio;
        }

        memset(&param, 0, sizeof(param));
        param.sched_priority = req_prio;

        ret = pthread_attr_setschedpolicy(&attr, policy);
        if (ret != 0) {
            fprintf(stderr, "os_intewell: setschedpolicy failed (%d), priority may not take effect\n", ret);
        }
        ret = pthread_attr_setschedparam(&attr, &param);
        if (ret != 0) {
            fprintf(stderr, "os_intewell: setschedparam failed (%d), priority may not take effect\n", ret);
        }
    }

    ret = pthread_create(&tid, &attr, stress_posix_thread_entry, wrapper);
    (void)pthread_attr_destroy(&attr);

    if (ret != 0) {
        free(wrapper);
        errno = ret;
        return STRESS_INVALID_TID;
    }

    return stress_tid_from_pthread(tid);
}

stress_tid_t stress_osal_thread_self(void)
{
    return stress_tid_from_pthread(pthread_self());
}

void stress_osal_thread_yield(void)
{
    (void)sched_yield();
}

void stress_osal_thread_delete(stress_tid_t tid)
{
    if (tid != STRESS_INVALID_TID) {
        (void)pthread_cancel(stress_tid_to_pthread(tid));
    }
}

/* =========================================================================
 * 4. 闁告艾鏈鐐哄嫉閸濆嫬鐓�
 * ========================================================================= */

stress_sem_t stress_osal_sem_create(const char *name, int initial_value)
{
    sem_t *sem;

    (void)name;

    if (initial_value < 0) {
        errno = EINVAL;
        return NULL;
    }

    sem = (sem_t *)malloc(sizeof(*sem));
    if (sem == NULL) {
        return NULL;
    }

    if (sem_init(sem, 0, (unsigned int)initial_value) != 0) {
        free(sem);
        return NULL;
    }

    return (stress_sem_t)sem;
}

void stress_osal_sem_delete(stress_sem_t sem)
{
    sem_t *psem = (sem_t *)sem;

    if (psem != NULL) {
        (void)sem_destroy(psem);
        free(psem);
    }
}

int stress_osal_sem_take(stress_sem_t sem, stress_tick_t timeout_ticks)
{
    sem_t *psem = (sem_t *)sem;
    int ret;

    if (psem == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (timeout_ticks == STRESS_WAIT_FOREVER) {
        do {
            ret = sem_wait(psem);
        } while ((ret != 0) && (errno == EINTR));
        return (ret == 0) ? 0 : -1;
    }

    if (timeout_ticks == STRESS_WAIT_NO_WAIT) {
        ret = sem_trywait(psem);
        return (ret == 0) ? 0 : -1;
    }

    {
        struct timespec abs_ts;

        if (stress_osal_make_abs_timeout(timeout_ticks, &abs_ts) != 0) {
            return -1;
        }

        do {
            ret = sem_timedwait(psem, &abs_ts);
        } while ((ret != 0) && (errno == EINTR));

        return (ret == 0) ? 0 : -1;
    }
}

void stress_osal_sem_release(stress_sem_t sem)
{
    sem_t *psem = (sem_t *)sem;

    if (psem != NULL) {
        (void)sem_post(psem);
    }
}

/* =========================================================================
 * 5. 闁哄啫鐖煎Λ鎸庣▔鎼淬垺顦ч梺鏂ゆ嫹
 * ========================================================================= */

stress_tick_t stress_osal_tick_get(void)
{
    struct timespec ts;
    clockid_t clk = stress_osal_monotonic_clock();

    if (clock_gettime(clk, &ts) != 0) {
        return (stress_tick_t)0;
    }

    return (stress_tick_t)((uint64_t)ts.tv_sec * (uint64_t)STRESS_OSAL_TICK_HZ +
                           (uint64_t)ts.tv_nsec * (uint64_t)STRESS_OSAL_TICK_HZ / 1000000000ULL);
}

uint32_t stress_osal_tick_hz(void)
{
    return STRESS_OSAL_TICK_HZ;
}

void stress_osal_sleep_ms(uint32_t ms)
{
    struct timespec req;
    struct timespec rem;

    req.tv_sec  = (time_t)(ms / 1000U);
    req.tv_nsec = (long)((ms % 1000U) * 1000000UL);

    while (nanosleep(&req, &rem) != 0) {
        if (errno != EINTR) {
            break;
        }
        req = rem;
    }
}

double stress_osal_time_now(void)
{
    struct timespec ts;
    clockid_t clk = stress_osal_monotonic_clock();

    if (clock_gettime(clk, &ts) != 0) {
        return 0.0;
    }

    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

/* =========================================================================
 * 6. 闁圭儤甯掔花锟�
 * ========================================================================= */

void stress_osal_qsort(void *base, size_t nmemb, size_t size,
                       int (*compar)(const void *, const void *))
{
    qsort(base, nmemb, size, compar);
}

/* =========================================================================
 * 7. 闁哄倸娲ｅ▎銏㈠寲閼姐倗鍩犻柟鎭掑劚瑜帮拷
 * ========================================================================= */

int stress_osal_mkdir(const char *path, int mode) { return mkdir(path, (mode_t)mode); }
int stress_osal_rmdir(const char *path) { return rmdir(path); }
int stress_osal_open(const char *path, int flags, int mode) { return open(path, flags, (mode_t)mode); }
int stress_osal_close(int fd) { return close(fd); }

int stress_osal_read(int fd, void *buf, size_t count)
{
    ssize_t rc = read(fd, buf, count);
    if (rc > (ssize_t)INT_MAX) {
        return INT_MAX;
    }
    return (int)rc;
}

int stress_osal_write(int fd, const void *buf, size_t count)
{
    ssize_t rc = write(fd, buf, count);
    if (rc > (ssize_t)INT_MAX) {
        return INT_MAX;
    }
    return (int)rc;
}

int stress_osal_rename(const char *oldpath, const char *newpath) { return rename(oldpath, newpath); }
int stress_osal_unlink(const char *path) { return unlink(path); }
int stress_osal_fsync(int fd) { return fsync(fd); }
int stress_osal_ftruncate(int fd, off_t length) { return ftruncate(fd, length); }

int stress_osal_stat(const char *path, struct stat *buf)
{
    if ((path == NULL) || (buf == NULL)) {
        errno = EINVAL;
        return -1;
    }
    return stat(path, buf);
}

int stress_osal_fstat(int fd, struct stat *buf)
{
    if ((fd < 0) || (buf == NULL)) {
        errno = EINVAL;
        return -1;
    }
    return fstat(fd, buf);
}
int stress_osal_pipe(int fd[2])
{
    fd[0] = -1;
    fd[1] = -1;
    errno = ENOSYS;   /* "Function not implemented" */
    return -1;
}
/* =========================================================================
 * 8. 閻庢稒顨堥浣圭▔闊厾鐟㈤悗娑欘殘椤戜線骞欏鍕▕
 * ========================================================================= */

int stress_osal_strcmp(const char *s1, const char *s2) { return strcmp(s1, s2); }
int stress_osal_strncmp(const char *s1, const char *s2, size_t n) { return strncmp(s1, s2, n); }
size_t stress_osal_strlen(const char *s) { return strlen(s); }
char *stress_osal_strcpy(char *dest, const char *src) { return strcpy(dest, src); }
char *stress_osal_strcat(char *dest, const char *src) { return strcat(dest, src); }
char *stress_osal_strchr(const char *s, int c) { return strchr(s, c); }
int stress_osal_tolower(int c) { return tolower(c); }

/* =========================================================================
 * 9. 閻㈩垰鎽滈弫銈囩不濡や胶銆婂☉鎾虫捣閳ユ牗绂掗懜闈涒枙閻犵儑鎷�
 * ========================================================================= */

void *stress_osal_memset(void *s, int c, size_t n) { return memset(s, c, n); }
void stress_osal_srand(unsigned int seed) { srand(seed); }
int stress_osal_rand(void) { return rand(); }

void stress_osal_mb(void)
{
#if defined(__GNUC__)
    __sync_synchronize();
#else
    asm volatile("" ::: "memory");
#endif
}

void stress_osal_cache_flush(void *addr, size_t len)
{
    (void)addr;
    (void)len;
    stress_osal_mb();
}

/* =========================================================================
 * 10. 闁轰焦婢橀鐔告交閹邦喚鏆柟鎭掑劚瑜帮拷
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
