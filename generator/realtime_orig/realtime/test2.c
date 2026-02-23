/*
 * File: benchmark/realtime/test2/test2.c
 * 项目：中断延迟
 * 插桩：是
 */

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "les.h"

#define TEST_ITERATIONS 500
static uint64_t t0s[TEST_ITERATIONS];
static uint64_t t1s[TEST_ITERATIONS];
static uint64_t durs[TEST_ITERATIONS];

void test2(uint64_t *address1, uint64_t *address2, uint64_t *address3)
{
	for (uint32_t i = 0; i < TEST_ITERATIONS; i++) {
		LES_interrupt_stub_enable();

		/* 内核的时钟中断会频繁触发。*/
		safe_usleep(200);

		t0s[i] = LES_interrupt_start_val;
		t1s[i] = LES_interrupt_end_val;		//执行LES_interrupt_end_stub()时，会停止起始两点的记录
	}
	
	uint64_t max_dur = 0;
	uint64_t min_dur = UINT64_MAX;
	uint64_t total_dur = 0;
	uint32_t valid_count = 0;
	
	for (uint32_t i = 0; i < TEST_ITERATIONS; i++) {
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
		
    *address1 = min_dur;
    *address2 = max_dur;
    *address3 = total_dur / valid_count;
}
