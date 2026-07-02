/**
 * @file test_schedule.c
 * @brief Schedulability verification test implementation
 * @details Implements concurrent periodic task execution with deadline tracking
 *          for RTOS scheduling performance evaluation.
 *
 * Algorithm:
 * 1. Measure WCET for each workload by running multiple iterations
 * 2. Use UUniFast to generate utilization distributions for each gradient
 * 3. Calculate period for each task: T = WCET / U (implicit deadline model)
 * 4. Run concurrent periodic tasks using platform threads
 * 5. Track deadline misses and calculate final score
 */

#include "test_schedule.h"
#include "uunifast.h"
#include "platform_abstraction.h"
#include "workload_registry.h"
#include "logging.h"
#include "test_schedule/sched_workloads.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef RT_THREAD_PLATFORM
#include <rtthread.h>
#define SCHED_PRINTF rt_kprintf
#define SCHED_THREAD_STACK_SIZE (32 * 1024)
#define SCHED_THREAD_PRIORITY   15
#else
#include <pthread.h>
#include <unistd.h>   /* usleep() for the watchdog polling loop */
#define SCHED_PRINTF printf
/* Per-task pthread stack.  Memory-constrained boards (e.g. OneOS Nezha D1H,
 * RISC-V) cannot afford 4 MB x N tasks, so shrink there.  Ruihua/ReWorks heap
 * likewise refuses 4 MB per task (EXCEPTION HELP CODE 0x13005), which distorts
 * the schedule gradient, so it uses the small stack too.  EKF is the known
 * stack-sensitive workload; bump if 256 KB proves insufficient. */
#if defined(ONEOS_PLATFORM) || defined(RUIHUA_PLATFORM)
#define SCHED_POSIX_STACK_SIZE (256 * 1024)
#else
#define SCHED_POSIX_STACK_SIZE (4 * 1024 * 1024)
#endif
#endif

/* Period upper bound and watchdog wall-clock budgets are defined centrally in
 * test_schedule.h (TEST_SCHEDULE_MAX_PERIOD_NS / *_BUDGET_MS).  Defaults keep
 * existing board behaviour; a board may override any of them with a single -D. */

/**
 * Global flag: when nonzero, workloads should suppress console output.
 * Set during concurrent task execution in test-schedule to prevent
 * printf → dfs_file_lock → _rt_mutex_take crashes on RT-Thread.
 */
volatile int g_sched_suppress_output = 0;

/* Static result storage */
static struct test_schedule_result g_result;

/* Task thread context */
struct task_thread_ctx {
	struct schedule_task_config *config;
	struct schedule_task_stats *stats;
	int target_cycles;
	volatile int running;
	volatile int completed;
	rtbench_sem_t period_sem;
	rtbench_timer_t period_timer;
#ifdef RT_THREAD_PLATFORM
	rt_thread_t thread;
#else
	pthread_t thread;
#endif
};

/* Forward declarations */
static uint64_t measure_wcet_ns(const struct rtosbench_workload *wl, int iterations);
static void task_thread_entry(void *param);
static int compare_double_desc(const void *a, const void *b);
static int compare_wcet_desc(const void *a, const void *b);
static int is_builtin_workload(const struct rtosbench_workload *wl);
static const struct sched_workload_wrapper *get_sched_wrapper_for_workload(
	const struct rtosbench_workload *wl);
static int sched_workload_init(const struct rtosbench_workload *wl,
			       const struct sched_workload_wrapper *wrapper);
static int sched_workload_exec(const struct rtosbench_workload *wl,
			       const struct sched_workload_wrapper *wrapper);
static void sched_workload_teardown(const struct rtosbench_workload *wl,
				    const struct sched_workload_wrapper *wrapper);

/**
 * @brief Check if a workload is a built-in utility workload
 * @details Built-in workloads (stub, busywait) have category "utility"
 *          and should be excluded from automated tests like test-schedule.
 *          They are only intended for explicit user invocation.
 */
static int is_builtin_workload(const struct rtosbench_workload *wl)
{
	if (!wl || !wl->category) {
		return 0;
	}
	/* Built-in utility workloads: stub, busywait */
	if (strcmp(wl->category, "utility") == 0) {
		return 1;
	}
	return 0;
}

static const struct sched_workload_wrapper *get_sched_wrapper_for_workload(
	const struct rtosbench_workload *wl)
{
	if (!wl || !wl->name) {
		return NULL;
	}
	return sched_get_wrapper(wl->name);
}

