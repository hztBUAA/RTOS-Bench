/**
 * @file test_realtime.c
 * @brief Realtime performance test implementation for RTOS-Bench
 * @details Ported from "基准测试程序（实时性能、统一调用）" package.
 *
 * This implementation follows the original test logic. Tests requiring kernel
 * instrumentation (LES) will measure total API call time instead of pure
 * scheduler overhead when instrumentation is not available.
 *
 * Test mapping:
 * - test1: Context switch latency
 * - test2: Interrupt latency (requires kernel instrumentation)
 * - test3: Syscall latency (requires kernel instrumentation)
 * - test4_1: Semaphore take/give (immediate)
 * - test4_2, test5_4: Semaphore take (suspend/wakeup), give (high prio resume)
 * - test5_3: Semaphore give (low prio ready)
 * - test6_1, test7_1: Message send/receive (immediate)
 * - test6_2, test7_4: Message send (suspend), receive (high prio resume)
 * - test6_3: Message send (low prio ready)
 * - test6_4, test7_2: Message send (high prio resume), receive (suspend)
 * - test7_3: Message receive (low prio ready)
 * - test8_1, test9_1: Mutex lock/unlock (immediate)
 * - test8_2, test9_4: Mutex lock (suspend), unlock (high prio resume)
 * - test9_3: Mutex unlock (low prio ready)
 * - test10_1, test11_1: Memory alloc/free
 *
 * Multicore tests (multicore_init):
 * - test_ipc_bw: IPC bandwidth between cores
 * - test_mem_bw: Memory bandwidth with various patterns
 * - test_task_lat: Task create/delete latency
 */

#include "test_realtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <fcntl.h>

/* Platform-specific includes and abstractions */
#ifdef RT_THREAD_PLATFORM
#include <rtthread.h>
#include <pthread.h>
#include <semaphore.h>
#include <mqueue.h>
#define rt_printf rt_kprintf

/* Priority levels - use POSIX style (higher number = higher priority)
 * RT-Thread's POSIX layer will convert these internally */
#define BENCHMARK_HIGH_PRIO   14
#define BENCHMARK_MIDDLE_PRIO 15
#define BENCHMARK_LOW_PRIO    16

#else /* POSIX platforms */

#include <pthread.h>
#include <semaphore.h>
#include <mqueue.h>
#include <sched.h>
#include <unistd.h>
#define rt_printf printf

/* Priority levels for POSIX (higher number = higher priority) */
#define BENCHMARK_HIGH_PRIO   14
#define BENCHMARK_MIDDLE_PRIO 15
#define BENCHMARK_LOW_PRIO    16

#endif

/* Test iteration count */
#define TEST_ITERATION 1000

/* ============================================================================
 * Multicore Test Configuration
 * ============================================================================ */

/* Number of processors to use for multicore tests */
#ifndef USE_PROCESSORS
#define USE_PROCESSORS 2
#endif

/* Multicore test parameters */
#define MULTICORE_TASK_REPETITION    500
#define MULTICORE_IPC_REPETITION     1000
#define MULTICORE_MEM_SIZE           (2 * 1024 * 1024)  /* 2MB per thread */
#define MULTICORE_MEM_REPETITION     1024
#define MULTICORE_IPC_MESSAGE_SIZE   (32 * 1024)        /* 32KB messages */
#define MULTICORE_MAX_WORKERS        8

/* CPU affinity compatibility layer */
#ifdef RT_THREAD_PLATFORM
/* RT-Thread: Use custom implementation since cpu_set_t may not be available */
typedef unsigned long multicore_cpu_set_t;
#define MULTICORE_CPU_ZERO(cpusetp)       (*(cpusetp) = 0)
#define MULTICORE_CPU_SET(cpu, cpusetp)   (*(cpusetp) |= (1UL << (cpu)))
#define MULTICORE_CPU_ISSET(cpu, cpusetp) (*(cpusetp) & (1UL << (cpu)))

static inline int multicore_pthread_setaffinity_np(pthread_t thread,
                                                    size_t cpusetsize,
                                                    const multicore_cpu_set_t *cpuset)
{
	(void)thread;
	(void)cpusetsize;
	(void)cpuset;
	/* RT-Thread: CPU affinity may require rt_thread_control with RT_THREAD_CTRL_BIND_CPU */
	return 0;
}
#else /* POSIX platforms (Linux, etc.) */
/* Use simple no-op implementation to avoid _GNU_SOURCE requirement */
typedef unsigned long multicore_cpu_set_t;
#define MULTICORE_CPU_ZERO(cpusetp)       (*(cpusetp) = 0)
#define MULTICORE_CPU_SET(cpu, cpusetp)   (*(cpusetp) |= (1UL << (cpu)))
#define MULTICORE_CPU_ISSET(cpu, cpusetp) (*(cpusetp) & (1UL << (cpu)))

static inline int multicore_pthread_setaffinity_np(pthread_t thread,
                                                    size_t cpusetsize,
                                                    const multicore_cpu_set_t *cpuset)
{
	(void)thread;
	(void)cpusetsize;
	(void)cpuset;
	/* No-op: CPU affinity not used in portable mode */
	return 0;
}
#endif

/* ============================================================================
 * Static Result Storage
 * ============================================================================ */

static struct test_realtime_result s_result;

/* Message queue filled behavior: 0 = can block, non-0 = returns error */
static int message_queue_filled_behavior = 0;

/* ============================================================================
 * Timer Implementation (from les.h)
 * ============================================================================ */

void realtime_timer_start(void)
{
	/* Most architectures have always-running timers, no action needed */
}

uint64_t realtime_freq_get(void)
{
	uint64_t freq = 0;

#if defined(__aarch64__)
	__asm__ volatile("mrs %0, cntfrq_el0" : "=r"(freq));
#elif defined(__x86_64__) || defined(_M_X64)
	freq = 1000000000ULL;
#elif defined(__riscv)
	freq = 10000000ULL;
#elif defined(__loongarch__)
	uint32_t val;
	__asm__ volatile("cpucfg %0, %1" : "=r"(val) : "r"(0x2));
	freq = (uint64_t)val;
#else
	freq = 1000000000ULL;
#endif

	return freq;
}

