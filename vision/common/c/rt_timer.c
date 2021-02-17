#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "rt_timer.h"
#include <semaphore.h>

/// This semaphore is used to determine if a job can be executed when the timer expires.
static sem_t job_sem;

/// A global variable which will indicate if the previous job has completed its execution.
static int job_completed;

/** \brief teardown function registered to be called when exit is called.
 * \param[in] edata Execution data struct, containing the job teardown function and its parameters.
 */
static void stop_benchmark(int status,void* arg){
	int res;
	execution_data* edata=(execution_data*) arg;
	printf("Deleting timer\n");
	res=timer_delete(edata->timer);
	if(res<0){
		perror("Error during timer deletion");
	}
	if(edata->teardown!=NULL){
		printf("Cleaning up job environment\n");
		edata->teardown(edata->parameters_num,edata->parameters);
	}
}

/// \brief SIGINT handler, causes the program to terminate in a clean way.
static void quit_handler(int signo,siginfo_t* info, void* context){
	exit(0);
}

/// The signal handler that will determine what has to be done when the timer expires.
static void timer_handler(int signo, siginfo_t* info, void* context){
	int res;
	if(!job_completed){
		///If the previous job is not completed, then a deadline miss is reported
		printf("Deadline missed!\n");
	} else{
		///Otherwise, the semaphore is unlocked, allowing the next job to be executed
		//We set job complete to 0 here, to avoid race conditions between the signal handler and the ::start_benchmark function.
		job_completed=0;
		res=sem_post(&job_sem);
		if(res<0){
			perror("Error during semaphore post");
		}
	}
}

/**
 * This function will prepare the environment for executing the job, initialize the timer and periodically report any missed deadlines. When a SIGINT is received, the timer will be destroyed and the environment for the job execution will be cleaned.
 *
 * The environment for the job execution is handled by calling the init and teardown functions in the ::execution_data struct.
 *
 * If the init or teardown function pointers in ::execution_data are NULL the corresponding function will not be called.
 */
int start_benchmark(execution_data *edata,long deadline_sec,long deadline_nsec){
	struct sigaction sa,sa_quit;
	struct sigevent ev;
	struct itimerspec timer_spec;
	int res;

	if(edata->execution==NULL){
		printf("Initializing timer without a job execution function, aborting");
		errno=EINVAL;
		return -1;
	}

	if(edata->init!=NULL){
		printf("Initializing job environment\n");
		res=edata->init(edata->parameters_num,edata->parameters);
		if(res==-1){
			perror("Error during job environment initialization");
			return res;
		}
		printf("Job environment initialization complete\n");
	}

	printf("Starting timer setup\n");
	// we prepare the mask for the SIGRTMIN handling
	res=sigemptyset(&sa.sa_mask);
	if(res==-1){
		perror("Error during sigemptyset for SIGRTMIN handler");
		return res;
	}
	//we don't want to be interrupted if we miss the deadline
	res=sigaddset(&sa.sa_mask,SIGRTMIN);
	if(res==-1){
		perror("Error during first sigaddset for SIGRTMIN handler");
		return res;
	}
	if(res==-1){
		perror("Error during second sigaddset for SIGRTMIN handler");
		return res;
	}
	sa.sa_flags= SA_SIGINFO;
	sa.sa_sigaction=timer_handler;
	//installing the signal handler for SIGRTMIN.
	res=sigaction(SIGRTMIN,&sa,NULL);
	if(res==-1){
		perror("Error during signal handler installation");
		return res;
	}

	//we also need to intercept SIGINT, to allow a clean termination
	res=sigemptyset(&sa_quit.sa_mask);
	if(res==-1){
		perror("Error during sigemptyset for SIGRTMIN handler");
		return res;
	}
	res=sigaddset(&sa_quit.sa_mask,SIGINT);
	if(res==-1){
		perror("Error during first sigaddset for SIGINT handler");
		return res;
	}
	res=sigaddset(&sa_quit.sa_mask,SIGRTMIN);
	if(res==-1){
		perror("Error during first sigaddset for SIGINT handler");
		return res;
	}
	sa_quit.sa_flags=SA_SIGINFO;
	sa_quit.sa_sigaction=quit_handler;
	res=sigaction(SIGINT,&sa_quit,NULL);
	if(res==-1){
		perror("Error during sigint handler installation");
		return res;
	}

	memset(&ev,0,sizeof(ev));
	//the timer will call the signal handler
	ev.sigev_notify=SIGEV_SIGNAL;
	//the signal used is SIGRTMIN, has specified by the installed handler
	ev.sigev_signo=SIGRTMIN;
	//creation of the timer
	res=timer_create(CLOCK_REALTIME,&ev,&edata->timer);
	if (res!=0){
		perror("Error during HR timer creation");
		return res;
	}

	//we setup the variables that are used in the execution pattern
	//since time has yet to start there are no previous jobs that are executing
	job_completed=1;
	//We initialize the semaphore to allow only the execution of one job at a time and to share it only between threads of the same process
	res=sem_init(&job_sem,1,1);
	if(res<0){
		perror("Error during job semaphore initialization");
		return res;
	}
	res=on_exit(stop_benchmark,edata);
	if(res!=0){
		printf("Error during on_exit function registration");
		return -1;
	}

	//setting when the timer must be fired, using the provided deadline parameters
	timer_spec.it_interval.tv_sec=deadline_sec;
	timer_spec.it_interval.tv_nsec=deadline_nsec;
	//the timer will start as soon as possible
	timer_spec.it_value.tv_sec=0;
	timer_spec.it_value.tv_nsec=1;
	res=timer_settime(edata->timer,0,&timer_spec,NULL);
	if(res<0){
		perror("Error during timer setup");
		return res;
	}

	printf("Timer setup complete\n");

	while(1){
		//we wait on the semaphore, to be sure to be the only job in execution, we need to consider that the signal handler will interrupt the sem_wait, so if it gets interrupted we need to retry it.
		do{
			res=sem_wait(&job_sem);
		}while(res<0 && errno==EINTR);
		if(res<0 && errno!=EINTR){
			perror("Error during semaphore wait");
			return res;
		}
		//we execute the job
		edata->execution(edata->parameters_num,edata->parameters);
		//we set ::job_completed to 1 since we have terminated the job execution.
		job_completed=1;
	}
}
