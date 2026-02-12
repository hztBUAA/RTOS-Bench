/*
 * File: benchmark/realtime/test1/test1.c
 * 项目: 上下文切换延迟
 * 插桩：否
 */
#include <pthread.h>
#include <inttypes.h>
#include <stdio.h>
#include <semaphore.h>
#include <sched.h>

#include "les.h"
#include "data_tools.h"

#define TEST_ITERATIONS 10000
static uint64_t t0, t1;
static uint64_t e0, e1;

static sem_t sem_start;
static sem_t sem_done;

static void* thread_b_entry(void* arg) {
    /* 1. 等待线程 A 准备就绪 */
    sem_wait(&sem_start);

    /* 2. 执行 N + 1 次 yield */
    for (int i = 0; i < TEST_ITERATIONS + 1; i++) {
        sched_yield();
    }

    /* 3. 测量结束，通知 A */
    sem_post(&sem_done);
    return NULL;
}

static void* thread_a_entry(void* arg) {
    /*  释放信号量唤醒 B，并执行第一次同步 yield */
    sem_post(&sem_start);
    sched_yield(); 

	/* 排除空循环的误差 */
	e0 = timeGet();
    for (int i = 0; i < TEST_ITERATIONS; i++) {
        __asm__ __volatile__("");
    }
    e1 = timeGet();

    /* --- 测量区间开始 --- */
    t0 = timeGet();

    for (int i = 0; i < TEST_ITERATIONS; i++) {
        sched_yield(); 
    }

    t1 = timeGet();
    /* --- 测量区间结束 --- */

    /* 等待线程 B 完成 */
    sem_wait(&sem_done);

    return NULL;
}

void test1(uint64_t *address) {
    pthread_t tid_a, tid_b;

    // 初始化信号量
    sem_init(&sem_start, 0, 0);
    sem_init(&sem_done, 0, 0);

	pthread_attr_t attr;
	pthread_attr_init(&attr);

	// 设置实时优先级
	struct sched_param param;
	param.sched_priority = 30;
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    // 创建线程
    if (pthread_create(&tid_b, &attr, thread_b_entry, NULL) != 0) {
        perror("Failed to create thread B");
        return;
    }
    if (pthread_create(&tid_a, &attr, thread_a_entry, NULL) != 0) {
        perror("Failed to create thread A");
        return;
    }

    // 等待线程结束
    pthread_join(tid_a, NULL);
    pthread_join(tid_b, NULL);

    sem_destroy(&sem_start);
    sem_destroy(&sem_done);
    
    uint64_t rtn = cycles_to_ns((t1 - t0) - (e1 - e0)) / TEST_ITERATIONS / 2;
    *address = rtn;

    return;
}
