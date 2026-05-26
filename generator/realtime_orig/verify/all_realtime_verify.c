#include <cpu_affinity.h>
#include <pthread.h>
#include <stdio.h>
#include "bench_verify.h"
#include "les.h"
#include "test_list.h"

static void *realtime_verify_all_thread(void *parameter) {
    printf("\n"
           "=============================================================\n"
           "[test-realtime] Verification\n"
           "=============================================================\n");
    freq_verify();
    printf("\n");
    time_verify();
    printf("\n");
    interrupt_stub_verify();
    printf("\n");
    schedule_stub_verify();
    printf("\n");
    cpu_bind_verify();
    printf("=============================================================\n");
    return NULL;
}

void realtime_verify_all(void) {
    thread_initialize();

    pthread_t tid;
    pthread_attr_t attr;
	pthread_attr_init(&attr);

    pthread_attr_setstacksize(&attr, 32768);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    struct sched_param param;
    param.sched_priority = BENCHMARK_HIGH_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    // 创建线程
    if (pthread_create(&tid, &attr, realtime_verify_all_thread, NULL) != 0) {
        perror("Failed to create main thread\n");
        return;
    }

    pthread_join(tid, NULL);

    pthread_attr_destroy(&attr);
}