/**
 * @file script_disparity.c
 * @ingroup disparity
 * @brief Functions used to run the disparity benchmark periodically.
 * @details
 * The original script has been broken down in three components:
 * - init: benchmark_init();
 * - execution: benchmark_execution();
 * - teardown: benchmark_teardown();
 *
 * This allows the benchmark to be run periodically, by re-running only the
 * execution portion.
 * @author Sravanthi Kota Venkata, for the original version
 */

#include "disparity.h"
#include "logging.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

/// Left image, used for disparity computation.
static I2D *imleft = NULL;
/// Right image, used for disparity computation.
static I2D *imright = NULL;

/**
 * @brief Will load the images that will be used by the benchmark.
 * @param[in] parameters_num Number of parameters passed, should be 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The required parameters array has the following structure:
 * - parameters[0]: image folder path;
 * First image will be loaded into `::imleft` and second image into `::imright`.
 * @returns `0` on success, `-1` on error, setting errno.
 */
int benchmark_init(int parameters_num, void **parameters) {
  char im1[100], im2[100];

  if (parameters_num < 1) {
    elogf(LOG_LEVEL_ERR, "Missing input path!\n");
    errno = EINVAL;
    return -1;
  }

  // load images

  sprintf(im1, "%s/1.bmp", (char *)parameters[0]);
  sprintf(im2, "%s/2.bmp", (char *)parameters[0]);

  imleft = readImage(im1);
  imright = readImage(im2);
  return 0;
}

/**
 * @brief This handler is where the disparity between the two images is
 * computed.
 * @param[in] parameters_num Number of passed parameters, should be 0 or 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The list of passed parameters must provide the output folder path (which is
 * generally the same as the input folder path) as only element of the
 * parameters array if self checking is enabled.
 */
void benchmark_execution(int parameters_num, void **parameters) {
  if (parameters_num < 1) {
    elogf(LOG_LEVEL_ERR, "Missing input path!\n");
    errno = EINVAL;
    exit(EXIT_FAILURE);
  }
  char *output = parameters[0];
  int WIN_SZ = 8, SHIFT = 64;
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
  retDisparity = getDisparity(imleft, imright, WIN_SZ, SHIFT);

  elogf(LOG_LEVEL_TRACE, "Input size\t\t- (%dx%d)\n", imleft->height,
        imleft->width);
#ifdef CHECK
  /* Self checking - use expected.txt from data directory  **/
  {
    int tol, ret = 0;
    tol = 2;
#ifdef GENERATE_OUTPUT
    writeMatrix(retDisparity, output);
#endif
    elogf(LOG_LEVEL_TRACE, "output: %s\n", output);
    ret = selfCheck(retDisparity, output, tol);
    if (ret == -1)
      elogf(LOG_LEVEL_ERR, "Error in Disparity Map\n");
  }
/* Self checking done **/
#endif

  // We free the resources allocated.
  iFreeHandle(retDisparity);
}

/**
 * @brief Will revert what `benchmark_init()` has done to initialize the
 * benchmark.
 * @param[in] parameters_num Ignored.
 * @param[in] parameters Ignored.
 * @details It will free images in `::imleft` and `::imright`.
 */
void benchmark_teardown(int parameters_num, void **parameters) {
  if (imleft != NULL) {
    iFreeHandle(imleft);
  }
  if (imright != NULL) {
    iFreeHandle(imright);
  }
}
