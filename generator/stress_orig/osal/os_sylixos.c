/* osal/os_sylixos.c */
#define _GNU_SOURCE

#include <SylixOS.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>

/* 引入 POSIX 线程与同步头文件 */
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>

#include "stress_osal.h"

/* =========================================================================
 * 1. 系统与日志
 * ========================================================================= */

void stress_osal_print(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    /* 强制刷新缓冲，确保在 Shell 中能实时看到输出，防止崩溃时丢失日志 */
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
 * 2. 内存管理
 * ========================================================================= */

void *stress_osal_malloc(size_t size) { return malloc(size); }
void stress_osal_free(void *ptr) { free(ptr); }
void *stress_osal_realloc(void *ptr, size_t size) { return realloc(ptr, size); }
void *stress_osal_calloc(size_t count, size_t size) { return calloc(count, size); }
char *stress_osal_strdup(const char *str) { return strdup(str); }

/* =========================================================================
 * 3. 线程管理 (POSIX Implementation)
 * ========================================================================= */

/* 包装结构体，用于传递参数 */
typedef struct {
    stress_entry_t entry;
    void *parameter;
} sys_thread_wrapper_t;

/* POSIX 线程入口包装 */
static void *posix_thread_entry(void *arg)
{
    sys_thread_wrapper_t *wrapper = (sys_thread_wrapper_t *)arg;
    if (wrapper && wrapper->entry) {
        wrapper->entry(wrapper->parameter);
    }
    free(wrapper);
    return NULL;
}

stress_tid_t stress_osal_thread_spawn(const char *name,
                                      stress_entry_t entry,
                                      void *parameter,
                                      uint32_t stack_size,
                                      uint8_t priority)
{
    pthread_t tid;
    pthread_attr_t attr;
    struct sched_param param;
    sys_thread_wrapper_t *wrapper;
    int ret;

    wrapper = malloc(sizeof(sys_thread_wrapper_t));
    if (!wrapper) return STRESS_INVALID_TID;

    wrapper->entry = entry;
    wrapper->parameter = parameter;

    pthread_attr_init(&attr);

    /*
     * [安全修正] 栈大小处理
     * 如果传入 0，给与一个安全的默认值 (16KB)，防止使用极小的系统默认栈导致溢出。
     * 同时检查 PTHREAD_STACK_MIN。
     */
    if (stack_size == 0) stack_size = 16 * 1024;

#ifdef PTHREAD_STACK_MIN
    if (stack_size < PTHREAD_STACK_MIN) stack_size = PTHREAD_STACK_MIN;
#else
    if (stack_size < 4096) stack_size = 4096;
#endif

    pthread_attr_setstacksize(&attr, stack_size);

    /*
     * 设置优先级
     * SylixOS 调度策略通常使用 SCHED_RR 或 SCHED_FIFO 才能设置优先级
     */
    pthread_attr_setschedpolicy(&attr, SCHED_RR);

    /* 简单的优先级钳制，防止越界 */
    int min_prio = sched_get_priority_min(SCHED_RR);
    int max_prio = sched_get_priority_max(SCHED_RR);
    if (priority < min_prio) priority = min_prio;
    if (priority > max_prio) priority = max_prio;

    param.sched_priority = priority;
    pthread_attr_setschedparam(&attr, &param);

    /*
     * 设置分离属性 (Detached)
     * 防止 stress 测试线程退出后变成僵尸线程占用资源
     */
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&tid, &attr, posix_thread_entry, wrapper);

    pthread_attr_destroy(&attr);

    if (ret != 0) {
        stress_osal_print("os_sylixos: pthread_create failed, ret=%d\n", ret);
        free(wrapper);
        return STRESS_INVALID_TID;
    }

    /*
     * 尝试设置线程名 (SylixOS 支持 pthread_setname_np)
     */
#ifdef __SYLIXOS__
    pthread_setname_np(tid, name);
#endif

    return (stress_tid_t)tid;
}

stress_tid_t stress_osal_thread_self(void)
{
    return (stress_tid_t)pthread_self();
}

void stress_osal_thread_yield(void)
{
    sched_yield();
}

void stress_osal_thread_delete(stress_tid_t tid)
{
    /* POSIX 线程通常自我退出，强杀使用 pthread_cancel */
    if (tid != STRESS_INVALID_TID) {
        pthread_cancel((pthread_t)tid);
    }
}

/* =========================================================================
 * 4. 同步机制 (POSIX Semaphore)
 * ========================================================================= */

stress_sem_t stress_osal_sem_create(const char *name, int initial_value)
{
    sem_t *sem = (sem_t *)malloc(sizeof(sem_t));
    if (!sem) {
        stress_osal_print("os_sylixos: OOM allocating semaphore struct for '%s'\n", name);
        return NULL;
    }

    /*
     * 参数2 pshared=0: 表示在同一进程的线程间共享
     * 参数3 value: 初始值
     */
    if (sem_init(sem, 0, initial_value) != 0) {
        stress_osal_print("os_sylixos: sem_init failed for '%s', errno=%d\n", name, errno);
        free(sem);
        return NULL;
    }

    return (stress_sem_t)sem;
}

