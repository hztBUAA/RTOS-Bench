#include <cpu_affinity.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdint.h>

#include "les.h"
#include "bench_verify.h"

static sem_t sem;

static uint64_t t0;
static uint64_t t1;
static volatile unsigned int offset;

static void *thread_b(void *parameter) {
    BIND_THREAD_TO_CPU(0);

    sem_post(&sem);
    sem_post(&sem);

    return NULL;
}

static void *thread_a(void *parameter) {
    BIND_THREAD_TO_CPU(0);

    pthread_t tid;
    pthread_attr_t attr;
    struct sched_param param;
	// Create thread:
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 4096);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = BENCHMARK_MIDDLE_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (pthread_create(&tid, &attr, thread_b, NULL) != 0) {
        perror("Failed to create thread.");
        return NULL;
    }

    sem_wait(&sem);

    LES_enable();
    sem_wait(&sem);
    LES_disable();

    offset = LES_getOffset();
    if (offset >= 1) {
		LES_getTimeVal(0, &t0);
	}
	if (offset >= 2) {
		LES_getTimeVal(1, &t1);
	}

    return NULL;
}

void schedule_stub_verify(void) {
    t0 = 0;
    t1 = 0;

    sem_init(&sem, 0, 0);

    pthread_t tid;
    pthread_attr_t attr;
    struct sched_param param;
	// Create thread:
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 4096);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = BENCHMARK_HIGH_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (pthread_create(&tid, &attr, thread_a, NULL) != 0) {
        perror("Failed to create thread.");
        return;
    }

    pthread_join(tid, NULL);

    pthread_attr_destroy(&attr);

    sem_destroy(&sem);

    printf("[schedule stub verification]\n"
           "stub buffer offset : expect 2, get %d\n", offset);
    if (offset == 2) {
        printf("stub buffer[0] : %llu cycles, since the beginning\n"
               "stub buffer[1] : %llu cycles, since the beginning\n", (unsigned long long)t0, (unsigned long long)t1);
        if (t0 < t1) {
            uint64_t dur = cycles_to_ns(t1 - t0);
            uint64_t dur_i = dur / 1000;
            uint64_t dur_f = dur % 1000;
            printf("Time lag : %llu.%03llu us (should be very short)\n", (unsigned long long)dur_i, (unsigned long long)dur_f);
        }
    }
}