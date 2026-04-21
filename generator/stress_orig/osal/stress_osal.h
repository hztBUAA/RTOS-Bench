/* osal/stress_osal.h */
#ifndef __STRESS_OSAL_H__
#define __STRESS_OSAL_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>  /* for SEEK_SET, SEEK_CUR, SEEK_END */

/* =========================================================================
 * 1. 基础类型定义 (Types)
 * 使用 void* 作为句柄，隐藏具体 OS 的结构体实现细节
 * ========================================================================= */

typedef void *stress_tid_t;     /* 线程句柄 (Thread Handle) */
typedef void *stress_sem_t;     /* 信号量句柄 (Semaphore Handle) */
typedef uint64_t stress_tick_t; /* 系统 Tick 类型 */
typedef int stress_bool_t;

/* 布尔类型兼容 */
#ifndef STRESS_TRUE
#define STRESS_TRUE  1
#endif
#ifndef STRESS_FALSE
#define STRESS_FALSE 0
#endif

/* 常量定义 */
#define STRESS_WAIT_FOREVER  ((stress_tick_t)-1)
#define STRESS_WAIT_NO_WAIT  (0)
#define STRESS_INVALID_TID   (NULL)

/* 线程名称最大长度 */
#ifndef STRESS_OSAL_NAME_MAX
#define STRESS_OSAL_NAME_MAX 32
#endif

/* =========================================================================
 * 2. 系统与日志接口 (System & Logging)
 * ========================================================================= */

/* 格式化打印 (类似 rt_kprintf 或 printf) */
void stress_osal_print(const char *fmt, ...);

/* 格式化字符串 (类似 snprintf) */
int stress_osal_snprintf(char *str, size_t size, const char *format, ...);

/* =========================================================================
 * 3. 内存管理 (Memory Management)
 * ========================================================================= */

void *stress_osal_malloc(size_t size);
void stress_osal_free(void *ptr);
void *stress_osal_realloc(void *ptr, size_t size);
void *stress_osal_calloc(size_t count, size_t size);
char *stress_osal_strdup(const char *str);

/* =========================================================================
 * 4. 线程管理 (Thread Management)
 * ========================================================================= */

/* 线程入口函数原型 */
typedef void (*stress_entry_t)(void *parameter);

/*
 * 创建并启动线程
 * 注意：RT-Thread 分为 create 和 startup，POSIX 通常 create 即运行。
 * 为了统一，我们使用 "spawn" 语义：创建即就绪。
 */
stress_tid_t stress_osal_thread_spawn(const char *name,
                                      stress_entry_t entry,
                                      void *parameter,
                                      uint32_t stack_size,
                                      uint8_t priority);

/* 获取当前线程 ID */
stress_tid_t stress_osal_thread_self(void);

/* 让出 CPU (Yield) */
void stress_osal_thread_yield(void);
void stress_osal_thread_delete(stress_tid_t tid);

/* =========================================================================
 * 5. 同步机制 (Synchronization)
 * 目前主要用到信号量用于 Jobfile 同步
 * ========================================================================= */

/* 创建信号量 */
stress_sem_t stress_osal_sem_create(const char *name, int initial_value);

/* 删除信号量 */
void stress_osal_sem_delete(stress_sem_t sem);

/* 等待信号量 (返回 0 表示成功，-1 表示超时或错误) */
int stress_osal_sem_take(stress_sem_t sem, stress_tick_t timeout_ticks);

/* 释放信号量 */
void stress_osal_sem_release(stress_sem_t sem);

/* =========================================================================
 * 6. 时间与时钟 (Time & Clock)
 * 这是计算负载率和 Bogo Ops 速率的关键
 * ========================================================================= */

/* 获取当前系统 Tick */
stress_tick_t stress_osal_tick_get(void);

/* 获取系统 Tick 频率 (Hz) - 用于将 Tick 换算为秒 */
uint32_t stress_osal_tick_hz(void);

/* 毫秒级睡眠 */
void stress_osal_sleep_ms(uint32_t ms);

/* 获取高精度时间 (秒, 用于性能计算 (double float) */
double stress_osal_time_now(void);

/* =========================================================================
 * 7. 排序
 * ========================================================================= */

void stress_osal_qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));

/* =========================================================================
 * 8. 文件系统接口 (File System)
 * 封装标准 POSIX 接口，以便适配不同的文件系统后端
 * ========================================================================= */

int stress_osal_mkdir(const char *path, int mode);
int stress_osal_rmdir(const char *path);
int stress_osal_open(const char *path, int flags, int mode);
int stress_osal_close(int fd);
int stress_osal_read(int fd, void *buf, size_t count);
int stress_osal_write(int fd, const void *buf, size_t count);
int stress_osal_rename(const char *oldpath, const char *newpath);
int stress_osal_unlink(const char *path);
int stress_osal_fsync(int fd);
int stress_osal_ftruncate(int fd, off_t  length);
int stress_osal_stat(const char *path, struct stat *buf);
int stress_osal_fstat(int fd, struct stat *buf);
int stress_osal_pipe(int fd[2]);
/* =========================================================================
 * 9. 字符串与字符操作 (String & Char)
 * 封装标准库函数，便于在不同 libc 或裸机环境中移植
 * ========================================================================= */

int stress_osal_strcmp(const char *s1, const char *s2);
int stress_osal_strncmp(const char *s1, const char *s2, size_t n);
size_t stress_osal_strlen(const char *s);
char *stress_osal_strcpy(char *dest, const char *src);
char *stress_osal_strcat(char *dest, const char *src);
char *stress_osal_strchr(const char *s, int c);
int stress_osal_tolower(int c);

/* =========================================================================
 * 10. 常用算法与硬件抽象 (Algo & Hardware)
 * ========================================================================= */

/* 内存设置 (memset) */
void *stress_osal_memset(void *s, int c, size_t n);

/* 随机数接口 */
void stress_osal_srand(unsigned int seed);
int stress_osal_rand(void);

/* 内存屏障 (Compiler Barrier / Memory Barrier) */
void stress_osal_mb(void);

/* CPU Data Cache 清除/无效化 */
void stress_osal_cache_flush(void *addr, size_t len);

/* =========================================================================
 * 11. 数学运算接口 (Math)
 * 封装 standard math library，便于处理 long double 兼容性问题
 * ========================================================================= */

double stress_osal_cos(double x);
float stress_osal_cosf(float x);
long double stress_osal_cosl(long double x);

double stress_osal_sin(double x);
float stress_osal_sinf(float x);
long double stress_osal_sinl(long double x);

double stress_osal_tan(double x);
float stress_osal_tanf(float x);
long double stress_osal_tanl(long double x);

double stress_osal_fabs(double x);
long double stress_osal_fabsl(long double x);

#endif /* __STRESS_OSAL_H__ */
