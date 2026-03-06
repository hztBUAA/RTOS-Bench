#include <pthread.h>
#include "cpu_set.h"


// rt-thread上的pthread_setaffinity_np示例实现
#if defined(RT_THREAD_PLATFORM)

#include <rtthread.h>
#include "pthread_internal.h"

int pthread_setaffinity_np(pthread_t thread, size_t cpusetsize, const cpu_set_t *cpuset)
{
    _pthread_data_t *ptd;

    if (cpuset == NULL) return -1;
    ptd = _pthread_get_data(thread);
    if (ptd == NULL || ptd->tid == RT_NULL)
    {
        rt_kprintf("Error: Invalid pthread_t %ld or thread already terminated.\n", (long)thread);
        return -1;
    }
    rt_thread_t rt_handle = ptd->tid;

    for (int i = 0; i < sizeof(cpu_set_t) * 8; i++) {
        if (CPU_ISSET(i, cpuset)) {
            rt_err_t result = rt_thread_control(rt_handle, RT_THREAD_CTRL_BIND_CPU, (void *)i);
            return (result == RT_EOK) ? 0 : -1;
        }
    }
    return 0;
}

#endif


// #else ...
// 其他操作系统上的实现
