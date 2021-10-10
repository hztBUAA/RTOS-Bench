/**
 * @file script_tracking.c
 * @ingroup tracking
 * @brief Functions used to run the tracking benchmark periodically.
 * @details
 * The original script has been broken down in three components:
 * - init: benchmark_init();
 * - execution: benchmark_execution();
 * - teardown: benchmark_teardown();
 *
 * This allows the benchmark to be run periodically, by re-running only the execution portion.
 * @author Sravanthi Kota Venkata, for the original version.
 */

#include "tracking.h"
#include <stdlib.h>
#include <errno.h>
/// Number of features considered
static int N_FEA;
/// Size of an image portion.
static int WINSZ;
/// Iterations of the tracking algorithm.
static int LK_ITER;
/// Number of images (base image excluded) that are used by the benchmark.
static int counter;
/// Accuracy of the tracking.
static int accuracy;
/// Array that will store the images needed from the benchmark
static I2D **Ic_arr;

/**
 * @brief Will load the images that will be used by the benchmark.
 * @param[in] parameters_num Number of parameters passed, should be 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The required parameters array has the following structure:
 * - parameters[0]: image folder path;
 * Image will be loaded into `::Ic_arr`.
 * @returns `0` on success, `-1` on error, setting errno.
 */
int benchmark_init(int parameters_num, void **parameters)
{
	char im1[100];
	int i;
	counter = 2;

	int rows, cols;

	if (parameters_num < 1) {
		elogf(LOG_LEVEL_ERR, "Missing input image path\n");
		errno = EINVAL;
		return -1;
	}

	N_FEA = 1600;
	WINSZ = 4;
	LK_ITER = 20;

#ifdef test
	WINSZ = 2;
	N_FEA = 100;
	LK_ITER = 2;
	counter = 2;
	accuracy = 0.1;
#endif
#ifdef sim_fast
	WINSZ = 2;
	N_FEA = 100;
	LK_ITER = 2;
	counter = 4;
#endif
#ifdef sim
	WINSZ = 2;
	N_FEA = 200;
	LK_ITER = 2;
	counter = 4;
#endif
#ifdef sqcif
	WINSZ = 8;
	N_FEA = 500;
	LK_ITER = 15;
	counter = 2;
#endif
#ifdef qcif
	WINSZ = 12;
	N_FEA = 400;
	LK_ITER = 15;
	counter = 4;
#endif
#ifdef cif
	WINSZ = 20;
	N_FEA = 500;
	LK_ITER = 20;
	counter = 4;
#endif
#ifdef vga
	WINSZ = 32;
	N_FEA = 400;
	LK_ITER = 20;
	counter = 4;
#endif
#ifdef wuxga
	WINSZ = 64;
	N_FEA = 500;
	LK_ITER = 20;
	counter = 4;
#endif
#ifdef fullhd
	WINSZ = 48;
	N_FEA = 500;
	LK_ITER = 20;
	counter = 4;
#endif

	/* Read input images **/
	Ic_arr = malloc(sizeof(I2D *) * counter);
	for (i = 0; i < counter; i++) {
		sprintf(im1, "%s/%d.bmp", parameters[0], i + 1);
		Ic_arr[i] = readImage(im1);
	}
	rows = Ic_arr[0]->height;
	cols = Ic_arr[0]->width;

	elogf(LOG_LEVEL_TRACE, "Input size\t\t- (%dx%d)\n", rows, cols);
	return 0;
}

/**
 * @brief This handler is where the tracking of feature between different images is done.
 * @param[in] parameters_num Number of passed parameters, should be 0 or 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The list of passed parameters must provide the output folder path (which is generally the same as the input folder path) as only element of the parameters array if self checking is enabled.
 */
