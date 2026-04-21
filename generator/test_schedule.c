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
#include <unistd.h>
#define SCHED_PRINTF printf
#endif

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
static uint64_t measure_wcet_ns_budgeted(const struct rtosbench_workload *wl,
					int target_iterations,
					int min_samples,
					int budget_sec,
					int *samples_taken,
					int *budget_limited);
static void task_thread_entry(void *param);
static int compare_double_desc(const void *a, const void *b);
static int compare_wcet_desc(const void *a, const void *b);
static int is_builtin_workload(const struct rtosbench_workload *wl);
static int is_schedule_eligible_workload(const struct rtosbench_workload *wl);
static int validate_schedule_params(int *cycles, int *util_start, int *util_end, int *util_step);
static void request_stop_all_tasks(struct task_thread_ctx *contexts, int num_tasks);
static int wait_for_tasks_completion(struct task_thread_ctx *contexts, int num_tasks, int timeout_ms);
static void fill_failed_gradient_result(struct schedule_gradient_result *result,
					int num_tasks, const struct schedule_task_config *tasks,
					int cycles, int attempts, int failure_reason);
static const char *gradient_failure_reason_to_string(int reason);

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

/**
 * @brief Check whether a workload should participate in test-schedule
 * @details test-schedule focuses on deterministic periodic execution.
 *          Utility workloads are excluded by default.
 */
static int is_schedule_eligible_workload(const struct rtosbench_workload *wl)
{
	if (!wl || !wl->name) {
		return 0;
	}

	if (is_builtin_workload(wl)) {
		return 0;
	}

	return 1;
}

static int validate_schedule_params(int *cycles, int *util_start, int *util_end, int *util_step)
{
	if (!cycles || !util_start || !util_end || !util_step) {
		return -1;
	}

	if (*cycles <= 0) {
		SCHED_PRINTF("[test-schedule] Invalid --cycles=%d (must be > 0)\n", *cycles);
		return -1;
	}

	if (*util_step <= 0) {
		SCHED_PRINTF("[test-schedule] Invalid --util-step=%d (must be > 0)\n", *util_step);
		return -1;
	}

	if (*util_start < 1 || *util_start > 100 ||
	    *util_end < 1 || *util_end > 100 || *util_start > *util_end) {
		SCHED_PRINTF("[test-schedule] Invalid utilization range: start=%d end=%d (valid: 1..100, start<=end)\n",
			     *util_start, *util_end);
		return -1;
	}

	return 0;
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
 * @brief Measure WCET with iteration and wall-clock budgets
 */
static uint64_t measure_wcet_ns_budgeted(const struct rtosbench_workload *wl,
					int target_iterations,
					int min_samples,
					int budget_sec,
					int *samples_taken,
					int *budget_limited)
{
	uint64_t max_duration = 0;
	long double phase_start;
	int prev_suppress;
	int i;

	if (samples_taken) {
		*samples_taken = 0;
	}
	if (budget_limited) {
		*budget_limited = 0;
	}

	if (!wl || !wl->name || !wl->exec || target_iterations <= 0) {
		return 0;
	}
	if (min_samples <= 0) {
		min_samples = 1;
	}
	if (min_samples > target_iterations) {
		min_samples = target_iterations;
	}
	if (budget_sec <= 0) {
		budget_sec = TEST_SCHEDULE_WCET_BUDGET_SEC;
	}

	/* Phase 1 also suppresses workload output to avoid console lock contention. */
	prev_suppress = g_sched_suppress_output;
	g_sched_suppress_output = 1;

	/* Initialize workload */
	if (wl->init) {
		if (wl->init(0, NULL) != 0) {
			SCHED_PRINTF("[test-schedule] WCET init failed for %s\n", wl->name);
			g_sched_suppress_output = prev_suppress;
			return 0;
		}
	}

	phase_start = rtbench_get_timestamp();

	/* Run iterations and record max */
	for (i = 0; i < target_iterations; i++) {
		long double start = rtbench_get_timestamp();
		wl->exec(0, NULL);
		long double end = rtbench_get_timestamp();

		uint64_t duration_ns = (uint64_t)((end - start) * 1000000000.0L);
		if (duration_ns == 0) {
			duration_ns = 1;
		}
		if (duration_ns > max_duration) {
			max_duration = duration_ns;
		}

		if (samples_taken) {
			*samples_taken = i + 1;
		}

		if ((i + 1) >= min_samples) {
			long double elapsed_sec = rtbench_get_timestamp() - phase_start;
			if (elapsed_sec >= (long double)budget_sec) {
				if (budget_limited) {
					*budget_limited = 1;
				}
				break;
			}
		}
	}

	/* Teardown workload */
	if (wl->teardown) {
		wl->teardown(0, NULL);
	}

	g_sched_suppress_output = prev_suppress;
	return max_duration;
}

/**
 * @brief Task thread entry point
 */
static void task_thread_entry(void *param)
{
	struct task_thread_ctx *ctx = (struct task_thread_ctx *)param;
	const struct rtosbench_workload *wl = NULL;

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

	/* Initialize workload */
	if (wl->init) {
		wl->init(0, NULL);
	}

	/* Create period semaphore */
	/* Start with one token so each task executes one immediate first job,
	 * then follows periodic activations from the timer. This avoids long
	 * initial idle waits for tasks with very large periods. */
	ctx->period_sem = rtbench_sem_create(1);
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
			if (!ctx->running) {
				break;
			}
			continue;
		}

		/* Record activation time */
		long double activation = rtbench_get_timestamp();

		/* Execute workload */
		wl->exec(0, NULL);

		/* Record completion time */
		long double completion = rtbench_get_timestamp();

		/* Calculate response time */
		uint64_t response_ns = (uint64_t)((completion - activation) * 1000000000.0L);
		ctx->stats->total_jobs++;
		ctx->stats->total_response_ns += response_ns;

		if (response_ns > ctx->stats->max_response_ns) {
			ctx->stats->max_response_ns = response_ns;
		}

		/* Check deadline (deadline = period for implicit deadline tasks) */
		if (response_ns > ctx->config->deadline_ns) {
			ctx->stats->deadline_misses++;
		}
	}

	/* Cleanup */
	if (ctx->period_timer) {
		rtbench_timer_delete(ctx->period_timer);
		ctx->period_timer = NULL;
	}
	if (ctx->period_sem) {
		rtbench_sem_destroy(ctx->period_sem);
		ctx->period_sem = NULL;
	}

	if (wl->teardown) {
		wl->teardown(0, NULL);
	}

	ctx->completed = 1;
}

