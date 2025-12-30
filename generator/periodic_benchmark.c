/** @file periodic_benchmark.c
 * @ingroup generator
 * @brief Implementation of a general periodic benchmark using real time timers.
 * @details Timer expiration triggers a real time POSIX signal and `SIGINT` is
 * used to stop the benchmark and terminate the program.
 * @author Mattia Nicolella
 *
 * **Dependencies**:
 * - POSIX.4 real-time signals.
 *
 * @copyright (C) 2021 - 2022, Mattia Nicolella <mnico@bu.edu> and the rt-bench contributors.
 * SPDX-License-Identifier: MIT
 */

#include "periodic_benchmark.h"
#include "performance_counters.h"
#include "performance_sampler.h"
#include "get_cpu_timestamp.h"
#include "logging.h"
#include "memory_watcher.h"
#include <bits/types/siginfo_t.h>
#include <bits/types/struct_itimerspec.h>
#include <errno.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/// This value in `::deadline_timer_status` determines that the deadline timer
/// must be used
#define DEADLINE_TIMER_IN_USE 1

/// The real-time signal that identifies the deadline occurrence.
#define SIGNAL_DEADLINE SIGRTMIN

/// The real-time signal that identifies the end of the period.
#define SIGNAL_END_PERIOD SIGRTMIN + 1

/// Default output path and filename for timing information.
#define DEFAULT_OUTPUT_PATH "./timing.csv"

/// Default output path and filename for performance counter runtime monitoring.
#define DEFAULT_PERFORMANCE_COUNTER_SAMPLING_OUTPUT_PATH "./perf.csv"

/// Indicates whether to print skipped deadelines with 0s
#define PRINT_SKIPPED_DEADLINE 0

/// Number of parameters passed to the benchmark.
static int benchmark_param_num = 0;

/// Benchmark parameters array.
static void **benchmark_params = NULL;

/// Real time timer, used to notify when the deadline is reached.
static timer_t deadline_timer = NULL;

/** @brief If the deadline timer must be used.
 * @details Possible values should be only `::DEADLINE_TIMER_IN_USE` when a
 * separate deadline timer is used or `!::DEADLINE_TIMER_IN_USE` when only the
 * period timer is used.
 */
static int deadline_timer_status;

/// Real time timer, used to notify when the period end is reached.
static timer_t period_timer;

/// The timing interval of a deadline, used to rearm the deadline timer.
static struct itimerspec deadline_timing;

/// The file pointer to the timing output file.
static FILE *filep = NULL;

#if defined(CORTEX_A53) || defined(CORE_I7)
/// The file pointer to the pruntime performance counter monitoring file.
static FILE *filep_sampler = NULL;
#endif

/// Semaphore used to determine if a new job can be started.
static sem_t period_sem;

/// Timestamp in clock cycles of when the last job ended, it can be 0 if the job has not
/// finished yet.
static unsigned long long job_end_timestamp_clocks = 0;

/// Timestamp in clock cycles of when the last deadline since the job start has occurred.
static unsigned long long last_deadline_timestamp_clocks = 0;

/// Timestamp in clock cycles of when the first deadline since the job start has occurred.
static unsigned long long job_deadline_timestamp_clocks = 0;

/// Timestamp in clock cycles of the period end.
static unsigned long long job_period_end_timestamp_clocks = 0;

/// Timestamp in clock cycles  of the period start.
static unsigned long long job_period_start_timestamp_clocks = 0;

/// Timestamp in seconds of when the last job ended, it can be 0 if the job has not
/// finished yet.
static long double job_end_timestamp = 0;

/// Timestamp in seconds of when the last deadline since the job start has occurred.
static long double last_deadline_timestamp = 0;

/// Timestamp in seconds clock cycles of when the first deadline since the job start has occurred.
static long double job_deadline_timestamp = 0;

/// Timestamp in seconds of the period end.
static long double job_period_end_timestamp = 0;

/// Timestamp in seconds of the period start.
static long double job_period_start_timestamp = 0;

