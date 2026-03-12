#include "platform_macro.h"
#include <inttypes.h>
#include <stdio.h>
#include "les.h"
#include "test_list.h"

#include "bench_verify.h"

#if defined(RUIHUA_PLATFORM)
#include "reworks_int.h"
extern int pthread_switch_hook_add(void(*)(thread_t, thread_t));
#endif


#define STR_BUFFER_LENGTH 32

static char buf[STR_BUFFER_LENGTH];

/* 储存结果 */
static uint64_t realtime_service_cost[8][4];	// ...
static int message_queue_filled_behavior;		// 0 = stall or time wait, !0 = return error
static uint64_t realtime_interrupt[3];			// [0]:Min, [1]:Max, [2]:Avg
static uint64_t realtime_context_switch;		// Avg
static uint64_t realtime_syscall[3];					// [0]:Min, [1]:Max, [2]:Avg
static uint64_t multicore_memory_bandwidth[8][4];		// [0][]-[7][]:rd/wr/cp/frd/fwr/fcp/memset/memcpy, [][0]-[][3]:concurrency level 1/2/4/8
static uint64_t multicore_ipc_bandwidth[4];				// ipc, [][0]-[][3]:concurrency level 1/2/4/8
static uint64_t multicore_intra_inter_bandwidth[2];		// [0]:intra-core, [1]:inter-core
static uint64_t multicore_init_dlt_latency[4];			// init&delete, [0]-[3]:concurrency level 1/2/4/8

/* 辅助输出 */
static const int has_check[8][4] = {
    {1, 1, 0, 0}, // 信号量获取
    {1, 0, 1, 1}, // 信号量释放
    {1, 1, 1, 1}, // 消息发送
    {1, 1, 1, 1}, // 消息接收
    {1, 1, 0, 0}, // 互斥锁获取
    {1, 0, 1, 1}, // 互斥锁释放
    {1, 0, 0, 0}, // 内存块申请
    {1, 0, 0, 0}  // 内存块释放
};

static const char *row_names_realtime[8] = {
    "信号量获取", "信号量释放",
    "消息发送  ", "消息接收  ",
    "互斥锁获取", "互斥锁释放",
    "内存块申请", "内存块释放"
};

static const char *row_names_multicore[8] = {
    "rd" ,"wr" ,"cp" ,
    "frd","fwr","fcp",
    "memset", "memcpy",
};

static void realtime_init(void) {
	LES_start_timer();

	test1(&realtime_context_switch);		printf("Finish test 1.\n");

	test2(&realtime_interrupt[0], &realtime_interrupt[1], &realtime_interrupt[2]);		printf("Finish test 2.\n");

	test3(&realtime_syscall[0], &realtime_syscall[1], &realtime_syscall[2]);		printf("Finish test 3.\n");

	test4_1(&realtime_service_cost[0][0], &realtime_service_cost[1][0]);		printf("Finish test 4_1, 4_1.\n");

	test4_2(&realtime_service_cost[0][1], &realtime_service_cost[1][3]);		printf("Finish test 4_2, 5_4.\n");

	test5_3(&realtime_service_cost[1][2]);		printf("Finish test 5_3.\n");

	test6_1(&realtime_service_cost[2][0], &realtime_service_cost[3][0]);		printf("Finish test 6_1, 7_1.\n");

	test6_3(&realtime_service_cost[2][2]);		printf("Finish test 6_3.\n");

	test6_4(&realtime_service_cost[2][3], &realtime_service_cost[3][1]);
	printf("Finish test 6_4, 7_2.\n");

	/* Test 6_0: check mqueue blocking behavior (informational only) */
	message_queue_filled_behavior = test6_0();
	if (message_queue_filled_behavior == 0) {
		printf("mqueue supports blocking on full queue.\n");
        test6_2(&realtime_service_cost[2][1], &realtime_service_cost[3][3]);
        printf("Finish test 6_2, 7_4.\n");
        test7_3(&realtime_service_cost[3][2]);
        printf("Finish test 7_3.\n");
	} else {
		printf("mqueue does NOT block on full queue.\n"
               "Skip test 6_2, 7_3, 7_4.\n");
	}
    
	test8_1(&realtime_service_cost[4][0], &realtime_service_cost[5][0]);
	printf("Finish test 8_1, 9_1.\n");

	test8_2(&realtime_service_cost[4][1], &realtime_service_cost[5][3]);
	printf("Finish test 8_2, 9_4.\n");

	test9_3(&realtime_service_cost[5][2]);
	printf("Finish test 9_3.\n");

	test10_1(&realtime_service_cost[6][0], &realtime_service_cost[7][0]);
	printf("Finish test 10_1, 11_1.\n");

}

