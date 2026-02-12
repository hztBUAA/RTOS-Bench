/*
 * File: benchmark/realtime/test4/test4_1.c
 * 场景: 信号量获取&信号量释放、立即执行
 * 插桩：否
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include "les.h"
#include "test_list.h"

#define TEST_ITERATION 1000

static sem_t sem;

static void malloc_test(uint64_t *a, uint64_t *b) {
	sem_init(&sem, 0, 0);

	uint64_t t0, t1;
	uint64_t wait_total_cycles = 0, post_total_cycles = 0;
	
	for (int i = 0; i < TEST_ITERATION; i++) {
		t0 = timeGet();
		sem_post(&sem);
		t1 = timeGet();
		
		wait_total_cycles += t1 - t0;
		
		t0 = timeGet();
		sem_wait(&sem);
		t1 = timeGet();
		
		post_total_cycles += t1 - t0;
	}
	
	sem_destroy(&sem);

	*a = cycles_to_ns(wait_total_cycles / TEST_ITERATION);
	*b = cycles_to_ns(post_total_cycles / TEST_ITERATION);
}

void test4_1(uint64_t *address1, uint64_t *address2) {
	if (address1 != NULL && address2 != NULL) {
		malloc_test(address1, address2);
	}
	return;
}
