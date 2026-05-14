#include "platform_macro.h"
#include "cpu_affinity.h"
#include <pthread.h>



/**
 * @brief Sets the CPU affinity for a specific thread.
 * * This function is part of the benchmark abstraction layer (Linux/GNU extension).
 * It restricts the specified @p thread to run only on the CPU cores defined in 
 * the @p cpuset mask.
 * * @note Implementation Detail: This abstraction specifically targets the 
 * lowest bit of the provided mask, effectively binding the thread to a 
 * single core and discarding higher-order bits.
 * * @param thread      The identifier of the thread whose affinity is to be set.
 * @param cpusetsize  The size (in bytes) of the buffer pointed to by @p cpuset.
 * @param cpuset      A pointer to the CPU set mask defining the target cores.
 * * @return 0 on success, or a non-zero error code on failure.
 */
// int pthread_setaffinity_np(pthread_t thread, size_t cpusetsize, const cpu_set_t *cpuset);

/**
 * @brief Retrieves the current CPU core ID executing the calling thread.
 * * This utility is designed to be called from within a running thread to 
 * verify its actual hardware placement during execution.
 * * @return The integer ID of the CPU core currently assigned to the thread.
 * Returns a negative value if the core ID cannot be determined.
 */
// int bench_get_cpu(void);



// Abstracion

// RT-Thread
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

int bench_get_cpu(void) {
    return rt_hw_cpu_id();
}

// SylixOS 翼辉
#elif defined(SYLIXOS_PLATFORM)
#include <SylixOS.h>

int bench_get_cpu(void) {
    // API
    return (int)API_CpuCurId();
}

// OneOS 中移
#elif defined(ONEOS_PLATFORM)
#include <os_cpu_id.h>

int bench_get_cpu(void) {
    // API
    os_cpu_id_get();
}

// ReWorks 锐华
#elif defined(RUIHUA_PLATFORM)

#include <pthread.h>
#include <cpuset.h>

int pthread_setaffinity_np(pthread_t thread, size_t cpusetsize, const cpu_set_t *cpusetp) {
	return pthread_affinity_set(thread, (cpuset_t)*cpusetp);
}

int bench_get_cpu(void) {
    // API
    return cpu_id_get();
}

// Intewell 东土
#elif defined(DONGTU_PLATFORM)

#include <pthread.h>

int bench_get_cpu(void) {
    // API
    return (int)VMK_CpuIDGet();
}


#endif