uint64_t realtime_time_get(void)
{
	uint64_t val = 0;

#if defined(__aarch64__)
	__asm__ volatile("mrs %0, cntpct_el0" : "=r"(val));
#elif defined(__x86_64__) || defined(_M_X64)
	uint32_t low, high;
	__asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
	val = ((uint64_t)high << 32) | low;
#elif defined(__riscv)
	#if __riscv_xlen == 64
		__asm__ volatile("rdtime %0" : "=r"(val));
	#else
		uint32_t low, high, temp;
		do {
			__asm__ volatile("rdtimeh %0" : "=r"(high));
			__asm__ volatile("rdtime %0" : "=r"(low));
			__asm__ volatile("rdtimeh %0" : "=r"(temp));
		} while (temp != high);
		val = ((uint64_t)high << 32) | low;
	#endif
#elif defined(__loongarch__)
	__asm__ volatile("rdtime.d %0, $zero" : "=r"(val));
#else
	#include <time.h>
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	val = (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
#endif

	return val;
}

uint64_t realtime_cycles_to_ns(uint64_t cycles)
{
	uint64_t freq = realtime_freq_get();
	if (freq == 0)
		return 0;

	uint64_t sec = cycles / freq;
	uint64_t rem = cycles % freq;
	uint64_t nanosec = (rem * 1000000000ULL) / freq;

	return (sec * 1000000000ULL) + nanosec;
}

/* Alias for compatibility with original code */
#define timeGet() realtime_time_get()
#define cycles_to_ns(c) realtime_cycles_to_ns(c)

/* ============================================================================
 * Thread Creation Helpers
 * ============================================================================ */

#ifdef RT_THREAD_PLATFORM

static rt_thread_t create_rt_thread(const char *name, void (*entry)(void *),
                                     void *arg, int prio)
{
	rt_thread_t tid = rt_thread_create(name, entry, arg, 8192, prio, 20);
	if (tid) {
		rt_thread_startup(tid);
	}
	return tid;
}

static void wait_thread_exit(const char *name)
{
	while (rt_thread_find(name)) {
		rt_thread_mdelay(10);
	}
}

#endif

/* ============================================================================
 * Test 1: Context Switch Latency
 * ============================================================================ */

static sem_t ctx_sem_start;
static sem_t ctx_sem_done;
static volatile uint64_t ctx_t0, ctx_t1;
static volatile uint64_t ctx_e0, ctx_e1;

static void *ctx_thread_b(void *arg)
{
	(void)arg;
	sem_wait(&ctx_sem_start);

	for (int i = 0; i < TEST_ITERATION + 1; i++) {
		sched_yield();
	}

	sem_post(&ctx_sem_done);
	return NULL;
}

static void *ctx_thread_a(void *arg)
{
	(void)arg;
	sem_post(&ctx_sem_start);
	sched_yield();

	/* Measure empty loop overhead */
	ctx_e0 = timeGet();
	for (int i = 0; i < TEST_ITERATION; i++) {
		__asm__ __volatile__("");
	}
	ctx_e1 = timeGet();

	/* Measure context switch time */
	ctx_t0 = timeGet();
	for (int i = 0; i < TEST_ITERATION; i++) {
		sched_yield();
	}
	ctx_t1 = timeGet();

	sem_wait(&ctx_sem_done);
	return NULL;
}

static void test1(uint64_t *result)
{
	sem_init(&ctx_sem_start, 0, 0);
	sem_init(&ctx_sem_done, 0, 0);

	pthread_t tid_a, tid_b;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = BENCHMARK_MIDDLE_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	if (pthread_create(&tid_b, &attr, ctx_thread_b, NULL) != 0) {
		rt_printf("test1: Failed to create thread B\n");
		*result = 0;
		return;
	}

	if (pthread_create(&tid_a, &attr, ctx_thread_a, NULL) != 0) {
		rt_printf("test1: Failed to create thread A\n");
		*result = 0;
		return;
	}

	pthread_join(tid_a, NULL);
	pthread_join(tid_b, NULL);
	pthread_attr_destroy(&attr);

	sem_destroy(&ctx_sem_start);
	sem_destroy(&ctx_sem_done);

	uint64_t cycles = (ctx_t1 - ctx_t0) - (ctx_e1 - ctx_e0);
	*result = cycles_to_ns(cycles) / TEST_ITERATION / 2;
}

/* ============================================================================
 * Test 4_1: Semaphore Immediate (take & give)
 * ============================================================================ */

static void test4_1(uint64_t *sem_take_lat, uint64_t *sem_give_lat)
{
	sem_t sem;
	uint64_t t0, t1;
	uint64_t give_cycles = 0, take_cycles = 0;

	sem_init(&sem, 0, 0);

	for (int i = 0; i < TEST_ITERATION; i++) {
		t0 = timeGet();
		sem_post(&sem);
		t1 = timeGet();
		give_cycles += t1 - t0;

		t0 = timeGet();
		sem_wait(&sem);
		t1 = timeGet();
		take_cycles += t1 - t0;
	}

	sem_destroy(&sem);

	*sem_give_lat = cycles_to_ns(give_cycles / TEST_ITERATION);
	*sem_take_lat = cycles_to_ns(take_cycles / TEST_ITERATION);
}

/* ============================================================================
 * Test 4_2 / 5_4: Semaphore Suspend/Wakeup
 * ============================================================================ */

static sem_t sem42_test;
static volatile uint64_t sem42_take_cycles;
static volatile uint64_t sem42_give_cycles;

static void *sem42_post_thread(void *arg)
{
	(void)arg;
	uint64_t t0, t1;
	uint64_t total = 0;

	/* Warm up */
	for (int i = 0; i < TEST_ITERATION; i++) {
		sem_post(&sem42_test);
	}

#ifdef RT_THREAD_PLATFORM
	rt_thread_mdelay(10);
#else
	usleep(10000);
#endif

	/* Measured: sem_give waking up high priority thread (test 5_4) */
	for (int i = 0; i < TEST_ITERATION; i++) {
		t0 = timeGet();
		sem_post(&sem42_test);
		t1 = timeGet();
		total += t1 - t0;
	}

	sem42_give_cycles = total;
	return NULL;
}

static void *sem42_wait_thread(void *arg)
{
	(void)arg;
	uint64_t t0, t1;
	uint64_t total = 0;

	/* Warm up - consume posted semaphores */
	for (int i = 0; i < TEST_ITERATION; i++) {
		sem_wait(&sem42_test);
	}

	/* Measured: sem_take with suspend (test 4_2) */
	for (int i = 0; i < TEST_ITERATION; i++) {
		t0 = timeGet();
		sem_wait(&sem42_test);
		t1 = timeGet();
		total += t1 - t0;
	}

	sem42_take_cycles = total;
	return NULL;
}

static void test4_2(uint64_t *sem_take_suspend, uint64_t *sem_give_wakeup)
{
	sem_init(&sem42_test, 0, 0);

	pthread_t post_tid, wait_tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

	/* Low priority poster */
	param.sched_priority = BENCHMARK_LOW_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(&post_tid, &attr, sem42_post_thread, NULL);

	/* High priority waiter */
	param.sched_priority = BENCHMARK_HIGH_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_create(&wait_tid, &attr, sem42_wait_thread, NULL);

	pthread_join(post_tid, NULL);
	pthread_join(wait_tid, NULL);
	pthread_attr_destroy(&attr);

	sem_destroy(&sem42_test);

	*sem_take_suspend = cycles_to_ns(sem42_take_cycles / TEST_ITERATION);
	*sem_give_wakeup = cycles_to_ns(sem42_give_cycles / TEST_ITERATION);
}

/* ============================================================================
 * Test 5_3: Semaphore Give (Low Priority Ready)
 * ============================================================================ */

static sem_t sem53_test;
static sem_t sem53_to_assist;
static sem_t sem53_to_high;
static volatile uint64_t sem53_give_cycles;

static void *sem53_assist_thread(void *arg)
{
	(void)arg;
	for (int i = 0; i < TEST_ITERATION; i++) {
		sem_wait(&sem53_to_assist);
		sem_post(&sem53_to_high);
	}
	return NULL;
}

static void *sem53_low_thread(void *arg)
{
	(void)arg;
	for (int i = 0; i < TEST_ITERATION; i++) {
		sem_wait(&sem53_test);
	}
	return NULL;
}

static void *sem53_high_thread(void *arg)
{
	(void)arg;
	uint64_t t0, t1;
	uint64_t total = 0;

	pthread_t low_tid, assist_tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

	/* Middle priority waiter */
	param.sched_priority = BENCHMARK_MIDDLE_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(&low_tid, &attr, sem53_low_thread, NULL);

	/* Lowest priority assist */
	param.sched_priority = BENCHMARK_LOW_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_create(&assist_tid, &attr, sem53_assist_thread, NULL);

	for (int i = 0; i < TEST_ITERATION; i++) {
		sem_post(&sem53_to_assist);
		sem_wait(&sem53_to_high);

		/* Now middle priority thread is waiting, we are highest */
		t0 = timeGet();
		sem_post(&sem53_test);
		t1 = timeGet();
		total += t1 - t0;
	}

	pthread_join(low_tid, NULL);
	pthread_join(assist_tid, NULL);
	pthread_attr_destroy(&attr);

	sem53_give_cycles = total;
	return NULL;
}

static void test5_3(uint64_t *sem_give_lowready)
{
	sem_init(&sem53_test, 0, 0);
	sem_init(&sem53_to_assist, 0, 0);
	sem_init(&sem53_to_high, 0, 0);

	pthread_t high_tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = BENCHMARK_HIGH_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	pthread_create(&high_tid, &attr, sem53_high_thread, NULL);
	pthread_join(high_tid, NULL);
	pthread_attr_destroy(&attr);

	sem_destroy(&sem53_test);
	sem_destroy(&sem53_to_assist);
	sem_destroy(&sem53_to_high);

	*sem_give_lowready = cycles_to_ns(sem53_give_cycles / TEST_ITERATION);
}

/* ============================================================================
 * Test 6_0: Message Queue Filled Behavior Check
 * ============================================================================ */

static mqd_t mq60;
static const char *mq60_msg = "Hi";
static volatile int mq60_can_stall = 0;

static void *mq60_receive_thread(void *arg)
{
	(void)arg;
#ifdef RT_THREAD_PLATFORM
	rt_thread_mdelay(1);
#else
	usleep(1000);
#endif
	char buffer[8];
	mq_receive(mq60, buffer, 8, NULL);
	return NULL;
}

static void *mq60_send_thread(void *arg)
{
	(void)arg;
	pthread_t recv_tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 4096);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = BENCHMARK_LOW_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(&recv_tid, &attr, mq60_receive_thread, NULL);

	int ret = mq_send(mq60, mq60_msg, strlen(mq60_msg) + 1, 0);
	mq60_can_stall = ret;

	pthread_join(recv_tid, NULL);
	pthread_attr_destroy(&attr);
	return NULL;
}

static int test6_0(void)
{
	static const char *mq_name = "/6_0_queue";
	struct mq_attr attr_m;

	attr_m.mq_flags = 0;
	attr_m.mq_maxmsg = 1;
	attr_m.mq_msgsize = 8;
	attr_m.mq_curmsgs = 0;

	mq_unlink(mq_name);
	mq60 = mq_open(mq_name, O_CREAT | O_RDWR | O_EXCL, 0644, &attr_m);
	if (mq60 == (mqd_t)-1) {
		rt_printf("test6_0: mq_open failed\n");
		return -1;
	}

	/* Fill the queue */
	mq_send(mq60, mq60_msg, strlen(mq60_msg) + 1, 0);

	pthread_t send_tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 4096);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = BENCHMARK_HIGH_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(&send_tid, &attr, mq60_send_thread, NULL);

	pthread_join(send_tid, NULL);
	pthread_attr_destroy(&attr);

	mq_close(mq60);
	mq_unlink(mq_name);

	return mq60_can_stall;
}

/* ============================================================================
 * Test 6_1 / 7_1: Message Queue Immediate
 * ============================================================================ */

static void test6_1(uint64_t *mq_send_lat, uint64_t *mq_recv_lat)
{
	static const char *mq_name = "/6_1_queue";
	static const char *msg = "Hi";
	struct mq_attr attr_m;
	mqd_t mq;
	uint64_t send_cycles = 0, recv_cycles = 0;

	attr_m.mq_flags = 0;
	attr_m.mq_maxmsg = 1;
	attr_m.mq_msgsize = 8;
	attr_m.mq_curmsgs = 0;

	mq_unlink(mq_name);
	mq = mq_open(mq_name, O_CREAT | O_RDWR | O_EXCL, 0644, &attr_m);
	if (mq == (mqd_t)-1) {
		rt_printf("test6_1: mq_open failed\n");
		*mq_send_lat = 0;
		*mq_recv_lat = 0;
		return;
	}

	char buffer[8];
	for (int i = 0; i < TEST_ITERATION; i++) {
		uint64_t t0, t1;

		t0 = timeGet();
		mq_send(mq, msg, strlen(msg) + 1, 0);
		t1 = timeGet();
		send_cycles += t1 - t0;

		t0 = timeGet();
		mq_receive(mq, buffer, 8, NULL);
		t1 = timeGet();
		recv_cycles += t1 - t0;
	}

	mq_close(mq);
	mq_unlink(mq_name);

	*mq_send_lat = cycles_to_ns(send_cycles / TEST_ITERATION);
	*mq_recv_lat = cycles_to_ns(recv_cycles / TEST_ITERATION);
}

