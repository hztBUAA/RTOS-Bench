/*
 * File: benchmark/realtime/test4/test4_2.c
 * 场景: 信号量获取（挂起睡眠）&信号量释放（高优先级恢复）
 * 插桩：是
 */

#include <cpu_affinity.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>

#include "les.h"

static sem_t sem;

#define TEST_ITERATION 1000

static volatile uint64_t unlock_total_cycles, lock_total_cycles;
static volatile uint32_t unlock_valid, lock_valid;


static void *post_thread(void *parameter) {
	BIND_THREAD_TO_CPU(0);

	// start
	sem_post(&sem);

	for (int i = 0; i < TEST_ITERATION; i++) {
		sem_post(&sem);
    }

	// mid
	sem_post(&sem);

	uint64_t t0, t1 = 0, t2 = 0, t3;
	uint64_t tmp_cycles = 0;
	uint32_t valid = 0;

	// wait高优先级挂起睡眠
	for (int i = 0; i < TEST_ITERATION; i++) {
		
		LES_enable();
		t0 = timeGet();
		sem_post(&sem);
		t3 = timeGet();
		LES_disable();
		
		t1 = 0;
		t2 = 0;
		if (LES_getTimeSlice(t0, t3, &t1, &t2) == GETTIMEVAL_EOK) {
			tmp_cycles += t3 - t2 + t1 - t0;
			valid++;
		}
		

    }
    
    
    unlock_total_cycles = tmp_cycles;
    unlock_valid = valid;
    return NULL;
}

static void *wait_thread(void *parameter) {
	BIND_THREAD_TO_CPU(0);

	pthread_t tid1;
    pthread_attr_t attr;
    struct sched_param param;

	// Create unlock_thread:
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 4096);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = BENCHMARK_LOW_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (pthread_create(&tid1, &attr, post_thread, NULL) != 0) {
        perror("6_2:Failed to create thread.");
        return NULL;
    }
    
    
    uint64_t t0, t1 = 0, t2 = 0, t3;
	uint64_t tmp_cycles = 0;
	uint32_t valid = 0;
	
	// start
	sem_wait(&sem);
	
	// post的高优先级恢复
	for (int i = 0; i < TEST_ITERATION; i++) {
	
		LES_enable();
		t0 = timeGet();
		sem_wait(&sem);
		t3 = timeGet();
		LES_disable();

		t1 = 0;
		t2 = 0;
		if (LES_getTimeSlice(t0, t3, &t1, &t2) == GETTIMEVAL_EOK) {
			tmp_cycles += t3 - t2 + t1 - t0;
			valid++;
		}
	
    }
    
	// mid
	sem_wait(&sem);

    for (int i = 0; i < TEST_ITERATION; i++) {
		sem_wait(&sem);
    }
    
    pthread_join(tid1, NULL);
    
    lock_total_cycles = tmp_cycles;
    lock_valid = valid;
    
    return NULL;
}

static void semaphore_test_high_susp(uint64_t *a, uint64_t *b) {

	sem_init(&sem, 0, 0);

	pthread_t tid;
    pthread_attr_t attr;
    struct sched_param param;

	// Create lock_thread:
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 4096);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = BENCHMARK_HIGH_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (pthread_create(&tid, &attr, wait_thread, NULL) != 0) {
        perror("6_2:Failed to create thread.");
        return;
    }
    
    pthread_attr_destroy(&attr);

    pthread_join(tid, NULL);
    
	sem_destroy(&sem);

    if (lock_valid != TEST_ITERATION || unlock_valid != TEST_ITERATION) {
        printf("[test4_2] skipped non-monotonic samples: lock=%u unlock=%u\n",
               TEST_ITERATION - lock_valid, TEST_ITERATION - unlock_valid);
    }
    *a = lock_valid ? cycles_to_ns(lock_total_cycles / lock_valid) : 0;
    *b = unlock_valid ? cycles_to_ns(unlock_total_cycles / unlock_valid) : 0;

    return;
}

void test4_2(uint64_t *address1, uint64_t *address2) {
	if (address1 != NULL && address2 != NULL) {
		semaphore_test_high_susp(address1, address2);
	}
	return;
}
