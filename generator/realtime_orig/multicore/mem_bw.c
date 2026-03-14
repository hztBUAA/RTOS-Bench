#include <cpu_affinity.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>

#include "les.h"


#define TEST_MEM_SIZE       (2 * 1024 * 1024)      /* 每个线程 2MB 缓冲区 */
#define TEST_REPETITION     500                   /* 重复次数 */
#define MAX_WORKERS         8                      /* 最大线程数支持 */
#define NUM                 (TEST_MEM_SIZE / sizeof(int))

/* 定义模式枚举，对应 0-7 */
enum mem_mode {
    MODE_RD = 0,    // 间隔读
    MODE_WR,        // 间隔写
    MODE_CP,        // 间隔拷贝
    MODE_FRD,       // 连续读
    MODE_FWR,       // 连续写
    MODE_FCP,       // 连续拷贝
    MODE_MSET,      // 标准 memset
    MODE_MCPY       // 标准 memcpy
};

static sem_t sem_order[MAX_WORKERS];
static sem_t sem_response[MAX_WORKERS];
static pthread_t worker_tids[MAX_WORKERS];
static int *test_buf_src = NULL;
static int *test_buf_dst[MAX_WORKERS];
static int current_mode; /* 全局存储当前的模式索引 */

/* 防止编译器优化的辅助变量 */
static volatile unsigned long long use_result_dummy = 0;
static void use_int(int result) { use_result_dummy += result; }

/* --- 各种内存测试负载函数 --- */

static void mem_rd(int *buf) {
    int i, sum = 0, *p, *lastone = buf + NUM;
    for (i = 0; i < TEST_REPETITION; i++) {
        p = buf;
        while (p < lastone) {
            #define RD_DO(n) p[n] +
            sum += RD_DO(0) RD_DO(8) RD_DO(16) RD_DO(24) RD_DO(32) RD_DO(40) RD_DO(48) RD_DO(56)
                   RD_DO(64) RD_DO(72) RD_DO(80) RD_DO(88) RD_DO(96) RD_DO(104) RD_DO(112) p[120];
            #undef RD_DO
            p += 128;
        }
    }
    use_int(sum);
}

static void mem_wr(int *buf) {
    int i, *p, *lastone = buf + NUM;
    for (i = 0; i < TEST_REPETITION; i++) {
        p = buf;
        while (p < lastone) {
            #define WR_DO(n) p[n]=1;
            WR_DO(0) WR_DO(8) WR_DO(16) WR_DO(24) WR_DO(32) WR_DO(40) WR_DO(48) WR_DO(56)
            WR_DO(64) WR_DO(72) WR_DO(80) WR_DO(88) WR_DO(96) WR_DO(104) WR_DO(112) WR_DO(120)
            #undef WR_DO
            p += 128;
        }
    }
}

static void mem_cp(int *dst, int *src) {
    int i, *pd, *ps, *lastone = dst + NUM;
    for (i = 0; i < TEST_REPETITION; i++) {
        pd = dst; ps = src;
        while (pd < lastone) {
            #define CP_DO(n) pd[n]=ps[n];
            CP_DO(0) CP_DO(8) CP_DO(16) CP_DO(24) CP_DO(32) CP_DO(40) CP_DO(48) CP_DO(56)
            CP_DO(64) CP_DO(72) CP_DO(80) CP_DO(88) CP_DO(96) CP_DO(104) CP_DO(112) CP_DO(120)
            #undef CP_DO
            pd += 128; ps += 128;
        }
    }
}

