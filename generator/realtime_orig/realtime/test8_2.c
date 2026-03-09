/*
 * File: benchmark/realtime/test8/test8_2.c
 * 场景: 互斥锁获取（挂起睡眠）&互斥锁释放（高优先级恢复）
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

static pthread_mutex_t lock;
static sem_t to_locker;
static sem_t to_unlocker;

#define TEST_ITERATION 1000

static volatile uint64_t unlock_total_cycles, lock_total_cycles;


static void *unlock_thread(void *parameter) {
	BIND_THREAD_TO_CPU(0);

	// start
	pthread_mutex_lock(&lock);
	sem_post(&to_locker);

	for (int i = 0; i < TEST_ITERATION; i++) {
		
		pthread_mutex_unlock(&lock);
		sem_wait(&to_unlocker);
		pthread_mutex_lock(&lock);
		sem_post(&to_locker);
    }

	// mid
	pthread_mutex_unlock(&lock);
	sem_wait(&to_unlocker);
	pthread_mutex_lock(&lock);
	sem_post(&to_locker);

	uint64_t t0, t1 = 0, t2 = 0, t3;
	uint64_t tmp_cycles = 0;

	// lock高优先级挂起睡眠
	for (int i = 0; i < TEST_ITERATION; i++) {
		
		LES_enable();
		t0 = timeGet();
		pthread_mutex_unlock(&lock);
		t3 = timeGet();
		LES_disable();
		
		sem_wait(&to_unlocker);
		
		int offset = LES_getOffset();
		if (offset >= 2) {
			LES_getTimeVal(0, &t1);
			LES_getTimeVal(offset - 1, &t2);
		}
		
		tmp_cycles += t3 - t2 + t1 - t0;
		
		pthread_mutex_lock(&lock);
		sem_post(&to_locker);
    }
    
    pthread_mutex_unlock(&lock);
    
    unlock_total_cycles = tmp_cycles;
    return NULL;
}

static void *lock_thread(void *parameter) {    
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
    if (pthread_create(&tid1, &attr, unlock_thread, NULL) != 0) {
        perror("8_2:Failed to create thread.");
        return NULL;
    }
    
    
    uint64_t t0, t1 = 0, t2 = 0, t3;
	uint64_t tmp_cycles = 0;
	
	// start
	sem_wait(&to_locker);
	
	// unlock的高优先级恢复
	for (int i = 0; i < TEST_ITERATION; i++) {
	
		LES_enable();
		t0 = timeGet();
		pthread_mutex_lock(&lock);
		t3 = timeGet();
		LES_disable();
		
		pthread_mutex_unlock(&lock);
		
		int offset = LES_getOffset();
		if (offset >= 2) {
			LES_getTimeVal(0, &t1);
			LES_getTimeVal(offset - 1, &t2);
		}
		
		tmp_cycles += t3 - t2 + t1 - t0;
		
		sem_post(&to_unlocker);
		sem_wait(&to_locker);
    }
    
	// mid
	pthread_mutex_lock(&lock);
	pthread_mutex_unlock(&lock);
	sem_post(&to_unlocker);
	sem_wait(&to_locker);
    
    for (int i = 0; i < TEST_ITERATION; i++) {
		pthread_mutex_lock(&lock);
		pthread_mutex_unlock(&lock);
		sem_post(&to_unlocker);
		sem_wait(&to_locker);
    }
    
    pthread_join(tid1, NULL);
    
    lock_total_cycles = tmp_cycles;
    
    return NULL;
}

static void mutex_test_high_susp(uint64_t *a, uint64_t *b) {

	sem_init(&to_locker, 0, 0);
	sem_init(&to_unlocker, 0, 0);

	pthread_mutex_init(&lock, NULL);
    
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
    if (pthread_create(&tid, &attr, lock_thread, NULL) != 0) {
        perror("8_2:Failed to create thread.");
        return;
    }
    
    pthread_attr_destroy(&attr);

    pthread_join(tid, NULL);
    
	pthread_mutex_destroy(&lock);
	
	sem_destroy(&to_locker);
	sem_destroy(&to_unlocker);
    
    *a = cycles_to_ns(lock_total_cycles / TEST_ITERATION);
    *b = cycles_to_ns(unlock_total_cycles / TEST_ITERATION);

    return;
}

void test8_2(uint64_t *address1, uint64_t *address2) {
	if (address1 != NULL && address2 != NULL) {
		mutex_test_high_susp(address1, address2);
	}
	return;
}
