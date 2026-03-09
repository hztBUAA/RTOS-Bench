#include <cpu_affinity.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "les.h"

#define TEST_REPETITION 	100
#define MAX_WORKERS         8


static sem_t father_to_son;
static sem_t son_to_father;

static volatile uint64_t cycles;

static void *tmp_thread(void* parameter) {
	return NULL;
}

static void *son_thread(void* parameter) {
	int number = (int)(long)parameter;
	BIND_THREAD_TO_CPU(number % USE_PROCESSORS);

	pthread_t tid[TEST_REPETITION];
    pthread_attr_t attr;
    struct sched_param param;

    pthread_attr_init(&attr);
    
    pthread_attr_setstacksize(&attr, 4096);
    
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = BENCHMARK_LOW_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);

	
	
	
	// prepared
	sem_post(&son_to_father);
	sem_wait(&father_to_son);
	
	
	
	// main work
	for (int i = 0; i < TEST_REPETITION; i++) {
		
		if (pthread_create(&tid[i], &attr, tmp_thread, (void *)(long)number) != 0) {

			sem_post(&son_to_father);

			printf("tmp thread create fail.\n");

			//clear
			for (int j = 0; j < i; j++) {
				pthread_join(tid[j], NULL);
			}

			pthread_attr_destroy(&attr);

			return NULL;
		}
	}
	
	
	
	// finish task

	for (int i = 0; i < TEST_REPETITION; i++) {
		pthread_join(tid[i], NULL);
	}

	sem_post(&son_to_father);

	pthread_attr_destroy(&attr);

	return NULL;
}

static void *father_thread(void* parameter) {
	BIND_THREAD_TO_CPU(0);
	int pair_count = (int)(long)parameter;

	pthread_t tid[MAX_WORKERS];
    pthread_attr_t attr;
    struct sched_param param;

    pthread_attr_init(&attr);

    pthread_attr_setstacksize(&attr, 16384);
    
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = BENCHMARK_MIDDLE_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	for (int i = 0; i < pair_count; i++) {
		if (pthread_create(&tid[i], &attr, son_thread, (void *)(long)i) != 0) {
		    return 0;
		}
	}
	

	// wait for sons to be prepared
	for (int i = 0; i < pair_count; i++) {
		sem_wait(&son_to_father);
	}
	

	uint64_t t0, t1;
	
	t0 = timeGet();
	
	// notify all
	for (int i = 0; i < pair_count; i++) {
		sem_post(&father_to_son);
	}
	
	// wait for all
	for (int i = 0; i < pair_count; i++) {
		sem_wait(&son_to_father);
	}
	
	t1 = timeGet();
	
	// clear
	for (int i = 0; i < pair_count; i++) {
		pthread_join(tid[i], NULL);
	}
	
	pthread_attr_destroy(&attr);
	
	cycles = t1 - t0;
	
    return NULL;
}


uint64_t multicore_task_lat(int pair_count) {
	if (pair_count < 1) pair_count = 1;
	if (pair_count > MAX_WORKERS) pair_count = MAX_WORKERS;
	

	sem_init(&father_to_son, 0, 0);
	sem_init(&son_to_father, 0, 0);


    pthread_t tid;
    pthread_attr_t attr;
    struct sched_param param;

    pthread_attr_init(&attr);

    pthread_attr_setstacksize(&attr, 16384);
    
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = BENCHMARK_HIGH_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    

    if (pthread_create(&tid, &attr, father_thread, (void *)(long)pair_count) != 0) {
        return 0;
    }


	pthread_join(tid, NULL);

    pthread_attr_destroy(&attr);
    
	sem_destroy(&father_to_son);
	sem_destroy(&son_to_father);
    
    return cycles_to_ns(cycles) / TEST_REPETITION / pair_count;
}

void test_task_lat(uint64_t *address, int concurrency) {
	if (concurrency <= 0) concurrency = 1;
    if (concurrency > MAX_WORKERS) concurrency = MAX_WORKERS;
    
	uint64_t total_lat = 0;
	total_lat = multicore_task_lat(concurrency);
	printf("end task_lt task %d.\n", concurrency);
	*address = total_lat;
}
