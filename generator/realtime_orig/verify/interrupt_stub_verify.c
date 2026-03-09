#include <cpu_affinity.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "les.h"
#include "bench_verify.h"

#define TEST_VALID_ITERATIONS 15
#define TEST_LIMIT_ITERATIONS 150
static uint64_t t0s[TEST_LIMIT_ITERATIONS];
static uint64_t t1s[TEST_LIMIT_ITERATIONS];
static uint64_t durs[TEST_LIMIT_ITERATIONS];

static int count_all = 0;
static int count_valid = 0;

static void *thread(void *parameter)
{
	BIND_THREAD_TO_CPU(0);

	while (count_valid < TEST_VALID_ITERATIONS && count_all < TEST_LIMIT_ITERATIONS) {
		LES_interrupt_stub_enable();

		safe_usleep(5000);
		//LES_interrupt_end_stub 桩函数会自动设置结束插桩 

		t0s[count_all] = LES_interrupt_start_val;
		t1s[count_all] = LES_interrupt_end_val;		//执行LES_interrupt_end_stub()时，会停止起始两点的记录

        // 无效数据
        if (t0s[count_all] >= t1s[count_all]) {
		    durs[count_all] = 0;
            count_all = count_all + 1;
		    continue;
		}

		uint64_t dur = cycles_to_ns(t1s[count_all] - t0s[count_all]);
		durs[count_all] = dur;

        count_all = count_all + 1;
        count_valid = count_valid + 1;
	}
	

	return NULL;
}

static void test()
{
    count_all = 0;
    count_valid = 0;

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
    
    pthread_attr_destroy(&attr);

    pthread_join(tid, NULL);

    int count_unexpected = 0;

    for (int i = 0; i < count_all; i++) {
        if (t0s[i] == 0 && t1s[i] > 0) {
            //printf("[Unexpected data] ");
            count_unexpected = count_unexpected + 1;
        }
		//printf("cycles from %llu to %llu, %llu ns", (unsigned long long)t0s[i], (unsigned long long)t1s[i], (unsigned long long)durs[i]);
        //if (durs[i] == 0) {
        //    printf(" (invalid)");
        //}
        //printf("\n");
	}

    printf("[interrupt stub verification]\n"
           "should get %d data in less than %d iterations,\n"
           "get %d data in %d iterations, %d unexpected data is found.\n",
           TEST_VALID_ITERATIONS, TEST_LIMIT_ITERATIONS, count_valid, count_all, count_unexpected);
}



void interrupt_stub_verify(void) {
    test();
}