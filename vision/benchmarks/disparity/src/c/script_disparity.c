#include "disparity_functions.h"
#include "rt_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/** \brief A small main function which will initialize the disparity benchmark to be run periodically, according to the given parameters.
 * \param[in] argc Number of given parameters, should be 2 or 3.
 * \param[in] argv given parameters array.
 * Parameters geinve in argc should be:
 * 1 - images directory path
 * 2 - deadline in seconds
 * 3 - deadline in nanoseconds, optional
 */
int main(int argc, char** argv){
	execution_data edata;
	long deadline_sec=0,deadline_nsec=0;
	int res;

	//Parsing of data directory path
	if(argc >= 3){
		edata.parameters=malloc(sizeof(void*));
		edata.parameters_num=1;
		edata.parameters[0]=(void*)argv[1];
	} else {
		printf("Missing data directory or deadline in seconds.\nArguments supported: [data path] [deadline in seconds] [deadline in nanoseconds, optional]\n");
		return EXIT_FAILURE;
	}

	// deadline parsing

	deadline_sec=strtol(argv[2],NULL,10);
	if(errno!=0){
		perror("error during deadline (seconds) parsing");
		return EXIT_FAILURE;
	}

	if(argc > 3){
		deadline_nsec=strtol(argv[3],NULL,10);
		if(errno!=0){
			perror("error during deadline (nanoseconds) parsing");
			return EXIT_FAILURE;
		}
	}

	//initialization of execution parameters
	edata.init=disparity_init;
	edata.execution=disparity_execution;
	edata.teardown=disparity_teardown;

	//timer initialization
	res=start_benchmark_timer(&edata, deadline_sec,deadline_nsec);
	if(res<0){
		return EXIT_FAILURE;
	}

	//cleanup of the execution parameters struct
	free(edata.parameters);
	return 0;
}