/* ============================================================================
 * Test 6_3: Message Send (Low Priority Ready)
 * ============================================================================ */

static mqd_t mq63;
static sem_t mq63_to_assist;
static sem_t mq63_to_high;
static volatile uint64_t mq63_send_cycles;
static const char *mq63_msg = "Hi";

static void *mq63_assist_thread(void *arg)
{
	(void)arg;
	for (int i = 0; i < TEST_ITERATION; i++) {
		sem_wait(&mq63_to_assist);
		sem_post(&mq63_to_high);
	}
	rt_printf("    [6_3] assist_thread done\n");
	return NULL;
}

static void *mq63_recv_thread(void *arg)
{
	(void)arg;
	char buffer[8];
	for (int i = 0; i < TEST_ITERATION; i++) {
		mq_receive(mq63, buffer, 8, NULL);
	}
	rt_printf("    [6_3] recv_thread done\n");
	return NULL;
}

static void *mq63_send_thread(void *arg)
{
	(void)arg;
	pthread_t recv_tid, assist_tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

	/* Middle priority receiver */
	param.sched_priority = BENCHMARK_MIDDLE_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(&recv_tid, &attr, mq63_recv_thread, NULL);

	/* Low priority assist */
	param.sched_priority = BENCHMARK_LOW_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_create(&assist_tid, &attr, mq63_assist_thread, NULL);

	rt_printf("    [6_3] send_thread starting main loop...\n");
	uint64_t t0, t1;
	uint64_t total = 0;
	for (int i = 0; i < TEST_ITERATION; i++) {
		sem_post(&mq63_to_assist);
		sem_wait(&mq63_to_high);

		t0 = timeGet();
		mq_send(mq63, mq63_msg, strlen(mq63_msg) + 1, 0);
		t1 = timeGet();
		total += t1 - t0;
	}

	rt_printf("    [6_3] send_thread main loop done, joining...\n");
	pthread_join(recv_tid, NULL);
	pthread_join(assist_tid, NULL);
	pthread_attr_destroy(&attr);

	mq63_send_cycles = total;
	rt_printf("    [6_3] send_thread done\n");
	return NULL;
}

static void test6_3(uint64_t *mq_send_lowready)
{
	static const char *mq_name = "/6_3_queue";
	struct mq_attr attr_m;

	sem_init(&mq63_to_assist, 0, 0);
	sem_init(&mq63_to_high, 0, 0);

	attr_m.mq_flags = 0;
	attr_m.mq_maxmsg = 1;
	attr_m.mq_msgsize = 8;
	attr_m.mq_curmsgs = 0;

	mq_unlink(mq_name);
	mq63 = mq_open(mq_name, O_CREAT | O_RDWR | O_EXCL, 0644, &attr_m);
	if (mq63 == (mqd_t)-1) {
		rt_printf("test6_3: mq_open failed\n");
		*mq_send_lowready = 0;
		return;
	}

	pthread_t send_tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = BENCHMARK_HIGH_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(&send_tid, &attr, mq63_send_thread, NULL);

	pthread_join(send_tid, NULL);
	pthread_attr_destroy(&attr);

	mq_close(mq63);
	mq_unlink(mq_name);
	sem_destroy(&mq63_to_assist);
	sem_destroy(&mq63_to_high);

	*mq_send_lowready = cycles_to_ns(mq63_send_cycles / TEST_ITERATION);
}

/* ============================================================================
 * Test 6_4 / 7_2: Message Send (High Prio Resume) / Receive (Suspend)
 * ============================================================================ */

static mqd_t mq64;
static volatile uint64_t mq64_send_cycles;
static volatile uint64_t mq64_recv_cycles;
static const char *mq64_msg = "Hi";

static void *mq64_send_thread(void *arg)
{
	(void)arg;
	uint64_t t0, t1;
	uint64_t total = 0;

	rt_printf("    [6_4] send_thread: warmup send 1\n");
	mq_send(mq64, mq64_msg, strlen(mq64_msg) + 1, 0);

	rt_printf("    [6_4] send_thread: warmup loop %d\n", TEST_ITERATION);
	for (int i = 0; i < TEST_ITERATION; i++) {
		mq_send(mq64, mq64_msg, strlen(mq64_msg) + 1, 0);
	}

	rt_printf("    [6_4] send_thread: sync send 1\n");
	mq_send(mq64, mq64_msg, strlen(mq64_msg) + 1, 0);

	rt_printf("    [6_4] send_thread: measured loop %d\n", TEST_ITERATION);
	/* Measured: mq_send waking high prio (test 6_4) */
	for (int i = 0; i < TEST_ITERATION; i++) {
		t0 = timeGet();
		mq_send(mq64, mq64_msg, strlen(mq64_msg) + 1, 0);
		t1 = timeGet();
		total += t1 - t0;
	}

	mq64_send_cycles = total;
	rt_printf("    [6_4] send_thread done\n");
	return NULL;
}

static void *mq64_recv_thread(void *arg)
{
	(void)arg;
	char buffer[8];
	uint64_t t0, t1;
	uint64_t total = 0;

	pthread_t send_tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = BENCHMARK_LOW_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	rt_printf("    [6_4] recv_thread: creating send_thread...\n");
	pthread_create(&send_tid, &attr, mq64_send_thread, NULL);

	rt_printf("    [6_4] recv_thread: warmup recv 1\n");
	mq_receive(mq64, buffer, 8, NULL);

	rt_printf("    [6_4] recv_thread: measured loop %d\n", TEST_ITERATION);
	/* Measured: mq_receive (high prio) resuming from low prio send (test 7_2) */
	for (int i = 0; i < TEST_ITERATION; i++) {
		t0 = timeGet();
		mq_receive(mq64, buffer, 8, NULL);
		t1 = timeGet();
		total += t1 - t0;
	}

	rt_printf("    [6_4] recv_thread: sync recv 1\n");
	mq_receive(mq64, buffer, 8, NULL);

	rt_printf("    [6_4] recv_thread: unmeasured loop %d\n", TEST_ITERATION);
	for (int i = 0; i < TEST_ITERATION; i++) {
		mq_receive(mq64, buffer, 8, NULL);
	}

	pthread_join(send_tid, NULL);
	pthread_attr_destroy(&attr);

	mq64_recv_cycles = total;
	rt_printf("    [6_4] recv_thread done\n");
	return NULL;
}

static void test6_4(uint64_t *mq_send_highresume, uint64_t *mq_recv_suspend)
{
	static const char *mq_name = "/6_4_queue";
	struct mq_attr attr_m;

	attr_m.mq_flags = 0;
	attr_m.mq_maxmsg = 1;
	attr_m.mq_msgsize = 8;
	attr_m.mq_curmsgs = 0;

	mq_unlink(mq_name);
	mq64 = mq_open(mq_name, O_CREAT | O_RDWR | O_EXCL, 0644, &attr_m);
	if (mq64 == (mqd_t)-1) {
		rt_printf("test6_4: mq_open failed\n");
		*mq_send_highresume = 0;
		*mq_recv_suspend = 0;
		return;
	}

	pthread_t recv_tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = BENCHMARK_HIGH_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(&recv_tid, &attr, mq64_recv_thread, NULL);

	pthread_join(recv_tid, NULL);
	pthread_attr_destroy(&attr);

	mq_close(mq64);
	mq_unlink(mq_name);

	*mq_send_highresume = cycles_to_ns(mq64_send_cycles / TEST_ITERATION);
	*mq_recv_suspend = cycles_to_ns(mq64_recv_cycles / TEST_ITERATION);
}

/* ============================================================================
 * Test 6_2 / 7_4: Message Send (Suspend) / Receive (High Prio Resume)
 * ============================================================================ */

static mqd_t mq62;
static volatile uint64_t mq62_send_cycles;
static volatile uint64_t mq62_recv_cycles;
static const char *mq62_msg = "Hi";

static void *mq62_recv_thread(void *arg)
{
	(void)arg;
	char buffer[8];
	uint64_t t0, t1;
	uint64_t total = 0;

	mq_receive(mq62, buffer, 8, NULL);

	for (int i = 0; i < TEST_ITERATION; i++) {
		mq_receive(mq62, buffer, 8, NULL);
	}

	mq_receive(mq62, buffer, 8, NULL);

	/* Measured: mq_receive (low prio) causing send suspend (test 7_4 recv) */
	for (int i = 0; i < TEST_ITERATION; i++) {
		t0 = timeGet();
		mq_receive(mq62, buffer, 8, NULL);
		t1 = timeGet();
		total += t1 - t0;
	}

	mq62_recv_cycles = total;
	return NULL;
}

