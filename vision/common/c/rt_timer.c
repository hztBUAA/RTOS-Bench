#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "rt_timer.h"

///This variable is used by the sigint handler to escape from the while with sigsuspend, allowing a clean termination.
static int quit_request;

/// \brief SIGINT handler, modifies only ::quit_request.
static void quit_handler(int signo,siginfo_t* info, void* context){
	quit_request=1;
}

/**
 * This function will prepare the environment for the handler, initialize the timer and periodically report any missed deadlines. When a SIGINT is received, the timer will be destroyed and the environment for the handler execution will be cleared.
 *
 * The environment for the handler execution is handled by calling the init and teardown functions in the ::execution_data struct.
 *
 * If the init or teardown function pointers in ::execution_data are NULL the corresponding function will not be called.
 */
int start_benchmark_timer(execution_data *edata,long deadline_sec,long deadline_nsec){
	struct sigaction sa,sa_quit;
	struct sigevent ev;
	sigset_t wait_mask, proc_mask;
	struct itimerspec timer_spec;
	int missed_deadlines;
	int res;

	if(edata->execution==NULL){
		printf("Initializing timer with NULL signal handler, aborting");
		errno=EINVAL;
		return -1;
	}

	if(edata->init!=NULL){
		printf("Initializing handler environment\n");
		res=edata->init(edata->parameters_num,edata->parameters);
		if(res==-1){
			perror("Error during handler environment initialization");
			return res;
		}
		printf("Handler environment initialization complete\n");
	}

	printf("Starting timer setup\n");
	//we ignore the sigrtmin signal outside of the while with the sigsuspend
	sigemptyset(&proc_mask);
	sigaddset(&proc_mask,SIGRTMIN);
	sigprocmask(SIG_BLOCK,&proc_mask,NULL);

	// we prepare the mask for the SIGRTMIN handling
	sigemptyset(&sa.sa_mask);
	//we don't want to be interrupted if we miss the deadline
	sigaddset(&sa.sa_mask,SIGRTMIN);
	//we will handle the SIGINT to quit in a clean way, but without interrupting the handler
	sigaddset(&sa.sa_mask,SIGINT);
	sa.sa_flags= SA_SIGINFO;
	sa.sa_sigaction=edata->execution;
	//installing the signal handler for SIGRTMIN.
	res=sigaction(SIGRTMIN,&sa,NULL);
	if(res==-1){
		perror("Error during signal handler installation");
		return res;
	}

	//we also need to intercept SIGINT, to allow a clean termination
	sigemptyset(&sa_quit.sa_mask);
	sigaddset(&sa_quit.sa_mask,SIGINT);
	sa_quit.sa_flags= SA_SIGINFO;
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

	//setting when the timer must be fired, using the provided deadline parameters
	timer_spec.it_interval.tv_sec=deadline_sec;
	timer_spec.it_interval.tv_nsec=deadline_nsec;
	//the timer will start as soon as possible
	timer_spec.it_value.tv_sec=0;
	timer_spec.it_value.tv_nsec=1;
	res=timer_settime(edata->timer,0,&timer_spec,NULL);

	//now we wait indefinitely for a SIGRTMIN to execute the beanchmark or for a SIGINT to terminate.
	sigemptyset(&wait_mask);
	quit_request=0;
	printf("Timer setup setup complete\n");

	while(!quit_request){
		sigsuspend(&wait_mask);
		//after executing the benchmark we report if we have missed some deadlines
		missed_deadlines=timer_getoverrun(edata->timer);
		if(missed_deadlines>0){
			printf("%d deadlines have been missed during last task execution!\n",missed_deadlines);
		}
	}
	printf("Deleting timer\n");
	timer_delete(edata->timer);
	if(edata->teardown!=NULL){
		printf("Cleaning up handler environment\n");
		edata->teardown(edata->parameters_num,edata->parameters);
	}
	return 0;
}
