#include <cpu_affinity.h>
#include <pthread.h>
#include <stdio.h>

#include "les.h"
#include "bench_verify.h"

#define ITERATION 4

static volatile int cpu_number[ITERATION];

static void *thread(void *parameter) {
    for (int i = 0; i < ITERATION; i++) {
        BIND_THREAD_TO_CPU(i % USE_PROCESSORS);
        cpu_number[i] = bench_get_cpu();
    }

    return NULL;
}

void cpu_bind_verify(void) {
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
    if (pthread_create(&tid, &attr, thread, NULL) != 0) {
        perror("Failed to create thread.");
        return;
    }

    pthread_join(tid, NULL);

    pthread_attr_destroy(&attr);

    int bind_flag = 1;
    int smp_flag = 0;

    printf("[cpu bind verification]\n");
    for (int i = 0; i < ITERATION; i++) {
        printf("iteration %d : expect %d, get %d\n", i, i % USE_PROCESSORS, cpu_number[i]);
        if (i % USE_PROCESSORS != cpu_number[i]) {
            bind_flag = 0;
        }
        if (cpu_number[i] != 0) {
            smp_flag = 1;
        }
    }
    if (bind_flag == 0) {
        printf("please check cpu affinity API (BIND_THREAD_TO_CPU & bench_get_cpu).\n");
    } else if (smp_flag == 0) {
        printf("please ensure turning on SMP.\n");
    }
}