/// Number of tasks launched.
static unsigned long long tasks_launched = 0;

/// Performance counters at the start of the period.
static struct perf_counters job_perf_counters_start;

/// Performance counters at the end of the period.
static struct perf_counters job_perf_counters_end;

/// Extra benchmark dependant measured data.
static float extra_measurement = 0.0f;

/**
 * @brief Teardown function registered to be called when exit is called.
 * @param[in] status The exit status.
 * @param[in] arg Ignored.
 * @details Will ensure that all the requested resourced are freed, the memory
 * watcher is stopped, and the output file is flushed and closed.
 */
static void stop_benchmark(int status, void *arg)
{
	int res;
	// we stop the memory watcher
	stop_memory_watcher();
	if (filep != NULL) {
		elogf(LOG_LEVEL_TRACE, "Closing output file\n");
		res = fclose(filep);
		if (res == EOF) {
			perror("Error during output file close");
		}
	}
#if (defined(AARCH64) && defined(CORTEX_A53)) ||                               \
	(defined(X86_64) && defined(CORE_I7))
	if (arg != NULL) {
		unsigned *memory_profiling_enable = (unsigned *)arg;
		if (*memory_profiling_enable) {
			log_samples(filep_sampler);
			if (filep_sampler != NULL) {
				elogf(LOG_LEVEL_TRACE,
				      "Closing performance counter monitoring file\n");
				res = teardown_perf_sampler();
				if (res != 0) {
					perror("Error during the closing of the performance sampler thread\n");
				}
				res = fclose(filep_sampler);
				if (res == EOF) {
					perror("Error during the closing of the performance counter monitoring output file\n");
				}
			}
		}
	}
#endif
	if (deadline_timer != NULL) {
		elogf(LOG_LEVEL_TRACE, "Deleting deadline timer\n");
		res = timer_delete(deadline_timer);
		if (res < 0) {
			perror("Error during deadline timer deletion");
		}
	}
	if (period_timer != NULL) {
		elogf(LOG_LEVEL_TRACE, "Deleting period timer\n");
		res = timer_delete(period_timer);
		if (res < 0) {
			perror("Error during period timer deletion");
		}
	}
	res = sem_destroy(&period_sem);
	if (res < 0) {
		perror("Error during period semaphore destruction");
	}
	elogf(LOG_LEVEL_TRACE, "Cleaning up job environment\n");
	benchmark_teardown(benchmark_param_num, benchmark_params);
#if (defined(AARCH64) && defined(CORTEX_A53)) ||                               \
	(defined(X86_64) && defined(CORE_I7))
	res = teardown_pmcs();
	if (res < 0) {
		perror("Error: performance counters file descriptors could not be closed\n");
	}
#endif
}

/**
 * @brief SIGINT handler, causes the program to terminate in a clean way.
 * @param signo Ignored.
 * @param info Ignored.
 * @param context Ignored.
 * @details Will call the exit function, it's invoked when a `SIGINT` is
 * received.
 */
static void quit_handler(int signo, siginfo_t *info, void *context)
{
	exit(EXIT_SUCCESS);
}

/**
 * @brief The signal handler that is executed when the deadline timer expires.
 * @param signo Ignored.
 * @param info Ignored.
 * @param context Ignored.
 * @details
 * When the deadline timer expires, the current timestamp is saved in
 * `::last_deadline_timestamp`, If this if the first deadline expiration since
 * the period start, then the timestamp values is also copied in
 * `::job_deadline_timestamp`.
 * Both `get_rdtsc()` and `get_timestamp()` are used, to be safe in case only one of these methods is working.
 */
static void deadline_handler(int signo, siginfo_t *info, void *context)
{
	// we save the current deadline
	last_deadline_timestamp_clocks = get_rdtsc();
	last_deadline_timestamp = get_timestamp();
	// the current deadline could be the job deadline
	if (job_deadline_timestamp_clocks == 0) {
		job_deadline_timestamp_clocks = last_deadline_timestamp_clocks;
	}
	if (job_deadline_timestamp == 0) {
		job_deadline_timestamp = last_deadline_timestamp;
	}
}

