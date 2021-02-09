/** \file disparity_functions.h
 * Functions used to run the disparity benchmark periodically.
 * Author of the original version: Sravanthi Kota Venkata.
 * Include -lrt to the compilation options.
 */

#include "signal.h"

/** \brief Init function to prepare variables which will store images and their path.
 * \param[in] parameters_num Number of parameters passed, should be 1.
 * \param[in] parameters The parameters array, should contain only the images path.
 * \returns 0 on success, -1 with errno set accordingly n case of failure.
 */
int disparity_init(int parameters_num,void** parameters);

/// \brief Benchmark execution, as a signal handler.
void disparity_execution(int signo, siginfo_t* info,void* context);

/** \brief Cleanup of the environment.
 * \param[in] parameters_num Number of parameters passed, ignored.
 * \param[in] parameters The parameters array, ignored.
 */
void disparity_teardown(int parameters_num,void** parameters);
