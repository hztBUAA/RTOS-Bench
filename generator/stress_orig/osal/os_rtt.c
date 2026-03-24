/* osal/os_rtt.c */

#include "stress_osal.h"
#include <rtthread.h>
#include <stdarg.h>
#include <stdio.h>

#include <stdlib.h>     /* qsort, rand, srand, malloc, free */
#include <string.h>     /* memset, strlen, strcpy, strcat, strcmp... */
#include <ctype.h>      /* tolower */
#include <math.h>

#ifdef RT_USING_POSIX
#include <unistd.h>     /* close, write, unlink, rmdir, fsync */
#include <fcntl.h>      /* open */
#include <sys/stat.h>   /* mkdir */
#else
/*
 * 如果没有开启 POSIX，为了通过编译，可能需要包含 DFS 特有头文件
 * 或者在 menuconfig 中开启 RT_USING_POSIX (推荐)
 *
 * 对于 qemu-virt64-aarch64 BSP，应该已经启用了 RT_USING_POSIX，
 * 如果没有则通过 DFS 的 POSIX 兼容层。
 */
#ifdef RT_USING_DFS
#include <dfs_file.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#else
/* 如果完全没有文件系统支持，提供空实现 */
static inline int mkdir(const char *p, int m) { (void)p; (void)m; return -1; }
static inline int rmdir(const char *p) { (void)p; return -1; }
static inline int open(const char *p, int f, int m) { (void)p; (void)f; (void)m; return -1; }
static inline int close(int fd) { (void)fd; return -1; }
static inline int write(int fd, const void *b, size_t c) { (void)fd; (void)b; (void)c; return -1; }
static inline int read(int fd, void *b, size_t c) { (void)fd; (void)b; (void)c; return -1; }
static inline int rename(const char *o, const char *n) { (void)o; (void)n; return -1; }
static inline int unlink(const char *p) { (void)p; return -1; }
static inline int fsync(int fd) { (void)fd; return -1; }
static inline int ftruncate(int fd, off_t length) { (void)fd; (void)length; return -1; }
#endif
#endif


/* =========================================================================
 * 1. 系统与日志
 * ========================================================================= */

void stress_osal_print(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    /* RT-Thread 的 kprintf 不支持 va_list，通常用 vsnprintf 格式化到 buffer */
    char log_buf[256];
    vsnprintf(log_buf, sizeof(log_buf), fmt, args);
    rt_kprintf("%s", log_buf);

    va_end(args);
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

void *stress_osal_malloc(size_t size)
{
    return rt_malloc(size);
}

void stress_osal_free(void *ptr)
{
    rt_free(ptr);
}

void *stress_osal_realloc(void *ptr, size_t size)
{
    return rt_realloc(ptr, size);
}

void *stress_osal_calloc(size_t count, size_t size)
{
    return rt_calloc(count, size);
}

char *stress_osal_strdup(const char *str)
{
    return rt_strdup(str);
}

/* =========================================================================
 * 3. 线程管理
 * ========================================================================= */

stress_tid_t stress_osal_thread_spawn(const char *name,
                                      stress_entry_t entry,
                                      void *parameter,
                                      uint32_t stack_size,
                                      uint8_t priority)
{
    rt_thread_t tid;

    /*
     * [安全修正] 栈大小保护
     * 1. 如果传入 0，给予默认值 16KB (RT-Thread 默认如果不配置可能非常小)
     * 2. 钳制最小值为 4096，防止爆栈
     */
    if (stack_size == 0) stack_size = 16 * 1024;
    if (stack_size < 4096) stack_size = 4096;

    /* RT-Thread 的优先级范围根据配置不同，这里直接透传 */
    tid = rt_thread_create(name,
                           (void (*)(void *))entry,
                           parameter,
                           stack_size,
                           priority,
                           10); /* 默认时间片 10 tick */

    if (tid != RT_NULL) {
        rt_thread_startup(tid);
    }

    return (stress_tid_t)tid;
}

stress_tid_t stress_osal_thread_self(void)
{
    return (stress_tid_t)rt_thread_self();
}

void stress_osal_thread_yield(void)
{
    rt_thread_yield();
}

void stress_osal_thread_delete(stress_tid_t tid)
{
    if (tid) {
        rt_thread_delete((rt_thread_t)tid);
    }
}
/* =========================================================================
 * 4. 同步机制
 * ========================================================================= */

stress_sem_t stress_osal_sem_create(const char *name, int initial_value)
{
    return (stress_sem_t)rt_sem_create(name, (rt_uint32_t)initial_value, RT_IPC_FLAG_FIFO);
}

void stress_osal_sem_delete(stress_sem_t sem)
{
    if (sem) {
        rt_sem_delete((rt_sem_t)sem);
    }
}

int stress_osal_sem_take(stress_sem_t sem, stress_tick_t timeout_ticks)
{
    rt_err_t result;
    rt_int32_t time = (rt_int32_t)timeout_ticks;

    /* 处理特殊等待时间常量 */
    if (timeout_ticks == STRESS_WAIT_FOREVER) {
        time = RT_WAITING_FOREVER;
    }

    result = rt_sem_take((rt_sem_t)sem, time);

    return (result == RT_EOK) ? 0 : -1;
}

void stress_osal_sem_release(stress_sem_t sem)
{
    rt_sem_release((rt_sem_t)sem);
}

/* =========================================================================
 * 5. 时间与时钟
 * ========================================================================= */

stress_tick_t stress_osal_tick_get(void)
{
    return (stress_tick_t)rt_tick_get();
}

uint32_t stress_osal_tick_hz(void)
{
    return RT_TICK_PER_SECOND;
}

void stress_osal_sleep_ms(uint32_t ms)
{
    rt_thread_mdelay(ms);
}

double stress_osal_time_now(void)
{
    /*
     * RTT 标准版没有高精度计时器接口(gettimeofday等依赖libc或posix)
     * 这里使用 tick 换算，精度为 1/RT_TICK_PER_SECOND 秒
     */
    return (double)rt_tick_get() / (double)RT_TICK_PER_SECOND;
}

/* =========================================================================
 * 7. 排序算法
 * ========================================================================= */

void stress_osal_qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
    qsort(base, nmemb, size, compar);
}

