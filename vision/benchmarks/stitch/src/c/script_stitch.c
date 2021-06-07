/**
 * @file script_stitch.c
 * @ingroup stitch
 * @brief Functions used to run the stitch benchmark periodically.
 * @details
 * The original script has been broken down in three components:
 * - init: benchmark_init();
 * - execution: benchmark_execution();
 * - teardown: benchmark_teardown();
 *
 * @bug This benchmark fails also in the original version at the moment.
 *
 * This allows the benchmark to be run periodically, by re-running only the execution portion.
 * @author Sravanthi Kota Venkata, for the original version.
 */

#include "stitch.h"

///Image used by the benchmark.
static I2D *Icur;

/**
 * @brief Will load the image that will be used by the benchmark.
 * @param[in] parameters_num Number of parameters passed, should be 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The required parameters array has the following structure:
 * - parameters[0]: image folder path;
 * Image will be loaded into `::Icur`.
 */
int benchmark_init(int parameters_num, void **parameters)
{
	char im1[100];

	if (parameters_num < 1) {
		elogf(LOG_LEVEL_ERR, "We need input image path\n");
		return -1;
	}

	sprintf(im1, "%s/1.bmp", parameters[0]);

	Icur = readImage(im1);

	elogf(LOG_LEVEL_TRACE, "Input size\t\t- (%dx%d)\n", Icur->height,
	      Icur->width);
	return 0;
}

/**
 * @brief This handler is where the texture stitching is performed.
 * @param[in] parameters_num Number of passed parameters, should be 0 or 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The list of passed parameters must provide the output folder path (which is generally the same as the input folder path) as only element of the parameters array if self checking is enabled.
 */
void benchmark_execution(int parameters_num, void **parameters)
{
	F2D *x, *y, *v, *interestPnts, *Fcur, *int1, *int2;
	int i, j;
	v = harris(Icur);
	interestPnts = getANMS(v, 24);

	int1 = fMallocHandle(interestPnts->height, 1);
	int2 = fSetArray(interestPnts->height, 1, 0);

	for (i = 0; i < int1->height; i++) {
		asubsref(int1, i) = subsref(interestPnts, i, 0);
		asubsref(int2, i) = subsref(interestPnts, i, 1);
	}

	Fcur = extractFeatures(Icur, int1, int2);

#ifdef CHECK
	/* Self checking - use expected.txt from data directory  */
	{
		if (parameters_num < 1) {
			elogf(LOG_LEVEL_ERR, "Missing input image path");
			exit(-1);
		}
		int ret = 0;
		float tol = 0.02;
#ifdef GENERATE_OUTPUT
		fWriteMatrix(Fcur, parameters[0]);
#endif
		ret = fSelfCheck(Fcur, parameters[0], tol);
		if (ret == -1)
			printf("Error in Stitch\n");
	}
	/* Self checking done */
#endif

	fFreeHandle(v);
	fFreeHandle(interestPnts);
	fFreeHandle(int1);
	fFreeHandle(int2);
	fFreeHandle(Fcur);
}

/**
 * @brief Will revert what `benchmark_init()` has done to initialize the benchmark.
 * @param[in] parameters_num Ignored.
 * @param[in] parameters Ignored.
 * @details It will deallocate image in `::Icur`.
 */
void benchmark_teardown(int parameters_num, void **parameters)
{
	iFreeHandle(Icur);
}
