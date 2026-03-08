#include <cpu_affinity.h>
#include <pthread.h>
#include <stdio.h>

#include "les.h"
#include "bench_verify.h"

static volatile int cpu_1st;
static volatile int cpu_2nd;

static void *thread(void *parameter) {
    BIND_THREAD_TO_CPU(0);
    cpu_1st = bench_get_cpu();
    BIND_THREAD_TO_CPU(1);
    cpu_2nd = bench_get_cpu();
    
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

    printf("[cpu bind verification]\n"
           "1st : expect 0, get %d\n"
           "2nd : expect 1, get %d\n", cpu_1st, cpu_2nd);
    if (cpu_1st != 0) {
        printf("please check cpu affinity function.\n");
    } else if (cpu_2nd != 1) {
        printf("please ensure turning on SMP, then check cpu affinity function.\n");
    }
}