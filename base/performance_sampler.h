#ifndef PERFORMANCE_SAMPLER_H
#define PERFORMACE_SAMPLER_H

#include <stdio.h>

#define KB 1024
#define MB KB*KB

struct sampling_data {
	unsigned len;
	long unsigned* samples;
};

int setup_perf_sampler(void);

/// Assumes stop has been performed before
/// Returns 0 on errors
int teardown_perf_sampler(void);

void start_sampling(void);

void stop_sampling(void);

void reset_sampling(void);

void log_samples(FILE* filep, unsigned iteration_number);

#endif /* PERFORMANCE_SAMPLER */
