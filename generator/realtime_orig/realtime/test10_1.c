/*
 * File: benchmark/realtime/test10/test10_1.c
 * 场景: 内存分配&内存释放、立即执行
 * 插桩：否
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "les.h"
#include "test_list.h"

#define TEST_ITERATION 1000

static void malloc_test(uint64_t *a, uint64_t *b) {
	uint64_t t0, t1;
	uint64_t malloc_total_cycles = 0, free_total_cycles = 0;
	int fail_flag = 0;
	
	int *x;
	
	
	for (int i = 0; i < TEST_ITERATION; i++) {
		t0 = timeGet();
		x = (int *)malloc(sizeof(int));
		t1 = timeGet();
		
		if (x == NULL) {
			fail_flag = 1;
			break;
		}
		
		malloc_total_cycles += t1 - t0;
		
		t0 = timeGet();
		free(x);
		t1 = timeGet();
		
		free_total_cycles += t1 - t0;
	}
	
	
	if (fail_flag == 1) {
		*a = 0;
		*b = 0;
	} else {
		*a = cycles_to_ns(malloc_total_cycles / TEST_ITERATION);
		*b = cycles_to_ns(free_total_cycles / TEST_ITERATION);
	}
}

void test10_1(uint64_t *address1, uint64_t *address2) {
	if (address1 != NULL && address2 != NULL) {
		malloc_test(address1, address2);
	}
	return;
}