/*
 * Board-adaptable workload allow / exclude lists for test-schedule.
 *
 * A workload that hard-faults the OS (page fault / OOM / driver wedge) cannot be
 * caught by the in-process watchdog; the only robust fallback is to never run
 * it.  These two compile-time, comma-separated lists let a board pin an explicit
 * allowlist or exclude known-bad workloads, so acceptance always completes with
 * a valid score over the remaining REAL workloads (not a spin/mock placeholder).
 * Defaults: both empty => every registered workload runs (no change for existing
 * boards).  Examples:
 *   -DTEST_SCHEDULE_WORKLOAD_ALLOWLIST="fast,pid,cusum,ewma"   (only these run)
 *   -DTEST_SCHEDULE_WORKLOAD_EXCLUDE="icp,ekf"                 (all but these)
 */
static int sched_csv_contains(const char *csv, const char *name)
{
	size_t nlen;
	const char *p = csv;
	if (!csv || !name) {
		return 0;
	}
	nlen = strlen(name);
	while (*p) {
		const char *start;
		size_t len;
		while (*p == ',' || *p == ' ') {
			p++;
		}
		start = p;
		while (*p && *p != ',') {
			p++;
		}
		len = (size_t)(p - start);
		while (len > 0 && start[len - 1] == ' ') {
			len--;
		}
		if (len == nlen && strncmp(start, name, nlen) == 0) {
			return 1;
		}
	}
	return 0;
}

static int is_workload_allowed(const char *name)
{
	if (!name) {
		return 0;
	}
#ifdef TEST_SCHEDULE_WORKLOAD_ALLOWLIST
	if (TEST_SCHEDULE_WORKLOAD_ALLOWLIST[0] != '\0' &&
	    !sched_csv_contains(TEST_SCHEDULE_WORKLOAD_ALLOWLIST, name)) {
		return 0;
	}
#endif
#ifdef TEST_SCHEDULE_WORKLOAD_EXCLUDE
	if (sched_csv_contains(TEST_SCHEDULE_WORKLOAD_EXCLUDE, name)) {
		return 0;
	}
#endif
	return 1;
}

static int sched_workload_init(const struct rtosbench_workload *wl,
			       const struct sched_workload_wrapper *wrapper)
{
	if (wrapper && wrapper->init) {
		return wrapper->init();
	}
	if (wl && wl->init) {
		return wl->init(0, NULL);
	}
	return 0;
}

static int sched_workload_exec(const struct rtosbench_workload *wl,
			       const struct sched_workload_wrapper *wrapper)
{
	if (wrapper && wrapper->quick_exec) {
		return wrapper->quick_exec();
	}
	if (wl && wl->exec) {
		wl->exec(0, NULL);
	}
	return 0;
}

static void sched_workload_teardown(const struct rtosbench_workload *wl,
				    const struct sched_workload_wrapper *wrapper)
{
	if (wrapper && wrapper->teardown) {
		wrapper->teardown();
		return;
	}
	if (wl && wl->teardown) {
		wl->teardown(0, NULL);
	}
}

/* Timer callback for period expiration */
static void period_timer_callback(void *user_data)
{
	struct task_thread_ctx *ctx = (struct task_thread_ctx *)user_data;
	if (ctx && ctx->period_sem) {
		rtbench_sem_post(ctx->period_sem);
	}
}

/**
 * @brief Measure WCET by running workload multiple times
 */
static uint64_t measure_wcet_ns(const struct rtosbench_workload *wl, int iterations)
{
	uint64_t max_duration = 0;
	const struct sched_workload_wrapper *wrapper = get_sched_wrapper_for_workload(wl);

	if (!wl || (!wl->exec && (!wrapper || !wrapper->quick_exec))) {
		return 0;
	}

	if (sched_workload_init(wl, wrapper) != 0) {
		SCHED_PRINTF("[test-schedule] Warning: %s schedule init returned error; continuing\n",
			     wl && wl->name ? wl->name : "unknown");
	}

	/* Run iterations and record max */
	for (int i = 0; i < iterations; i++) {
		long double start = rtbench_get_timestamp();
		int exec_rc = sched_workload_exec(wl, wrapper);
		long double end = rtbench_get_timestamp();
		if (exec_rc != 0) {
			SCHED_PRINTF("[test-schedule] Warning: %s schedule wrapper returned %d during WCET; continuing\n",
				     wl && wl->name ? wl->name : "unknown",
				     exec_rc);
		}

		uint64_t duration_ns = (uint64_t)((end - start) * 1000000000.0L);
		if (duration_ns > max_duration) {
			max_duration = duration_ns;
		}
	}

	sched_workload_teardown(wl, wrapper);

	return max_duration;
}

/**
 * @brief Task thread entry point
 */
