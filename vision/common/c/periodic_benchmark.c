#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <semaphore.h>
#include "periodic_benchmark.h"

///Struct used to pass data to the ::stop_benchmark function.
struct execution_data {
	timer_t timer; //< The timer associated with the benchmark
	int parameters_num; //< The parameters array length
	void **parameters; //< The parameters array
};

///The benchmark init function, which will be defined by the benchmark itself
extern int benchmark_init(int parameters_num, void **parameters);

///The benchmark execution function, which will be defined by the benchmark itself
extern void benchmark_execution(int parameters_num, void **parameters);

///The benchmark teardown function, which will be defined by the benchmark itself
extern void benchmark_teardown(int parameters_num, void **parameters);

/// This semaphore is used to determine if a job can be executed when the timer expires.
static sem_t job_sem;

/// A global variable which will indicate if the previous job has completed its execution.
static int job_completed;

/** Teardown function registered to be called when exit is called.
 * @param[in] status the exit status.
 * @param[in] arg Execution data struct, containing the job teardown function and its parameters.
 */
static void stop_benchmark(int status, void *arg)
{
	int res;
	struct execution_data *edata = (struct execution_data *)arg;
	printf("Deleting timer\n");
	res = timer_delete(edata->timer);
	if (res < 0) {
		perror("Error during timer deletion");
	}
	printf("Cleaning up job environment\n");
	benchmark_teardown(edata->parameters_num, edata->parameters);
}

/// \brief SIGINT handler, causes the program to terminate in a clean way.
static void quit_handler(int signo, siginfo_t *info, void *context)
{
	exit(0);
}

/// The signal handler that will determine what has to be done when the timer expires.
static void timer_handler(int signo, siginfo_t *info, void *context)
{
	int res;
	if (!job_completed) {
		///If the previous job is not completed, then a deadline miss is reported
		printf("Deadline missed!\n");
	} else {
		///Otherwise, the semaphore is unlocked, allowing the next job to be executed
		//We set job complete to 0 here, to avoid race conditions between the signal handler and the ::start_benchmark function.
		job_completed = 0;
		res = sem_post(&job_sem);
		if (res < 0) {
			perror("Error during semaphore post");
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

	struct execution_data edata;
	edata.parameters_num = exec_opts->args_num;
	edata.parameters = (void **)exec_opts->args;

	printf("Initializing job environment\n");
	res = benchmark_init(edata.parameters_num, edata.parameters);
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

	memset(&ev, 0, sizeof(ev));
	//the timer will call the signal handler
	ev.sigev_notify = SIGEV_SIGNAL;
	//the signal used is SIGRTMIN, has specified by the installed handler
	ev.sigev_signo = SIGRTMIN;
	//creation of the timer
	res = timer_create(CLOCK_REALTIME, &ev, &edata.timer);
	if (res != 0) {
		perror("Error during HR timer creation");
		return res;
	}

	//we setup the variables that are used in the execution pattern
	//since time has yet to start there are no previous jobs that are executing
	job_completed = 1;
	//We initialize the semaphore to allow only the execution of one job at a time and to share it only between threads of the same process
	res = sem_init(&job_sem, 1, 1);
	if (res < 0) {
		perror("Error during job semaphore initialization");
		return res;
	}
	res = on_exit(stop_benchmark, (void *)&edata);
	if (res != 0) {
		printf("Error during on_exit function registration");
		return -1;
	}

	//setting when the timer must be fired, using the provided deadline parameters
	timer_spec.it_interval.tv_sec = exec_opts->deadline_sec;
	timer_spec.it_interval.tv_nsec = exec_opts->deadline_nsec;
	//the timer will start as soon as possible
	timer_spec.it_value.tv_sec = 0;
	timer_spec.it_value.tv_nsec = 1;
	res = timer_settime(edata.timer, 0, &timer_spec, NULL);
	if (res < 0) {
		perror("Error during timer setup");
		return res;
	}

	printf("Benchmark setup complete\n");
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
		benchmark_execution(edata.parameters_num, edata.parameters);
		//we set ::job_completed to 1 since we have terminated the job execution.
		job_completed = 1;
	}
}