/**
 * @brief The signal handler that is executed when the period timer expires.
 * @param signo Ignored.
 * @param info Ignored.
 * @param context Ignored.
 * @details
 * When the period expires and the job has terminated its execution, the
 * deadline timer is rearmed, the job's stats are reported, the semaphore is
 * unlocked and the reporting variables are reset. The start of the next period
 * matches with the end of the previous period. The next job starts as soon as
 * the semaphore is unlocked, and this creates a slight overhead, since before
 * unlocking the semaphore the previous job stats must be reported. When the
 * period ends but no start timestamp was recorded
 * (`::job_period_start_timestamp` is `0`), no reporting will be done.
 *
 * If the job has not terminated when the period ends, a deadline skip is
 * reported if the deadline that has been missed is not the first since the job
 * start.
 *
 * Finally, if the deadline matches the period, the period handler will also
 * perform the same operations as `deadline_handler()`, but will use the
 * timestamp of the period end.
 *
 * Both `get_rdtsc()` and `get_timestamp()` are used, to be safe in case only one of these methods is working.
 *
 * Reporting is done using `print_statistics()`.
 */
static void period_handler(int signo, siginfo_t *info, void *context)
{
	int res;
	unsigned long long period_end_timestamp_clocks = get_rdtsc();
	long double period_end_timestamp = get_timestamp();
	//
	if (job_period_end_timestamp_clocks == 0) {
		job_period_end_timestamp_clocks = period_end_timestamp_clocks;
	}
	if (job_period_end_timestamp == 0) {
		job_period_end_timestamp = period_end_timestamp;
	}
	// if the deadline occurs before the end of the period it has a dedicated
	// timer and handler
	if (deadline_timer_status == DEADLINE_TIMER_IN_USE) {
		// we rearm the deadline timer
		res = timer_settime(deadline_timer, 0, &deadline_timing, NULL);
		if (res < 0) {
			perror("Cannot rearm the deadline timer");
			exit(EXIT_FAILURE);
		}
	} // otherwise the period handler covers also the deadline occurrence
	// management.
	else {
		if (job_deadline_timestamp_clocks == 0) {
			job_deadline_timestamp_clocks =
				period_end_timestamp_clocks;
		}
		last_deadline_timestamp_clocks = period_end_timestamp_clocks;
		if (job_deadline_timestamp == 0) {
			job_deadline_timestamp = period_end_timestamp;
		}
		last_deadline_timestamp = period_end_timestamp;
	}
	// we report only at the beginning of a legitimate period.
	if (job_period_start_timestamp_clocks > 0 ||
	    job_period_start_timestamp > 0) {
		// we report a job completion
		if (job_end_timestamp_clocks > 0 || job_end_timestamp > 0) {
			print_statistics(filep,
					 job_period_start_timestamp_clocks,
					 job_period_end_timestamp_clocks,
					 job_end_timestamp_clocks,
					 job_deadline_timestamp_clocks,
					 job_period_start_timestamp,
					 job_period_end_timestamp,
					 job_end_timestamp,
					 job_deadline_timestamp,
					 job_perf_counters_start.l1_references,
					 job_perf_counters_start.l1_refills,
					 job_perf_counters_start.l2_references,
					 job_perf_counters_start.l2_refills,
					 job_perf_counters_start.inst_retired,
					 job_perf_counters_start.clock_count,
					 job_perf_counters_end.l1_references,
					 job_perf_counters_end.l1_refills,
					 job_perf_counters_end.l2_references,
					 job_perf_counters_end.l2_refills,
					 job_perf_counters_end.inst_retired,
					 job_perf_counters_end.clock_count,
					 extra_measurement);

		}
#ifdef PRINT_SKIPPED_DEADLINE
		// or the skipped deadline
		else {
			if (last_deadline_timestamp_clocks !=
				    job_deadline_timestamp_clocks ||
			    last_deadline_timestamp != job_deadline_timestamp) {
				print_statistics(filep, 0, 0, 0,
						 last_deadline_timestamp_clocks,
						 0.0, 0.0, 0.0,
						 last_deadline_timestamp,
						 job_end_timestamp,
						 job_deadline_timestamp, 0, 0,
						 0, 0, 0, 0, 0, 0, 0, 0, 0.0);
			}
		}
#endif /* PRINT_SKIPPED_DEADLINE */
	}
	// If a job has ended or we are starting for the first time we need to reset
	// the reporting variables and unlock the semaphore.
	if (job_end_timestamp_clocks > 0 ||
	    job_period_start_timestamp_clocks == 0 || job_end_timestamp > 0 ||
	    job_period_start_timestamp == 0) {
		// we reset the reporting variables
		// the start of the new period is the end of the previous period
		job_period_start_timestamp_clocks = period_end_timestamp_clocks;
		job_period_end_timestamp_clocks = 0;
		job_end_timestamp_clocks = 0;
		job_deadline_timestamp_clocks = 0;
		job_period_start_timestamp = period_end_timestamp;
		job_period_end_timestamp = 0;
		job_end_timestamp = 0;
		job_deadline_timestamp = 0;
#ifdef EXTENDED_REPORT
		extra_measurement = 0.0f;
#endif
		// we unlock the semaphore to allow the next job to start
		res = sem_post(&period_sem);
		if (res < 0) {
			perror("Cannot post on period semaphore");
			exit(EXIT_FAILURE);
		}
	}
}