static void schedule_sleep_ms(int ms)
{
	if (ms <= 0) {
		return;
	}

#ifdef RT_THREAD_PLATFORM
	rt_thread_mdelay(ms);
#else
	usleep((useconds_t)ms * 1000U);
#endif
}

static void request_stop_all_tasks(struct task_thread_ctx *contexts, int num_tasks)
{
	int i;

	if (!contexts || num_tasks <= 0) {
		return;
	}

	for (i = 0; i < num_tasks; i++) {
		contexts[i].running = 0;
		if (contexts[i].period_sem) {
			rtbench_sem_post(contexts[i].period_sem);
		}
	}
}

static int wait_for_tasks_completion(struct task_thread_ctx *contexts, int num_tasks, int timeout_ms)
{
	const int poll_ms = 50;
	int waited_ms = 0;
	int all_done;

	if (!contexts || num_tasks <= 0) {
		return -1;
	}

	while (1) {
		all_done = 1;
		for (int i = 0; i < num_tasks; i++) {
			if (!contexts[i].completed) {
				all_done = 0;
				break;
			}
		}

		if (all_done) {
			return 0;
		}

		if (timeout_ms >= 0 && waited_ms >= timeout_ms) {
			return -1;
		}

		schedule_sleep_ms(poll_ms);
		waited_ms += poll_ms;
	}
}

