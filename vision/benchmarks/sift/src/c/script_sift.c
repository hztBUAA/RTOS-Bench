/** @file script_sift.c
 * @ingroup SIFT
 * @brief Functions used to run the sift benchmark periodically.
 * @details
 * The original script has been broken down in three components:
 * - init: benchmark_init();
 * - execution: benchmark_execution();
 * - teardown: benchmark_teardown();
 *
 * This allows the benchmark to be run periodically, by re-running only the execution portion.
 * @author Sravanthi Kota Venkata, for the original version
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "sift.h"

/// The input image.
static I2D *im;

/** @brief Normalizes the input image.
 * @param[in,out] image The image that will be normalized.
 * @details
 * Will normalize the image to have values between 0 and 1.
 */
void normalizeImage(F2D *image)
{
	int i;
	int rows;
	int cols;
	int tempMin = 10000, tempMax = -1;
	rows = image->height;
	cols = image->width;

	for (i = 0; i < (rows * cols); i++)
		if (tempMin > asubsref(image, i))
			tempMin = asubsref(image, i);

	for (i = 0; i < (rows * cols); i++)
		asubsref(image, i) = asubsref(image, i) - tempMin;

	for (i = 0; i < (rows * cols); i++)
		if (tempMax < asubsref(image, i))
			tempMax = asubsref(image, i);

	for (i = 0; i < (rows * cols); i++)
		asubsref(image, i) = (asubsref(image, i) / (tempMax + 0.0));
}

/**
 * @brief Will load the image that will be used by the benchmark.
 * @param[in] parameters_num Number of parameters passed, should be 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The required parameters array has the following structure:
 * - parameters[0]: image folder path;
 * First image will be loaded into `::im`.
 * @returns `0` on success, `-1` on error, setting errno.
 */
int benchmark_init(int parameters_num, void **parameters)
{
	char imSrc[100];

	if (parameters_num < 1) {
		elogf(LOG_LEVEL_ERR, "Missing input image path\n");
		errno = EINVAL;
		return -1;
	}

	sprintf(imSrc, "%s/1.bmp", parameters[0]);

	im = readImage(imSrc);
	return 0;
}

/**
 * @brief This handler is where the image normalization and feature extraction will be performed.
 * @param[in] parameters_num Number of passed parameters, should be 0 or 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The list of passed parameters must provide the output folder path (which is generally the same as the input folder path) as only element of the parameters array when self checking is enabled.
 */
void benchmark_execution(int parameters_num, void **parameters)
{
	F2D *frames, *image;
	int rows, cols;
#ifdef CHECK
	if (parameters_num < 1) {
		elogf(LOG_LEVEL_ERR, "Missing output folder path.");
		errno = EINVAL;
		exit(EXIT_FAILURE);
	}
#endif
	image = fiDeepCopy(im);
	rows = image->height;
	cols = image->width;
	/** Normalize the input image to lie between 0-1 **/
	normalizeImage(image);
	/** Extract sift features for the normalized image **/
	frames = sift(image);

	elogf(LOG_LEVEL_TRACE, "Input size\t\t- (%dx%d)\n", rows, cols);

#ifdef CHECK
	{
		int ret = 0;
		float tol = 0.2;
#ifdef GENERATE_OUTPUT
		fWriteMatrix(frames, parameters[0]);
#endif
		ret = fSelfCheck(frames, parameters[0], tol);
		if (ret == -1)
			elogf(LOG_LEVEL_ERR, "Error in SIFT\n");
	}
#endif
	if (frames != NULL) {
		fFreeHandle(frames);
	}
}

/**
 * @brief Will revert what `benchmark_init()` has done to initialize the benchmark.
 * @param[in] parameters_num Ignored.
 * @param[in] parameters Ignored.
 * @details It will deallocate image in `::im`.
 */
void benchmark_teardown(int parameters_num, void **parameters)
{
	if (im != NULL) {
		iFreeHandle(im);
	}
}
