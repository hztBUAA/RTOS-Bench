#include "performance_sampler.h"
#include "performance_counters.h"
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>


#define NANOSECONDS  (1UL)
#define MICROSECONDS (1000*NANOSECONDS)
#define MILLISECONDS (1000*MICROSECONDS)
#define SECONDS      (1000*MILLISECONDS)
#define MINUTES      (60*SECONDS)

#define TIME_BUCKET  (10*MILLISECONDS)

static pthread_t sampler_thread;


static sem_t sampler_sync;


static unsigned sampling_alive = 0;

static unsigned sampling_active = 0;

static struct timespec time_bucket;
static struct timespec rem;


static unsigned long first_l2_refills_sample;
static unsigned sampling_counter;
static struct sampling_data* sampling_data;


static void* sampling(void* dummy)
{
	while (sampling_alive) {
		if (sampling_active) {
			if (sampling_data[sampling_counter].len == 0) {
				first_l2_refills_sample = pmcs_get_value().l2_refills;
				sampling_data[sampling_counter].samples[sampling_data[sampling_counter].len] = 0;
			}
			else {
				sampling_data[sampling_counter].samples[sampling_data[sampling_counter].len] = pmcs_get_value().l2_refills-first_l2_refills_sample;
			}
			sampling_data[sampling_counter].len++;
		}
		nanosleep(&time_bucket, &rem);
	}
}


int setup_perf_sampler(unsigned iterations)
{
	sampling_alive = 1;
	time_bucket.tv_sec = 0;
	time_bucket.tv_nsec = TIME_BUCKET;
	// setup sampling struct
	sampling_counter = 0;
	sampling_data = (struct sampling_data*)malloc(iterations*sizeof(struct sampling_data));
	for (unsigned i = 0; i < iterations; i++) {
		sampling_data[i].len = 0;
		sampling_data[i].samples = (long unsigned*)malloc(MINUTES/TIME_BUCKET*sizeof(long unsigned));
	}
	// pthread_attr
	// Start thread
	int ret = pthread_create(&sampler_thread, NULL, sampling, NULL);
	return ret;

}

/// Assumes stop has been performed before
/// Returns 0 on success
int teardown_perf_sampler(void)
{
	sampling_alive = 0;
	int res = pthread_join(sampler_thread, NULL);
	for (unsigned i = 0; i < sampling_counter; i++) {
		free(sampling_data[i].samples);
	}
	return res;
}

void start_sampling(void)
{
	sampling_active = 1;
}

void stop_sampling(void)
{
	sampling_active = 0;
	sampling_counter++;
}

void log_samples(FILE* filep)
{
	for (unsigned i = 0; i < sampling_counter; i++) {
		for (int j = 0; j < sampling_data[i].len; j++) {
			fprintf(filep, "%lu,", sampling_data[i].samples[j]);
		}
		fprintf(filep, "\n");
	}
}