static void multicore_init(void) {
	LES_start_timer();
	
	for (int i = 0; i < 4; i++) {
		test_ipc_bw(&multicore_ipc_bandwidth[i], 2, (1<<i));
		printf("end ipc_bw task inter core %d\n", (1<<i));
	}
	test_ipc_bw(&multicore_intra_inter_bandwidth[0], 0, 1);		printf("end ipc_bw task on same core\n");
	test_ipc_bw(&multicore_intra_inter_bandwidth[1], 1, 1);		printf("end ipc_bw task on different core\n");
	
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 4; j++) {
			test_mem_bw(&multicore_memory_bandwidth[i][j], i, (1<<j));
			printf("end mem_bw task %s %d\n", row_names_multicore[i], (1<<j));
		}
	}
	
	test_task_lat(&multicore_init_dlt_latency[0], 1);
	test_task_lat(&multicore_init_dlt_latency[1], 2);
	test_task_lat(&multicore_init_dlt_latency[2], 4);
	test_task_lat(&multicore_init_dlt_latency[3], 8);
	
	return;
}

static void div1000_print(uint64_t full_n_data, char* buf) {
    uint64_t u_xx = full_n_data / 1000;
    uint64_t n_xx = full_n_data % 1000;

    snprintf(buf, STR_BUFFER_LENGTH, "%llu" ".%03llu", (unsigned long long)u_xx, (unsigned long long)n_xx);
}

static void realtime_print(void) {
    // 打印系统服务实时指标realtime_service_cost
    printf("系统延迟（单位：us）:\n");
    printf("%-12s | %-14s | %-14s | %-14s | %-14s\n", "指标", "立即执行", "挂起睡眠", "低优就绪", "高优恢复");
    printf("----------------------------------------------------------------------\n");

    for (int i = 0; i < 8; i++) {
        printf("%-12s", row_names_realtime[i]);
        
        for (int j = 0; j < 4; j++) {
            printf(" | ");
            if ((i == 2 && j == 1) || (i == 3 && j == 2) || (i == 3 && j == 3)) {
            	if (message_queue_filled_behavior != 0) {
            		printf("%-10s", "-");
            		continue;
            	}
            }
            if (has_check[i][j]) {
                div1000_print(realtime_service_cost[i][j], buf);
                printf("%-10s", buf);
            } else {
                printf("%-10s", " "); 
            }
        }
        printf("\n");
    }
    
    // 打印系统延迟context_switch、interrupt、syscall
    printf("上下文切换延迟    AVG: ");
    div1000_print(realtime_context_switch, buf);
    printf("%s\n", buf);

	printf("中断软件延迟    MIN: ");
    div1000_print(realtime_interrupt[0], buf);
    printf("%s", buf);
    printf("  MAX: ");
    div1000_print(realtime_interrupt[1], buf);
    printf("%s", buf);
    printf("  AVG: ");
    div1000_print(realtime_interrupt[2], buf);
    printf("%s\n", buf);
    
    if (!LES_NO_GETPID) {
        printf("系统调用延迟    MIN: ");
        div1000_print(realtime_syscall[0], buf);
        printf("%s", buf);
        printf("  MAX: ");
        div1000_print(realtime_syscall[1], buf);
        printf("%s", buf);
        printf("  AVG: ");
        div1000_print(realtime_syscall[2], buf);
        printf("%s\n", buf);
    }

	printf("\n");
}