static void task_thread_entry(void *param)
{
	struct task_thread_ctx *ctx = (struct task_thread_ctx *)param;
	const struct rtosbench_workload *wl = NULL;
	const struct sched_workload_wrapper *wrapper = NULL;

	if (!ctx || !ctx->config) {
		return;
	}

	/* Get workload by index */
	wl = rtosbench_get_workload(ctx->config->workload_idx);
	if (!wl) {
		SCHED_PRINTF("[test-schedule] Task %s: workload not found\n",
			     ctx->config->name);
		ctx->completed = 1;
		return;
	}

	wrapper = get_sched_wrapper_for_workload(wl);
	if (sched_workload_init(wl, wrapper) != 0) {
		SCHED_PRINTF("[test-schedule] Task %s: init warning, continuing\n",
			     ctx->config->name);
	}

	/* Create period semaphore */
	ctx->period_sem = rtbench_sem_create(0);
	if (!ctx->period_sem) {
		SCHED_PRINTF("[test-schedule] Task %s: sem_create failed\n",
			     ctx->config->name);
		ctx->completed = 1;
		return;
	}

	/* Create and start periodic timer */
	ctx->period_timer = rtbench_timer_create(RTBENCH_TIMER_PERIOD,
						  period_timer_callback, ctx);
	if (!ctx->period_timer) {
		SCHED_PRINTF("[test-schedule] Task %s: timer_create failed\n",
			     ctx->config->name);
		rtbench_sem_destroy(ctx->period_sem);
		ctx->completed = 1;
		return;
	}

	long period_sec = (long)(ctx->config->period_ns / 1000000000ULL);
	long period_nsec = (long)(ctx->config->period_ns % 1000000000ULL);

	if (rtbench_timer_settime(ctx->period_timer, period_sec, period_nsec) < 0) {
		SCHED_PRINTF("[test-schedule] Task %s: timer_settime failed\n",
			     ctx->config->name);
		rtbench_timer_delete(ctx->period_timer);
		rtbench_sem_destroy(ctx->period_sem);
		ctx->completed = 1;
		return;
	}

	/* Main execution loop */
	while (ctx->running && ctx->stats->total_jobs < (uint64_t)ctx->target_cycles) {
		/* Wait for period signal */
		if (rtbench_sem_wait(ctx->period_sem) < 0) {
			continue;
		}

		/* Record activation time */
		long double activation = rtbench_get_timestamp();

		/* Execute workload */
		int exec_rc = sched_workload_exec(wl, wrapper);

		/* Record completion time */
		long double completion = rtbench_get_timestamp();

		/* Calculate response time */
		uint64_t response_ns = (uint64_t)((completion - activation) * 1000000000.0L);
		ctx->stats->total_jobs++;
		ctx->stats->total_response_ns += response_ns;

		if (response_ns > ctx->stats->max_response_ns) {
			ctx->stats->max_response_ns = response_ns;
		}

		if (exec_rc != 0) {
			SCHED_PRINTF("[test-schedule] Task %s: workload warning rc=%d, counted as miss\n",
				     ctx->config->name, exec_rc);
			ctx->stats->deadline_misses++;
		}

		/* Check deadline (deadline = period for implicit deadline tasks) */
		if (response_ns > ctx->config->deadline_ns) {
			ctx->stats->deadline_misses++;
		}
	}

	/* Cleanup */
	rtbench_timer_delete(ctx->period_timer);
	rtbench_sem_destroy(ctx->period_sem);

	sched_workload_teardown(wl, wrapper);

	ctx->completed = 1;
}

/**
 * @brief Compare doubles descending (for qsort)
 */
static int compare_double_desc(const void *a, const void *b)
{
	double da = *(const double *)a;
	double db = *(const double *)b;
	if (da < db) return 1;
	if (da > db) return -1;
	return 0;
}

/**
 * @brief Compare tasks by WCET descending (for qsort)
 */
static int compare_wcet_desc(const void *a, const void *b)
{
	const struct schedule_task_config *ta = (const struct schedule_task_config *)a;
	const struct schedule_task_config *tb = (const struct schedule_task_config *)b;
	if (ta->wcet_ns < tb->wcet_ns) return 1;
	if (ta->wcet_ns > tb->wcet_ns) return -1;
	return 0;
}

#ifdef RT_THREAD_PLATFORM
/**
 * @brief Create and start task thread on RT-Thread
 */
static int create_task_thread(struct task_thread_ctx *ctx, const char *name)
{
	char thread_name[RT_NAME_MAX];
	snprintf(thread_name, RT_NAME_MAX, "ts_%s", name);

	ctx->thread = rt_thread_create(thread_name,
				       task_thread_entry,
				       ctx,
				       SCHED_THREAD_STACK_SIZE,
				       SCHED_THREAD_PRIORITY,
				       20);
	if (ctx->thread == RT_NULL) {
		return -1;
	}

	return rt_thread_startup(ctx->thread);
}