void benchmark_execution(int parameters_num, void **parameters)
{
	int i, j, k, rows, cols;
	int endR, endC;
	F2D *blurredImage, *previousFrameBlurred_level1,
		*previousFrameBlurred_level2, *blurred_level1, *blurred_level2;
	F2D *verticalEdgeImage, *horizontalEdgeImage, *verticalEdge_level1,
		*verticalEdge_level2, *horizontalEdge_level1,
		*horizontalEdge_level2, *interestPnt;
	F2D *lambda, *lambdaTemp, *features;
	I2D *Ic, *status;
	float SUPPRESION_RADIUS;
	F2D *newpoints;

	int numFind, m, n;
	F2D *np_temp;

	accuracy = 0.03;
	int count;
	SUPPRESION_RADIUS = 10.0;

	/* IMAGE PRE-PROCESSING **/
	Ic = iDeepCopy(Ic_arr[0]);

	/* Blur the image to remove noise - weighted average filter **/
	blurredImage = imageBlur(Ic);

	/* Scale down the image to build Image Pyramid. We find features across all scales of the image **/
	blurred_level1 = blurredImage; /* Scale 0 **/
	blurred_level2 = imageResize(blurredImage); /* Scale 1 **/

	/* Edge Images - From pre-processed images, build gradient images, both horizontal and vertical **/
	verticalEdgeImage = calcSobel_dX(blurredImage);
	horizontalEdgeImage = calcSobel_dY(blurredImage);

	/* Edge images are used for feature detection. So, using the verticalEdgeImage and horizontalEdgeImage images, we compute feature strength
        across all pixels. Lambda matrix is the feature strength matrix returned by calcGoodFeature **/

	lambda = calcGoodFeature(verticalEdgeImage, horizontalEdgeImage,
				 verticalEdgeImage->width,
				 verticalEdgeImage->height, WINSZ);
	endR = lambda->height;
	endC = lambda->width;
	lambdaTemp = fReshape(lambda, endR * endC, 1);

	/* We sort the lambda matrix based on the strengths **/
	/* Fill features matrix with top N_FEA features **/
	fFreeHandle(lambdaTemp);
	lambdaTemp = fillFeatures(lambda, N_FEA, WINSZ);
	features = fTranspose(lambdaTemp);

	/* Suppress features that have approximately similar strength and belong to close neighborhood **/
	interestPnt = getANMS(features, SUPPRESION_RADIUS);

	/* Refill interestPnt in features matrix **/
	fFreeHandle(features);
	features = fSetArray(2, interestPnt->height, 0);
	for (i = 0; i < 2; i++) {
		for (j = 0; j < interestPnt->height; j++) {
			subsref(features, i, j) = subsref(interestPnt, j, i);
		}
	}

	fFreeHandle(verticalEdgeImage);
	fFreeHandle(horizontalEdgeImage);
	fFreeHandle(interestPnt);
	fFreeHandle(lambda);
	fFreeHandle(lambdaTemp);
	iFreeHandle(Ic);

	/* Until now, we processed base frame. The following for loop processes other frames **/
	for (count = 0; count < counter; count++) {
		Ic = iDeepCopy(Ic_arr[count]);
		rows = Ic->height;
		cols = Ic->width;

		/* Blur image to remove noise **/
		blurredImage = imageBlur(Ic);
		previousFrameBlurred_level1 = fDeepCopy(blurred_level1);
		previousFrameBlurred_level2 = fDeepCopy(blurred_level2);

		fFreeHandle(blurred_level1);
		fFreeHandle(blurred_level2);

		/* Image pyramid **/
		blurred_level1 = blurredImage;
		blurred_level2 = imageResize(blurredImage);

		/* Gradient image computation, for all scales **/
		verticalEdge_level1 = calcSobel_dX(blurred_level1);
		horizontalEdge_level1 = calcSobel_dY(blurred_level1);

		verticalEdge_level2 = calcSobel_dX(blurred_level2);
		horizontalEdge_level2 = calcSobel_dY(blurred_level2);

		newpoints = fSetArray(2, features->width, 0);

		/* Based on features computed in the previous frame, find correspondence in the current frame. "status" returns the index of corresponding features **/
		status = calcPyrLKTrack(
			previousFrameBlurred_level1,
			previousFrameBlurred_level2, verticalEdge_level1,
			verticalEdge_level2, horizontalEdge_level1,
			horizontalEdge_level2, blurred_level1, blurred_level2,
			features, features->width, WINSZ, accuracy, LK_ITER,
			newpoints);

		fFreeHandle(verticalEdge_level1);
		fFreeHandle(verticalEdge_level2);
		fFreeHandle(horizontalEdge_level1);
		fFreeHandle(horizontalEdge_level2);
		fFreeHandle(previousFrameBlurred_level1);
		fFreeHandle(previousFrameBlurred_level2);

		/* Populate newpoints with features that had correspondence with previous frame features **/
		np_temp = fDeepCopy(newpoints);
		if (status->width > 0) {
			k = 0;
			numFind = 0;
			for (i = 0; i < status->width; i++) {
				if (asubsref(status, i) == 1)
					numFind++;
			}
			fFreeHandle(newpoints);
			newpoints = fSetArray(2, numFind, 0);

			for (i = 0; i < status->width; i++) {
				if (asubsref(status, i) == 1) {
					subsref(newpoints, 0, k) =
						subsref(np_temp, 0, i);
					subsref(newpoints, 1, k++) =
						subsref(np_temp, 1, i);
				}
			}
		}

		iFreeHandle(status);
		iFreeHandle(Ic);
		fFreeHandle(np_temp);
		fFreeHandle(features);
		/* Populate newpoints into features **/
		features = fDeepCopy(newpoints);
		fFreeHandle(newpoints);
	}

#ifdef CHECK
	/* Self checking */
	{
		if (parameters_num < 1) {
			elogf(LOG_LEVEL_ERR, "Missing input folder path");
			errno = EINVAL;
			exit(EXIT_FAILURE);
		}
		int ret = 0;
		float tol = 2.0;
#ifdef GENERATE_OUTPUT
		fWriteMatrix(features, parameters[0]);
#endif
		ret = fSelfCheck(features, parameters[0], tol);
		if (ret == -1)
			elogf(LOG_LEVEL_ERR, "Error in Tracking Map\n");
	}
#endif
	fFreeHandle(blurred_level1);
	fFreeHandle(blurred_level2);
	fFreeHandle(features);
}

/**
 * @brief Will revert what `benchmark_init()` has done to initialize the benchmark.
 * @param[in] parameters_num Ignored.
 * @param[in] parameters Ignored.
 * @details It will deallocate images in `::Ic_arr`.
 */
void benchmark_teardown(int parameters_num, void **parameters)
{
	int i;
	if (Ic_arr != NULL) {
		for (i = 0; i < counter; i++) {
			if (Ic_arr[i] != NULL) {
				iFreeHandle(Ic_arr[i]);
			}
		}
		free(Ic_arr);
	}
}
