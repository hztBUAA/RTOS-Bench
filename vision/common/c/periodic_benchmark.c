/** @file periodic_benchmark.c
 * @brief Implementation of a general periodic benchmark using a real time timer.
 * @details Timer expiration triggers a `SIGRTMIN` and `SIGINT` is used to stop and destroy the timer.
 */

#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <semaphore.h>
#include "periodic_benchmark.h"
#include "get_cpu_timestamp.h"
#include "logging.h"

/// Deadline missed status
#define DEADLINE_MISSED 0
/// Deadline met status
#define DEADLINE_MET 1

///Name of the output file with timing information
#define OUTPUT_FNAME "timing.csv"

///Number of parameters passed to the benchmark
static int benchmark_param_num = 0;

///Benchmark parameters array
static void **benchmark_params = NULL;

///Real time timer, used to notify when the deadline is reached.
static timer_t timer = NULL;

///The file pointer to the output file
static FILE *filep = NULL;

/// This semaphore is used to determine if a job can be executed when the timer expires.
static sem_t job_sem;

///Timestamp of when the last job has started its execution, it is 0 if no job is running.
static unsigned long long job_start_timestamp = 0;

///Timestamp of when the last job execution has completed, 0 is the job has not finished or is not running.
static unsigned long long job_end_timestamp = 0;

///If the current job has either met or missed the deadline.
static int job_deadline_status = DEADLINE_MET;

/** @brief The timestamp of the job deadline. 
 * The job deadline is the first deadline that occurs after the job has started. 
 * 0 is used to indicate that the deadline did not occur yet.
 */
static unsigned long long job_deadline_timestamp = 0;

/**
 * @brief Teardown function registered to be called when exit is called.
 * @param[in] status the exit status.
 * @param[in] arg Ignored.
 * @details Will ensure that all the requested resourced are freed and the ouput file is flushed and closed.
 */
static void stop_benchmark(int status, void *arg)
{
	int res;
	if (filep != NULL) {
		elogf(LOG_LEVEL_TRACE, "Flushing output file buffer\n");
		res = fflush(filep);
		if (res == EOF) {
			perror("Cannot flush file buffer");
		}
		elogf(LOG_LEVEL_TRACE, "Closing output file\n");
		res = fclose(filep);
		if (res == EOF) {
			perror("Error during output file close");
		}
	}
	if (timer != NULL) {
		elogf(LOG_LEVEL_TRACE, "Deleting timer\n");
		res = timer_delete(timer);
		if (res < 0) {
			perror("Error during timer deletion");
		}
	}
	res = sem_destroy(&job_sem);
	if (res < 0) {
		perror("Error during semaphore destruction");
	}
	elogf(LOG_LEVEL_TRACE, "Cleaning up job environment\n");
	benchmark_teardown(benchmark_param_num, benchmark_params);
}

/**
 * @brief SIGINT handler, causes the program to terminate in a clean way.
 * @param signo Ignored.
 * @param info Ignored.
 * @param context Ignored.
 * @details Will call the exit function, it's invoked when a `SIGINT` is received
 */
static void quit_handler(int signo, siginfo_t *info, void *context)
{
	exit(0);
}

