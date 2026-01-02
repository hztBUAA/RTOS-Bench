/**
 * @file sync.c
 * @brief RT-Thread synchronization primitives for rt-bench
 */

#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_RTTHREAD

#include <rtthread.h>
#include <stdlib.h>

struct rtbench_sem_internal {
	rt_sem_t sem;
};

rtbench_sem_t rtbench_sem_create(unsigned int initial_value)
{
	struct rtbench_sem_internal *sem;
	char sem_name[RT_NAME_MAX];
	static int sem_counter = 0;

	sem = (struct rtbench_sem_internal *)rt_malloc(sizeof(*sem));
	if (sem == NULL) {
		return NULL;
	}

	rt_snprintf(sem_name, RT_NAME_MAX, "rtbench_s%d", sem_counter++);
	sem->sem = rt_sem_create(sem_name, initial_value, RT_IPC_FLAG_FIFO);
	if (sem->sem == RT_NULL) {
		rt_free(sem);
		return NULL;
	}

	return (rtbench_sem_t)sem;
}

int rtbench_sem_wait(rtbench_sem_t sem)
{
	struct rtbench_sem_internal *s = (struct rtbench_sem_internal *)sem;

	if (s == NULL || s->sem == RT_NULL) {
		return -1;
	}

	return (rt_sem_take(s->sem, RT_WAITING_FOREVER) == RT_EOK) ? 0 : -1;
}

int rtbench_sem_post(rtbench_sem_t sem)
{
	struct rtbench_sem_internal *s = (struct rtbench_sem_internal *)sem;

	if (s == NULL || s->sem == RT_NULL) {
		return -1;
	}

	return (rt_sem_release(s->sem) == RT_EOK) ? 0 : -1;
}

int rtbench_sem_destroy(rtbench_sem_t sem)
{
	struct rtbench_sem_internal *s = (struct rtbench_sem_internal *)sem;

	if (s == NULL) {
		return -1;
	}

	if (s->sem != RT_NULL) {
		rt_sem_delete(s->sem);
		s->sem = RT_NULL;
	}

	rt_free(s);
	return 0;
}

#endif /* RTBENCH_PLATFORM_RTTHREAD */