#else /* Linux/POSIX */

static void *pthread_entry_wrapper(void *param)
{
	task_thread_entry(param);
	return NULL;
}

static int create_task_thread(struct task_thread_ctx *ctx, const char *name)
{
	pthread_attr_t attr;
	int ret;

	(void)name;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, SCHED_POSIX_STACK_SIZE);
	ret = pthread_create(&ctx->thread, &attr, pthread_entry_wrapper, ctx);
	pthread_attr_destroy(&attr);

	return ret;
}

#endif

/* ----------------------------------------------------------------------------
 * Watchdog: bound the join wall-clock so test-schedule always reaches Phase 3
 * and prints Final Score, even if a workload job blocks forever.  All four
 * target boards (OneOS / Ruihua / Dongtu / SylixOS) use the POSIX path; the
 * RT_THREAD path is QEMU-dev only and keeps its original polling semantics.
 * ------------------------------------------------------------------------- */

/* Sleep helper for the watchdog polling loop (100 ms granularity). */
static void sched_watchdog_sleep_ms(unsigned ms)
{
#ifdef RT_THREAD_PLATFORM
	rt_thread_mdelay((rt_int32_t)ms);
#else
	usleep(ms * 1000u);
#endif
}

/* Per-task progress-stall watchdog.
 *
 * The previous implementation bounded the *wall-clock* of a gradient with a
 * fixed budget (e.g. 60 s).  That conflates "slow" with "stuck": a real
 * workload whose single job legitimately takes seconds (FAST ~10 s/img on
 * LoongArch, MODBUS ~9 s) keeps making progress yet blew the budget and was
 * force-abandoned -- which then exposed the timer double-free.  The correct
 * "stuck" signal is not elapsed time but *no forward progress*: a task that
 * never completes another job (deadlocked, spinning, or sem_wait that will
 * never be posted).
 *
 * So we watch each task's own job counter.  A task is considered stuck only
 * when it produces no new job for longer than a window derived from its OWN
 * period -- stall_window[i] = period_ns[i] * K + margin.  The longest legal
 * gap between two jobs is roughly one period (sem_wait for the next tick) plus
 * one execution, so K periods of zero progress means it is genuinely wedged,
 * not merely waiting.  Slow tasks (large period) get a proportionally larger
 * window and are never misjudged; fast tasks (small period) are caught quickly.
 *
 * `deadline_ts` remains as an absolute outermost backstop so the command is
 * guaranteed to terminate even in pathological cases the per-task logic does
 * not cover; under normal acceptance it is never reached.
 *
 * Returns 1 if all tasks completed; 0 if the backstop fired or every
 * not-completed task is individually stalled. */

/* Zero-progress periods tolerated before a task is declared stuck. */
#ifndef TEST_SCHEDULE_STALL_PERIODS
#define TEST_SCHEDULE_STALL_PERIODS 3u
#endif
/* Lower bound on a stall window (ns), so tasks with tiny periods still get a
 * sane grace span and we never thrash on sub-second windows. */
#ifndef TEST_SCHEDULE_STALL_MIN_NS
#define TEST_SCHEDULE_STALL_MIN_NS (5ULL * 1000000000ULL) /* 5 s */
#endif

static int wait_all_tasks_deadline(struct task_thread_ctx *contexts, int n,
				   long double deadline_ts)
{
	long double now = rtbench_get_timestamp();
	int i;

	/* Per-task progress tracking, parallel to contexts[].  Heap-allocated to
	 * avoid assuming a board-specific max task count on the stack. */
	uint64_t *last_jobs = (uint64_t *)calloc((size_t)n, sizeof(uint64_t));
	long double *last_progress_ts =
		(long double *)calloc((size_t)n, sizeof(long double));

	if (!last_jobs || !last_progress_ts) {
		/* Allocation failure: fall back to the absolute backstop only. */
		free(last_jobs);
		free(last_progress_ts);
		for (;;) {
			int done = 1;
			for (i = 0; i < n; i++) {
				if (!contexts[i].completed) {
					done = 0;
					break;
				}
			}
			if (done) {
				return 1;
			}
			if (rtbench_get_timestamp() >= deadline_ts) {
				return 0;
			}
			sched_watchdog_sleep_ms(100);
		}
	}

	for (i = 0; i < n; i++) {
		last_jobs[i] = contexts[i].stats->total_jobs;
		last_progress_ts[i] = now;
	}

	for (;;) {
		int done = 1;
		int all_stalled = 1;

		now = rtbench_get_timestamp();

		for (i = 0; i < n; i++) {
			uint64_t jobs;
			long double window_s;

			if (contexts[i].completed) {
				continue; /* finished: neither pending nor stalled */
			}
			done = 0;

			/* Refresh the progress timestamp whenever this task has
			 * completed at least one more job since we last looked. */
			jobs = contexts[i].stats->total_jobs;
			if (jobs != last_jobs[i]) {
				last_jobs[i] = jobs;
				last_progress_ts[i] = now;
			}

			/* Stall window derived from this task's own period. */
			{
				uint64_t win_ns =
					contexts[i].config->period_ns *
					(uint64_t)TEST_SCHEDULE_STALL_PERIODS;
				if (win_ns < TEST_SCHEDULE_STALL_MIN_NS) {
					win_ns = TEST_SCHEDULE_STALL_MIN_NS;
				}
				window_s = (long double)win_ns / 1000000000.0L;
			}

			if (now - last_progress_ts[i] < window_s) {
				all_stalled = 0; /* still within its grace window */
			}
		}

		if (done) {
			free(last_jobs);
			free(last_progress_ts);
			return 1;
		}

		/* Stuck only if EVERY not-completed task has exceeded its own
		 * stall window with zero new jobs -- genuine wedge, not slowness. */
		if (all_stalled) {
			free(last_jobs);
			free(last_progress_ts);
			return 0;
		}

		/* Absolute outermost backstop: guarantees termination. */
		if (rtbench_get_timestamp() >= deadline_ts) {
			free(last_jobs);
			free(last_progress_ts);
			return 0;
		}

		sched_watchdog_sleep_ms(100);
	}
}

