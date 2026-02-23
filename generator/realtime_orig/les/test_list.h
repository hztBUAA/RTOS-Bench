/* test_list.h
 * 测试子程序一览
 */

#include <stdint.h>

#ifndef __TEST_LIST_H__
#define __TEST_LIST_H__

void test1(uint64_t*);	// context_switch
void test2(uint64_t*, uint64_t*, uint64_t*);	// interrupt
void test3(uint64_t*, uint64_t*, uint64_t*);	// syscall

void test4_1(uint64_t*, uint64_t*);		//semaphore
void test4_2(uint64_t*, uint64_t*);
void test5_3(uint64_t*);
int test6_0(void);						//mqueue
void test6_1(uint64_t*, uint64_t*);
void test6_2(uint64_t*, uint64_t*);
void test6_3(uint64_t*);
void test6_4(uint64_t*, uint64_t*);
void test7_3(uint64_t*);
void test8_1(uint64_t*, uint64_t*);	//mutex
void test8_2(uint64_t*, uint64_t*);
void test9_3(uint64_t*);
void test10_1(uint64_t*, uint64_t*);	//malloc&free

void test_task_lat(uint64_t*, int);
void test_ipc_bw(uint64_t *address, int mode, int concurrency);
void test_mem_bw(uint64_t *address, int mode, int concurrency);

#endif /* __TEST_LIST_H__ */
