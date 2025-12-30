/** @file performance_sampler.h
 * @ingroup generator
 * @brief Functions exported by the performance sampler.
 * @author Denis Hoornaert
 *
 * @copyright (C) 2021 - 2022, Denis Hoornaert <denis.hoornaert@tum.de> and the rt-bench contributors.
 * SPDX-License-Identifier: MIT
 */

#ifndef PERFORMANCE_SAMPLER_H
#define PERFORMANCE_SAMPLER_H

#include <sched.h>
#include <stdio.h>
#include "performance_counters.h"

/// 1 KB in bytes
#define KB 1024
/// 1 MB in bytes
#define MB KB*KB

///Struct used to hold data sampled from the counters.
struct sampling_data {
	long unsigned samples; ///< The amount of measurements recorded.
	struct perf_counters sum; ///< Sum of measurement recorded.
};

int setup_perf_sampler(unsigned iterations, cpu_set_t core_affinity, long unsigned time_bucket);

/// Assumes stop has been performed before
/// Returns 0 on errors
int teardown_perf_sampler(void);

void start_sampling(void);

void stop_sampling(void);

void log_samples(FILE* filep);

#endif /* PERFORMANCE_SAMPLER */