static void *mq62_send_thread(void *arg)
{
	(void)arg;
	uint64_t t0, t1;
	uint64_t total = 0;

	pthread_t recv_tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = BENCHMARK_LOW_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(&recv_tid, &attr, mq62_recv_thread, NULL);

	mq_send(mq62, mq62_msg, strlen(mq62_msg) + 1, 0);

	/* Measured: mq_send waking high prio recv (test 6_2) */
	for (int i = 0; i < TEST_ITERATION; i++) {
		t0 = timeGet();
		mq_send(mq62, mq62_msg, strlen(mq62_msg) + 1, 0);
		t1 = timeGet();
		total += t1 - t0;
	}

	mq_send(mq62, mq62_msg, strlen(mq62_msg) + 1, 0);

	for (int i = 0; i < TEST_ITERATION; i++) {
		mq_send(mq62, mq62_msg, strlen(mq62_msg) + 1, 0);
	}

	pthread_join(recv_tid, NULL);
	pthread_attr_destroy(&attr);

	mq62_send_cycles = total;
	return NULL;
}

static void test6_2(uint64_t *mq_send_suspend, uint64_t *mq_recv_highresume)
{
	static const char *mq_name = "/6_2_queue";
	struct mq_attr attr_m;

	attr_m.mq_flags = 0;
	attr_m.mq_maxmsg = 1;
	attr_m.mq_msgsize = 8;
	attr_m.mq_curmsgs = 0;

	mq_unlink(mq_name);
	mq62 = mq_open(mq_name, O_CREAT | O_RDWR | O_EXCL, 0644, &attr_m);
	if (mq62 == (mqd_t)-1) {
		rt_printf("test6_2: mq_open failed\n");
		*mq_send_suspend = 0;
		*mq_recv_highresume = 0;
		return;
	}

	/* Pre-fill queue */
	mq_send(mq62, mq62_msg, strlen(mq62_msg) + 1, 0);

	pthread_t send_tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = BENCHMARK_HIGH_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(&send_tid, &attr, mq62_send_thread, NULL);

	pthread_join(send_tid, NULL);
	pthread_attr_destroy(&attr);

	mq_close(mq62);
	mq_unlink(mq_name);

	*mq_send_suspend = cycles_to_ns(mq62_send_cycles / TEST_ITERATION);
	*mq_recv_highresume = cycles_to_ns(mq62_recv_cycles / TEST_ITERATION);
}

/* ============================================================================
 * Test 7_3: Message Receive (Low Priority Ready)
 * ============================================================================ */

static mqd_t mq73;
static sem_t mq73_to_assist;
static sem_t mq73_to_high;
static volatile uint64_t mq73_recv_cycles;
static const char *mq73_msg = "Hi";

static void *mq73_assist_thread(void *arg)
{
	(void)arg;
	for (int i = 0; i < TEST_ITERATION; i++) {
		sem_wait(&mq73_to_assist);
		sem_post(&mq73_to_high);
	}
	return NULL;
}

static void *mq73_send_thread(void *arg)
{
	(void)arg;
	for (int i = 0; i < TEST_ITERATION; i++) {
		mq_send(mq73, mq73_msg, strlen(mq73_msg) + 1, 0);
	}
	return NULL;
}

static void *mq73_recv_thread(void *arg)
{
	(void)arg;
	pthread_t send_tid, assist_tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

	/* Middle priority sender */
	param.sched_priority = BENCHMARK_MIDDLE_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(&send_tid, &attr, mq73_send_thread, NULL);

	/* Low priority assist */
	param.sched_priority = BENCHMARK_LOW_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_create(&assist_tid, &attr, mq73_assist_thread, NULL);

	char buffer[8];
	uint64_t t0, t1;
	uint64_t total = 0;

	for (int i = 0; i < TEST_ITERATION; i++) {
		sem_post(&mq73_to_assist);
		sem_wait(&mq73_to_high);

		t0 = timeGet();
		mq_receive(mq73, buffer, 8, NULL);
		t1 = timeGet();
		total += t1 - t0;
	}

	pthread_join(send_tid, NULL);
	pthread_join(assist_tid, NULL);
	pthread_attr_destroy(&attr);

	mq73_recv_cycles = total;
	return NULL;
}

static void test7_3(uint64_t *mq_recv_lowready)
{
	static const char *mq_name = "/7_3_queue";
	struct mq_attr attr_m;

	sem_init(&mq73_to_assist, 0, 0);
	sem_init(&mq73_to_high, 0, 0);

	attr_m.mq_flags = 0;
	attr_m.mq_maxmsg = 1;
	attr_m.mq_msgsize = 8;
	attr_m.mq_curmsgs = 0;

	mq_unlink(mq_name);
	mq73 = mq_open(mq_name, O_CREAT | O_RDWR | O_EXCL, 0644, &attr_m);
	if (mq73 == (mqd_t)-1) {
		rt_printf("test7_3: mq_open failed\n");
		*mq_recv_lowready = 0;
		return;
	}

	/* Pre-fill queue */
	mq_send(mq73, mq73_msg, strlen(mq73_msg) + 1, 0);

	pthread_t recv_tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = BENCHMARK_HIGH_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(&recv_tid, &attr, mq73_recv_thread, NULL);

	pthread_join(recv_tid, NULL);
	pthread_attr_destroy(&attr);

	mq_close(mq73);
	mq_unlink(mq_name);
	sem_destroy(&mq73_to_assist);
	sem_destroy(&mq73_to_high);

	*mq_recv_lowready = cycles_to_ns(mq73_recv_cycles / TEST_ITERATION);
}

/* ============================================================================
 * Test 8_1 / 9_1: Mutex Immediate
 * ============================================================================ */

static void test8_1(uint64_t *mutex_lock_lat, uint64_t *mutex_unlock_lat)
{
	pthread_mutex_t mtx;
	uint64_t t0, t1;
	uint64_t lock_cycles = 0, unlock_cycles = 0;

	pthread_mutex_init(&mtx, NULL);

	for (int i = 0; i < TEST_ITERATION; i++) {
		t0 = timeGet();
		pthread_mutex_lock(&mtx);
		t1 = timeGet();
		lock_cycles += t1 - t0;

		t0 = timeGet();
		pthread_mutex_unlock(&mtx);
		t1 = timeGet();
		unlock_cycles += t1 - t0;
	}

	pthread_mutex_destroy(&mtx);

	*mutex_lock_lat = cycles_to_ns(lock_cycles / TEST_ITERATION);
	*mutex_unlock_lat = cycles_to_ns(unlock_cycles / TEST_ITERATION);
}

/* ============================================================================
 * Test 8_2 / 9_4: Mutex Suspend/Wakeup
 * ============================================================================ */

static pthread_mutex_t mtx82;
static sem_t mtx82_to_locker;
static sem_t mtx82_to_unlocker;
static volatile uint64_t mtx82_lock_cycles;
static volatile uint64_t mtx82_unlock_cycles;

static void *mtx82_unlock_thread(void *arg)
{
	(void)arg;
	uint64_t t0, t1;
	uint64_t total = 0;

	pthread_mutex_lock(&mtx82);
	sem_post(&mtx82_to_locker);

	/* Warmup phase */
	for (int i = 0; i < TEST_ITERATION; i++) {
		pthread_mutex_unlock(&mtx82);
		sem_wait(&mtx82_to_unlocker);
		pthread_mutex_lock(&mtx82);
		sem_post(&mtx82_to_locker);
	}

	/* Sync point */
	pthread_mutex_unlock(&mtx82);
	sem_wait(&mtx82_to_unlocker);
	pthread_mutex_lock(&mtx82);
	sem_post(&mtx82_to_locker);

	/* Measured: unlock waking high priority (test 9_4) */
	for (int i = 0; i < TEST_ITERATION; i++) {
		t0 = timeGet();
		pthread_mutex_unlock(&mtx82);
		t1 = timeGet();
		total += t1 - t0;

		sem_wait(&mtx82_to_unlocker);
		pthread_mutex_lock(&mtx82);
		sem_post(&mtx82_to_locker);
	}

	pthread_mutex_unlock(&mtx82);
	mtx82_unlock_cycles = total;
	return NULL;
}

static void *mtx82_lock_thread(void *arg)
{
	(void)arg;
	uint64_t t0, t1;
	uint64_t total = 0;

	pthread_t unlock_tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = BENCHMARK_LOW_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(&unlock_tid, &attr, mtx82_unlock_thread, NULL);

	sem_wait(&mtx82_to_locker);

	/* Warmup phase */
	for (int i = 0; i < TEST_ITERATION; i++) {
		pthread_mutex_lock(&mtx82);
		pthread_mutex_unlock(&mtx82);
		sem_post(&mtx82_to_unlocker);
		sem_wait(&mtx82_to_locker);
	}

	/* Sync point */
	pthread_mutex_lock(&mtx82);
	pthread_mutex_unlock(&mtx82);
	sem_post(&mtx82_to_unlocker);
	sem_wait(&mtx82_to_locker);

	/* Measured: lock with suspend (test 8_2) */
	for (int i = 0; i < TEST_ITERATION; i++) {
		t0 = timeGet();
		pthread_mutex_lock(&mtx82);
		t1 = timeGet();
		total += t1 - t0;

		pthread_mutex_unlock(&mtx82);
		sem_post(&mtx82_to_unlocker);
		sem_wait(&mtx82_to_locker);
	}

	pthread_join(unlock_tid, NULL);
	pthread_attr_destroy(&attr);

	mtx82_lock_cycles = total;
	return NULL;
}

