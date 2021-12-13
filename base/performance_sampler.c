#include "performance_sampler.h"
#include "performance_counters.h"
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>


static pthread_t sampler_thread;


static sem_t sampler_sync;


static unsigned sampling_alive = 0;

static unsigned sampling_active = 0;

static struct timespec time_bucket;
static struct timespec rem;


static struct sampling_data sampling_data;


static void* sampling(void* dummy)
{
	while (sampling_alive) {
		if (sampling_active) {
			sampling_data.len++;
			sampling_data.samples[sampling_data.len] = pmcs_get_value().l2_refills;
		}
		nanosleep(&time_bucket, &rem);
	}
}


int setup_perf_sampler(void)
{
	sampling_alive = 1;
	time_bucket.tv_sec = 0;
	time_bucket.tv_nsec = 10000000;
	// setup sampling struct
	sampling_data.len = 0;
	sampling_data.samples = (long unsigned*)malloc(16*MB);
	// pthread_attr
	// Start thread
	int ret = pthread_create(&sampler_thread, NULL, sampling, NULL);
	return ret;

}

/// Assumes stop has been performed before
/// Returns 0 on errors
int teardown_perf_sampler(void)
{
	sampling_alive = 0;
	return pthread_join(sampler_thread, NULL);
}

void start_sampling(void)
{
	sampling_active = 1;
}

void stop_sampling(void)
{
	sampling_active = 0;
}

void reset_sampling(void)
{
	sampling_data.len = 0;
}

void log_samples(FILE* filep, unsigned iteration_number)
{
	for (int i = 0; i < sampling_data.len; i++) {
		fprintf(filep, "%u, %lu\n", iteration_number, sampling_data.samples[i]);
	}
}