void stress_osal_sem_delete(stress_sem_t sem)
{
    if (sem) {
        /* 销毁内核资源并释放内存 */
        sem_destroy((sem_t *)sem);
        free((void *)sem);
    }
}

int stress_osal_sem_take(stress_sem_t sem, stress_tick_t timeout_ticks)
{
    int ret;
    sem_t *psem = (sem_t *)sem;

    if (timeout_ticks == STRESS_WAIT_FOREVER) {
        ret = sem_wait(psem);
    } else if (timeout_ticks == STRESS_WAIT_NO_WAIT) {
        ret = sem_trywait(psem);
    } else {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);

        /* 计算绝对时间 */
        uint64_t nsec = ts.tv_nsec + (uint64_t)timeout_ticks * (1000000000 / stress_osal_tick_hz());
        ts.tv_sec += nsec / 1000000000;
        ts.tv_nsec = nsec % 1000000000;

        ret = sem_timedwait(psem, &ts);
    }

    return (ret == 0) ? 0 : -1;
}

void stress_osal_sem_release(stress_sem_t sem)
{
    sem_post((sem_t *)sem);
}

/* =========================================================================
 * 5. 时间与时钟
 * ========================================================================= */

stress_tick_t stress_osal_tick_get(void)
{
    /* 使用 CLOCK_MONOTONIC 模拟 Tick */
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    /* 假定 Tick 频率为 1000Hz (1ms) */
    uint64_t ticks = (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    return (stress_tick_t)ticks;
}

uint32_t stress_osal_tick_hz(void)
{
    return 1000;
}

void stress_osal_sleep_ms(uint32_t ms)
{
    usleep(ms * 1000);
}

double stress_osal_time_now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

/* =========================================================================
 * 6. 排序与算法
 * ========================================================================= */

void stress_osal_qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
    qsort(base, nmemb, size, compar);
}

/* =========================================================================
 * 7. 文件系统接口 (POSIX)
 * ========================================================================= */

int stress_osal_mkdir(const char *path, int mode) { return mkdir(path, (mode_t)mode); }
int stress_osal_rmdir(const char *path) { return rmdir(path); }
int stress_osal_open(const char *path, int flags, int mode) { return open(path, flags, mode); }
int stress_osal_close(int fd) { return close(fd); }
int stress_osal_write(int fd, const void *buf, size_t count) { return write(fd, buf, count); }
int stress_osal_rename(const char *oldpath, const char *newpath) { return rename(oldpath, newpath); }
int stress_osal_unlink(const char *path) { return unlink(path); }
int stress_osal_fsync(int fd) { return fsync(fd); }
int stress_osal_read(int fd, void *buf, size_t count) {return read(fd, buf, count);}
int stress_osal_ftruncate(int fd, off_t  length) {return ftruncate(fd, length);}
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
/* =========================================================================
 * 8. 字符串与字符操作
 * ========================================================================= */

int stress_osal_strcmp(const char *s1, const char *s2) { return strcmp(s1, s2); }
int stress_osal_strncmp(const char *s1, const char *s2, size_t n) { return strncmp(s1, s2, n); }
size_t stress_osal_strlen(const char *s) { return strlen(s); }
char *stress_osal_strcpy(char *dest, const char *src) { return strcpy(dest, src); }
char *stress_osal_strcat(char *dest, const char *src) { return strcat(dest, src); }
char *stress_osal_strchr(const char *s, int c) { return strchr(s, c); }
int stress_osal_tolower(int c) { return tolower(c); }

/* =========================================================================
 * 9. 常用算法与硬件抽象
 * ========================================================================= */

void *stress_osal_memset(void *s, int c, size_t n) { return memset(s, c, n); }
void stress_osal_srand(unsigned int seed) { srand(seed); }
int stress_osal_rand(void) { return rand(); }

void stress_osal_mb(void)
{
    /* GCC 内置内存屏障 */
    __sync_synchronize();
}

void stress_osal_cache_flush(void *addr, size_t len)
{
    (void)addr;
    (void)len;
    __sync_synchronize();
}

/* =========================================================================
 * 10. 数学运算接口
 * ========================================================================= */
double stress_osal_cos(double x) { return cos(x); }
float stress_osal_cosf(float x) { return cosf(x); }

double stress_osal_sin(double x) { return sin(x); }
float stress_osal_sinf(float x) { return sinf(x); }

double stress_osal_tan(double x) { return tan(x); }
float stress_osal_tanf(float x) { return tanf(x); }

double stress_osal_fabs(double x) { return fabs(x); }

long double stress_osal_cosl(long double x) { return (long double)cos((double)x); }
long double stress_osal_sinl(long double x) { return (long double)sin((double)x); }
long double stress_osal_tanl(long double x) { return (long double)tan((double)x); }
long double stress_osal_fabsl(long double x) { return (long double)fabs((double)x); }


