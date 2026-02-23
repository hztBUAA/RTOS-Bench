/*
 * File: benchmark/realtime/test6/test6_3.c
 * 场景: 消息发送、低优先级就绪
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

/* Debug mode: set to 1 for verbose output, 0 for production */
#define TEST6_3_DEBUG 0

static mqd_t mq;
static sem_t to_highest;
static sem_t to_assist;
static volatile uint64_t total_cycles;
static volatile int recv_count = 0;
static volatile int send_count = 0;
static volatile int assist_count = 0;

static const char* msg = "Hi";


static void *assist_thread(void *parameter) {
#if TEST6_3_DEBUG
	printf("[6_3] assist_thread started\n");
#endif
	for (int i = 0; i < TEST_ITERATION; i++) {
		sem_wait(&to_assist);
		assist_count++;
		sem_post(&to_highest);
#if TEST6_3_DEBUG
		if (i < 5 || i == TEST_ITERATION - 1 || (i % 200 == 0)) {
			printf("[6_3] assist: iter %d done\n", i);
		}
#endif
	}
#if TEST6_3_DEBUG
	printf("[6_3] assist_thread done, count=%d\n", assist_count);
#endif
	return NULL;
}

static void *receive_thread(void *parameter) {
#if TEST6_3_DEBUG
	printf("[6_3] receive_thread started\n");
#endif
	for (int i = 0; i < TEST_ITERATION; i++) {
		char buffer[8];
		ssize_t bytes_read = mq_receive(mq, buffer, 8, NULL);
		if (bytes_read > 0) {
			recv_count++;
		} else {
			printf("[6_3] recv FAILED: iter %d, bytes=%zd, errno=%d\n", i, bytes_read, errno);
		}
#if TEST6_3_DEBUG
		if (i < 5 || i == TEST_ITERATION - 1 || (i % 200 == 0)) {
			printf("[6_3] recv: iter %d, bytes=%zd, total=%d\n", i, bytes_read, recv_count);
		}
#endif
    }
#if TEST6_3_DEBUG
    printf("[6_3] receive_thread done, count=%d\n", recv_count);
#endif
    return NULL;
}

static void *send_thread(void *parameter) {

	pthread_t tid1, tid2;
    pthread_attr_t attr;
    struct sched_param param;

#if TEST6_3_DEBUG
	printf("[6_3] send_thread started, creating children...\n");
#endif

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
#if TEST6_3_DEBUG
	printf("[6_3] receive_thread created (prio=%d)\n", BENCHMARK_MIDDLE_PRIO);
#endif

    // Create assist_thread:
    pthread_attr_setstacksize(&attr, 8192);
    param.sched_priority = BENCHMARK_LOW_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    if (pthread_create(&tid2, &attr, assist_thread, NULL) != 0) {
        perror("6_3:Failed to create thread.");
        return NULL;
    }
#if TEST6_3_DEBUG
	printf("[6_3] assist_thread created (prio=%d)\n", BENCHMARK_LOW_PRIO);
	printf("[6_3] send_thread entering main loop (prio=%d)...\n", BENCHMARK_HIGH_PRIO);
#endif


    uint64_t t0, t1;
	uint64_t tmp_send_cycles = 0;
	for (int i = 0; i < TEST_ITERATION; i++) {

		sem_post(&to_assist);
		sem_wait(&to_highest);

		t0 = timeGet();
		int ret;
		int retry_count = 0;
		/* RT-Thread mqueue may return error instead of blocking when full.
		 * Workaround: retry with delay to let receiver run. */
		do {
			ret = mq_send(mq, msg, strlen(msg) + 1, 0);
			if (ret != 0) {
				retry_count++;
				/* Force delay to let lower priority receiver run */
				rt_thread_mdelay(1);
			}
		} while (ret != 0 && retry_count < 50);
		t1 = timeGet();

		if (ret == 0) {
			send_count++;
		}
#if TEST6_3_DEBUG
		if (ret != 0) {
			printf("[6_3] send FAILED: iter %d, ret=%d, errno=%d, retries=%d\n",
			       i, ret, errno, retry_count);
		}
		if (i < 5 || i == TEST_ITERATION - 1) {
			printf("[6_3] send: iter %d, ret=%d, retries=%d\n", i, ret, retry_count);
		}
#endif

		tmp_send_cycles += t1 - t0;
    }

#if TEST6_3_DEBUG
    printf("[6_3] send_thread main loop done, send_count=%d\n", send_count);
    printf("[6_3] send_thread joining children...\n");
#endif

    pthread_join(tid1, NULL);
#if TEST6_3_DEBUG
    printf("[6_3] receive_thread joined\n");
#endif
    pthread_join(tid2, NULL);
#if TEST6_3_DEBUG
    printf("[6_3] assist_thread joined\n");
#endif

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
