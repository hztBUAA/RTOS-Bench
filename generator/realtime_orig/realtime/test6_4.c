/*
 * File: benchmark/realtime/test6/test6_4.c
 * 场景: 消息发送（高优先级恢复）&消息接收（挂起睡眠）
 * 插桩：是
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

#include "les.h"

#define TEST_ITERATION 1000

static mqd_t mq;
static volatile uint64_t send_total_cycles, receive_total_cycles;

static const char* msg = "Hi";


static void *send_thread(void *parameter) {
	BIND_THREAD_TO_CPU(0);
	mq_send(mq, msg, strlen(msg) + 1, 0);

	for (int i = 0; i < TEST_ITERATION; i++) {
		mq_send(mq, msg, strlen(msg) + 1, 0);
    }

	mq_send(mq, msg, strlen(msg) + 1, 0);

	uint64_t t0, t1 = 0, t2 = 0, t3;
	uint64_t tmp_cycles = 0;

	// 消息接收的高优先级挂起睡眠
	for (int i = 0; i < TEST_ITERATION; i++) {
		
		LES_enable();
		t0 = timeGet();
		mq_send(mq, msg, strlen(msg) + 1, 0);
		t3 = timeGet();
		LES_disable();
		
		int offset = LES_getOffset();
		if (offset >= 2) {
			LES_getTimeVal(0, &t1);
			LES_getTimeVal(offset - 1, &t2);
		}
		
		tmp_cycles += t3 - t2 + t1 - t0;
    }
    
    send_total_cycles = tmp_cycles;
    return NULL;
}

static void *receive_thread(void *parameter) {    
	BIND_THREAD_TO_CPU(0);
	pthread_t tid1;
    pthread_attr_t attr;
    struct sched_param param;

	// Create send_thread:
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = BENCHMARK_LOW_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (pthread_create(&tid1, &attr, send_thread, NULL) != 0) {
        perror("6_4:Failed to create thread.");
        return NULL;
    }
    
    char buffer[8];
    
    uint64_t t0, t1 = 0, t2 = 0, t3;
	uint64_t tmp_cycles = 0;
	
	mq_receive(mq, buffer, 8, NULL);
	// 消息发送的高优先级恢复
	for (int i = 0; i < TEST_ITERATION; i++) {
	
		LES_enable();
		t0 = timeGet();
		mq_receive(mq, buffer, 8, NULL);
		t3 = timeGet();
		LES_disable();
		
		int offset = LES_getOffset();
		if (offset >= 2) {
			LES_getTimeVal(0, &t1);
			LES_getTimeVal(offset - 1, &t2);
		}
		
		tmp_cycles += t3 - t2 + t1 - t0;
    }
    
    mq_receive(mq, buffer, 8, NULL);
    
    for (int i = 0; i < TEST_ITERATION; i++) {
		mq_receive(mq, buffer, 8, NULL);
    }
    
    pthread_join(tid1, NULL);
    
    receive_total_cycles = tmp_cycles;
    
    return NULL;
}

static void message_test_high_preempt(uint64_t *a, uint64_t *b) {

	static const char *mq_name = "/6_4_queue";
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

	// Create receive_thread:
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = BENCHMARK_HIGH_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (pthread_create(&tid, &attr, receive_thread, NULL) != 0) {
        perror("6_4:Failed to create thread.");
        return;
    }
    
    pthread_attr_destroy(&attr);

    pthread_join(tid, NULL);
    
    mq_close(mq);
    mq_unlink(mq_name);
    
    *a = cycles_to_ns(send_total_cycles / TEST_ITERATION);
    *b = cycles_to_ns(receive_total_cycles / TEST_ITERATION);

    return;
}

void test6_4(uint64_t *address1, uint64_t *address2) {
	if (address1 != NULL && address2 != NULL) {
		message_test_high_preempt(address1, address2);
	}
	return;
}