/* =========================================================================
 * 8. 文件系统接口
 * RT-Thread 通过 DFS 组件支持标准 POSIX 接口
 * ========================================================================= */

int stress_osal_mkdir(const char *path, int mode)
{
    return mkdir(path, (mode_t)mode);
}

int stress_osal_rmdir(const char *path)
{
    return rmdir(path);
}

int stress_osal_open(const char *path, int flags, int mode)
{
    return open(path, flags, mode);
}

int stress_osal_close(int fd)
{
    return close(fd);
}

int stress_osal_write(int fd, const void *buf, size_t count)
{
    return write(fd, buf, count);
}

int stress_osal_rename(const char *oldpath, const char *newpath)
{
    return rename(oldpath, newpath);
}

int stress_osal_unlink(const char *path)
{
    return unlink(path);
}

int stress_osal_fsync(int fd)
{
    return fsync(fd);
}
int stress_osal_read(int fd, void *buf, size_t count) {return read(fd, buf, count);}
int stress_osal_ftruncate(int fd, off_t  length) {return ftruncate(fd, length);}
int stress_osal_stat(const char *path, struct stat *buf)
{
#if defined(RT_USING_POSIX) || defined(RT_USING_DFS)
    if (!path || !buf) {
        return -1;
    }
    return stat(path, buf);
#else
    (void)path;
    (void)buf;
    return -1;
#endif
}

int stress_osal_fstat(int fd, struct stat *buf)
{
#if defined(RT_USING_POSIX) || defined(RT_USING_DFS)
    if (fd < 0 || !buf) {
        return -1;
    }
    return fstat(fd, buf);
#else
    (void)fd;
    (void)buf;
    return -1;
#endif
}

/* =========================================================================
 * 9. 字符串与字符操作
 * ========================================================================= */

int stress_osal_strcmp(const char *s1, const char *s2)
{
    return strcmp(s1, s2);
}

int stress_osal_strncmp(const char *s1, const char *s2, size_t n)
{
    return strncmp(s1, s2, n);
}

size_t stress_osal_strlen(const char *s)
{
    return strlen(s);
}

char *stress_osal_strcpy(char *dest, const char *src)
{
    return strcpy(dest, src);
}

char *stress_osal_strcat(char *dest, const char *src)
{
    return strcat(dest, src);
}

char *stress_osal_strchr(const char *s, int c)
{
    return strchr(s, c);
}

int stress_osal_tolower(int c)
{
    return tolower(c);
}

/* =========================================================================
 * 10. 常用算法与硬件抽象
 * ========================================================================= */

void *stress_osal_memset(void *s, int c, size_t n)
{
    return memset(s, c, n);
}

void stress_osal_srand(unsigned int seed)
{
    srand(seed);
}

int stress_osal_rand(void)
{
    return rand();
}

void stress_osal_mb(void)
{
    /*
     * [修复] 使用 GCC 内置函数生成真正的硬件内存屏障 (如 DMB ISH)
     * 原来的 "" ::: "memory" 只是编译器屏障
     */
    __sync_synchronize();
}

void stress_osal_cache_flush(void *addr, size_t len)
{
    (void)addr;
    (void)len;
    /*
     * [关键安全修正]
     * 移除 rt_hw_cpu_dcache_ops 调用。
     * 虽然 RTT 提供了内核接口，但在不同 BSP 移植或 Smart 模式下，
     * 这里的行为可能不一致或涉及特权指令。
     * 降级为 Memory Barrier 是最稳妥的跨平台压力测试方案。
     */
    __sync_synchronize();
}

/* =========================================================================
 * 11. 数学运算接口
 * ========================================================================= */

double stress_osal_cos(double x) { return cos(x); }
float stress_osal_cosf(float x) { return cosf(x); }
long double stress_osal_cosl(long double x) { return cosl(x); }

double stress_osal_sin(double x) { return sin(x); }
float stress_osal_sinf(float x) { return sinf(x); }
long double stress_osal_sinl(long double x) { return sinl(x); }

double stress_osal_tan(double x) { return tan(x); }
float stress_osal_tanf(float x) { return tanf(x); }
long double stress_osal_tanl(long double x) { return tanl(x); }

double stress_osal_fabs(double x) { return fabs(x); }
long double stress_osal_fabsl(long double x) { return fabsl(x); }
