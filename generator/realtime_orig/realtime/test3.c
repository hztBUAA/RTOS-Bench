/*
 * File: benchmark/realtime/test12/test12_1.c
 * 测试项: 内存分配&内存释放、立即执行
 * 插桩：否
 */

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "les.h"

#define TEST_ITERATION 1000

static int getpid_temp_alternative() {
	LES_syscall_stub();
	return 0;
}

void test3(uint64_t *address1, uint64_t *address2, uint64_t *address3) {
	uint64_t t0, t1;
	uint64_t max_dur = 0;
	uint64_t min_dur = UINT64_MAX;
	uint64_t total_dur = 0;
	uint32_t valid_count = 0;
	
	int fail_flag = 0;
	
	for (int i = 0; i < TEST_ITERATION; i++) {
		LES_syscall_enable();
		if (LES_NO_GETPID) {
			t0 = timeGet();
			getpid_temp_alternative();
			//printf("使用默认函数测量\n");
		} else {		
			t0 = timeGet();
			getpid();
			//printf("使用系统函数测量\n");
		}
		LES_syscall_disable();
		t1 = LES_get_syscall_val();
		
		if (t1 == 0) {
			fail_flag = 1;
			LES_syscall_enable();
			t0 = timeGet();
			getpid_temp_alternative();
			LES_syscall_disable();
			t1 = LES_get_syscall_val();
		}
		
		uint64_t dur = cycles_to_ns(t1 - t0);
		
		if (i == 0) continue;
		
		if (dur > 0) {
			if (dur > max_dur) max_dur = dur;
			if (dur < min_dur) min_dur = dur;
			total_dur += dur;
			valid_count++;
		}
	}
	
	if (fail_flag == 1) {
		//printf("系统调用插桩点未能成功获取值，使用默认函数测量\n");
	}
	
	*address1 = min_dur;
    *address2 = max_dur;
    *address3 = total_dur / valid_count;
}
