/* cpu_set.h
 * CPU affinity support for RT-Thread
 */

#ifndef __CPU_SET_H__
#define __CPU_SET_H__

#include <stddef.h>

/* 使用核心数 */
#define USE_PROCESSORS 2

/* cpu_set_t definition */
#ifndef cpu_set_t
typedef unsigned long cpu_set_t;
#endif

#define CPU_ZERO(cpusetp)       (*(cpusetp) = 0)
#define CPU_SET(cpu, cpusetp)   (*(cpusetp) |= (1UL << (cpu)))
#define CPU_ISSET(cpu, cpusetp) (*(cpusetp) & (1UL << (cpu)))

/* pthread_setaffinity_np - only when pthread is available */
#ifdef __RT_THREAD_H__
#include <pthread.h>
int pthread_setaffinity_np(pthread_t thread, size_t cpusetsize, const cpu_set_t *cpuset);
#endif

#endif /* __CPU_SET_H__ */
