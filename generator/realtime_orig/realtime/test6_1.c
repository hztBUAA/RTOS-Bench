/*
 * File: benchmark/realtime/test6/test6_1.c
 * 场景: 消息接收&消息发送、立即执行
 * 插桩：否
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <mqueue.h>
#include <fcntl.h>

#include "les.h"

#define TEST_ITERATION 1000

static mqd_t mq;
static uint64_t send_total_cycles, receive_total_cycles;

static const char* msg = "Hi";


static void message_test_immd(uint64_t *a, uint64_t *b) {

	static const char *mq_name = "/6_1_queue";
    struct mq_attr attr_m;
    memset(&attr_m, 0, sizeof(attr_m));

    attr_m.mq_flags = 0;
    attr_m.mq_maxmsg = 1;
    attr_m.mq_msgsize = 8;
    attr_m.mq_curmsgs = 0;

	mq_unlink(mq_name);
    mq = mq_open(mq_name, O_CREAT | O_RDWR | O_EXCL, 0644, &attr_m);
    if (mq == (mqd_t)-1) {
        if (errno == EEXIST) {
            printf("报错：队列 '%s' 已存在，无法重新创建。\n", mq_name);
        } else {
            perror("mq_open 发生其他错误\n");
        }
        exit(1);
    }
    
    char buffer[8];

	for (int i = 0; i < TEST_ITERATION; i++) {
		uint64_t t0, t1;
		t0 = timeGet();
		mq_send(mq, msg, strlen(msg) + 1, 0);
		t1 = timeGet();
		
		
		send_total_cycles += t1 - t0;
		
		t0 = timeGet();
		mq_receive(mq, buffer, 8, NULL);
		t1 = timeGet();
		
		receive_total_cycles += t1 - t0;
	}

    
    mq_close(mq);
    mq_unlink(mq_name);
    
    *a = cycles_to_ns(send_total_cycles / TEST_ITERATION);
    *b = cycles_to_ns(receive_total_cycles / TEST_ITERATION);

    return;
}

void test6_1(uint64_t *address1, uint64_t *address2) {
    send_total_cycles = 0;
    receive_total_cycles = 0;
	if (address1 != NULL && address2 != NULL) {
		message_test_immd(address1, address2);
	}
	return;
}
