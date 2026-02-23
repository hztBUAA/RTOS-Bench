/*
 * File: benchmark/realtime/test7/test7_3.c
 * 场景: 消息接收、低优先级就绪
 * 插桩：否
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <mqueue.h>
#include <fcntl.h>
#include <semaphore.h>
#include <rtthread.h>

#include "les.h"

#define TEST_ITERATION 1000

static mqd_t mq;
static sem_t to_highest;
static sem_t to_assist;
static volatile uint64_t total_cycles;

static const char* msg = "Hi";

/* Helper: send with retry for RT-Thread mqueue bug workaround */
static int mq_send_retry(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio)
{
	int ret;
	int retry = 0;
	do {
		ret = mq_send(mqdes, msg_ptr, msg_len, msg_prio);
		if (ret != 0) {
			retry++;
			rt_thread_mdelay(1);
		}
	} while (ret != 0 && retry < 50);
	return ret;
}


static void *assist_thread(void *parameter) {
	for (int i = 0; i < TEST_ITERATION; i++) {
		sem_wait(&to_assist);
		sem_post(&to_highest);
	}
	return NULL;
}

static void *send_thread(void *parameter) {
	for (int i = 0; i < TEST_ITERATION; i++) {
		mq_send_retry(mq, msg, strlen(msg) + 1, 0);
    }

    return NULL;
}

static void *receive_thread(void *parameter) {    

	pthread_t tid1, tid2;
    pthread_attr_t attr;
    struct sched_param param;

	// Create send_thread:
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = BENCHMARK_MIDDLE_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (pthread_create(&tid1, &attr, send_thread, NULL) != 0) {
        perror("7_3:Failed to create thread.");
        return NULL;
    }
    
    // Create assist_thread:
    pthread_attr_setstacksize(&attr, 8192);
    param.sched_priority = BENCHMARK_LOW_PRIO;
    if (pthread_create(&tid2, &attr, assist_thread, NULL) != 0) {
        perror("7_3:Failed to create thread.");
        return NULL;
    }
    
    
    uint64_t t0, t1;
	uint64_t tmp_send_cycles = 0;
	for (int i = 0; i < TEST_ITERATION; i++) {
		
		sem_post(&to_assist);
		sem_wait(&to_highest);
	
		char buffer[8];
		
		t0 = timeGet();
		mq_receive(mq, buffer, 8, NULL);
		t1 = timeGet();
		tmp_send_cycles += t1 - t0;
    }
    
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    
    total_cycles = tmp_send_cycles;
    
    return NULL;
}

static void message_test_low_ready(uint64_t *a) {

	sem_init(&to_highest, 0, 0);
	sem_init(&to_assist, 0, 0);

	static const char *mq_name = "/7_3_queue";
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
    
    // 将消息队列填满
    if (mq_send(mq, msg, strlen(msg) + 1, 0) == -1) {
        perror("mq_send");
    } else {
        //printf("Sent: %s (Priority: 0)\n", msg);
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
        perror("7_3:Failed to create thread.");
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

void test7_3(uint64_t *address) {
	if (address != NULL) {
		message_test_low_ready(address);
	}
	return;
}