/**
 * @brief The signal handler that will report what happens when the timer expires (and thus a deadline is met).
 * @param signo Ignored.
 * @param info Ignored.
 * @param context Ignored.
 * @details
 * When a job completes, we report its timing information, we reset the timing variables to their default value and the
 * semaphore (`::job_sem`) value is incremented, to allow the next job to start.
 *
 * The job timing is composed by the following elements:
 * 1. job start timestamp (`::job_start_timestamp`);
 * 2. job end timestamp (`::job_end_timestamp`);
 * 3. job elapsed timestamp (computed as `::job_end_timestamp - ::job_start_timestamp`);
 * 4. timestamp of the first deadline since job start, called "job deadline" (`::job_deadline_timestamp`);
 * 5. job deadline status: 
 *   - `1` if the job deadline was met.
 *   - `0` if the job deadline was missed.
 *
 * Job timing is reported in a single line where these information are separated by commas: `29191750731621,29191750836938,105317,29191750746215,0`.
 *
 * If the deadline arrives before the job has completed, a deadline miss will be reported once the job completes.
 * Since jobs that miss a deadline are not killed, more than a deadline can occur during a job execution.
 * We report any deadline that occur during the job execution, after the first missed deadline, in the following way:\n
 *
 * All the timing information is set to `0`, except for the deadline timestamp: `0,0,0,29191750951240,0`.
*/
static void timer_handler(int signo, siginfo_t *info, void *context)
{
	int res;
	unsigned long long elapsed_timestamp = 0;
	unsigned long long deadline_timestamp = get_cpu_timestamp();
	//we need to remember the first deadline since the job has started.
	if (job_deadline_timestamp == 0) {
		job_deadline_timestamp = deadline_timestamp;
	}
	elogf(LOG_LEVEL_TRACE, "\n\n\tDeadline reached at:%llu\n",
	      deadline_timestamp);
	if (job_start_timestamp > 0 && job_end_timestamp > 0) {
		// report timing for completed job
		elapsed_timestamp = job_end_timestamp - job_start_timestamp;
		elogf(LOG_LEVEL_TRACE,
		      "Job completed\nstart_timestamp: %llu\nend_timestamp: %llu\nelapsed: %llu\ndeadline status (1=met):%d\n",
		      job_start_timestamp, job_end_timestamp, elapsed_timestamp,
		      job_deadline_status);
		flogf(LOG_LEVEL_FILE, filep, "%llu,%llu,%llu,%llu,%d\n",
		      job_start_timestamp, job_end_timestamp, elapsed_timestamp,
		      job_deadline_timestamp, job_deadline_status);
		logf(LOG_LEVEL_INFO, "%llu,%llu,%llu,%llu,%d\n",
		     job_start_timestamp, job_end_timestamp, elapsed_timestamp,
		     job_deadline_timestamp, job_deadline_status);
		// reset timing information
		job_deadline_status = DEADLINE_MET;
		job_deadline_timestamp = 0;
		job_end_timestamp = 0;
		job_start_timestamp = 0;
		// unlock next job
		res = sem_post(&job_sem);
		if (res < 0) {
			perror("Error during semaphore post");
			exit(-1);
		}
	}
	// job has not terminated when deadline is reached
	else {
		job_deadline_status = DEADLINE_MISSED;
		// report skipped deadlines after the first missed
		if (job_deadline_timestamp != deadline_timestamp) {
			elogf(LOG_LEVEL_TRACE,
			      "Deadline MISSED\nstart_timestamp: %llu\n",
			      job_start_timestamp);
			flogf(LOG_LEVEL_FILE, filep, "0,0,0,%llu,%d\n",
			      deadline_timestamp, DEADLINE_MISSED);
			logf(LOG_LEVEL_INFO, "0,0,0,%llu,%d\n",
			     deadline_timestamp, DEADLINE_MISSED);
		}
	}
}

/** @details
 * This function will prepare the environment for executing the job, initialize the timer and periodically report any missed deadlines.
 * When the environment for the periodic benchmark is initialized, the benchmark will be periodically executed.
 * When a SIGINT is received, the timer will be destroyed and the environment for the job execution will be cleaned.
 *
 * The environment for the job execution is handled by calling the benchmark_init() and benchmark_teardown() functions.
 */
