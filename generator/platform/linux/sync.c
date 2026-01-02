/**
 * @file sync.c
 * @brief Linux synchronization primitives for rt-bench platform abstraction.
 */

#include "platform_abstraction.h"

#ifdef RTBENCH_PLATFORM_LINUX

#include <stdlib.h>
#include <semaphore.h>

struct rtbench_sem_internal {
	sem_t sem;
};

rtbench_sem_t rtbench_sem_create(unsigned int initial_value)
{
	struct rtbench_sem_internal *sem = NULL;
	int res;

	sem = malloc(sizeof(*sem));
	if (sem == NULL) {
		return NULL;
	}

	res = sem_init(&sem->sem, 0, initial_value);
	if (res < 0) {
		free(sem);
		return NULL;
	}

	return (rtbench_sem_t)sem;
}

int rtbench_sem_wait(rtbench_sem_t sem)
{
	struct rtbench_sem_internal *s = (struct rtbench_sem_internal *)sem;

	if (s == NULL) {
		return -1;
	}

	return (sem_wait(&s->sem) == 0) ? 0 : -1;
}

int rtbench_sem_post(rtbench_sem_t sem)
{
	struct rtbench_sem_internal *s = (struct rtbench_sem_internal *)sem;

	if (s == NULL) {
		return -1;
	}

	return (sem_post(&s->sem) == 0) ? 0 : -1;
}

int rtbench_sem_destroy(rtbench_sem_t sem)
{
	struct rtbench_sem_internal *s = (struct rtbench_sem_internal *)sem;
	int res;

	if (s == NULL) {
		return -1;
	}

	res = sem_destroy(&s->sem);
	free(s);
	return (res == 0) ? 0 : -1;
}

#endif /* RTBENCH_PLATFORM_LINUX */
