/********************************
 Author of the original version: Sravanthi Kota Venkata
 include -lrt to the compilation options!
 ********************************/

#include <stdio.h>
#include <stdlib.h>
#include "disparity.h"
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <time.h>

///The variables below are set by the main function and then only read by the signal handler, they contain the images.
I2D *imleft, *imright;
///The output path, to write the task result in a file, it is only read by the signal handler
char *output;
///This variable is used by the sigint handler to escape from the while with sigsuspend, allowing a clean termination.
int quit_request;

/// \brief SIGINT handler, modifies only ::quit_request.
void quit_handler(int signo,siginfo_t* info, void* context){
	quit_request=1;
}

/** \brief SIGRTMIN handler.
 * This handler contains the core part of the benchmark, where the disparity between the two images is computed.
 */
void signal_handler(int signo, siginfo_t* info,void* context){
	//We check that the SIGRTMIN has been received from a timer
	if(signo==SIGRTMIN && info->si_code == SI_TIMER){
		//We execute the benchmark
		unsigned int *start, *endC, *elapsed;
		int WIN_SZ=8, SHIFT=64;
		I2D *retDisparity;
		#ifdef test
		WIN_SZ = 2;
		SHIFT = 1;
		#endif
		#ifdef sim_fast
		WIN_SZ = 4;
		SHIFT = 4;
		#endif
		#ifdef sim
		WIN_SZ = 4;
		SHIFT = 8;
		#endif
		start = photonStartTiming();
		retDisparity = getDisparity(imleft, imright, WIN_SZ, SHIFT);
		endC = photonEndTiming();

		printf("Input size\t\t- (%dx%d)\n", imleft->height, imleft->width);
		#ifdef CHECK
		/** Self checking - use expected.txt from data directory  **/
		{
			int tol, ret=0;
			tol = 2;
			#ifdef GENERATE_OUTPUT
			writeMatrix(retDisparity, output);
			#endif
			printf("output: %s\n",output);
			ret = selfCheck(retDisparity, output, tol);
			if (ret == -1)
				printf("Error in Disparity Map\n");
		}
		/** Self checking done **/
		#endif

		elapsed = photonReportTiming(start, endC);
		photonPrintTiming(elapsed);
		//We free the resources allocated.
		iFreeHandle(retDisparity);
		free(start);
		free(endC);
		free(elapsed);
	} else {
		//Is the signal was not received by a timer we quit without doing anything
		printf("ERROR: rogue sigrtmin received!\n");
		quit_request=1;
	}
}

/** \brief Benchmark main function
 * The main will only setup the timer, the signal handlers and load the images that will be used by the benchmark.
 * It takes two or three parameters:
 * 1 - the folder where the images are located, and which will be also used as output.
 * 2 - the deadline of a single benchmark in seconds.
 * 3 - the deadline of a single benchmark in nanoseconds.
 */
int main(int argc, char* argv[])
{
	int res;
	char im1[100], im2[100];

	long deadline_nsec=0, deadline_sec=0;

	struct sigaction sa,sa_quit;
	struct sigevent ev;
	sigset_t wait_mask, proc_mask;
	timer_t timer;
	struct itimerspec timer_spec;
	int missed_deadlines;

	printf("Starting setup of periodic disparity benchmark\n");
	//we ignore the sigrtmin signal outside of the while with the sigsuspend
	sigemptyset(&proc_mask);
	sigaddset(&proc_mask,SIGRTMIN);
	sigprocmask(SIG_BLOCK,&proc_mask,NULL);

	if(argc < 3){
		printf("argc: %d\n",argc);
		printf("usage: [input/output folder] [deadline in seconds] optional: [deadline in nanoseconds]\n");
		return -1;
	}
	//we move the input/output folder to the output variable, to make it accessible from the signal handler.
	output=argv[1];

	//we parse the deadline in seconds
	deadline_sec=strtol(argv[2],NULL,10);
	if(errno==ERANGE || errno==EINVAL){
		perror("error while parsing deadline in seconds");
		return EXIT_FAILURE;
	}

	//if provided, we parse the deadline in nanoseconds
	if(argc>3){
		deadline_nsec=strtol(argv[3],NULL,10);
		if(errno==ERANGE || errno == EINVAL){
			perror("error while parsing deadline in nanoseconds");
			return EXIT_FAILURE;
		}
	}

	//load images

	sprintf(im1, "%s/1.bmp", argv[1]);
	sprintf(im2, "%s/2.bmp", argv[1]);

	imleft = readImage(im1);
	imright = readImage(im2);

	// we prepare the mask for the SIGRTMIN handling
	sigemptyset(&sa.sa_mask);
	//we don't want to be interrupted if we miss the deadline
	sigaddset(&sa.sa_mask,SIGRTMIN);
	//we will handle the SIGINT to quit in a clean way, but without interrupting the handler
	sigaddset(&sa.sa_mask,SIGINT);
	sa.sa_flags= SA_SIGINFO;
	sa.sa_sigaction=signal_handler;
	//installing the signal handler for SIGRTMIN.
	res=sigaction(SIGRTMIN,&sa,NULL);
	if(res==-1){
		perror("error during signal handler installation");
		return EXIT_FAILURE;
	}

	//we also need to intercept SIGINT, to allow a clean termination
	sigemptyset(&sa_quit.sa_mask);
	sigaddset(&sa_quit.sa_mask,SIGINT);
	sa_quit.sa_flags= SA_SIGINFO;
	sa_quit.sa_sigaction=quit_handler;
	res=sigaction(SIGINT,&sa_quit,NULL);
	if(res==-1){
		perror("error during sigint handler installation");
		return EXIT_FAILURE;
	}

	memset(&ev,0,sizeof(ev));
	//the timer will call the signal handler
	ev.sigev_notify=SIGEV_SIGNAL;
	//the signal used is SIGRTMIN, has specified by the installed handler
	ev.sigev_signo=SIGRTMIN;
	//creation of the timer
	res=timer_create(CLOCK_REALTIME,&ev,&timer);
	if (res!=0){
		perror("can't create HR timer");
		return EXIT_FAILURE;
	}

	//setting when the timer must be fired, using the provided deadline parameters
	timer_spec.it_interval.tv_sec=deadline_sec;
	timer_spec.it_interval.tv_nsec=deadline_nsec;
	//the timer will start as soon as possible
	timer_spec.it_value.tv_sec=0;
	timer_spec.it_value.tv_nsec=1;
	res=timer_settime(timer,0,&timer_spec,NULL);

	//now we wait indefinitely for a SIGRTMIN to execute the beanchmark or for a SIGINT to terminate.
	sigemptyset(&wait_mask);
	quit_request=0;
	printf("disparity benchmark setup complete\n");
	while(!quit_request){
		sigsuspend(&wait_mask);
		//after executing the benchmark we report if we have missed some deadlines
		missed_deadlines=timer_getoverrun(timer);
		if(missed_deadlines>0){
			printf("%d deadlines have been missed during last task execution!\n",missed_deadlines);
		}
	}
	//when a SIGINT is received execution will reach this point, where we free the allocated resources.
	printf("quitting\n");
	timer_delete(timer);
	iFreeHandle(imleft);
	iFreeHandle(imright);
	return 0;
}
