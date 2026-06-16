#include <cpu_affinity.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>

#include "les.h"

#define TEST_REPETITION 	1000
#define MESSAGE_SIZE 		32 * 1024	//此值过大可能会影响程序运行。RTT qemu-virt64-aarch64环境下最大可跑通值是32 * 1024
#define MAX_WORKERS         8

static mqd_t mq[MAX_WORKERS];

static sem_t father_to_son;
static sem_t son_to_tmp[MAX_WORKERS];
static sem_t tmp_to_son[MAX_WORKERS];
static sem_t son_to_father;

static int ipc_mode = 0;
static volatile uint64_t cycles;


static void *tmp_thread(void* parameter) {
	int number = (int)(long)parameter;
	// ipc_mode == 1 : different core; ipc_mode == 0: same core 
	BIND_THREAD_TO_CPU((number + ipc_mode) % USE_PROCESSORS);
	
	char *buffer1 = (char*)malloc(MESSAGE_SIZE);
	char *buffer2 = (char*)malloc(MESSAGE_SIZE);

	// prepared	
	
	
	// main work
	for (int i = 0; i < TEST_REPETITION; i++) {
		// start tmp
		sem_wait(&son_to_tmp[number]);
		
		
		
		// receive message
		mq_receive(mq[number], buffer1, MESSAGE_SIZE, NULL);
		mq_receive(mq[number], buffer2, MESSAGE_SIZE, NULL);
		
		// start next turn
		sem_post(&tmp_to_son[number]);
	}
	
	
	// finish task
	
	free(buffer1);
	free(buffer2);
	
	return NULL;
}

static void *son_thread(void* parameter) {
	int number = (int)(long)parameter;
	
	BIND_THREAD_TO_CPU(number % USE_PROCESSORS);
	
	
	char *buffer1 = (char*)malloc(MESSAGE_SIZE);
	char *buffer2 = (char*)malloc(MESSAGE_SIZE);
	
	
	pthread_t tid;
    pthread_attr_t attr;
    struct sched_param param;

    pthread_attr_init(&attr);

    pthread_attr_setstacksize(&attr, 8192);
    
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = BENCHMARK_LOW_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	if (pthread_create(&tid, &attr, tmp_thread, (void *)(long)number) != 0) {
		sem_post(&son_to_father);
		printf("tmp thread create fail.\n");
		return NULL;
	}
	
	// prepared
	sem_post(&son_to_father);
	sem_wait(&father_to_son);
	
	
	
	// main work
	for (int i = 0; i < TEST_REPETITION; i++) {
		// send message
		mq_send(mq[number], buffer1, MESSAGE_SIZE, 0);
		mq_send(mq[number], buffer2, MESSAGE_SIZE, 0);
				
		// start tmp
		sem_post(&son_to_tmp[number]);
		
		// wait for next turn
		sem_wait(&tmp_to_son[number]);
	}
	
	
	
	// finish task


	pthread_join(tid, NULL);

	sem_post(&son_to_father);
	
	free(buffer1);
	free(buffer2);

	return NULL;
}

static void *father_thread(void* parameter) {
	BIND_THREAD_TO_CPU(0);
	int pair_count = (int)(long)parameter;

	pthread_t tid[MAX_WORKERS];
    pthread_attr_t attr;
    struct sched_param param;

    pthread_attr_init(&attr);

    pthread_attr_setstacksize(&attr, 8192);
    
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = BENCHMARK_LOW_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	for (int i = 0; i < pair_count; i++) {
		if (pthread_create(&tid[i], &attr, son_thread, (void *)(long)i) != 0) {
		    return NULL;
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


uint64_t multicore_ipc_bw(int mode, int pair_count) {
	if (pair_count < 1) pair_count = 1;
	if (pair_count > MAX_WORKERS) pair_count = MAX_WORKERS;
	
	static const char *mq_name[MAX_WORKERS] = {
		"/ipc_bw_queue_0", "/ipc_bw_queue_1",
		"/ipc_bw_queue_2", "/ipc_bw_queue_3",
		"/ipc_bw_queue_4", "/ipc_bw_queue_5",
		"/ipc_bw_queue_6", "/ipc_bw_queue_7"
	};
    struct mq_attr attr_m;
	memset(&attr_m, 0, sizeof(attr_m));

    attr_m.mq_flags = 0;
    attr_m.mq_maxmsg = 2;
    attr_m.mq_msgsize = MESSAGE_SIZE;
    attr_m.mq_curmsgs = 0;

	for (int i = 0; i < pair_count; i++) {
		mq_unlink(mq_name[i]);
		mq[i] = mq_open(mq_name[i], O_CREAT | O_RDWR | O_EXCL, 0644, &attr_m);
		if (mq[i] == (mqd_t)-1) {
		    if (errno == EEXIST) {
		        printf("报错：队列 '%s' 已存在，无法重新创建。\n", mq_name[i]);
		    } else {
		        perror("mq_open 发生其他错误\n");
		    }
		    exit(1);
		}
	}

	for (int i = 0; i < pair_count; i++) {
		sem_init(&son_to_tmp[i], 0, 0);
		sem_init(&tmp_to_son[i], 0, 0);
	}
	sem_init(&father_to_son, 0, 0);
	sem_init(&son_to_father, 0, 0);


    pthread_t tid;
    pthread_attr_t attr;
    struct sched_param param;

    pthread_attr_init(&attr);

    pthread_attr_setstacksize(&attr, 8192);
    
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = BENCHMARK_HIGH_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    

    if (pthread_create(&tid, &attr, father_thread, (void *)(long)pair_count) != 0) {
        return 0;
    }


	pthread_join(tid, NULL);

    pthread_attr_destroy(&attr);
    
    for (int i = 0; i < pair_count; i++) {
		sem_destroy(&son_to_tmp[i]);
		sem_destroy(&tmp_to_son[i]);
	}
	sem_destroy(&father_to_son);
	sem_destroy(&son_to_father);
    
    for (int i = 0; i < pair_count; i++) {
    	mq_close(mq[i]);
    	mq_unlink(mq_name[i]);
    }
    
    return 32 * TEST_REPETITION * pair_count * 1000000000ULL / cycles_to_ns(cycles) / 1024;
}

void test_ipc_bw(uint64_t *address, int mode, int concurrency) {
	if (concurrency < 1) concurrency = 1;
	if (concurrency > MAX_WORKERS) concurrency = MAX_WORKERS;
	
	if (mode == 0 || mode == 1) concurrency = 1;
	ipc_mode = mode;

    if (address != NULL) {    
        *address = multicore_ipc_bw(mode, concurrency);
    }
}
