/*
 * File: benchmark/realtime/test6/test6_3.c
 * 场景: 消息发送、低优先级就绪
 * 插桩：否
 */
#include <cpu_affinity.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <mqueue.h>
#include <fcntl.h>
#include <semaphore.h>

#include "les.h"

#define TEST_ITERATION 1000

static mqd_t mq;
static sem_t to_highest;
static sem_t to_assist;
static volatile uint64_t total_cycles;

static const char* msg = "Hi";


static void *assist_thread(void *parameter) {
	BIND_THREAD_TO_CPU(0);
	for (int i = 0; i < TEST_ITERATION; i++) {
		sem_wait(&to_assist);
		sem_post(&to_highest);
	}
	return NULL;
}

static void *receive_thread(void *parameter) {
	BIND_THREAD_TO_CPU(0);
	for (int i = 0; i < TEST_ITERATION; i++) {
		char buffer[8];
		mq_receive(mq, buffer, 8, NULL);
    }
    return NULL;
}

static void *send_thread(void *parameter) {    
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
    if (pthread_create(&tid1, &attr, receive_thread, NULL) != 0) {
        perror("6_3:Failed to create thread.");
        return NULL;
    }
    
    // Create assist_thread:
    pthread_attr_setstacksize(&attr, 8192);
    param.sched_priority = BENCHMARK_LOW_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    if (pthread_create(&tid2, &attr, assist_thread, NULL) != 0) {
        perror("6_3:Failed to create thread.");
        return NULL;
    }
    
    
    uint64_t t0, t1;
	uint64_t tmp_send_cycles = 0;
	for (int i = 0; i < TEST_ITERATION; i++) {
	
		sem_post(&to_assist);
		sem_wait(&to_highest);
		
		t0 = timeGet();
		mq_send(mq, msg, strlen(msg) + 1, 0);
		t1 = timeGet();

		
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

	static const char *mq_name = "/6_3_queue";
    struct mq_attr attr_m;

    attr_m.mq_flags = 0;
    attr_m.mq_maxmsg = 1;
    attr_m.mq_msgsize = 8;
    attr_m.mq_curmsgs = 0;

	mq_unlink(mq_name);
    mq = mq_open(mq_name, O_CREAT | O_RDWR | O_EXCL, 0644, &attr_m);
    if (mq == (mqd_t)-1) {
        if (errno == EEXIST) {
            printf("报错：队列 '%s' 已存在，无法重新创建。\n", mq_name);
        } else {
            perror("mq_open 发生其他错误\n");
        }
        exit(1);
    }



	pthread_t tid;
    pthread_attr_t attr;
    struct sched_param param;

	// Create send_thread:
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = BENCHMARK_HIGH_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (pthread_create(&tid, &attr, send_thread, NULL) != 0) {
        perror("6_3:Failed to create thread.");
        return;
    }
    
    pthread_attr_destroy(&attr);

    pthread_join(tid, NULL);
    
    mq_close(mq);
    mq_unlink(mq_name);
    
    sem_destroy(&to_highest);
    sem_destroy(&to_assist);
    
    *a = cycles_to_ns(total_cycles / TEST_ITERATION);

    return;
}

void test6_3(uint64_t *address) {
	if (address != NULL) {
		message_test_low_ready(address);
	}
	return;
}