static void fill_failed_gradient_result(struct schedule_gradient_result *result,
					int num_tasks, const struct schedule_task_config *tasks,
					int cycles, int attempts, int failure_reason)
{
	int max_tasks;

	if (!result || !tasks || num_tasks <= 0 || cycles <= 0) {
		return;
	}

	memset(result->task_stats, 0, sizeof(result->task_stats));
	result->num_tasks = num_tasks;
	result->attempts = attempts;
	result->passed = 0;
	result->degraded = 1;
	result->failure_reason = failure_reason;
	result->total_jobs = 0;
	result->total_misses = 0;

	max_tasks = num_tasks;
	if (max_tasks > TEST_SCHEDULE_MAX_TASKS) {
		max_tasks = TEST_SCHEDULE_MAX_TASKS;
	}

	for (int i = 0; i < max_tasks; i++) {
		result->task_stats[i].name = tasks[i].name;
		result->task_stats[i].total_jobs = (uint64_t)cycles;
		result->task_stats[i].deadline_misses = (uint64_t)cycles;
		result->total_jobs += (uint64_t)cycles;
		result->total_misses += (uint64_t)cycles;
	}

	result->miss_rate = (result->total_jobs > 0) ? 1.0 : 0.0;
}

static const char *gradient_failure_reason_to_string(int reason)
{
	switch (reason) {
	case SCHEDULE_GRADIENT_REASON_NONE:
		return "none";
	case SCHEDULE_GRADIENT_REASON_TIMEOUT:
		return "timeout";
	case SCHEDULE_GRADIENT_REASON_SETUP:
		return "setup_failure";
	case SCHEDULE_GRADIENT_REASON_UNKNOWN:
	default:
		return "unknown";
	}
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
	(void)name;
	return pthread_create(&ctx->thread, NULL, pthread_entry_wrapper, ctx);
}

#endif

/**
 * @brief Run one gradient of the schedulability test
 */
static int run_gradient(int num_tasks, struct schedule_task_config *tasks,
			struct schedule_task_stats *stats, int cycles, int timeout_sec,
			struct schedule_gradient_result *result, int *failure_reason)
{
	struct task_thread_ctx *contexts;
	int i;
	int started_count = 0;
	int ret = -1;
	int timed_out = 0;
	int timeout_ms;
	int stop_grace_ms;

	if (!tasks || !stats || !result || num_tasks <= 0 || cycles <= 0) {
		return -1;
	}

	if (failure_reason) {
		*failure_reason = SCHEDULE_GRADIENT_REASON_UNKNOWN;
	}

	if (timeout_sec <= 0) {
		timeout_sec = TEST_SCHEDULE_GRADIENT_TIMEOUT_SEC;
	}
	timeout_ms = timeout_sec * 1000;
	stop_grace_ms = TEST_SCHEDULE_GRADIENT_STOP_GRACE_SEC * 1000;

	contexts = (struct task_thread_ctx *)calloc(num_tasks,
						    sizeof(struct task_thread_ctx));
	if (!contexts) {
		SCHED_PRINTF("[test-schedule] Failed to allocate thread contexts\n");
		if (failure_reason) {
			*failure_reason = SCHEDULE_GRADIENT_REASON_SETUP;
		}
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
			if (failure_reason) {
				*failure_reason = SCHEDULE_GRADIENT_REASON_SETUP;
			}
			started_count = i;
			goto cleanup_threads;
		}
		started_count++;
	}

	if (wait_for_tasks_completion(contexts, num_tasks, timeout_ms) != 0) {
		SCHED_PRINTF("[test-schedule] Gradient runtime window reached (%d sec), collecting partial samples\n",
			     timeout_sec);
		timed_out = 1;
		ret = 0;
		if (failure_reason) {
			*failure_reason = SCHEDULE_GRADIENT_REASON_TIMEOUT;
		}
		goto cleanup_threads;
	}

	ret = 0;
	if (failure_reason) {
		*failure_reason = SCHEDULE_GRADIENT_REASON_NONE;
	}

cleanup_threads:
	/* Always request stop to unblock semaphore waits in both success/failure paths. */
	request_stop_all_tasks(contexts, started_count);
	if (started_count > 0) {
		if (wait_for_tasks_completion(contexts, started_count, stop_grace_ms) != 0) {
			SCHED_PRINTF("[test-schedule] Warning: tasks did not stop within grace period\n");
			if (failure_reason && *failure_reason == SCHEDULE_GRADIENT_REASON_NONE) {
				*failure_reason = SCHEDULE_GRADIENT_REASON_UNKNOWN;
			}
		}
	}
#ifndef RT_THREAD_PLATFORM
	for (i = 0; i < started_count; i++) {
		pthread_join(contexts[i].thread, NULL);
	}
