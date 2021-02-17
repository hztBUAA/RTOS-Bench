/**
 \file disparity_functions.c
 * Functions used to run the disparity benchmark periodically.
 * Author of the original version: Sravanthi Kota Venkata.
 * Include -lrt to the compilation options.
 */

#include <stdio.h>
#include <stdlib.h>
#include "disparity.h"
#include <signal.h>
#include <errno.h>

/** Will load the images that will be used by the benchmark.
 * The parameters array will have the following structure after this function has terminated correctly:
 * parameters[0]: image folder path
 * parameters[1]: left image (as an I2D*)
 * parameters[2]: right image (as an I2D*)
 */
int disparity_init(int parameters_num,void** parameters){
	char im1[100], im2[100];

	if(parameters_num<3){
		printf("wrong parameters list supplied!\n");
		errno=EINVAL;
		return -1;
	}

	//load images

	sprintf(im1, "%s/1.bmp", parameters[0]);
	sprintf(im2, "%s/2.bmp", parameters[0]);

	parameters[1] = readImage(im1);
	parameters[2] = readImage(im2);
	return 0;
}

///This handler contains the core part of the benchmark, where the disparity between the two images is computed.
void disparity_execution(int parameters_num, void** parameters){
	if(parameters_num<3){
		printf("wrong parameters list supplied!\n");
		errno=EINVAL;
		return;
	}
	char* output=parameters[0];
	I2D *imleft=parameters[1], *imright=parameters[2];
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
}

///It will deallocate the images structure created by ::disparity_init.
void disparity_teardown(int parameters_num,void** parameters){
	if(parameters_num<3){
		printf("wrong parameters list supplied!\n");
		errno=EINVAL;
		return;
	}
	iFreeHandle(parameters[1]);
	iFreeHandle(parameters[2]);
}
