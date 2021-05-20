/**
 * @file script_svm.c
 * @brief Functions used to run the svm benchmark periodically.
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
#include "svm.h"

/// First training image
F2D *trn1 = NULL;

/// Second training image
F2D *trn2 = NULL;

/// First test image
F2D *tst1 = NULL;

/// Second test image
F2D *tst2 = NULL;

/**
 * @brief Will load the images that will be used by the benchmark.
 * @param[in] parameters_num Number of parameters passed, should be 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The required parameters array has the following structure:
 * - parameters[0]: image folder path;
 * Input images for training SVM will be loaded into `::trn1` and `::trn2`, while test images will be loaded into `::tst1` and `::tst2`.
 */
int benchmark_init(int parameters_num, void **parameters)
{
	char im1[256];

	if (parameters_num < 1) {
		elogf(LOG_LEVEL_ERR, "We need input image path\n");
		return -1;
	}

	sprintf(im1, "%s/d16trn_1.txt", parameters[0]);
	trn1 = readFile(im1);

	sprintf(im1, "%s/d16trn_2.txt", parameters[0]);
	trn2 = readFile(im1);

	sprintf(im1, "%s/d16tst_1.txt", parameters[0]);
	tst1 = readFile(im1);

	sprintf(im1, "%s/d16tst_2.txt", parameters[0]);
	tst2 = readFile(im1);
}

/**
 * @brief This handler is where the  support vector machines are trained and tested.
 * @param[in] parameters_num Number of passed parameters, should be 0 or 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The list of passed parameters must provide the output folder path (which is generally the same as the input folder path) as only element of the parameters array.
 */
void benchmark_execution(int parameters_num, void **parameters)
{
	int iter, N, Ntst, i, j, k, n;
	F2D *Yoffset;
	alphaRet *alpha;
	F2D *a_result, *result;
	F2D *s;
	F2D *b_result;
	F2D *Xtst, *Ytst;
	int dim = 256;

	N = 100;
	Ntst = 100;
	iter = 10;
#ifdef test
	N = 4;
	Ntst = 4;
	iter = 2;
#endif

#ifdef sim_fast
	N = 20;
	Ntst = 20;
	iter = 2;
#endif

#ifdef sim
	N = 16;
	Ntst = 16;
	iter = 8;
#endif

#ifdef sqcif
	N = 60;
	Ntst = 60;
	iter = 6;
#endif

#ifdef qcif
	N = 72;
	Ntst = 72;
	iter = 8;
#endif

#ifdef vga
	N = 450;
	Ntst = 450;
	iter = 15;
#endif

#ifdef wuxga
	N = 1000;
	Ntst = 1000;
	iter = 20;
#endif
#ifdef CHECK
	if (parameters_num < 1) {
		elogf(LOG_LEVEL_ERR, "Missing output folder path.");
	}
#endif
	elogf(LOG_LEVEL_TRACE, "Input size\t\t- (%dx%dx%d)\n", N, Ntst, iter);

	alpha = getAlphaFromTrainSet(N, trn1, trn2, iter);
	a_result = alpha->a_result;
	b_result = alpha->b_result;
	Yoffset = fSetArray(iter, N, 0);

	Xtst = usps_read_partial(tst1, tst2, -1, 1, Ntst / iter, iter);
	Ytst = usps_read_partial(tst1, tst2, -1, 0, Ntst / iter, iter);

	for (i = 0; i < iter; i++) {
		F2D *temp;
		temp = usps_read_partial(trn1, trn2, i, 0, N / iter, iter);
		for (j = 0; j < N; j++)
			subsref(Yoffset, i, j) = asubsref(temp, j);
		fFreeHandle(temp);
	}

	result = fSetArray(Ntst, 1, 0);
	for (n = 0; n < Ntst; n++) {
		float maxs = 0;
		s = fSetArray(iter, 1, 0);
		for (i = 0; i < iter; i++) {
			for (j = 0; j < N; j++) {
				if (subsref(a_result, i, j) > 0) {
					F2D *Xtemp, *XtstTemp, *X;
					X = alpha->X;
					Xtemp = fDeepCopyRange(X, j, 1, 0,
							       X->width);
					XtstTemp = fDeepCopyRange(Xtst, n, 1, 0,
								  Xtst->width);
					asubsref(s, i) =
						asubsref(s, i) +
						subsref(a_result, i, j) *
							subsref(Yoffset, i, j) *
							polynomial(3, Xtemp,
								   XtstTemp,
								   dim);
					fFreeHandle(Xtemp);
					fFreeHandle(XtstTemp);
				}
			}
			asubsref(s, i) = asubsref(s, i) - asubsref(b_result, i);
			if (asubsref(s, i) > maxs)
				maxs = asubsref(s, i);
		}

		fFreeHandle(s);
		asubsref(result, n) = maxs;
	}

#ifdef CHECK
	/* Self checking - use expected.txt from data directory  **/
	{
		int ret = 0;
		float tol = 0.5;
#ifdef GENERATE_OUTPUT
		fWriteMatrix(result, parameters[0]);
#endif
		ret = fSelfCheck(result, parameters[0], tol);
		if (ret == -1)
			elogf(LOG_LEVEL_ERR, "Error in SVM\n");
	}
	/* Self checking done **/
#endif
	fFreeHandle(Yoffset);
	fFreeHandle(result);
	fFreeHandle(alpha->a_result);
	fFreeHandle(alpha->b_result);
	fFreeHandle(alpha->X);
	free(alpha);
	fFreeHandle(Xtst);
	fFreeHandle(Ytst);
}

/**
 * @brief Will revert what `benchmark_init()` has done to initialize the benchmark.
 * @param[in] parameters_num Ignored.
 * @param[in] parameters Ignored.
 * @details It will deallocate images in `::trn1`, `::trn2`, `::tst1` and `:tst2`.
 */
void benchmark_teardown(int parameters_num, void **parameters)
{
	fFreeHandle(trn1);
	fFreeHandle(tst1);
	fFreeHandle(trn2);
	fFreeHandle(tst2);
}