static void test8_2(uint64_t *mutex_lock_suspend, uint64_t *mutex_unlock_wakeup)
{
	pthread_mutex_init(&mtx82, NULL);
	sem_init(&mtx82_to_locker, 0, 0);
	sem_init(&mtx82_to_unlocker, 0, 0);

	pthread_t lock_tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = BENCHMARK_HIGH_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(&lock_tid, &attr, mtx82_lock_thread, NULL);

	pthread_join(lock_tid, NULL);
	pthread_attr_destroy(&attr);

	pthread_mutex_destroy(&mtx82);
	sem_destroy(&mtx82_to_locker);
	sem_destroy(&mtx82_to_unlocker);

	*mutex_lock_suspend = cycles_to_ns(mtx82_lock_cycles / TEST_ITERATION);
	*mutex_unlock_wakeup = cycles_to_ns(mtx82_unlock_cycles / TEST_ITERATION);
}

/* ============================================================================
 * Test 9_3: Mutex Unlock (Low Priority Ready)
 * ============================================================================ */

static pthread_mutex_t mtx93;
static sem_t mtx93_to_assist;
static sem_t mtx93_to_high;
static volatile uint64_t mtx93_unlock_cycles;

static void *mtx93_assist_thread(void *arg)
{
	(void)arg;
	for (int i = 0; i < TEST_ITERATION; i++) {
		sem_wait(&mtx93_to_assist);
		sem_post(&mtx93_to_high);
	}
	return NULL;
}

static void *mtx93_low_thread(void *arg)
{
	(void)arg;
	for (int i = 0; i < TEST_ITERATION; i++) {
		pthread_mutex_lock(&mtx93);
		pthread_mutex_unlock(&mtx93);
	}
	return NULL;
}

static void *mtx93_high_thread(void *arg)
{
	(void)arg;
	uint64_t t0, t1;
	uint64_t total = 0;

	pthread_t low_tid, assist_tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

	/* Middle priority locker */
	param.sched_priority = BENCHMARK_MIDDLE_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(&low_tid, &attr, mtx93_low_thread, NULL);

	/* Low priority assist */
	param.sched_priority = BENCHMARK_LOW_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_create(&assist_tid, &attr, mtx93_assist_thread, NULL);

	for (int i = 0; i < TEST_ITERATION; i++) {
		pthread_mutex_lock(&mtx93);
		sem_post(&mtx93_to_assist);
		sem_wait(&mtx93_to_high);

		t0 = timeGet();
		pthread_mutex_unlock(&mtx93);
		t1 = timeGet();
		total += t1 - t0;
	}

	pthread_join(low_tid, NULL);
	pthread_join(assist_tid, NULL);
	pthread_attr_destroy(&attr);

	mtx93_unlock_cycles = total;
	return NULL;
}

static void test9_3(uint64_t *mutex_unlock_lowready)
{
	pthread_mutex_init(&mtx93, NULL);
	sem_init(&mtx93_to_assist, 0, 0);
	sem_init(&mtx93_to_high, 0, 0);

	pthread_t high_tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = BENCHMARK_HIGH_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_create(&high_tid, &attr, mtx93_high_thread, NULL);

	pthread_join(high_tid, NULL);
	pthread_attr_destroy(&attr);

	pthread_mutex_destroy(&mtx93);
	sem_destroy(&mtx93_to_assist);
	sem_destroy(&mtx93_to_high);

	*mutex_unlock_lowready = cycles_to_ns(mtx93_unlock_cycles / TEST_ITERATION);
}

/* ============================================================================
 * Test 10_1 / 11_1: Memory Allocation
 * ============================================================================ */

static void test10_1(uint64_t *malloc_lat, uint64_t *free_lat)
{
	uint64_t t0, t1;
	uint64_t malloc_cycles = 0, free_cycles = 0;
	int *ptr;

	for (int i = 0; i < TEST_ITERATION; i++) {
		t0 = timeGet();
		ptr = (int *)malloc(sizeof(int));
		t1 = timeGet();

		if (ptr == NULL) {
			*malloc_lat = 0;
			*free_lat = 0;
			return;
		}
		malloc_cycles += t1 - t0;

		t0 = timeGet();
		free(ptr);
		t1 = timeGet();
		free_cycles += t1 - t0;
	}

	*malloc_lat = cycles_to_ns(malloc_cycles / TEST_ITERATION);
	*free_lat = cycles_to_ns(free_cycles / TEST_ITERATION);
}

/* ============================================================================
 * Single-core Test Runner
 * ============================================================================ */

int test_realtime_singlecore(struct realtime_singlecore_result *result)
{
	if (result == NULL)
		return -1;

	memset(result, 0, sizeof(*result));
	realtime_timer_start();

	/* Test 1: Context switch */
	test1(&result->context_switch_avg);
	rt_printf("Finish test 1.\n");

	/* Test 2 & 3: Interrupt/Syscall (require kernel instrumentation) */
	rt_printf("Skip test 2 (interrupt) - requires kernel instrumentation.\n");
	rt_printf("Skip test 3 (syscall) - requires kernel instrumentation.\n");

	/* Test 4_1: Semaphore immediate */
	test4_1(&result->service_cost[0][0], &result->service_cost[1][0]);
	rt_printf("Finish test 4_1, 5_1.\n");

	/* Test 4_2 / 5_4: Semaphore suspend/wakeup */
	test4_2(&result->service_cost[0][1], &result->service_cost[1][3]);
	rt_printf("Finish test 4_2, 5_4.\n");

	/* Test 5_3: Semaphore low priority ready */
	test5_3(&result->service_cost[1][2]);
	rt_printf("Finish test 5_3.\n");

	/* Test 6_1 / 7_1: Message queue immediate */
	test6_1(&result->service_cost[2][0], &result->service_cost[3][0]);
	rt_printf("Finish test 6_1, 7_1.\n");

	/* Test 6_0: Check if mqueue blocks on full queue */
	rt_printf("Running test 6_0 (mqueue blocking check)...\n");
	message_queue_filled_behavior = test6_0();
	if (message_queue_filled_behavior == 0) {
		rt_printf("  mqueue supports blocking on full queue.\n");
	} else {
		rt_printf("  mqueue does NOT block on full queue (ret=%d).\n",
			  message_queue_filled_behavior);
	}

	/* Test 6_3: Message send (low priority ready) */
	rt_printf("Running test 6_3...\n");
	test6_3(&result->service_cost[2][2]);
	rt_printf("Finish test 6_3.\n");

	/* Test 6_4 / 7_2: Message send (high prio resume) / receive (suspend) */
	rt_printf("Running test 6_4, 7_2...\n");
	test6_4(&result->service_cost[2][3], &result->service_cost[3][1]);
	rt_printf("Finish test 6_4, 7_2.\n");

	/* Test 6_2 / 7_4: Message send (suspend) / receive (high prio resume)
	 * These tests require mqueue to block on full queue */
	if (message_queue_filled_behavior == 0) {
		rt_printf("Running test 6_2, 7_4...\n");
		test6_2(&result->service_cost[2][1], &result->service_cost[3][3]);
		rt_printf("Finish test 6_2, 7_4.\n");

		/* Test 7_3: Message receive (low priority ready) */
		rt_printf("Running test 7_3...\n");
		test7_3(&result->service_cost[3][2]);
		rt_printf("Finish test 7_3.\n");
	} else {
		rt_printf("Skip test 6_2, 7_3, 7_4 (mqueue does not block on full).\n");
	}

	/* Test 8_1 / 9_1: Mutex immediate */
	test8_1(&result->service_cost[4][0], &result->service_cost[5][0]);
	rt_printf("Finish test 8_1, 9_1.\n");

	/* Test 8_2 / 9_4: Mutex suspend/wakeup */
	test8_2(&result->service_cost[4][1], &result->service_cost[5][3]);
	rt_printf("Finish test 8_2, 9_4.\n");

	/* Test 9_3: Mutex unlock low priority ready */
	test9_3(&result->service_cost[5][2]);
	rt_printf("Finish test 9_3.\n");

	/* Test 10_1 / 11_1: Memory allocation */
	test10_1(&result->service_cost[6][0], &result->service_cost[7][0]);
	rt_printf("Finish test 10_1, 11_1.\n");

	return 0;
}

/* ============================================================================
 * Multicore Test: Task Create/Delete Latency
 * ============================================================================ */

static sem_t mc_task_father_to_son;
static sem_t mc_task_son_to_father;
static volatile uint64_t mc_task_cycles;

static void *mc_task_tmp_thread(void *arg)
{
	(void)arg;
	return NULL;
}

static void *mc_task_son_thread(void *arg)
{
	int number = (int)(long)arg;
	pthread_t tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = BENCHMARK_LOW_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	/* Signal ready */
	sem_post(&mc_task_son_to_father);
	sem_wait(&mc_task_father_to_son);

	/* Main work: create threads */
	for (int i = 0; i < MULTICORE_TASK_REPETITION; i++) {
		if (pthread_create(&tid, &attr, mc_task_tmp_thread, (void *)(long)number) != 0) {
			sem_post(&mc_task_son_to_father);
			rt_printf("mc_task: tmp thread create fail.\n");
			pthread_attr_destroy(&attr);
			return NULL;
		}
		pthread_join(tid, NULL);
	}

	pthread_attr_destroy(&attr);
	sem_post(&mc_task_son_to_father);
	return NULL;
}