#endif
	g_sched_suppress_output = 0;

	if (ret == 0) {
		/* Aggregate results */
		result->total_jobs = 0;
		result->total_misses = 0;
		result->num_tasks = num_tasks;
		result->passed = 1;
		result->degraded = timed_out ? 1 : 0;
		result->failure_reason = timed_out
			? SCHEDULE_GRADIENT_REASON_TIMEOUT
			: SCHEDULE_GRADIENT_REASON_NONE;

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
	}

	free(contexts);
	return ret;
}

int test_schedule_run_custom(int cycles, int util_start, int util_end, int util_step)
{
	int num_workloads;
	int num_gradients;
	int gradient_idx;
	int i, u_percent;
	struct schedule_task_config *tasks;
	struct schedule_task_stats *stats;
	double *generated_u;
	double sum_mr;
	int total_workloads;
	int valid_idx;
	int excluded_count;
	int is_quick;
	int gradient_timeout_sec;
	int wcet_iters;
	int wcet_min_samples;
	int wcet_budget_sec;
	int wcet_budget_limited_count;
	uint64_t max_observed_wcet;
	int ret;

	/* Clear previous results */
	memset(&g_result, 0, sizeof(g_result));
	ret = -1;
	tasks = NULL;
	stats = NULL;
	generated_u = NULL;

	if (validate_schedule_params(&cycles, &util_start, &util_end, &util_step) != 0) {
		return -1;
	}

	g_result.configured_cycles = cycles;
	g_result.configured_util_start = util_start;
	g_result.configured_util_end = util_end;
	g_result.configured_util_step = util_step;

	total_workloads = rtosbench_workload_count();
	if (total_workloads == 0) {
		SCHED_PRINTF("[test-schedule] No workloads registered\n");
		return -1;
	}

	/* Count workloads eligible for schedulability testing */
	num_workloads = 0;
	excluded_count = 0;
	for (i = 0; i < total_workloads; i++) {
		const struct rtosbench_workload *wl = rtosbench_get_workload(i);
		if (is_schedule_eligible_workload(wl)) {
			num_workloads++;
		} else if (wl && wl->name) {
			excluded_count++;
		}
	}

	if (num_workloads == 0) {
		SCHED_PRINTF("[test-schedule] No industrial workloads registered (only utility workloads found)\n");
		return -1;
	}

	SCHED_PRINTF("[test-schedule] Found %d industrial workloads (excluded %d utility workloads)\n",
		     num_workloads, excluded_count);

	if (num_workloads > TEST_SCHEDULE_MAX_TASKS) {
		SCHED_PRINTF("[test-schedule] Truncating workload set to %d (from %d)\n",
			     TEST_SCHEDULE_MAX_TASKS, num_workloads);
		num_workloads = TEST_SCHEDULE_MAX_TASKS;
	}

	/* Allocate arrays */
	tasks = (struct schedule_task_config *)calloc(num_workloads,
						      sizeof(struct schedule_task_config));
	stats = (struct schedule_task_stats *)calloc(num_workloads,
						     sizeof(struct schedule_task_stats));
	generated_u = (double *)calloc(num_workloads, sizeof(double));

	if (!tasks || !stats || !generated_u) {
		SCHED_PRINTF("[test-schedule] Memory allocation failed\n");
		goto cleanup;
	}

	/* ================================================================
	 * Phase 1: WCET Measurement (skip builtin utility workloads)
	 * ================================================================ */
	SCHED_PRINTF("=============================================================\n");
	SCHED_PRINTF("[Phase 1] Measuring WCET for %d workloads...\n", num_workloads);
	SCHED_PRINTF("=============================================================\n");

	is_quick = (cycles <= TEST_SCHEDULE_QUICK_CYCLES);
	gradient_timeout_sec = TEST_SCHEDULE_GRADIENT_TIMEOUT_SEC;
	if (is_quick) {
		gradient_timeout_sec = TEST_SCHEDULE_QUICK_GRADIENT_TIMEOUT_SEC;
	}
	wcet_iters = is_quick
		? TEST_SCHEDULE_QUICK_WCET_ITERATIONS
		: TEST_SCHEDULE_WCET_ITERATIONS;
	wcet_min_samples = is_quick
		? TEST_SCHEDULE_QUICK_WCET_MIN_SAMPLES
		: TEST_SCHEDULE_WCET_MIN_SAMPLES;
	wcet_budget_sec = is_quick
		? TEST_SCHEDULE_QUICK_WCET_BUDGET_SEC
		: TEST_SCHEDULE_WCET_BUDGET_SEC;
	wcet_budget_limited_count = 0;
	max_observed_wcet = 0;

	valid_idx = 0;
	for (i = 0; i < total_workloads && valid_idx < num_workloads; i++) {
		const struct rtosbench_workload *wl = rtosbench_get_workload(i);
		uint64_t wcet;
		int samples_taken = 0;
		int budget_limited = 0;

		if (!is_schedule_eligible_workload(wl)) {
			continue;
		}

		wcet = measure_wcet_ns_budgeted(wl, wcet_iters, wcet_min_samples,
						wcet_budget_sec, &samples_taken,
						&budget_limited);
		if (budget_limited) {
			wcet_budget_limited_count++;
		}

		if (wcet > max_observed_wcet) {
			max_observed_wcet = wcet;
		}

		if (wcet == 0) {
			uint64_t fallback_wcet = max_observed_wcet;
			if (fallback_wcet == 0) {
				fallback_wcet = TEST_SCHEDULE_WCET_FALLBACK_NS;
			}
			wcet = fallback_wcet;
			SCHED_PRINTF("  [%s]: WCET unavailable, fallback = %.3f ms\n",
				     wl->name, (double)wcet / 1000000.0);
		} else {
			SCHED_PRINTF("  [%s]: WCET = %.3f ms (%d/%d iters%s)\n",
				     wl->name, (double)wcet / 1000000.0,
				     samples_taken, wcet_iters,
				     budget_limited ? ", budget-limited" : "");
		}
		if (wcet > max_observed_wcet) {
			max_observed_wcet = wcet;
		}

		tasks[valid_idx].name = wl->name;
		tasks[valid_idx].workload_idx = i;
		tasks[valid_idx].wcet_ns = wcet;
		valid_idx++;
	}

	num_workloads = valid_idx;
	SCHED_PRINTF("[Phase 1] %d workloads measured (%d budget-limited)\n",
		     num_workloads, wcet_budget_limited_count);
	if (num_workloads <= 0) {
		SCHED_PRINTF("[test-schedule] No workloads left after WCET filtering\n");
		goto cleanup;
	}

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
	if (num_gradients <= 0) {
		SCHED_PRINTF("[test-schedule] No utilization gradients generated\n");
		goto cleanup;
	}

	gradient_idx = 0;
	for (u_percent = util_start; u_percent <= util_end && gradient_idx < num_gradients;
	     u_percent += util_step) {
		double target_u = (double)u_percent / 100.0;

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

			/* Implicit deadline: D = T */
			tasks[i].deadline_ns = tasks[i].period_ns;

			SCHED_PRINTF("%-12s | %-10.3f | %-8.2f | %-12.3f\n",
				     tasks[i].name,
				     (double)tasks[i].wcet_ns / 1000000.0,
				     tasks[i].utilization * 100.0,
				     (double)tasks[i].period_ns / 1000000.0);
		}

		/* Run this task set */
		SCHED_PRINTF("Running task set for %d cycles (timeout: %ds, max attempts: %d)...\n",
			     cycles, gradient_timeout_sec, TEST_SCHEDULE_GRADIENT_MAX_ATTEMPTS);

		g_result.gradients[gradient_idx].utilization_percent = u_percent;
		g_result.gradients[gradient_idx].actual_utilization = target_u;
		g_result.gradients[gradient_idx].failure_reason = SCHEDULE_GRADIENT_REASON_NONE;

		int attempts_used = 0;
		int gradient_success = 0;
		int failure_reason = SCHEDULE_GRADIENT_REASON_UNKNOWN;

		for (int attempt = 1; attempt <= TEST_SCHEDULE_GRADIENT_MAX_ATTEMPTS; attempt++) {
			attempts_used = attempt;
			if (run_gradient(num_workloads, tasks, stats, cycles, gradient_timeout_sec,
					 &g_result.gradients[gradient_idx], &failure_reason) == 0) {
				gradient_success = 1;
				break;
			}

			if (attempt < TEST_SCHEDULE_GRADIENT_MAX_ATTEMPTS) {
				SCHED_PRINTF("[test-schedule] Gradient %d%% attempt %d failed (%s), retrying...\n",
					     u_percent, attempt,
					     gradient_failure_reason_to_string(failure_reason));
				schedule_sleep_ms(200);
			}
		}

		g_result.gradients[gradient_idx].attempts = attempts_used;
		if (attempts_used > 1) {
			g_result.total_retry_count += (attempts_used - 1);
		}

		if (!gradient_success) {
			fill_failed_gradient_result(&g_result.gradients[gradient_idx],
						   num_workloads, tasks, cycles,
						   attempts_used, failure_reason);
			g_result.failed_gradients++;
			g_result.completed_with_degradation = 1;
			SCHED_PRINTF("Gradient %d%% failed after %d attempts (%s), fallback MR=1.0000\n",
				     u_percent, attempts_used,
				     gradient_failure_reason_to_string(failure_reason));
		} else {
			if (g_result.gradients[gradient_idx].degraded) {
				g_result.failed_gradients++;
				g_result.completed_with_degradation = 1;
				SCHED_PRINTF("Gradient %d%% complete%s (time-window): MR = %.4f (%llu/%llu)\n",
					     u_percent,
					     (attempts_used > 1) ? " (after retry)" : "",
					     g_result.gradients[gradient_idx].miss_rate,
					     (unsigned long long)g_result.gradients[gradient_idx].total_misses,
					     (unsigned long long)g_result.gradients[gradient_idx].total_jobs);
			} else {
				SCHED_PRINTF("Gradient %d%% complete%s: MR = %.4f (%llu/%llu)\n",
					     u_percent,
					     (attempts_used > 1) ? " (after retry)" : "",
					     g_result.gradients[gradient_idx].miss_rate,
					     (unsigned long long)g_result.gradients[gradient_idx].total_misses,
					     (unsigned long long)g_result.gradients[gradient_idx].total_jobs);
			}
		}

		gradient_idx++;
	}

	g_result.num_gradients = gradient_idx;
	if (g_result.num_gradients <= 0) {
		SCHED_PRINTF("[test-schedule] No gradients completed\n");
		goto cleanup;
	}

	/* ================================================================
	 * Phase 3: Calculate Final Score
	 * ================================================================ */
	SCHED_PRINTF("\n=============================================================\n");
	SCHED_PRINTF("[Phase 3] Final Results\n");
	SCHED_PRINTF("=============================================================\n");

	sum_mr = 0.0;
	for (i = 0; i < g_result.num_gradients; i++) {
		const struct schedule_gradient_result *gr = &g_result.gradients[i];
		sum_mr += g_result.gradients[i].miss_rate;
		SCHED_PRINTF("U=%3d%%: MR=%.4f (%llu misses / %llu jobs)%s\n",
			     gr->utilization_percent,
			     gr->miss_rate,
			     (unsigned long long)gr->total_misses,
			     (unsigned long long)gr->total_jobs,
			     gr->degraded
				? (gr->failure_reason == SCHEDULE_GRADIENT_REASON_TIMEOUT
					? " [time-window]"
					: " [fallback]")
				: "");
	}

	g_result.average_miss_rate = sum_mr / (double)g_result.num_gradients;
	g_result.final_score = 100.0 * (1.0 - g_result.average_miss_rate);
	if (g_result.final_score < 0.0) {
		g_result.final_score = 0.0;
	}
	if (g_result.final_score > 100.0) {
		g_result.final_score = 100.0;
	}

	SCHED_PRINTF("\n------------------------------------------------------\n");
	SCHED_PRINTF("Average Miss Rate: %.4f\n", g_result.average_miss_rate);
	SCHED_PRINTF("Final Score: %.2f / 100\n", g_result.final_score);
	SCHED_PRINTF("Failed Gradients: %d / %d\n", g_result.failed_gradients,
		     g_result.num_gradients);
	SCHED_PRINTF("Retries Used: %d\n", g_result.total_retry_count);
	if (g_result.completed_with_degradation) {
		SCHED_PRINTF("Status: PASSED_WITH_DEGRADATION\n");
	} else {
		SCHED_PRINTF("Status: PASSED\n");
	}
	SCHED_PRINTF("------------------------------------------------------\n");
	ret = 0;

cleanup:
	if (tasks) free(tasks);
	if (stats) free(stats);
	if (generated_u) free(generated_u);

	return ret;
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