static void mem_frd(int *buf) {
    int i, sum = 0, *p, *lastone = buf + NUM;
    for (i = 0; i < TEST_REPETITION; i++) {
        p = buf;
        while (p < lastone) {
            #define DOIT(n) p[n] +
            sum += 
            DOIT(0) DOIT(1) DOIT(2) DOIT(3) DOIT(4) DOIT(5) DOIT(6) DOIT(7)
			DOIT(8) DOIT(9) DOIT(10) DOIT(11) DOIT(12) DOIT(13) DOIT(14) DOIT(15)
			DOIT(16) DOIT(17) DOIT(18) DOIT(19) DOIT(20) DOIT(21) DOIT(22) DOIT(23)
			DOIT(24) DOIT(25) DOIT(26) DOIT(27) DOIT(28) DOIT(29) DOIT(30) DOIT(31)
			DOIT(32) DOIT(33) DOIT(34) DOIT(35) DOIT(36) DOIT(37) DOIT(38) DOIT(39)
			DOIT(40) DOIT(41) DOIT(42) DOIT(43) DOIT(44) DOIT(45) DOIT(46) DOIT(47)
			DOIT(48) DOIT(49) DOIT(50) DOIT(51) DOIT(52) DOIT(53) DOIT(54) DOIT(55)
			DOIT(56) DOIT(57) DOIT(58) DOIT(59) DOIT(60) DOIT(61) DOIT(62) DOIT(63)
			DOIT(64) DOIT(65) DOIT(66) DOIT(67) DOIT(68) DOIT(69) DOIT(70) DOIT(71)
			DOIT(72) DOIT(73) DOIT(74) DOIT(75) DOIT(76) DOIT(77) DOIT(78) DOIT(79)
			DOIT(80) DOIT(81) DOIT(82) DOIT(83) DOIT(84) DOIT(85) DOIT(86) DOIT(87)
			DOIT(88) DOIT(89) DOIT(90) DOIT(91) DOIT(92) DOIT(93) DOIT(94) DOIT(95)
			DOIT(96) DOIT(97) DOIT(98) DOIT(99) DOIT(100) DOIT(101) DOIT(102)
			DOIT(103) DOIT(104) DOIT(105) DOIT(106) DOIT(107) DOIT(108) DOIT(109)
			DOIT(110) DOIT(111) DOIT(112) DOIT(113) DOIT(114) DOIT(115) DOIT(116)
			DOIT(117) DOIT(118) DOIT(119) DOIT(120) DOIT(121) DOIT(122) DOIT(123)
			DOIT(124) DOIT(125) DOIT(126) DOIT(127) 1;
            #undef DOIT
            p += 128;
        }
    }
    use_int(sum);
}

static void mem_fwr(int *buf) {
    int i, *p, *lastone = buf + NUM;
    for (i = 0; i < TEST_REPETITION; i++) {
        p = buf;
        while (p < lastone) {
            #define DOIT(n) p[n]=1;
            DOIT(0) DOIT(1) DOIT(2) DOIT(3) DOIT(4) DOIT(5) DOIT(6) DOIT(7)
			DOIT(8) DOIT(9) DOIT(10) DOIT(11) DOIT(12) DOIT(13) DOIT(14) DOIT(15)
			DOIT(16) DOIT(17) DOIT(18) DOIT(19) DOIT(20) DOIT(21) DOIT(22) DOIT(23)
			DOIT(24) DOIT(25) DOIT(26) DOIT(27) DOIT(28) DOIT(29) DOIT(30) DOIT(31)
			DOIT(32) DOIT(33) DOIT(34) DOIT(35) DOIT(36) DOIT(37) DOIT(38) DOIT(39)
			DOIT(40) DOIT(41) DOIT(42) DOIT(43) DOIT(44) DOIT(45) DOIT(46) DOIT(47)
			DOIT(48) DOIT(49) DOIT(50) DOIT(51) DOIT(52) DOIT(53) DOIT(54) DOIT(55)
			DOIT(56) DOIT(57) DOIT(58) DOIT(59) DOIT(60) DOIT(61) DOIT(62) DOIT(63)
			DOIT(64) DOIT(65) DOIT(66) DOIT(67) DOIT(68) DOIT(69) DOIT(70) DOIT(71)
			DOIT(72) DOIT(73) DOIT(74) DOIT(75) DOIT(76) DOIT(77) DOIT(78) DOIT(79)
			DOIT(80) DOIT(81) DOIT(82) DOIT(83) DOIT(84) DOIT(85) DOIT(86) DOIT(87)
			DOIT(88) DOIT(89) DOIT(90) DOIT(91) DOIT(92) DOIT(93) DOIT(94) DOIT(95)
			DOIT(96) DOIT(97) DOIT(98) DOIT(99) DOIT(100) DOIT(101) DOIT(102)
			DOIT(103) DOIT(104) DOIT(105) DOIT(106) DOIT(107) DOIT(108) DOIT(109)
			DOIT(110) DOIT(111) DOIT(112) DOIT(113) DOIT(114) DOIT(115) DOIT(116)
			DOIT(117) DOIT(118) DOIT(119) DOIT(120) DOIT(121) DOIT(122) DOIT(123)
			DOIT(124) DOIT(125) DOIT(126) DOIT(127)
            #undef DOIT
            p += 128;
        }
    }
}