static void *mc_task_father_thread(void *arg)
{
	int pair_count = (int)(long)arg;
	pthread_t tid[MULTICORE_MAX_WORKERS];
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = BENCHMARK_MIDDLE_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	for (int i = 0; i < pair_count; i++) {
		if (pthread_create(&tid[i], &attr, mc_task_son_thread, (void *)(long)i) != 0) {
			pthread_attr_destroy(&attr);
			return NULL;
		}
		/* Set CPU affinity */
		multicore_cpu_set_t cpuset;
		MULTICORE_CPU_ZERO(&cpuset);
		MULTICORE_CPU_SET(i % USE_PROCESSORS, &cpuset);
		multicore_pthread_setaffinity_np(tid[i], sizeof(cpuset), &cpuset);
	}

	/* Wait for sons to be ready */
	for (int i = 0; i < pair_count; i++) {
		sem_wait(&mc_task_son_to_father);
	}

	uint64_t t0 = timeGet();

	/* Notify all */
	for (int i = 0; i < pair_count; i++) {
		sem_post(&mc_task_father_to_son);
	}

	/* Wait for all to complete */
	for (int i = 0; i < pair_count; i++) {
		sem_wait(&mc_task_son_to_father);
	}

	uint64_t t1 = timeGet();

	/* Cleanup */
	for (int i = 0; i < pair_count; i++) {
		pthread_join(tid[i], NULL);
	}

	pthread_attr_destroy(&attr);
	mc_task_cycles = t1 - t0;
	return NULL;
}

static uint64_t multicore_task_lat(int pair_count)
{
	if (pair_count < 1) pair_count = 1;
	if (pair_count > MULTICORE_MAX_WORKERS) pair_count = MULTICORE_MAX_WORKERS;

	sem_init(&mc_task_father_to_son, 0, 0);
	sem_init(&mc_task_son_to_father, 0, 0);

	pthread_t tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = BENCHMARK_HIGH_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	if (pthread_create(&tid, &attr, mc_task_father_thread, (void *)(long)pair_count) != 0) {
		pthread_attr_destroy(&attr);
		sem_destroy(&mc_task_father_to_son);
		sem_destroy(&mc_task_son_to_father);
		return 0;
	}

	pthread_join(tid, NULL);
	pthread_attr_destroy(&attr);
	sem_destroy(&mc_task_father_to_son);
	sem_destroy(&mc_task_son_to_father);

	return cycles_to_ns(mc_task_cycles) / MULTICORE_TASK_REPETITION / pair_count;
}

/* ============================================================================
 * Multicore Test: IPC Bandwidth
 * ============================================================================ */

static mqd_t mc_ipc_mq[MULTICORE_MAX_WORKERS];
static sem_t mc_ipc_father_to_son;
static sem_t mc_ipc_son_to_tmp[MULTICORE_MAX_WORKERS];
static sem_t mc_ipc_tmp_to_son[MULTICORE_MAX_WORKERS];
static sem_t mc_ipc_son_to_father;
static int mc_ipc_mode = 0;
static volatile uint64_t mc_ipc_cycles;

static void *mc_ipc_tmp_thread(void *arg)
{
	int number = (int)(long)arg;
	char *buffer1 = (char *)malloc(MULTICORE_IPC_MESSAGE_SIZE);
	char *buffer2 = (char *)malloc(MULTICORE_IPC_MESSAGE_SIZE);

	if (!buffer1 || !buffer2) {
		if (buffer1) free(buffer1);
		if (buffer2) free(buffer2);
		return NULL;
	}

	for (int i = 0; i < MULTICORE_IPC_REPETITION; i++) {
		sem_wait(&mc_ipc_son_to_tmp[number]);
		mq_receive(mc_ipc_mq[number], buffer1, MULTICORE_IPC_MESSAGE_SIZE, NULL);
		mq_receive(mc_ipc_mq[number], buffer2, MULTICORE_IPC_MESSAGE_SIZE, NULL);
		sem_post(&mc_ipc_tmp_to_son[number]);
	}

	free(buffer1);
	free(buffer2);
	return NULL;
}

static void *mc_ipc_son_thread(void *arg)
{
	int number = (int)(long)arg;
	char *buffer1 = (char *)malloc(MULTICORE_IPC_MESSAGE_SIZE);
	char *buffer2 = (char *)malloc(MULTICORE_IPC_MESSAGE_SIZE);

	if (!buffer1 || !buffer2) {
		sem_post(&mc_ipc_son_to_father);
		if (buffer1) free(buffer1);
		if (buffer2) free(buffer2);
		return NULL;
	}

	memset(buffer1, 'A', MULTICORE_IPC_MESSAGE_SIZE);
	memset(buffer2, 'B', MULTICORE_IPC_MESSAGE_SIZE);

	pthread_t tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = BENCHMARK_LOW_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	if (pthread_create(&tid, &attr, mc_ipc_tmp_thread, (void *)(long)number) != 0) {
		sem_post(&mc_ipc_son_to_father);
		pthread_attr_destroy(&attr);
		free(buffer1);
		free(buffer2);
		return NULL;
	}

	/* Set CPU affinity based on mode */
	multicore_cpu_set_t cpuset;
	MULTICORE_CPU_ZERO(&cpuset);
	if (mc_ipc_mode == 1) {
		/* Different core */
		MULTICORE_CPU_SET((number + 1) % USE_PROCESSORS, &cpuset);
	} else {
		/* Same core */
		MULTICORE_CPU_SET(number % USE_PROCESSORS, &cpuset);
	}
	multicore_pthread_setaffinity_np(tid, sizeof(cpuset), &cpuset);

	pthread_attr_destroy(&attr);

	/* Signal ready */
	sem_post(&mc_ipc_son_to_father);
	sem_wait(&mc_ipc_father_to_son);

	/* Main work: send messages */
	for (int i = 0; i < MULTICORE_IPC_REPETITION; i++) {
		mq_send(mc_ipc_mq[number], buffer1, MULTICORE_IPC_MESSAGE_SIZE, 0);
		mq_send(mc_ipc_mq[number], buffer2, MULTICORE_IPC_MESSAGE_SIZE, 0);
		sem_post(&mc_ipc_son_to_tmp[number]);
		sem_wait(&mc_ipc_tmp_to_son[number]);
	}

	pthread_join(tid, NULL);
	sem_post(&mc_ipc_son_to_father);
	free(buffer1);
	free(buffer2);
	return NULL;
}

static void *mc_ipc_father_thread(void *arg)
{
	int pair_count = (int)(long)arg;
	pthread_t tid[MULTICORE_MAX_WORKERS];
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = BENCHMARK_LOW_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	for (int i = 0; i < pair_count; i++) {
		if (pthread_create(&tid[i], &attr, mc_ipc_son_thread, (void *)(long)i) != 0) {
			pthread_attr_destroy(&attr);
			return NULL;
		}
		if (mc_ipc_mode == 0 || mc_ipc_mode == 1 || mc_ipc_mode == 2) {
			multicore_cpu_set_t cpuset;
			MULTICORE_CPU_ZERO(&cpuset);
			MULTICORE_CPU_SET(i % USE_PROCESSORS, &cpuset);
			multicore_pthread_setaffinity_np(tid[i], sizeof(cpuset), &cpuset);
		}
	}

	/* Wait for sons to be ready */
	for (int i = 0; i < pair_count; i++) {
		sem_wait(&mc_ipc_son_to_father);
	}

	uint64_t t0 = timeGet();

	/* Notify all */
	for (int i = 0; i < pair_count; i++) {
		sem_post(&mc_ipc_father_to_son);
	}

	/* Wait for all to complete */
	for (int i = 0; i < pair_count; i++) {
		sem_wait(&mc_ipc_son_to_father);
	}

	uint64_t t1 = timeGet();

	/* Cleanup */
	for (int i = 0; i < pair_count; i++) {
		pthread_join(tid[i], NULL);
	}

	pthread_attr_destroy(&attr);
	mc_ipc_cycles = t1 - t0;
	return NULL;
}

static uint64_t multicore_ipc_bw(int mode, int pair_count)
{
	if (pair_count < 1) pair_count = 1;
	if (pair_count > MULTICORE_MAX_WORKERS) pair_count = MULTICORE_MAX_WORKERS;

	static const char *mq_name[MULTICORE_MAX_WORKERS] = {
		"/mc_ipc_q0", "/mc_ipc_q1", "/mc_ipc_q2", "/mc_ipc_q3",
		"/mc_ipc_q4", "/mc_ipc_q5", "/mc_ipc_q6", "/mc_ipc_q7"
	};
	struct mq_attr attr_m;

	attr_m.mq_flags = 0;
	attr_m.mq_maxmsg = 2;
	attr_m.mq_msgsize = MULTICORE_IPC_MESSAGE_SIZE;
	attr_m.mq_curmsgs = 0;

	for (int i = 0; i < pair_count; i++) {
		mq_unlink(mq_name[i]);
		mc_ipc_mq[i] = mq_open(mq_name[i], O_CREAT | O_RDWR | O_EXCL, 0644, &attr_m);
		if (mc_ipc_mq[i] == (mqd_t)-1) {
			rt_printf("multicore_ipc_bw: mq_open failed for queue %d\n", i);
			/* Cleanup already created queues */
			for (int j = 0; j < i; j++) {
				mq_close(mc_ipc_mq[j]);
				mq_unlink(mq_name[j]);
			}
			return 0;
		}
	}

	for (int i = 0; i < pair_count; i++) {
		sem_init(&mc_ipc_son_to_tmp[i], 0, 0);
		sem_init(&mc_ipc_tmp_to_son[i], 0, 0);
	}
	sem_init(&mc_ipc_father_to_son, 0, 0);
	sem_init(&mc_ipc_son_to_father, 0, 0);

	mc_ipc_mode = mode;

	pthread_t tid;
	pthread_attr_t attr;
	struct sched_param param;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 8192);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	param.sched_priority = BENCHMARK_HIGH_PRIO;
	pthread_attr_setschedparam(&attr, &param);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

	if (pthread_create(&tid, &attr, mc_ipc_father_thread, (void *)(long)pair_count) != 0) {
		pthread_attr_destroy(&attr);
		for (int i = 0; i < pair_count; i++) {
			mq_close(mc_ipc_mq[i]);
			mq_unlink(mq_name[i]);
			sem_destroy(&mc_ipc_son_to_tmp[i]);
			sem_destroy(&mc_ipc_tmp_to_son[i]);
		}
		sem_destroy(&mc_ipc_father_to_son);
		sem_destroy(&mc_ipc_son_to_father);
		return 0;
	}

	pthread_join(tid, NULL);
	pthread_attr_destroy(&attr);

	for (int i = 0; i < pair_count; i++) {
		sem_destroy(&mc_ipc_son_to_tmp[i]);
		sem_destroy(&mc_ipc_tmp_to_son[i]);
		mq_close(mc_ipc_mq[i]);
		mq_unlink(mq_name[i]);
	}
	sem_destroy(&mc_ipc_father_to_son);
	sem_destroy(&mc_ipc_son_to_father);

	/* Calculate bandwidth: 32KB * 2 * repetitions * pairs / time = KB/s */
	uint64_t ns = cycles_to_ns(mc_ipc_cycles);
	if (ns == 0) return 0;
	return (uint64_t)32 * MULTICORE_IPC_REPETITION * pair_count * 1000000000ULL / ns / 1024;
}