/**
 * @brief A simple function that is used to install a signal handler.
 * @param[in] handled_signal The signal that is to be associated to the handler.
 * @param[in] handler The handler that is to be associated to the signal.
 * @param[in] masked_signals An array, containing the signals that must be
 * masked during the handler execution.
 * @param[in] masked_signals_num The number of element of `masked_signals`.
 */
static int setup_signal(int handled_signal,
			void (*handler)(int, siginfo_t *, void *),
			int *masked_signals, int masked_signals_num)
{
	struct sigaction sa;
	int res = 0, i;
	// we set the signals to ignore while handling the specified signal
	res = sigemptyset(&sa.sa_mask);
	if (res == -1) {
		perror("Error during sigemptyset for signal handler");
		return res;
	}
	// we mask the requested signals
	for (i = 0; i < masked_signals_num; i++) {
		res = sigaddset(&sa.sa_mask, masked_signals[i]);
		if (res == -1) {
			perror("Error during sigaddset for signal handler");
			return res;
		}
	}
	// install the signal handler.
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = handler;
	res = sigaction(handled_signal, &sa, NULL);
	if (res == -1) {
		perror("Error during signal handler installation");
		return res;
	}
	return 0;
}

/** @brief A function that creates and arms a real-time timer.
 * @param[out] timer The pointer that will contain the created timer.
 * @param[in] signal_generated The signal that the timer must generated when it
 * expires.
 * @param[in] interval_sec The seconds after which the timer will expire.
 * @param[in] interval_nsec The nanoseconds after which the timer will expire.
 * @details
 * The function will create and arm the timer for a periodic execution, with the
 * specified timing. The timer will be armed immediately after creation.\n
 * `interval_sec` and `interval_nsec` can be used in conjunction to specify when
 * the timer must expire and if they are both set to `0` the timer will not be
 * armed.
 */
static int setup_timer(timer_t *timer, int signal_generated, long interval_sec,
		       long interval_nsec)
{
	struct sigevent event;
	struct itimerspec timer_spec;
	int res = 0;
	memset(&event, 0, sizeof(event));
	// the timer will generate a signal
	event.sigev_notify = SIGEV_SIGNAL;
	// the signal generated by the timer
	event.sigev_signo = signal_generated;
	// creation of the timer
	res = timer_create(CLOCK_REALTIME, &event, timer);
	if (res != 0) {
		perror("Error during HR timer creation");
		return res;
	}
	// setting when the timer must be fired, using the provided deadline
	// parameters
	timer_spec.it_interval.tv_sec = interval_sec;
	timer_spec.it_interval.tv_nsec = interval_nsec;
	// the timer will start according to the setup deadline
	timer_spec.it_value.tv_sec = interval_sec;
	timer_spec.it_value.tv_nsec = interval_nsec;
	res = timer_settime(*timer, 0, &timer_spec, NULL);
	if (res < 0) {
		perror("Error during timer setup");
		return res;
	}
	return 0;
}

