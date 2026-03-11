/*
 * File: benchmark/realtime/test2/test2.c
 * 项目：中断延迟
 * 插桩：是
 */
#include <cpu_affinity.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "les.h"

#define TEST_VALID_ITERATIONS 200
#define TEST_LIMIT_ITERATIONS 1000
static uint64_t t0s[TEST_LIMIT_ITERATIONS];
static uint64_t t1s[TEST_LIMIT_ITERATIONS];
static uint64_t durs[TEST_LIMIT_ITERATIONS];

static volatile uint64_t min, max, avg;

static int count_all = 0;
static int count_valid = 0;

static void *thread(void *parameter)
{
	BIND_THREAD_TO_CPU(0);

	while (count_valid < TEST_VALID_ITERATIONS && count_all < TEST_LIMIT_ITERATIONS) {
		LES_interrupt_stub_enable();

		/* 内核的时钟中断会频繁触发。*/
		safe_usleep(100);
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

void test2(uint64_t *address1, uint64_t *address2, uint64_t *address3)
{
	count_valid = 0;
	count_all = 0;
	
	min = 0;
	max = 0;
	avg = 0;

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
        perror("2:Failed to create thread.");
        return;
    }

    pthread_join(tid, NULL);

	pthread_attr_destroy(&attr);

	if (count_all >= TEST_LIMIT_ITERATIONS) {
		printf("Warning: interrupt latency test only get %d/%d data.\n", count_valid, TEST_VALID_ITERATIONS);
	}

	uint64_t max_dur = 0;
	uint64_t min_dur = UINT64_MAX;
	uint64_t total_dur = 0;
	uint32_t valid_count = 0;
	
	for (int i = 0; i < count_all; i++) {
		if (t0s[i] >= t1s[i]) {
		    durs[i] = 0;
		    continue;
		}

		uint64_t dur = cycles_to_ns(t1s[i] - t0s[i]);
		durs[i] = dur;
		
		if (i == 0) continue;
		
		if (dur > 0) {
			if (dur > max_dur) max_dur = dur;
			if (dur < min_dur) min_dur = dur;
			total_dur += dur;
			valid_count++;
		}
	}
	
	if (valid_count > 0) {
		min = min_dur;
		max = max_dur;
		avg = total_dur / valid_count;
	} else {
		min = 0;
		max = 0;
		avg = 0;
	}
    

    *address1 = min;
    *address2 = max;
    *address3 = avg;
}