/* ============================================================================
 * Multicore Test: Memory Bandwidth
 * ============================================================================ */

#define MC_MEM_NUM (MULTICORE_MEM_SIZE / sizeof(int))

/* Memory test modes */
enum mc_mem_mode {
	MC_MODE_RD = 0,    /* Sparse read */
	MC_MODE_WR,        /* Sparse write */
	MC_MODE_CP,        /* Sparse copy */
	MC_MODE_FRD,       /* Full read */
	MC_MODE_FWR,       /* Full write */
	MC_MODE_FCP,       /* Full copy */
	MC_MODE_MSET,      /* memset */
	MC_MODE_MCPY       /* memcpy */
};

static sem_t mc_mem_order[MULTICORE_MAX_WORKERS];
static sem_t mc_mem_response[MULTICORE_MAX_WORKERS];
static pthread_t mc_mem_tids[MULTICORE_MAX_WORKERS];
static int *mc_mem_src = NULL;
static int *mc_mem_dst[MULTICORE_MAX_WORKERS];
static int mc_mem_mode;

/* Prevent compiler optimization */
static volatile unsigned long long mc_mem_dummy = 0;
static void mc_mem_use(int result) { mc_mem_dummy += result; }

static void mc_mem_rd(int *buf)
{
	int i, sum = 0, *p, *lastone = buf + MC_MEM_NUM;
	for (i = 0; i < MULTICORE_MEM_REPETITION; i++) {
		p = buf;
		while (p < lastone) {
			sum += p[0] + p[8] + p[16] + p[24] + p[32] + p[40] + p[48] + p[56]
			     + p[64] + p[72] + p[80] + p[88] + p[96] + p[104] + p[112] + p[120];
			p += 128;
		}
	}
	mc_mem_use(sum);
}

static void mc_mem_wr(int *buf)
{
	int i, *p, *lastone = buf + MC_MEM_NUM;
	for (i = 0; i < MULTICORE_MEM_REPETITION; i++) {
		p = buf;
		while (p < lastone) {
			p[0]=1; p[8]=1; p[16]=1; p[24]=1; p[32]=1; p[40]=1; p[48]=1; p[56]=1;
			p[64]=1; p[72]=1; p[80]=1; p[88]=1; p[96]=1; p[104]=1; p[112]=1; p[120]=1;
			p += 128;
		}
	}
}

static void mc_mem_cp(int *dst, int *src)
{
	int i, *pd, *ps, *lastone = dst + MC_MEM_NUM;
	for (i = 0; i < MULTICORE_MEM_REPETITION; i++) {
		pd = dst; ps = src;
		while (pd < lastone) {
			pd[0]=ps[0]; pd[8]=ps[8]; pd[16]=ps[16]; pd[24]=ps[24];
			pd[32]=ps[32]; pd[40]=ps[40]; pd[48]=ps[48]; pd[56]=ps[56];
			pd[64]=ps[64]; pd[72]=ps[72]; pd[80]=ps[80]; pd[88]=ps[88];
			pd[96]=ps[96]; pd[104]=ps[104]; pd[112]=ps[112]; pd[120]=ps[120];
			pd += 128; ps += 128;
		}
	}
}

static void mc_mem_frd(int *buf)
{
	int i, sum = 0, *p, *lastone = buf + MC_MEM_NUM;
	for (i = 0; i < MULTICORE_MEM_REPETITION; i++) {
		p = buf;
		while (p < lastone) {
			for (int j = 0; j < 128; j++) sum += p[j];
			p += 128;
		}
	}
	mc_mem_use(sum);
}

static void mc_mem_fwr(int *buf)
{
	int i, *p, *lastone = buf + MC_MEM_NUM;
	for (i = 0; i < MULTICORE_MEM_REPETITION; i++) {
		p = buf;
		while (p < lastone) {
			for (int j = 0; j < 128; j++) p[j] = 1;
			p += 128;
		}
	}
}

static void mc_mem_fcp(int *dst, int *src)
{
	int i, *pd, *ps, *lastone = dst + MC_MEM_NUM;
	for (i = 0; i < MULTICORE_MEM_REPETITION; i++) {
		pd = dst; ps = src;
		while (pd < lastone) {
			for (int j = 0; j < 128; j++) pd[j] = ps[j];
			pd += 128; ps += 128;
		}
	}
}

static void mc_mem_mset(int *buf)
{
	for (int i = 0; i < MULTICORE_MEM_REPETITION; i++)
		memset(buf, 0, MULTICORE_MEM_SIZE);
}

static void mc_mem_mcpy(int *dst, int *src)
{
	for (int i = 0; i < MULTICORE_MEM_REPETITION; i++)
		memcpy(dst, src, MULTICORE_MEM_SIZE);
}

static void *mc_mem_worker(void *arg)
{
	int pid = (int)(long)arg;
	mc_mem_dst[pid] = (int *)malloc(MULTICORE_MEM_SIZE);
	if (!mc_mem_dst[pid]) {
		sem_post(&mc_mem_response[pid]);
		return NULL;
	}

	sem_post(&mc_mem_response[pid]); /* Ready */
	sem_wait(&mc_mem_order[pid]);    /* Wait for start */

	switch (mc_mem_mode) {
	case MC_MODE_RD:   mc_mem_rd(mc_mem_src); break;
	case MC_MODE_WR:   mc_mem_wr(mc_mem_dst[pid]); break;
	case MC_MODE_CP:   mc_mem_cp(mc_mem_dst[pid], mc_mem_src); break;
	case MC_MODE_FRD:  mc_mem_frd(mc_mem_src); break;
	case MC_MODE_FWR:  mc_mem_fwr(mc_mem_dst[pid]); break;
	case MC_MODE_FCP:  mc_mem_fcp(mc_mem_dst[pid], mc_mem_src); break;
	case MC_MODE_MSET: mc_mem_mset(mc_mem_dst[pid]); break;
	case MC_MODE_MCPY: mc_mem_mcpy(mc_mem_dst[pid], mc_mem_src); break;
	default: break;
	}

	sem_post(&mc_mem_response[pid]); /* Done */
	return NULL;
}

static uint64_t multicore_mem_bw(int mode, int worker_count)
{
	if (mode < 0 || mode > 7 || worker_count <= 0) return 0;
	if (worker_count > MULTICORE_MAX_WORKERS) worker_count = MULTICORE_MAX_WORKERS;

	mc_mem_mode = mode;
	mc_mem_src = (int *)malloc(MULTICORE_MEM_SIZE);
	if (mc_mem_src) memset(mc_mem_src, 0x55, MULTICORE_MEM_SIZE);

	for (int i = 0; i < worker_count; i++) {
		mc_mem_dst[i] = NULL;
		sem_init(&mc_mem_order[i], 0, 0);
		sem_init(&mc_mem_response[i], 0, 0);
		if (pthread_create(&mc_mem_tids[i], NULL, mc_mem_worker, (void *)(long)i) == 0) {
			multicore_cpu_set_t cpuset;
			MULTICORE_CPU_ZERO(&cpuset);
			MULTICORE_CPU_SET(i % USE_PROCESSORS, &cpuset);
			multicore_pthread_setaffinity_np(mc_mem_tids[i], sizeof(cpuset), &cpuset);
		}
	}

	/* Wait for all workers to be ready */
	for (int i = 0; i < worker_count; i++)
		sem_wait(&mc_mem_response[i]);

	uint64_t t_start = timeGet();

	/* Start all workers */
	for (int i = 0; i < worker_count; i++)
		sem_post(&mc_mem_order[i]);

	/* Wait for all workers to complete */
	for (int i = 0; i < worker_count; i++)
		sem_wait(&mc_mem_response[i]);

	uint64_t t_end = timeGet();

	/* Cleanup */
	for (int i = 0; i < worker_count; i++) {
		pthread_join(mc_mem_tids[i], NULL);
		sem_destroy(&mc_mem_order[i]);
		sem_destroy(&mc_mem_response[i]);
		if (mc_mem_dst[i]) free(mc_mem_dst[i]);
	}
	if (mc_mem_src) free(mc_mem_src);

	uint64_t dur = cycles_to_ns(t_end - t_start);
	if (dur == 0) return 0;

	uint64_t data_per_worker;
	if (mode == MC_MODE_RD || mode == MC_MODE_WR) {
		data_per_worker = MULTICORE_MEM_SIZE / 8; /* Sparse: 1/8 effective */
	} else if (mode == MC_MODE_CP) {
		data_per_worker = (MULTICORE_MEM_SIZE / 8) * 2; /* Read + write */
	} else if (mode == MC_MODE_FCP || mode == MC_MODE_MCPY) {
		data_per_worker = (uint64_t)MULTICORE_MEM_SIZE * 2; /* Full read + write */
	} else {
		data_per_worker = (uint64_t)MULTICORE_MEM_SIZE;
	}

	uint64_t total_bytes = data_per_worker * MULTICORE_MEM_REPETITION * worker_count;
	uint64_t bandwidth_mb_s = total_bytes * 1000000000ULL / (dur * 1048576ULL);

	return bandwidth_mb_s;
}