/** @details
 * This function will prepare the environment for executing the job, initialize
 * the timers and signal handlers. If `bytes_to_preallocate` in
 * `::execution_options` is not `0` (which is set in the command line via the
 * `-m` option) the memory watcher is also initialized. When the environment for
 * the periodic benchmark is initialized, the benchmark will be periodically
 * executed.
 *
 * To execute the benchmark periodically we use two timers that fire different
 * real time signals:
 * - `::SIGNAL_DEADLINE` which will be fired when the deadline occurs, if the
 * deadline is less than the period.
 * - `::SIGNAL_END_PERIOD` which will be fired when the period ends.
 *
 * After the setup, the periodic benchmark will start after a
 * `::SIGNAL_END_PERIOD` is received, to allow a start with reduced delay.
 *
 * When a `SIGINT` is received, the timer will be destroyed and the environment
 * for the job execution will be cleaned.
 *
 * The environment for the job execution is handled by calling the
 * `benchmark_init()` and `benchmark_teardown()` functions.
 *
 * Both `get_rdtsc()` and `get_timestamp()` are used, to be safe in case only one of these methods is working.
 */
int periodic_benchmark(struct execution_options *exec_opts)
{
	// variables used to handle signals
	int job_masked_signals_num = 2;
	int job_masked_signals[] = { SIGNAL_DEADLINE, SIGNAL_END_PERIOD };
	int quit_masked_signals_num = 3;
	int quit_masked_signals[] = { SIGNAL_DEADLINE, SIGNAL_END_PERIOD,
				      SIGINT };
	// variables to handle timers
	// variables used to handle the output file
	char *fname;
	// status variables
	int res;

#if (defined(AARCH64) && defined(CORTEX_A53)) ||                               \
	(defined(X86_64) && defined(CORE_I7))
	// Initialize the performance sampler thread
	if (exec_opts->memory_profiling_enable) {
		elogf(LOG_LEVEL_TRACE,
		      "Initializing runtime performance sampling\n");
		char *perf_fname = NULL;
		const char *perf_fname_postfix = "_perf_profile";
		int perf_fname_postfix_len, perf_fname_len;
		char *fname_no_ext;
		if (exec_opts->output_path != NULL) {
			// write "_perf" before the .csv extension
			perf_fname_postfix_len = strlen(perf_fname_postfix);
			perf_fname_len = strlen(exec_opts->output_path) +
					 perf_fname_postfix_len + 1;
			perf_fname = malloc(sizeof(char) * perf_fname_len);
			memset(perf_fname, 0, sizeof(char) * perf_fname_len);
			fname_no_ext =
				malloc(sizeof(char) *
				       (strlen(exec_opts->output_path) - 3));
			snprintf(fname_no_ext,
				 sizeof(char) *
					 (strlen(exec_opts->output_path) - 3),
				 "%s", exec_opts->output_path);
			snprintf(perf_fname, perf_fname_len, "%s%s%s",
				 fname_no_ext, perf_fname_postfix, ".csv");
			free(fname_no_ext);
		} else {
			perf_fname_len =
				(1 +
				 strlen(DEFAULT_PERFORMANCE_COUNTER_SAMPLING_OUTPUT_PATH));
			perf_fname = malloc(sizeof(char) * perf_fname_len);
			strcpy(perf_fname,
			       DEFAULT_PERFORMANCE_COUNTER_SAMPLING_OUTPUT_PATH);
		}
		filep_sampler = fopen(perf_fname, "w");
		free(perf_fname);
		res = setup_perf_sampler(
			exec_opts->tasks_to_launch,
			exec_opts->memory_profiling_core_affinity,
			exec_opts->memory_profiling_time_bucket);
		if (res != 0) {
			perror("Error during the creation of the performance sampler thread\n");
			return res;
		}
	}
#endif
	elogf(LOG_LEVEL_TRACE, "Starting setup of execution environment\n");
	// we initialize the period semaphore to 0, to wait for the period end.
	res = sem_init(&period_sem, 1, 0);
	if (res < 0) {
		perror("Error during deadline semaphore initialization");
		return res;
	}
#if (defined(AARCH64) && defined(CORTEX_A53)) ||                               \
	(defined(X86_64) && defined(CORE_I7))
	res = on_exit(stop_benchmark,
		      (void *)&(exec_opts->memory_profiling_enable));
#else
	res = on_exit(stop_benchmark, NULL);
#endif
	if (res != 0) {
		elogf(LOG_LEVEL_ERR,
		      "Error during on_exit function registration");
		return -1;
	}
	benchmark_param_num = exec_opts->args_num;
	benchmark_params = (void **)exec_opts->args;
	// we set the core affinity if we were not provided an empty mask
	if (CPU_COUNT(&exec_opts->core_affinity)) {
		res = sched_setaffinity(0, sizeof(cpu_set_t),
					&exec_opts->core_affinity);
		if (res < 0) {
			elogf(LOG_LEVEL_ERR,
			      "Error during core affinity setup.\n");
			return -1;
		}
	}
	elogf(LOG_LEVEL_TRACE, "Execution environment setup complete\n");
	if (benchmark_verbosity == LOG_LEVEL_FILE) {
		elogf(LOG_LEVEL_TRACE, "Starting output file setup\n");
		if (exec_opts->output_path == NULL) {
			fname = DEFAULT_OUTPUT_PATH;
		} else {
			fname = exec_opts->output_path;
		}
		filep = fopen(fname, "w+");
		char log_header[1024];
		memset(log_header, 0, 1024);
		strcat(log_header,
		       "period_start(clock_cycles),period_end(clock_cycles),job_end(clock_cycles),job_deadline(clock_cycles),job_elapsed(clock_cycles),period_start(seconds),period_end(seconds),job_end(seconds),job_deadline(seconds),job_elapsed(seconds),deadline_status(1=met),job_utilization,job_density");
#if (defined(AARCH64) && defined(CORTEX_A53)) ||                               \
	(defined(X86_64) && defined(CORE_I7))
		strcat(log_header,
		       ",job_l1_references,job_l1_misses,job_l1_miss_ratio(%%),job_l2_references,job_l2_misses,job_l2_miss_ratio(%%),instructions_retired,cpu_clock_count");
#endif
#ifdef EXTENDED_REPORT
		strcat(log_header, benchmark_log_header());
#endif
		strcat(log_header, "\n");
		fprintf(filep, "%s", log_header);
		if (exec_opts->output_path != NULL) {
			free(exec_opts->output_path);
		}
		if (filep == NULL) {
			perror("Cannot open output file");
			return -1;
		}
		elogf(LOG_LEVEL_TRACE, "Output file setup complete\n");
	}
	elogf(LOG_LEVEL_TRACE, "Initializing job environment\n");
	res = benchmark_init(benchmark_param_num, benchmark_params);
	if (res < 0) {
		perror("Error during job environment initialization");
		return res;
	}
	elogf(LOG_LEVEL_TRACE, "Job environment initialization complete\n");

	elogf(LOG_LEVEL_TRACE, "Starting signal handlers setup...\n");
	res = setup_signal(SIGNAL_DEADLINE, deadline_handler,
			   job_masked_signals, job_masked_signals_num);
	if (res < 0) {
		return res;
	}
	elogf(LOG_LEVEL_TRACE, "Deadline handler setup completed.\n");
	res = setup_signal(SIGNAL_END_PERIOD, period_handler,
			   job_masked_signals, job_masked_signals_num);
	if (res < 0) {
		return res;
	}
	elogf(LOG_LEVEL_TRACE, "Period handler setup completed.\n");
	res = setup_signal(SIGINT, quit_handler, quit_masked_signals,
			   quit_masked_signals_num);
	if (res < 0) {
		return res;
	}
	elogf(LOG_LEVEL_TRACE, "Quit handler setup completed.\n");
	elogf(LOG_LEVEL_TRACE, "Signal handlers installed.\n");

	if (exec_opts->bytes_to_preallocate > 0) {
		start_memory_watcher(exec_opts->bytes_to_preallocate);
	}

#if (defined(AARCH64) && defined(CORTEX_A53)) ||                               \
	(defined(X86_64) && defined(CORE_I7))
	res = setup_pmcs();
	if (res < 0) {
		return res;
	}
#endif

	elogf(LOG_LEVEL_TRACE, "Configuring timers...\n");
	// the deadline timer is created only if deadline and period differ
	if (exec_opts->parsed_deadline != exec_opts->parsed_period) {
		deadline_timer_status = DEADLINE_TIMER_IN_USE;
		// the deadline timer is setup with 0 interval since it will be armed once a
		// period starts
		res = setup_timer(&deadline_timer, SIGNAL_DEADLINE, 0, 0);
		if (res < 0) {
			return res;
		}
		// we setup the itimerspec struct to be used by the period handler
		deadline_timing.it_value.tv_nsec = exec_opts->deadline_nsec;
		deadline_timing.it_value.tv_sec = exec_opts->deadline_sec;
		deadline_timing.it_interval.tv_sec = 0;
		deadline_timing.it_interval.tv_nsec = 0;
		elogf(LOG_LEVEL_TRACE, "Deadline timer setup complete\n");
	} else {
		deadline_timer_status = !DEADLINE_TIMER_IN_USE;
	}

	res = setup_timer(&period_timer, SIGNAL_END_PERIOD,
			  exec_opts->period_sec, exec_opts->period_nsec);
	if (res < 0) {
		return res;
	}
	elogf(LOG_LEVEL_TRACE, "Period timer setup complete\n");
	elogf(LOG_LEVEL_TRACE, "Timers setup complete\n");
	// since timer will start shortly there are no previous jobs that are
	// executing we get the timestamp of the first period
	// This cycle will proceed infinitely if the user has not set a specific number of benchmarks to run or it will just terminate after having launched the specified amount of benchmarks.
	while (tasks_launched < exec_opts->tasks_to_launch ||
	       exec_opts->tasks_to_launch == 0) {
		// we wait for the period to finish
		do {
			res = sem_wait(&period_sem);

		} while (res < 0 && errno == EINTR);

		if (res < 0 && errno != EINTR) {
			perror("Error during period semaphore wait");
			return res;
		}
// we start executing the job
#if (defined(AARCH64) && defined(CORTEX_A53)) ||                               \
	(defined(X86_64) && defined(CORE_I7))
		if (exec_opts->memory_profiling_enable) {
			start_sampling();
		}
		job_perf_counters_start = pmcs_get_value();
#endif
		benchmark_execution(benchmark_param_num, benchmark_params);
#if (defined(AARCH64) && defined(CORTEX_A53)) ||                               \
	(defined(X86_64) && defined(CORE_I7))
		job_perf_counters_end = pmcs_get_value();
		if (exec_opts->memory_profiling_enable) {
			stop_sampling();
		}
#endif
		job_end_timestamp_clocks = get_rdtsc();
		job_end_timestamp = get_timestamp();
#ifdef EXTENDED_REPORT
		extra_measurement = benchmark_log_data();
#endif
		// we update the number of launched benchmarks
		tasks_launched++;
	}
	// we wait for the last period to finish before exiting.
	do {
		res = sem_wait(&period_sem);
	} while (res < 0 && errno == EINTR);
	return EXIT_SUCCESS;
}