static void multicore_print(void) {
	
    // 打印多核指标memory_bandwidth、multicore_ipc_bandwidth、multicore_intra_inter_bandwidth、multicore_init_dlt_latency
    printf("多核存取性能:\n");
	printf("%-13s | %-10s | %-10s | %-10s | %-10s\n", "并发度", "1", "2", "4", "8");
    printf("----------------------------------------------------------------------\n");
    for (int i = 0; i < 8; i++) {
        printf("%-10s", row_names_multicore[i]);
        
        for (int j = 0; j < 4; j++) {
            printf(" | ");
            div1000_print(multicore_memory_bandwidth[i][j], buf);
            printf("%-10s", buf);
        }
        printf("\n");
    }
    
    printf("多核系统服务:\n");
    printf("%-13s | %-10s | %-10s | %-10s | %-10s\n", "并发度", "1", "2", "4", "8");
    
    printf("任务间通信");
    for (int j = 0; j < 4; j++) {
        printf(" | ");
        div1000_print(multicore_ipc_bandwidth[j], buf);
        printf("%-10s", buf);
    }
    printf("\n");
    
    printf("创建与删除");
    for (int j = 0; j < 4; j++) {
        printf(" | ");
        div1000_print(multicore_init_dlt_latency[j], buf);
        printf("%-10s", buf);
    }
    printf("（单位为us）\n");
    printf("任务在同核与异核上通信比较: \n");
    printf("同核通信: ");
    div1000_print(multicore_intra_inter_bandwidth[0], buf);
    printf("%s\n", buf);
    printf("异核通信: ");
    div1000_print(multicore_intra_inter_bandwidth[1], buf);
    printf("%s\n", buf);
    
    printf("（除特殊说明外，上述单位均为GB/s）\n");
}


/* =========================================================================
 * Public API for test_realtime wrapper
 * ========================================================================= */

/**
 * @brief Run all realtime performance tests
 * @return 0 on success
 */
int realtime_benchmark_run(void)
{
#if defined(RUIHUA_PLATFORM)
	pthread_switch_hook_add(task_switch_hook);
#endif

    realtime_init();
    realtime_print();
    return 0;
}

/**
 * @brief Run multicore performance tests
 * @return 0 on success
 */
int realtime_benchmark_run_multicore(void)
{
#if defined(RUIHUA_PLATFORM)
	pthread_switch_hook_add(task_switch_hook);
#endif

    multicore_init();
    multicore_print();
    return 0;
}

/**
 * @brief Run both realtime and multicore tests
 * @param run_multicore 1 to include multicore tests, 0 for realtime only
 * @return 0 on success
 */
int realtime_benchmark_run_all(int run_multicore)
{
#if defined(RUIHUA_PLATFORM)
	pthread_switch_hook_add(task_switch_hook);
#endif

    realtime_init();
    realtime_print();

    if (run_multicore) {
        multicore_init();
        multicore_print();
    }

    return 0;
}

/* =========================================================================
 * Getter functions to expose measured data for result collection
 * Values are raw nanoseconds as stored by the benchmark tests.
 * ========================================================================= */

uint64_t *get_realtime_service_cost(void)
{
    return &realtime_service_cost[0][0];
}

uint64_t *get_realtime_interrupt(void)
{
    return realtime_interrupt;
}

uint64_t get_realtime_context_switch(void)
{
    return realtime_context_switch;
}

uint64_t *get_realtime_syscall(void)
{
    return realtime_syscall;
}

uint64_t *get_multicore_memory_bandwidth(void)
{
    return &multicore_memory_bandwidth[0][0];
}

uint64_t *get_multicore_ipc_bandwidth(void)
{
    return multicore_ipc_bandwidth;
}

uint64_t *get_multicore_intra_inter_bandwidth(void)
{
    return multicore_intra_inter_bandwidth;
}

uint64_t *get_multicore_init_dlt_latency(void)
{
    return multicore_init_dlt_latency;
}