/* ============================================================================
 * Multicore Test Runner
 * ============================================================================ */

int test_realtime_multicore(struct realtime_multicore_result *result)
{
	if (result == NULL)
		return -1;

	memset(result, 0, sizeof(*result));
	realtime_timer_start();

	rt_printf("Running IPC bandwidth tests...\n");

	/* IPC bandwidth tests with different concurrency levels */
	for (int i = 0; i < 4; i++) {
		int concurrency = 1 << i;  /* 1, 2, 4, 8 */
		result->ipc_bandwidth[i] = multicore_ipc_bw(2, concurrency);
		rt_printf("  IPC bandwidth (concurrency %d): %" PRIu64 " GB/s\n",
			  concurrency, result->ipc_bandwidth[i]);
	}

	/* Same-core vs different-core IPC comparison */
	result->ipc_same_core = multicore_ipc_bw(0, 1);
	rt_printf("  IPC same-core: %" PRIu64 " GB/s\n", result->ipc_same_core);

	result->ipc_diff_core = multicore_ipc_bw(1, 1);
	rt_printf("  IPC diff-core: %" PRIu64 " GB/s\n", result->ipc_diff_core);

	rt_printf("Running memory bandwidth tests...\n");

	/* Memory bandwidth tests */
	static const char *mem_mode_names[8] = {
		"rd", "wr", "cp", "frd", "fwr", "fcp", "memset", "memcpy"
	};
	for (int mode = 0; mode < 8; mode++) {
		for (int c = 0; c < 4; c++) {
			int concurrency = 1 << c;  /* 1, 2, 4, 8 */
			result->mem_bandwidth[mode][c] = multicore_mem_bw(mode, concurrency);
			rt_printf("  Memory %s (concurrency %d): %" PRIu64 " MB/s\n",
				  mem_mode_names[mode], concurrency, result->mem_bandwidth[mode][c]);
		}
	}

	rt_printf("Running task create/delete latency tests...\n");

	/* Task create/delete latency */
	for (int i = 0; i < 4; i++) {
		int concurrency = 1 << i;  /* 1, 2, 4, 8 */
		result->task_latency[i] = multicore_task_lat(concurrency);
		rt_printf("  Task latency (concurrency %d): %" PRIu64 " us\n",
			  concurrency, result->task_latency[i] / 1000);
	}

	return 0;
}

/* ============================================================================
 * Result Printing
 * ============================================================================ */

static void print_us(uint64_t ns)
{
	uint64_t us = ns / 1000;
	uint64_t frac = (ns % 1000);
	rt_printf("%" PRIu64 ".%03" PRIu64, us, frac);
}

void test_realtime_print_singlecore(const struct realtime_singlecore_result *r)
{
	static const char *row_names[8] = {
		"信号量获取", "信号量释放",
		"消息发送  ", "消息接收  ",
		"互斥锁获取", "互斥锁释放",
		"内存块申请", "内存块释放"
	};

	static const int has_check[8][4] = {
		{1, 1, 0, 0}, /* sem take: immediate, suspend */
		{1, 0, 1, 1}, /* sem give: immediate, low ready, high resume */
		{1, 1, 1, 1}, /* mq send: immediate, suspend, low ready, high resume */
		{1, 1, 1, 1}, /* mq recv: immediate, suspend, low ready, high resume */
		{1, 1, 0, 0}, /* mutex lock: immediate, suspend */
		{1, 0, 1, 1}, /* mutex unlock: immediate, low ready, high resume */
		{1, 0, 0, 0}, /* malloc: immediate only */
		{1, 0, 0, 0}  /* free: immediate only */
	};

	rt_printf("\n单核系统服务开销 (单位: us):\n");
	rt_printf("%-12s | %-10s | %-10s | %-10s | %-10s\n",
		  "指标", "立即执行", "挂起睡眠", "低优就绪", "高优恢复");
	rt_printf("----------------------------------------------------------------------\n");

	for (int i = 0; i < 8; i++) {
		rt_printf("%-12s", row_names[i]);
		for (int j = 0; j < 4; j++) {
			rt_printf(" | ");
			/* Check for mqueue tests that may be skipped */
			if ((i == 2 && j == 1) || (i == 3 && j == 2) || (i == 3 && j == 3)) {
				if (message_queue_filled_behavior != 0) {
					rt_printf("%-10s", "-");
					continue;
				}
			}
			if (has_check[i][j] && r->service_cost[i][j] > 0) {
				print_us(r->service_cost[i][j]);
			} else {
				rt_printf("-         ");
			}
		}
		rt_printf("\n");
	}

	rt_printf("\n上下文切换延迟 AVG: ");
	print_us(r->context_switch_avg);
	rt_printf(" us\n");

	if (r->interrupt_avg > 0) {
		rt_printf("中断软件延迟 MIN: ");
		print_us(r->interrupt_min);
		rt_printf(" MAX: ");
		print_us(r->interrupt_max);
		rt_printf(" AVG: ");
		print_us(r->interrupt_avg);
		rt_printf(" us\n");
	}

	if (r->syscall_avg > 0) {
		rt_printf("系统调用延迟 MIN: ");
		print_us(r->syscall_min);
		rt_printf(" MAX: ");
		print_us(r->syscall_max);
		rt_printf(" AVG: ");
		print_us(r->syscall_avg);
		rt_printf(" us\n");
	}
}

void test_realtime_print_multicore(const struct realtime_multicore_result *r)
{
	static const char *mem_mode_names[8] = {
		"rd    ", "wr    ", "cp    ", "frd   ", "fwr   ", "fcp   ", "memset", "memcpy"
	};

	rt_printf("\n多核存取性能 (单位: MB/s):\n");
	rt_printf("%-10s | %-10s | %-10s | %-10s | %-10s\n",
		  "模式/并发", "1", "2", "4", "8");
	rt_printf("----------------------------------------------------------------------\n");

	for (int i = 0; i < 8; i++) {
		rt_printf("%-10s", mem_mode_names[i]);
		for (int j = 0; j < 4; j++) {
			rt_printf(" | %-10" PRIu64, r->mem_bandwidth[i][j]);
		}
		rt_printf("\n");
	}

	rt_printf("\n多核系统服务:\n");
	rt_printf("%-10s | %-10s | %-10s | %-10s | %-10s\n",
		  "指标/并发", "1", "2", "4", "8");
	rt_printf("----------------------------------------------------------------------\n");

	rt_printf("任务间通信");
	for (int j = 0; j < 4; j++) {
		rt_printf(" | %-10" PRIu64, r->ipc_bandwidth[j]);
	}
	rt_printf(" (GB/s)\n");

	rt_printf("创建与删除");
	for (int j = 0; j < 4; j++) {
		uint64_t us = r->task_latency[j] / 1000;
		rt_printf(" | %-10" PRIu64, us);
	}
	rt_printf(" (us)\n");

	rt_printf("\n任务在同核与异核上通信比较:\n");
	rt_printf("  同核通信: %" PRIu64 " GB/s\n", r->ipc_same_core);
	rt_printf("  异核通信: %" PRIu64 " GB/s\n", r->ipc_diff_core);
}

/* ============================================================================
 * Main Entry Point
 * ============================================================================ */

int test_realtime_run(int run_multicore)
{
	int ret;

	rt_printf("\n");
	rt_printf("=============================================================\n");
	rt_printf("           RTOS-Bench Realtime Performance Test\n");
	rt_printf("=============================================================\n\n");

	rt_printf("[Phase 1] Running single-core realtime tests...\n");
	rt_printf("-------------------------------------------------------------\n");

	ret = test_realtime_singlecore(&s_result.singlecore);
	if (ret != 0) {
		rt_printf("Single-core tests failed: %d\n", ret);
		return ret;
	}

	if (run_multicore) {
		rt_printf("\n[Phase 2] Running multicore tests...\n");
		rt_printf("-------------------------------------------------------------\n");

		ret = test_realtime_multicore(&s_result.multicore);
		if (ret != 0) {
			rt_printf("Multicore tests failed: %d\n", ret);
			return ret;
		}
		s_result.multicore_tested = 1;
	} else {
		s_result.multicore_tested = 0;
	}

	rt_printf("\n=============================================================\n");
	rt_printf("                       Test Results\n");
	rt_printf("=============================================================\n");

	test_realtime_print_singlecore(&s_result.singlecore);

	if (s_result.multicore_tested) {
		test_realtime_print_multicore(&s_result.multicore);
	}

	rt_printf("\n=============================================================\n");
	rt_printf("                     Test Complete\n");
	rt_printf("=============================================================\n");

	return 0;
}

const struct test_realtime_result *test_realtime_get_result(void)
{
	return &s_result;
}
