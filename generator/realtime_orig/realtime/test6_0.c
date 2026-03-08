/*
 * File: benchmark/realtime/test6/test6_0.c
 * 场景: 消息队列满时阻塞检测
 * 在消息队列已满时，向其中发送消息的请求只有能够阻塞（或至少忙等一段时间），才可以进行6.2/7.3/7.4测试
 */
#include <cpu_affinity.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <mqueue.h>
#include <fcntl.h>

#include "les.h"

static mqd_t mq;
static const char* msg = "Hi";
static volatile int can_stall = 0;

static void *receive_thread(void *parameter) {
    BIND_THREAD_TO_CPU(0);
	// 睡1ms，看看send_thread能否阻塞等待消息队列出现空位
	safe_usleep(1000);
	
	char buffer[8];
	mq_receive(mq, buffer, 8, NULL);
    return NULL;
}

static void *send_thread(void *parameter) {
    BIND_THREAD_TO_CPU(0);

	pthread_t tid1;
    pthread_attr_t attr;
    struct sched_param param;

	// Create send_thread:
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 4096);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = BENCHMARK_LOW_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (pthread_create(&tid1, &attr, receive_thread, NULL) != 0) {
        return NULL;
    }
    
	int ret = mq_send(mq, msg, strlen(msg) + 1, 0);
    can_stall = ret;
    
    pthread_join(tid1, NULL);
    
    return NULL;
}

static int message_pre_test() {

	static const char *mq_name = "/6_0_queue";
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

	// Create send_thread:
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 4096);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = BENCHMARK_HIGH_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (pthread_create(&tid, &attr, send_thread, NULL) != 0) {
        perror("8_0:Failed to create thread.");
        return -1;
    }
    
    pthread_attr_destroy(&attr);

    pthread_join(tid, NULL);
    
    mq_close(mq);
    mq_unlink(mq_name);
    
    return can_stall;
}

int test6_0(void) {
	return message_pre_test();
}