static void mem_fcp(int *dst, int *src) {
    int i, *pd, *ps, *lastone = dst + NUM;
    for (i = 0; i < TEST_REPETITION; i++) {
        pd = dst; ps = src;
        while (pd < lastone) {
            #define DOIT(n) pd[n]=ps[n];
            DOIT(0) DOIT(1) DOIT(2) DOIT(3) DOIT(4) DOIT(5) DOIT(6) DOIT(7)
			DOIT(8) DOIT(9) DOIT(10) DOIT(11) DOIT(12) DOIT(13) DOIT(14) DOIT(15)
			DOIT(16) DOIT(17) DOIT(18) DOIT(19) DOIT(20) DOIT(21) DOIT(22) DOIT(23)
			DOIT(24) DOIT(25) DOIT(26) DOIT(27) DOIT(28) DOIT(29) DOIT(30) DOIT(31)
			DOIT(32) DOIT(33) DOIT(34) DOIT(35) DOIT(36) DOIT(37) DOIT(38) DOIT(39)
			DOIT(40) DOIT(41) DOIT(42) DOIT(43) DOIT(44) DOIT(45) DOIT(46) DOIT(47)
			DOIT(48) DOIT(49) DOIT(50) DOIT(51) DOIT(52) DOIT(53) DOIT(54) DOIT(55)
			DOIT(56) DOIT(57) DOIT(58) DOIT(59) DOIT(60) DOIT(61) DOIT(62) DOIT(63)
			DOIT(64) DOIT(65) DOIT(66) DOIT(67) DOIT(68) DOIT(69) DOIT(70) DOIT(71)
			DOIT(72) DOIT(73) DOIT(74) DOIT(75) DOIT(76) DOIT(77) DOIT(78) DOIT(79)
			DOIT(80) DOIT(81) DOIT(82) DOIT(83) DOIT(84) DOIT(85) DOIT(86) DOIT(87)
			DOIT(88) DOIT(89) DOIT(90) DOIT(91) DOIT(92) DOIT(93) DOIT(94) DOIT(95)
			DOIT(96) DOIT(97) DOIT(98) DOIT(99) DOIT(100) DOIT(101) DOIT(102)
			DOIT(103) DOIT(104) DOIT(105) DOIT(106) DOIT(107) DOIT(108) DOIT(109)
			DOIT(110) DOIT(111) DOIT(112) DOIT(113) DOIT(114) DOIT(115) DOIT(116)
			DOIT(117) DOIT(118) DOIT(119) DOIT(120) DOIT(121) DOIT(122) DOIT(123)
			DOIT(124) DOIT(125) DOIT(126) DOIT(127)
            #undef DOIT
            pd += 128; ps += 128;
        }
    }
}

static void mem_mset(int *buf) {
    for (int i = 0; i < TEST_REPETITION; i++) memset(buf, 0, TEST_MEM_SIZE);
}

static void mem_mcpy(int *dst, int *src) {
    for (int i = 0; i < TEST_REPETITION; i++) memcpy(dst, src, TEST_MEM_SIZE);
}

