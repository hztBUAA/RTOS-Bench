/**
 * @file overhead.c
 * @ingroup overhead
 * @brief Benchmark to measure the RT-Bench framework overhead.
 * @author Mattia Nicolella
 * @details
 * The bnchmark is composed by three dummy function, that only have a return statement.
 * This allows to measure the time used by the framework in each period.
 *
 * @copyright (C) 2021 - 2022, Mattia Nicolella <mnico@bu.edu> and the rt-bench contributors.
 * SPDX-License-Identifier: MIT
 */
// Libraries used by rt-bench
#include "logging.h"
#include "periodic_benchmark.h"

/**
 * @brief No computation is done.
 * @param[in] parameters_num Ignored.
 * @param[in] parameters Ignored.
 */
int benchmark_init(int parameters_num, void **parameters)
{
	return 0;
}

/**
 * @brief No computation is done.
 * @param[in] parameters_num Ignored.
 * @param[in] parameters Ignored.
 */
void benchmark_execution(int parameters_num, void **parameters)
{
	return;
}

/**
 * @brief No computation is done.
 * @param[in] parameters_num Ignored.
 * @param[in] parameters Ignored.
 */
void benchmark_teardown(int parameters_num, void **parameters)
{
	return;
}
