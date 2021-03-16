#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <semaphore.h>
#include "periodic_benchmark.h"
#include "get_cpu_timestamp.h"

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
static unsigned long long start_timestamp = 0;

///Timestamp of when the last job execution has completed, 0 is the job has not finished or is not running.
static unsigned long long end_timestamp = 0;

/** Teardown function registered to be called when exit is called.
 * @param[in] status the exit status.
 * @param[in] arg Ignored.
 */
static void stop_benchmark(int status, void *arg)
{
	int res;
	if (filep != NULL) {
		printf("Flushing output file buffer\n");
		res = fflush(filep);
		if (res == EOF) {
			perror("Cannot flush file buffer");
		}
		printf("Closing output file\n");
		res = fclose(filep);
		if (res == EOF) {
			perror("Error during output file close");
		}
	}
	if (timer != NULL) {
		printf("Deleting timer\n");
		res = timer_delete(timer);
		if (res < 0) {
			perror("Error during timer deletion");
		}
	}
	res = sem_destroy(&job_sem);
	if (res < 0) {
		perror("Error during semaphore destruction");
	}
	printf("Cleaning up job environment\n");
	benchmark_teardown(benchmark_param_num, benchmark_params);
}

/// \brief SIGINT handler, causes the program to terminate in a clean way.
static void quit_handler(int signo, siginfo_t *info, void *context)
{
	exit(0);
}

/** The signal handler that will determine what has to be done when the timer expires.
 * If the benchmark is not completed within the dealdine ( start or end timestamp does not have a non-zero value) a deadline miss is reported.
 * If the benchmark has completed (start and end timestamps have both a non-zero value) the job timing is reported.
 * Regardless of the job status the semaphore (::job_sem) value is incremented if it is 0, to allow the next job to start as soon as possible.
 *
 * The semaphore incrementation does not create race conditions between a job that ha missed the deadline and the next one, since jobs are executed sequentially. 
*/
static void timer_handler(int signo, siginfo_t *info, void *context)
{
	int res;
	int sem_val = 0;
	unsigned long long elapsed_timestamp = 0;
	unsigned long long deadline_timestamp = get_cpu_timestamp();
	//we have met a deadline if the job has been completed.
	printf("\n\n\tDeadline reached at:%llu\n", deadline_timestamp);
	if (start_timestamp > 0 && end_timestamp > 0) {
		//We print the timing information of the last completed job.
		elapsed_timestamp = end_timestamp - start_timestamp;
		printf("Deadline MET\nstart_timestamp: %llu\nend_timestamp: %llu\nelapsed: %llu\n",
		       start_timestamp, end_timestamp, elapsed_timestamp);
		res = fprintf(filep, "%llu,%llu,%llu,%llu,%d\n",
			      start_timestamp, end_timestamp, elapsed_timestamp,
			      deadline_timestamp, DEADLINE_MET);
		if (res < 0) {
			perror("cannot write on output file");
			exit(-1);
		}
	} else {
		//we notify that the deadline has been missed
		printf("Deadline MISSED\nstart_timestamp: %llu\n",
		       start_timestamp);
		res = fprintf(filep, "%llu,%llu,%llu,%llu,%d\n",
			      start_timestamp, end_timestamp, elapsed_timestamp,
			      deadline_timestamp, DEADLINE_MISSED);
		if (res < 0) {
			perror("cannot write on output file");
			exit(-1);
		}
	}
	//we reset the timestamps to avoid reporting the same job status more than once
	start_timestamp = 0;
	end_timestamp = 0;
	res = sem_getvalue(&job_sem, &sem_val);
	if (res < 0) {
		perror("Cannot read semaphore value");
		exit(-1);
	}
	//we unlock the job execution for the next deadline, if is not already unlocked.
	if (sem_val == 0) {
		//we reset both start_timestamp and end_timestamp to 0, to avoid issues between different deadlines.
		res = sem_post(&job_sem);
		if (res < 0) {
			perror("Error during semaphore post");
			exit(-1);
		}
	}
}

/**
 * This function will prepare the environment for executing the job, initialize the timer and periodically report any missed deadlines. 
 * When the environment for the periodic benchmark is initialized, the benchmark will be periodically executed.
 * When a SIGINT is received, the timer will be destroyed and the environment for the job execution will be cleaned.
 *
 * The environment for the job execution is handled by calling the ::benchmark_init and ::benchmark_teardown functions.
 *
 */
int periodic_benchmark(struct execution_options *exec_opts)
{
	struct sigaction sa, sa_quit;
	struct sigevent ev;
	struct itimerspec timer_spec;
	int res;
	char *fname;
	int fname_len;

	printf("Starting setup of execution environment\n");
	//we setup the variables that are used in the execution pattern
	//We initialize the semaphore to allow only the execution of one job at a time and to share it only between threads of the same process
	res = sem_init(&job_sem, 1, 1);
	if (res < 0) {
		perror("Error during job semaphore initialization");
		return res;
	}
	res = on_exit(stop_benchmark, NULL);
	if (res != 0) {
		printf("Error during on_exit function registration");
		return -1;
	}
	benchmark_param_num = exec_opts->args_num;
	benchmark_params = (void **)exec_opts->args;
	printf("Execution environment setup complete\n");

	printf("Starting output file setup\n");
	//we construct the file path
	fname_len = strlen(exec_opts->output_path) + strlen(OUTPUT_FNAME) +
		    strlen("/") + 1;
	fname = malloc(sizeof(char) * fname_len);
	memset(fname, 0, fname_len);
	res = snprintf(fname, fname_len, "%s/%s", exec_opts->output_path,
		       OUTPUT_FNAME);

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
	//we write the csv header
	res = fprintf(
		filep,
		"start_timestamp(0=benchmark not started),end_timestamp(0=benchmark not completed yet),elapsed,deadline_timestamp,status(%d=deadline met %d=deadline missed)\n",
		DEADLINE_MET, DEADLINE_MISSED);
	if (res < 0) {
		perror("cannot write on output file");
		return -1;
	}
	printf("Output file setup complete\n");

	printf("Initializing job environment\n");
	res = benchmark_init(benchmark_param_num, benchmark_params);
	if (res == -1) {
		perror("Error during job environment initialization");
		return res;
	}
	printf("Job environment initialization complete\n");

	printf("Starting timer setup\n");
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
	printf("Signal handlers installed\n");

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
	printf("Timer created\n");

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

	printf("Timer setup complete\n");
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
		//we execute the job
		start_timestamp = get_cpu_timestamp();
		benchmark_execution(benchmark_param_num, benchmark_params);
		end_timestamp = get_cpu_timestamp();
	}
}