/* Reap finished threads; abandon (never hard-kill) any still running after a
 * watchdog timeout.  Hard-kill (pthread_cancel / rt_thread_delete) is avoided:
 * a thread stuck in sem_wait / mid-exec would skip its own cleanup and leave
 * the kernel in an unknown state. */
static void sched_reap_or_abandon(struct task_thread_ctx *contexts, int n)
{
#ifdef RT_THREAD_PLATFORM
	/* Dynamic RT-Thread threads self-recycle once their entry returns; a
	 * genuinely stuck one is left resident until the command ends. */
	(void)contexts;
	(void)n;
#else
	int i;
	for (i = 0; i < n; i++) {
		if (contexts[i].completed) {
			pthread_join(contexts[i].thread, NULL);
		} else {
			pthread_detach(contexts[i].thread);
		}
	}
#endif
}

/**
 * @brief Run one gradient of the schedulability test
 */
static int run_gradient(int num_tasks, struct schedule_task_config *tasks,
			struct schedule_task_stats *stats, int cycles,
			struct schedule_gradient_result *result,
			long double test_deadline_ts)
{
	struct task_thread_ctx *contexts;
	int i;

	contexts = (struct task_thread_ctx *)calloc(num_tasks,
						    sizeof(struct task_thread_ctx));
	if (!contexts) {
		SCHED_PRINTF("[test-schedule] Failed to allocate thread contexts\n");
		return -1;
	}

	/* Initialize contexts and reset stats */
	for (i = 0; i < num_tasks; i++) {
		contexts[i].config = &tasks[i];
		contexts[i].stats = &stats[i];
		contexts[i].target_cycles = cycles;
		contexts[i].running = 1;
		contexts[i].completed = 0;

		memset(&stats[i], 0, sizeof(stats[i]));
		stats[i].name = tasks[i].name;
	}

	/* Suppress workload output during concurrent execution.
	 * Workloads like FAST/EKF/MODBUS call printf during exec(), which on
	 * RT-Thread goes through dfs_file_lock → _rt_mutex_take.  Under heavy
	 * concurrent load this can trigger "scheduler is not available" assertion
	 * when threads contend on the console mutex. */
	g_sched_suppress_output = 1;

	/* Create and start all task threads */
	for (i = 0; i < num_tasks; i++) {
		if (create_task_thread(&contexts[i], tasks[i].name) != 0) {
			SCHED_PRINTF("[test-schedule] Failed to create thread for %s\n",
				     tasks[i].name);
			/* Stop already-started threads */
			for (int j = 0; j < i; j++) {
				contexts[j].running = 0;
			}
			g_sched_suppress_output = 0;
			free(contexts);
			return -1;
		}
	}

	/* Wait for all threads.  Progress (per-task stall detection inside
	 * wait_all_tasks_deadline) decides "stuck"; the only hard time cap is the
	 * overall test backstop, which bounds the whole run regardless of how many
	 * gradients remain.  There is deliberately no fixed per-gradient wall-clock
	 * budget any more: a gradient whose real workloads are slow but still
	 * completing jobs must be allowed to finish, not cut at an arbitrary
	 * deadline (that was the old false-positive that forced 7/17 completions). */
	long double grad_deadline = test_deadline_ts;

	if (!wait_all_tasks_deadline(contexts, num_tasks, grad_deadline)) {
		int leaked = 0;

		SCHED_PRINTF("[test-schedule] WATCHDOG: tasks stalled (no job progress) "
			     "or backstop reached, forcing completion\n");

		/* Signal every thread to stop; count the not-completed ones as
		 * leaked.  Do NOT touch contexts[i].period_timer here: a thread
		 * that is just finishing frees its own timer (task_thread_entry)
		 * BEFORE it sets `completed`, leaving period_timer dangling.  If
		 * the watchdog freed it too, a `completed == 0` read followed by
		 * the thread's own free would double-free the timer struct
		 * (ReWorks EXCEPTION 0x14001 "Invalid pointer to be freed").
		 * Per-thread resources are owned solely by their thread; a
		 * genuinely-stuck thread's timer is leaked, consistent with the
		 * bounded context leak below. */
		for (i = 0; i < num_tasks; i++) {
			contexts[i].running = 0;
			if (!contexts[i].completed) {
				leaked++;
			}
		}

		/* Reap finished threads; detach (abandon) stuck ones. */
		sched_reap_or_abandon(contexts, num_tasks);

		/* Always restore output before returning. */
		g_sched_suppress_output = 0;

		/* Aggregate whatever the threads managed to record. */
		result->total_jobs = 0;
		result->total_misses = 0;
		result->num_tasks = num_tasks;
		for (i = 0; i < num_tasks; i++) {
			result->total_jobs += stats[i].total_jobs;
			result->total_misses += stats[i].deadline_misses;
			result->task_stats[i] = stats[i];
		}
		if (result->total_jobs > 0) {
			result->miss_rate = (double)result->total_misses /
					    (double)result->total_jobs;
		} else {
			result->miss_rate = 0.0;
		}

		/* Deliberately do NOT free(contexts): a detached/stuck thread or a
		 * late timer callback may still reference it, so freeing risks a
		 * use-after-free.  Bounded one-shot leak. */
		if (leaked > 0) {
			SCHED_PRINTF("[test-schedule] WATCHDOG: abandoned %d stuck task(s); "
				     "leaking contexts to avoid use-after-free\n", leaked);
		}
		return 0;
	}

	g_sched_suppress_output = 0;

	/* Aggregate results */
	result->total_jobs = 0;
	result->total_misses = 0;
	result->num_tasks = num_tasks;

	for (i = 0; i < num_tasks; i++) {
		result->total_jobs += stats[i].total_jobs;
		result->total_misses += stats[i].deadline_misses;
		result->task_stats[i] = stats[i];
	}

	if (result->total_jobs > 0) {
		result->miss_rate = (double)result->total_misses / (double)result->total_jobs;
	} else {
		result->miss_rate = 0.0;
	}

	free(contexts);
	return 0;
}

