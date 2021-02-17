/** \file disparity_functions.h
 * Functions used to run the disparity benchmark periodically.
 * Author of the original version: Sravanthi Kota Venkata.
 * Include -lrt to the compilation options.
 */

#include "signal.h"

/** \brief Init function to prepare variables which will store images and their path.
 * \param[in] parameters_num Number of parameters passed, should be 3 because we need to reserve space for the images that will be loaded in the array.
 * \param[in,out] parameters The parameters array, should contain the only images path before this function is called. After this function is executed.
 * \returns 0 on success, -1 with errno set accordingly n case of failure.
 */
int disparity_init(int parameters_num,void** parameters);

/** \brief Benchmark execution.
 * \param[in] parameters_num Number of parameters passed, should be 3.
 * \param[in] parameters The paramters array with the images path and the I2D* that represent the left and right image.
 */
void disparity_execution(int parameters_num, void** parameters);

/** \brief Cleanup of the environment.
 * \param[in] parameters_num Number of parameters passed, should be 3.
 * \param[in] parameters The parameters array,  with the images path and the I2D* that represent the left and right image.
 */
void disparity_teardown(int parameters_num,void** parameters);
