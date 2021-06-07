/**
 * @file script_multi_ncut.c
 * @ingroup multi_ncut
 * @brief Functions used to run the multi_ncut  benchmark periodically.
 * @details
 * The original script has been broken down in three components:
 * - init: benchmark_init();
 * - execution: benchmark_execution();
 * - teardown: benchmark_teardown();
 *
 * This allows the benchmark to be run periodically, by re-running only the execution portion.
 * @author Sravanthi Kota Venkata, for the original version.
 */

#include <stdio.h>
#include <stdlib.h>
#include "segment.h"

///The input image.
static I2D *im;

/**
 * @brief Will load the images that will be used by the benchmark.
 * @param[in] parameters_num Number of parameters passed, should be 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The required parameters array has the following structure:
 * - parameters[0]: image folder path;
 * Image will be loaded into `::im`.
 */
int benchmark_init(int parameters_num, void **parameters)
{
	char im1[256];

	if (parameters_num < 1) {
		elogf(LOG_LEVEL_ERR,
		      "We need input image path and output path\n");
		return -1;
	}

	sprintf(im1, "%s/1.bmp", parameters[0]);
	im = readImage(im1);
	return 0;
}

/**
 * @brief This handler is where the disparity between the two images is computed.
 * @param[in] parameters_num Number of passed parameters, should be 0 or 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The list of passed parameters must provide the output folder path as only element of the parameters array when the macro `CHECK` is defined.
 */
void benchmark_execution(int parameters_num, void **parameters)
{
	float sigma = 0.6;
	float k = 4;
	int min_size = 10;
	int num_ccs[1] = { 0 };
	I2D *out;
	I2D *seg;
#ifdef CHECK
	if (parameters_num < 1) {
		elogf(LOG_LEVEL_ERR, "Missing output folder path\n");
		exit(-1);
	}
#endif
	elogf(LOG_LEVEL_TRACE, "Input size\t\t- (%dx%d)\n", im->height,
	      im->width);
	seg = segment_image(im, sigma, k, min_size, num_ccs);
	out = seg;

#ifdef CHECK
	/** Self checking - use expected.txt from data directory  **/
	{
		int ret = 0;
		float tol = 0;

#ifdef GENERATE_OUTPUT
		writeMatrix(out, parameters[0]);
#endif

		ret = selfCheck(out, parameters[0], tol);
		if (ret < 0)
			printf("Error in Multi N Cut\n");
	}
#endif
	iFreeHandle(seg);
}

/**
 * @brief Will revert what benchmark_init() has done to initialize the benchmark.
 * @param[in] parameters_num Ignored.
 * @param[in] parameters Ignored.
 * @details It will deallocate `::im`.
 */
void benchmark_teardown(int parameters_num, void **parameters)
{
	iFreeHandle(im);
}
