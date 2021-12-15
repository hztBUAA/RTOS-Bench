#define _GNU_SOURCE

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


static pthread_t sampler_thread;
static pthread_attr_t attr;
static struct sched_param params;

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


int setup_perf_sampler(unsigned iterations, cpu_set_t core_affinity, long unsigned input_time_bucket)
{
	sampling_alive = 1;
	time_bucket.tv_sec = 0;
	time_bucket.tv_nsec = input_time_bucket;
	// setup sampling struct
	sampling_counter = 0;
	sampling_data = (struct sampling_data*)malloc(iterations*sizeof(struct sampling_data));
	for (unsigned i = 0; i < iterations; i++) {
		sampling_data[i].len = 0;
		sampling_data[i].samples = (long unsigned*)malloc(MINUTES/input_time_bucket*sizeof(long unsigned));
	}
	// pthread_attr
	int res = pthread_attr_init(&attr);
	if (res != 0) {
		return res;
	}
	res = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if (res != 0) {
		return res;
	}
	res = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (res != 0) {
		return res;
	}
	res = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	if (res != 0) {
		return res;
	}
	params.sched_priority = 51;
	res = pthread_attr_setschedparam(&attr, &params);
	if (res != 0) {
		return res;
	}
	// If no core specified (i.e., core affinity == 0), then inherit from parent thread (i.e., skip setaffinity)
	if (CPU_COUNT(&core_affinity) > 0) {
		res = pthread_attr_setaffinity_np(&attr, sizeof(core_affinity), &core_affinity);
		if (res != 0) {
			return res;
		}
	}
	// Start thread
	res = pthread_create(&sampler_thread, &attr, sampling, NULL);
	return res;

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
