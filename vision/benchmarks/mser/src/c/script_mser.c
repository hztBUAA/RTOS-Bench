/**
 * @file script_mser.c
 * @ingroup mser
 * @brief Functions used to run the mser benchmark periodically.
 * @details
 * The original script has been broken down in three components:
 * - init: benchmark_init();
 * - execution: benchmark_execution();
 * - teardown: benchmark_teardown();
 *
 * This allows the benchmark to be run periodically, by re-running only the execution portion.
 * @author Sravanthi Kota Venkata, for the original version.
 */

#include "mser.h"
#include <errno.h>

#define min(a, b) (a < b) ? a : b
#define max(a, b) (a > b) ? a : b

///The image that will be used in the benchmark.
static I2D *It;

///Number of rows of the input image.
static int rows = 196;

///Number of columns of the input image.
static int cols = 98;

/**
 * @brief Will load the image that will be used by the benchmark.
 * @param[in] parameters_num Number of parameters passed, should be 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The required parameters array has the following structure:
 * - parameters[0]: image folder path;
 * Image data will be loaded into `::It`.
 * @returns `0` on success, `-1` on error, setting errno.
 */
int benchmark_init(int parameters_num, void **parameters)
{
	int i, j, k;
	I2D *I;

	char im1[100];

	if (parameters_num < 1) {
		elogf(LOG_LEVEL_ERR, "Missing input image path\n");
		errno = EINVAL;
		return -1;
	}

	sprintf(im1, "%s/1.bmp", parameters[0]);

	I = readImage(im1);
	rows = I->height;
	cols = I->width;

	It = readImage(im1);

	k = 0;
	for (i = 0; i < cols; i++) {
		for (j = 0; j < rows; j++) {
			asubsref(It, k++) = subsref(I, j, i);
		}
	}
	iFreeHandle(I);
	return 0;
}

/**
 * @brief This handler is where the mser benchmark is executed.
 * @param[in] parameters_num Number of passed parameters, should be 0 or 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The list of passed parameters must provide the output folder path as only element of the parameters array if the `CHECK` macro is defined.
 *
 * The benchmark will compute a set of maximally stable regions, which result in the image segmentation.
 */
void benchmark_execution(int parameters_num, void **parameters)
{
	I2D *idx;

#ifdef CHECK
	if (parameters_num < 1) {
		elogf(LOG_LEVEL_ERR, "Missing output path\n");
		errno = EINVAL;
		exit(EXIT_FAILURE);
	}
#endif
	elogf(LOG_LEVEL_TRACE, "Input size\t\t- (%dx%d)\n", rows, cols);
	idx = mser(It, 2);

#ifdef CHECK
	/** Self checking - use expected.txt from data directory  **/
	{
		int tol, ret = 0;
		tol = 1;
#ifdef GENERATE_OUTPUT
		writeMatrix(idx, parameters[0]);
#endif
		ret = selfCheck(idx, parameters[0], tol);
		if (ret == -1)
			elogf(LOG_LEVEL_ERR, "Error in MSER\n");
	}
	/** Self checking done **/
	iFreeHandle(idx);
#endif
}

/**
 * @brief Will revert what benchmark_init() has done to initialize the benchmark.
 * @param[in] parameters_num Ignored.
 * @param[in] parameters Ignored.
 * @details It will free `::It`.
 */
void benchmark_teardown(int parameters_num, void **parameters)
{
	if (It != NULL) {
		iFreeHandle(It);
	}
}
