/* cpu_affinity.h
 * 
 */

#ifndef __CPU_AFFINITY_H__
#define __CPU_AFFINITY_H__

#include "platform_macro.h"
#include "safe_sleep.h"

/* “设置线程亲和度”宏 */
#define BIND_THREAD_TO_CPU(cpu_id) do { \
    cpu_set_t cpuset; \
    CPU_ZERO(&cpuset); \
    CPU_SET((cpu_id), &cpuset); \
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset); \
    safe_usleep(10); \
} while(0)

/* 使用核心数。注意：请勿修改此数。此数设置为2，表示多核测试部分均只使用两个核心进行测试，*/
#define USE_PROCESSORS 2  

int bench_get_cpu(void);



// pthread_setaffinity_np API
// RT-Thread
#if defined(RT_THREAD_PLATFORM)

#include <pthread.h>
typedef unsigned long cpu_set_t;
#define CPU_ZERO(cpusetp)		(*(cpusetp) = 0)
#define CPU_SET(cpu, cpusetp)	(*(cpusetp) |= (1UL << (cpu)))
#define CPU_ISSET(cpu, cpusetp)	(*(cpusetp) & (1UL << (cpu)))
int pthread_setaffinity_np(pthread_t thread, size_t cpusetsize, const cpu_set_t *cpuset);

// SylixOS 翼辉
#elif defined(SYLIXOS_PLATFORM)

#include <pthread_np.h> // 待确认

// OneOS 中移
#elif defined(ONEOS_PLATFORM)

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sched.h>
#include <pthread.h>

// ReWorks 锐华
#elif defined(RUIHUA_PLATFORM)
#include <pthread.h>
#include <cpuset.h>
typedef cpuset_t cpu_set_t;
#define CPU_ZERO(p_cupset) CPUSET_ZERO(*p_cupset)
#define CPU_SET(n, p_cupset) CPUSET_SET(*p_cupset, n)
int pthread_setaffinity_np(pthread_t thread, size_t cpusetsize, const cpu_set_t *cpuset);

// Intewell 东土
#elif defined(DONGTU_PLATFORM)

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sched.h>
#include <pthread.h>

#endif

#endif /* __CPU_AFFINITY.H__ */