int periodic_benchmark(struct execution_options *exec_opts)
{
	struct sigaction sa, sa_quit;
	struct sigevent ev;
	struct itimerspec timer_spec;
	int res;
	char *fname;
	int fname_len;

	elogf(LOG_LEVEL_TRACE, "Starting setup of execution environment\n");
	//we setup the variables that are used in the execution pattern
	//We initialize the semaphore to allow only the execution of one job at a time and to share it only between threads of the same process
	res = sem_init(&job_sem, 1, 1);
	if (res < 0) {
		perror("Error during job semaphore initialization");
		return res;
	}
	res = on_exit(stop_benchmark, NULL);
	if (res != 0) {
		elogf(LOG_LEVEL_ERR,
		      "Error during on_exit function registration");
		return -1;
	}
	benchmark_param_num = exec_opts->args_num;
	benchmark_params = (void **)exec_opts->args;
	elogf(LOG_LEVEL_TRACE, "Execution environment setup complete\n");
	if (benchmark_verbosity >= LOG_LEVEL_FILE) {
		elogf(LOG_LEVEL_TRACE, "Starting output file setup\n");
		//we construct the file path
		fname_len = strlen(exec_opts->output_path) +
			    strlen(OUTPUT_FNAME) + strlen("/") + 1;
		fname = malloc(sizeof(char) * fname_len);
		memset(fname, 0, fname_len);
		res = snprintf(fname, fname_len, "%s/%s",
			       exec_opts->output_path, OUTPUT_FNAME);

		//we exclude the terminator char from the check
		if (res < fname_len - 1) {
			perror("Cannot generate output file path");
			free(fname);
			return -1;
		}
		//we open the file where we will write
		filep = fopen(fname, "w+");
		free(fname);
		if (filep == NULL) {
			perror("Cannot open output file");
			return -1;
		}
	}
	//we write the csv header
	flogf(LOG_LEVEL_FILE, filep,
	      "start_timestamp(0=benchmark not started),end_timestamp(0=benchmark not completed yet),elapsed,deadline_timestamp,status(%d=deadline met %d=deadline missed)\n",
	      DEADLINE_MET, DEADLINE_MISSED);
	elogf(LOG_LEVEL_TRACE, "Output file setup complete\n");

	elogf(LOG_LEVEL_TRACE, "Initializing job environment\n");
	res = benchmark_init(benchmark_param_num, benchmark_params);
	if (res == -1) {
		perror("Error during job environment initialization");
		return res;
	}
	elogf(LOG_LEVEL_TRACE, "Job environment initialization complete\n");

	elogf(LOG_LEVEL_TRACE, "Starting timer setup\n");
	// we prepare the mask for the SIGRTMIN handling
	res = sigemptyset(&sa.sa_mask);
	if (res == -1) {
		perror("Error during sigemptyset for SIGRTMIN handler");
		return res;
	}
	//we don't want to be interrupted if we miss the deadline
	res = sigaddset(&sa.sa_mask, SIGRTMIN);
	if (res == -1) {
		perror("Error during first sigaddset for SIGRTMIN handler");
		return res;
	}
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = timer_handler;
	//installing the signal handler for SIGRTMIN.
	res = sigaction(SIGRTMIN, &sa, NULL);
	if (res == -1) {
		perror("Error during signal handler installation");
		return res;
	}

	//we also need to intercept SIGINT, to allow a clean termination
	res = sigemptyset(&sa_quit.sa_mask);
	if (res == -1) {
		perror("Error during sigemptyset for SIGRTMIN handler");
		return res;
	}
	res = sigaddset(&sa_quit.sa_mask, SIGINT);
	if (res == -1) {
		perror("Error during first sigaddset for SIGINT handler");
		return res;
	}
	res = sigaddset(&sa_quit.sa_mask, SIGRTMIN);
	if (res == -1) {
		perror("Error during first sigaddset for SIGINT handler");
		return res;
	}
	sa_quit.sa_flags = SA_SIGINFO;
	sa_quit.sa_sigaction = quit_handler;
	res = sigaction(SIGINT, &sa_quit, NULL);
	if (res == -1) {
		perror("Error during sigint handler installation");
		return res;
	}
	elogf(LOG_LEVEL_TRACE, "Signal handlers installed\n");

	memset(&ev, 0, sizeof(ev));
	//the timer will call the signal handler
	ev.sigev_notify = SIGEV_SIGNAL;
	//the signal used is SIGRTMIN, has specified by the installed handler
	ev.sigev_signo = SIGRTMIN;
	//creation of the timer
	res = timer_create(CLOCK_REALTIME, &ev, &timer);
	if (res != 0) {
		perror("Error during HR timer creation");
		return res;
	}
	elogf(LOG_LEVEL_TRACE, "Timer created\n");

	//setting when the timer must be fired, using the provided deadline parameters
	timer_spec.it_interval.tv_sec = exec_opts->deadline_sec;
	timer_spec.it_interval.tv_nsec = exec_opts->deadline_nsec;
	//the timer will start according to the setup deadline
	timer_spec.it_value.tv_sec = exec_opts->deadline_sec;
	timer_spec.it_value.tv_nsec = exec_opts->deadline_nsec;
	res = timer_settime(timer, 0, &timer_spec, NULL);
	if (res < 0) {
		perror("Error during timer setup");
		return res;
	}

	elogf(LOG_LEVEL_TRACE, "Timer setup complete\n");
	//since timer will start shortly there are no previous jobs that are executing
	while (1) {
		//we wait on the semaphore, to be sure to be the only job in execution, we need to consider that the signal handler will interrupt the sem_wait, so if it gets interrupted we need to retry it.
		do {
			res = sem_wait(&job_sem);

		} while (res < 0 && errno == EINTR);

		if (res < 0 && errno != EINTR) {
			perror("Error during semaphore wait");
			return res;
		}
		//we start executing the job
		job_start_timestamp = get_cpu_timestamp();
		benchmark_execution(benchmark_param_num, benchmark_params);
		job_end_timestamp = get_cpu_timestamp();
	}
}