static void *mem_worker_entry(void *parameter) {
    int number = (int)(long)parameter;
    BIND_THREAD_TO_CPU(number % USE_PROCESSORS);

    test_buf_dst[number] = (int *)malloc(TEST_MEM_SIZE);
    if (!test_buf_dst[number]) {
        sem_post(&sem_response[number]);
        return NULL;
    }

    sem_post(&sem_response[number]); // 就绪通知
    sem_wait(&sem_order[number]);    // 等待开始指令

    switch (current_mode) {
        case MODE_RD:   mem_rd(test_buf_src); break;
        case MODE_WR:   mem_wr(test_buf_dst[number]); break;
        case MODE_CP:   mem_cp(test_buf_dst[number], test_buf_src); break;
        case MODE_FRD:  mem_frd(test_buf_src); break;
        case MODE_FWR:  mem_fwr(test_buf_dst[number]); break;
        case MODE_FCP:  mem_fcp(test_buf_dst[number], test_buf_src); break;
        case MODE_MSET: mem_mset(test_buf_dst[number]); break;
        case MODE_MCPY: mem_mcpy(test_buf_dst[number], test_buf_src); break;
        default: break;
    }

    sem_post(&sem_response[number]); // 完成通知
    return NULL;
}

/* --- 主测试函数 --- */

/**
 * mode: 0-7 对应的测试模式
 * number: 线程数量
 */
uint64_t multicore_mem_bw(int mode, int number) {
    int i, worker_count;
    if (mode < 0 || mode > 7 || number <= 0) return 0;

    current_mode = mode;
    worker_count = (number > MAX_WORKERS) ? MAX_WORKERS : number;

    test_buf_src = (int *)malloc(TEST_MEM_SIZE);
    if (test_buf_src) memset(test_buf_src, 0x55, TEST_MEM_SIZE);
    
    pthread_attr_t attr;
    struct sched_param param;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 32768);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = BENCHMARK_MIDDLE_PRIO;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    for (i = 0; i < worker_count; i++) {
        sem_init(&sem_order[i], 0, 0);
        sem_init(&sem_response[i], 0, 0);
        pthread_create(&worker_tids[i], &attr, mem_worker_entry, (void *)(long)i);
    }

    /* 等待就绪 -> 开始计时 -> 触发指令 */
    for (i = 0; i < worker_count; i++) sem_wait(&sem_response[i]);
    uint64_t t_start = timeGet();
    for (i = 0; i < worker_count; i++) sem_post(&sem_order[i]);

    /* 等待完成 -> 停止计时 */
    for (i = 0; i < worker_count; i++) sem_wait(&sem_response[i]);
    uint64_t t_end = timeGet();

    /* 资源清理 */
    for (i = 0; i < worker_count; i++) {
        pthread_join(worker_tids[i], NULL);
        sem_destroy(&sem_order[i]);
        sem_destroy(&sem_response[i]);
        if (test_buf_dst[i]) free(test_buf_dst[i]);
    }
    if (test_buf_src) free(test_buf_src);

    pthread_attr_destroy(&attr);
    
    uint64_t dur = cycles_to_ns(t_end - t_start);
    uint64_t data_per_worker;

    if (current_mode == MODE_RD || current_mode == MODE_WR) {
        data_per_worker = TEST_MEM_SIZE / 8; // 间隔读写仅1/8有效
    } else if (current_mode == MODE_CP) {
        data_per_worker = (TEST_MEM_SIZE / 8) * 2; // 读+写
    } else if (current_mode == MODE_FCP || current_mode == MODE_MCPY) {
        data_per_worker = (uint64_t)TEST_MEM_SIZE * 2; // 连续拷贝包含读和写
    } else {
        data_per_worker = (uint64_t)TEST_MEM_SIZE;
    }

    uint64_t total_bytes = data_per_worker * TEST_REPETITION * worker_count;
    uint64_t bandwidth_mb_s = total_bytes * 1000000000ULL / (dur * 1048576ULL);

    return bandwidth_mb_s;
}

void test_mem_bw(uint64_t *address, int mode, int concurrency) {
	int worker_count = (concurrency > MAX_WORKERS) ? MAX_WORKERS : concurrency;
    *address = multicore_mem_bw(mode, worker_count);
}
