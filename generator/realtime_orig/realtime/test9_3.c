/*
 * File: benchmark/realtime/test9/test9_3.c
 * 场景: 互斥锁释放、低优先级就绪
 * 插桩：否
 */
#include <cpu_affinity.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#include "les.h"

#define TEST_ITERATION 1000

static pthread_mutex_t lock;
static sem_t to_highest;
static sem_t to_assist;
static volatile uint64_t total_cycles;


static void *assist_thread(void *parameter) {
    BIND_THREAD_TO_CPU(0);
	for (int i = 0; i < TEST_ITERATION; i++) {
		sem_wait(&to_assist);
		sem_post(&to_highest);
	}
	return NULL;
}

static void *lock_thread(void *parameter) {
    BIND_THREAD_TO_CPU(0);
	for (int i = 0; i < TEST_ITERATION; i++) {
		pthread_mutex_lock(&lock);
		pthread_mutex_unlock(&lock);
    }
    
    return NULL;
}

static void *unlock_thread(void *parameter) {    
    BIND_THREAD_TO_CPU(0);

	pthread_t tid1, tid2;
    pthread_attr_t attr;
    struct sched_param param;

	// Create receive_thread:
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = BENCHMARK_MIDDLE_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (pthread_create(&tid1, &attr, lock_thread, NULL) != 0) {
        perror("9_3:Failed to create thread.");
        return NULL;
    }
    
    // Create assist_thread:
    pthread_attr_setstacksize(&attr, 8192);
    param.sched_priority = BENCHMARK_LOW_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    if (pthread_create(&tid2, &attr, assist_thread, NULL) != 0) {
        perror("9_3:Failed to create thread.");
        return NULL;
    }
    
    
    uint64_t t0, t1;
	uint64_t tmp_send_cycles = 0;
	for (int i = 0; i < TEST_ITERATION; i++) {
	
		pthread_mutex_lock(&lock);
		sem_post(&to_assist);
		sem_wait(&to_highest);
		
		//此处无线程切换
		//LES_enable();
		
		t0 = timeGet();
		pthread_mutex_unlock(&lock);
		t1 = timeGet();
		
		//LES_disable();
		
		/*
		int offset = LES_getOffset();
		if (offset >= 2) {
			printf("switch occur, it's wrong.\n");
		}
		*/
		
		tmp_send_cycles += t1 - t0;
    }
    
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    
    total_cycles = tmp_send_cycles;
    
    pthread_attr_destroy(&attr);
    
    return NULL;
}

static void message_test_low_ready(uint64_t *a) {

	sem_init(&to_highest, 0, 0);
	sem_init(&to_assist, 0, 0);

	pthread_mutex_init(&lock, NULL);

	pthread_t tid;
    pthread_attr_t attr;
    struct sched_param param;

	// Create send_thread:
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 4096);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = BENCHMARK_HIGH_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (pthread_create(&tid, &attr, unlock_thread, NULL) != 0) {
        perror("9_3:Failed to create thread.");
        return;
    }
    
    pthread_attr_destroy(&attr);

    pthread_join(tid, NULL);
    
	pthread_mutex_destroy(&lock);
    
    sem_destroy(&to_highest);
    sem_destroy(&to_assist);
    
    *a = cycles_to_ns(total_cycles / TEST_ITERATION);

    return;
}

void test9_3(uint64_t *address) {
	if (address != NULL) {
		message_test_low_ready(address);
	}
	return;
}