int test_schedule_run_custom(int cycles, int util_start, int util_end, int util_step)
{
	int num_workloads;
	int num_gradients;
	int gradient_idx;
	int i, u_percent;
	struct schedule_task_config *tasks;
	struct schedule_task_stats *stats;
	double *wcets_ns;
	double *generated_u;
	int *workload_indices;
	double sum_mr;
	int total_workloads;
	int valid_idx;

	/* Clear previous results */
	memset(&g_result, 0, sizeof(g_result));

	total_workloads = rtosbench_workload_count();
	if (total_workloads == 0) {
		SCHED_PRINTF("[test-schedule] No workloads registered\n");
		return -1;
	}

	/* Count non-builtin workloads (exclude utility workloads like stub/busywait) */
	num_workloads = 0;
	for (i = 0; i < total_workloads; i++) {
		const struct rtosbench_workload *wl = rtosbench_get_workload(i);
		if (wl && wl->name && !is_builtin_workload(wl)) {
			num_workloads++;
		}
	}

	if (num_workloads == 0) {
		SCHED_PRINTF("[test-schedule] No industrial workloads registered (only utility workloads found)\n");
		return -1;
	}

	SCHED_PRINTF("[test-schedule] Found %d industrial workloads (excluded %d utility workloads)\n",
		     num_workloads, total_workloads - num_workloads);

	if (num_workloads > TEST_SCHEDULE_MAX_TASKS) {
		num_workloads = TEST_SCHEDULE_MAX_TASKS;
	}

	/* Allocate arrays */
	tasks = (struct schedule_task_config *)calloc(num_workloads,
						      sizeof(struct schedule_task_config));
	stats = (struct schedule_task_stats *)calloc(num_workloads,
						     sizeof(struct schedule_task_stats));
	wcets_ns = (double *)calloc(num_workloads, sizeof(double));
	generated_u = (double *)calloc(num_workloads, sizeof(double));
	workload_indices = (int *)calloc(num_workloads, sizeof(int));

	if (!tasks || !stats || !wcets_ns || !generated_u || !workload_indices) {
		SCHED_PRINTF("[test-schedule] Memory allocation failed\n");
		goto cleanup;
	}

	/* ================================================================
	 * Phase 1: WCET Measurement (skip builtin utility workloads)
	 * ================================================================ */
	SCHED_PRINTF("=============================================================\n");
	SCHED_PRINTF("[Phase 1] Measuring WCET for %d workloads...\n", num_workloads);
	SCHED_PRINTF("=============================================================\n");

	int is_quick = (cycles <= TEST_SCHEDULE_QUICK_CYCLES);

	/* Overall watchdog deadline: bounds the whole run (WCET measurement +
	 * every gradient) so Phase 3 / Final Score is always reached. */
	long double test_deadline_ts = rtbench_get_timestamp() +
		(long double)TEST_SCHEDULE_TOTAL_BUDGET_MS / 1000.0L;

	valid_idx = 0;
	for (i = 0; i < total_workloads && valid_idx < num_workloads; i++) {
		const struct rtosbench_workload *wl = rtosbench_get_workload(i);
		if (!wl || !wl->name) {
			continue;
		}

		/* Skip builtin utility workloads (stub, busywait) */
		if (is_builtin_workload(wl)) {
			continue;
		}

		/* Board fallback: skip workloads excluded by allow/exclude list,
		 * BEFORE any execution, so a known OS-crashing workload is never
		 * run and acceptance still completes over the rest. */
		if (!is_workload_allowed(wl->name)) {
			SCHED_PRINTF("  [%s]: skipped by board workload allow/exclude list\n",
				     wl->name);
			continue;
		}

		tasks[valid_idx].name = wl->name;
		tasks[valid_idx].workload_idx = i;
		workload_indices[valid_idx] = i;

		int wcet_iters = is_quick
			? TEST_SCHEDULE_QUICK_WCET_ITERATIONS
			: TEST_SCHEDULE_WCET_ITERATIONS;
		uint64_t wcet = measure_wcet_ns(wl, wcet_iters);
		/* Use the real measured WCET -- no cap.  Capping WCET (formerly
		 * wrapper->max_wcet_ms) distorts the period T = WCET / U so the
		 * task's deadline no longer matches its true execution time,
		 * producing meaningless miss rates.  The schedulability result
		 * must be faithful to the measured workload, per the acceptance
		 * report baseline. */
		tasks[valid_idx].wcet_ns = wcet;
		wcets_ns[valid_idx] = (double)wcet;

		SCHED_PRINTF("  [%s]: WCET = %.3f ms (%d iters)\n",
			     wl->name, (double)wcet / 1000000.0, wcet_iters);

		valid_idx++;
	}

	/* Full mode keeps every workload; quick mode keeps bounded smoke cases. */
	num_workloads = valid_idx;
	if (num_workloads == 0) {
		SCHED_PRINTF("[test-schedule] No workloads remain after quick-mode filtering\n");
		goto cleanup;
	}
	SCHED_PRINTF("[Phase 1] %d workloads measured\n", num_workloads);

	/* Sort tasks by WCET descending */
	qsort(tasks, num_workloads, sizeof(struct schedule_task_config),
	      compare_wcet_desc);

	/* ================================================================
	 * Phase 2: Run Task Sets for each utilization gradient
	 * ================================================================ */
	SCHED_PRINTF("\n=============================================================\n");
	SCHED_PRINTF("[Phase 2] Running Task Sets (U: %d%% - %d%%, step %d%%)\n",
		     util_start, util_end, util_step);
	SCHED_PRINTF("=============================================================\n");

	num_gradients = (util_end - util_start) / util_step + 1;
	if (num_gradients > TEST_SCHEDULE_NUM_GRADIENTS) {
		num_gradients = TEST_SCHEDULE_NUM_GRADIENTS;
	}

	gradient_idx = 0;
	for (u_percent = util_start; u_percent <= util_end && gradient_idx < num_gradients;
	     u_percent += util_step) {
		double target_u = (double)u_percent / 100.0;

		/* Overall watchdog: stop launching gradients once the total
		 * budget is exhausted; remaining gradients are simply not run and
		 * num_gradients reflects only what completed. */
		if (rtbench_get_timestamp() >= test_deadline_ts) {
			SCHED_PRINTF("[test-schedule] WATCHDOG: total budget exhausted, "
				     "skipping remaining gradients\n");
			break;
		}

		SCHED_PRINTF("\n>>> Utilization Gradient: %d%% <<<\n", u_percent);

		/* Generate utilization distribution using UUniFast */
		uunifast(num_workloads, target_u, generated_u);

		/* Sort generated utilizations descending */
		qsort(generated_u, num_workloads, sizeof(double), compare_double_desc);

		/* Assign utilizations and calculate periods */
		SCHED_PRINTF("%-12s | %-10s | %-8s | %-12s\n",
			     "Name", "WCET(ms)", "Util(%)", "Period(ms)");
		SCHED_PRINTF("------------------------------------------------------\n");

		for (i = 0; i < num_workloads; i++) {
			tasks[i].utilization = generated_u[i];

			if (generated_u[i] > 0.001) {
				/* T = C / U */
				tasks[i].period_ns = (uint64_t)((double)tasks[i].wcet_ns /
								generated_u[i]);
			} else {
				/* Very low utilization - use a very long period */
				tasks[i].period_ns = 10000000000ULL; /* 10 seconds */
			}

			/* Clamp period to the board-adaptable upper bound.  Default
			 * (UINT64_MAX) is a no-op; boards tighten via -D. */
			if (tasks[i].period_ns > TEST_SCHEDULE_MAX_PERIOD_NS) {
				tasks[i].period_ns = TEST_SCHEDULE_MAX_PERIOD_NS;
			}

			/* Implicit deadline: D = T */
			tasks[i].deadline_ns = tasks[i].period_ns;

			SCHED_PRINTF("%-12s | %-10.3f | %-8.2f | %-12.3f\n",
				     tasks[i].name,
				     (double)tasks[i].wcet_ns / 1000000.0,
				     tasks[i].utilization * 100.0,
				     (double)tasks[i].period_ns / 1000000.0);
		}

		/* Run this task set */
		SCHED_PRINTF("Running task set for %d cycles...\n", cycles);

		g_result.gradients[gradient_idx].utilization_percent = u_percent;
		g_result.gradients[gradient_idx].actual_utilization = target_u;

		if (run_gradient(num_workloads, tasks, stats, cycles,
				 &g_result.gradients[gradient_idx],
				 test_deadline_ts) != 0) {
			SCHED_PRINTF("Failed to run gradient %d%%\n", u_percent);
		} else {
			SCHED_PRINTF("Gradient %d%% complete: MR = %.4f (%llu/%llu)\n",
				     u_percent,
				     g_result.gradients[gradient_idx].miss_rate,
				     (unsigned long long)g_result.gradients[gradient_idx].total_misses,
				     (unsigned long long)g_result.gradients[gradient_idx].total_jobs);
		}

		gradient_idx++;
	}

	g_result.num_gradients = gradient_idx;

	/* ================================================================
	 * Phase 3: Calculate Final Score
	 * ================================================================ */
	SCHED_PRINTF("\n=============================================================\n");
	SCHED_PRINTF("[Phase 3] Final Results\n");
	SCHED_PRINTF("=============================================================\n");

	sum_mr = 0.0;
	for (i = 0; i < g_result.num_gradients; i++) {
		sum_mr += g_result.gradients[i].miss_rate;
		SCHED_PRINTF("U=%3d%%: MR=%.4f (%llu misses / %llu jobs)\n",
			     g_result.gradients[i].utilization_percent,
			     g_result.gradients[i].miss_rate,
			     (unsigned long long)g_result.gradients[i].total_misses,
			     (unsigned long long)g_result.gradients[i].total_jobs);
	}

	if (g_result.num_gradients > 0) {
		g_result.average_miss_rate = sum_mr / (double)g_result.num_gradients;
	} else {
		/* Watchdog skipped every gradient: no data, report worst case. */
		SCHED_PRINTF("[test-schedule] WATCHDOG: no gradient completed; "
			     "reporting miss rate 1.0\n");
		g_result.average_miss_rate = 1.0;
	}
	g_result.final_score = 100.0 * (1.0 - g_result.average_miss_rate);

	SCHED_PRINTF("\n------------------------------------------------------\n");
	SCHED_PRINTF("Average Miss Rate: %.4f\n", g_result.average_miss_rate);
	SCHED_PRINTF("Final Score: %.2f / 100\n", g_result.final_score);
	SCHED_PRINTF("------------------------------------------------------\n");

cleanup:
	if (tasks) free(tasks);
	if (stats) free(stats);
	if (wcets_ns) free(wcets_ns);
	if (generated_u) free(generated_u);
	if (workload_indices) free(workload_indices);

	return 0;
}

int test_schedule_run(void)
{
	return test_schedule_run_custom(TEST_SCHEDULE_CYCLES,
					TEST_SCHEDULE_UTIL_START,
					TEST_SCHEDULE_UTIL_END,
					TEST_SCHEDULE_UTIL_STEP);
}

const struct test_schedule_result *test_schedule_get_result(void)
{
	return &g_result;
}
