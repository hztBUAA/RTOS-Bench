/* cpu_set.h
 * 
 */

#ifndef __CPU_SET_H__
#define __CPU_SET_H__

#include <pthread.h>



/* 使用核心数。注意：请勿修改此数。此数设置为2，表示多核测试部分均只使用两个核心进行测试，*/
#define USE_PROCESSORS 2  


// 示例实现
#ifdef __RT_THREAD_H__

typedef unsigned long cpu_set_t;

#define CPU_ZERO(cpusetp)		(*(cpusetp) = 0)
#define CPU_SET(cpu, cpusetp)	(*(cpusetp) |= (1UL << (cpu)))
#define CPU_ISSET(cpu, cpusetp)	(*(cpusetp) & (1UL << (cpu)))

#endif
//#else ...







int pthread_setaffinity_np(pthread_t thread, size_t cpusetsize, const cpu_set_t *cpuset);

#endif /* __CPU_SET.H__ */
