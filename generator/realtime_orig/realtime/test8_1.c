/*
 * File: benchmark/realtime/test8/test8_1.c
 * 场景: 互斥锁获取&互斥锁释放、立即执行
 * 插桩：否
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "les.h"
#include "test_list.h"

#define TEST_ITERATION 1000

static pthread_mutex_t lock;

static void mutex_test(uint64_t *a, uint64_t *b) {
	pthread_mutex_init(&lock, NULL);

	uint64_t t0, t1;
	uint64_t lock_total_cycles = 0, unlock_total_cycles = 0;	
	
	for (int i = 0; i < TEST_ITERATION; i++) {
		t0 = timeGet();
		pthread_mutex_lock(&lock);
		t1 = timeGet();
		
		lock_total_cycles += t1 - t0;
		
		t0 = timeGet();
		pthread_mutex_unlock(&lock);
		t1 = timeGet();
		
		unlock_total_cycles += t1 - t0;
	}
	

	*a = cycles_to_ns(lock_total_cycles / TEST_ITERATION);
	*b = cycles_to_ns(unlock_total_cycles / TEST_ITERATION);
	
	pthread_mutex_destroy(&lock);
}

void test8_1(uint64_t *address1, uint64_t *address2) {
	if (address1 != NULL && address2 != NULL) {
		mutex_test(address1, address2);
	}
	return;
}
