/**
 * @file latency.c
 * @ingroup latency
 * @brief RT-Bench-compatible latency benchmark.
 * @author Heechul Yun <heechul@illinois.edu>
 * @copyright (C) 2012 This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 * @details
 * The original script has been broken down in three components:
 * - init: benchmark_init();
 * - execution: benchmark_execution();
 * - teardown: benchmark_teardown();
 *
 * This allows the benchmark to be run periodically, by re-running only the
 * execution portion.
 */

/**************************************************************************
 * Conditional Compilation Options
 **************************************************************************/

/**************************************************************************
 * Included Files
 **************************************************************************/
#define _GNU_SOURCE /* See feature_test_macros(7) */
#include "list.h"
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// Libraries used by rt-bench
#include "logging.h"
#include "periodic_benchmark.h"
#include <string.h>
/**************************************************************************
 * Public Definitions
 **************************************************************************/
#define CACHE_LINE_SIZE 64
#ifdef __arm__
#define DEFAULT_ALLOC_SIZE_KB 4096
#else
#define DEFAULT_ALLOC_SIZE_KB 16384
#endif
#define DEFAULT_ITER 100

/**************************************************************************
 * Public Types
 **************************************************************************/
/// Linked list entry
struct item {
	int data; ///< Data to read/write.
	int in_use; /// If the data is in use.
	struct list_head list; /// List where the item belongs.
} __attribute__((aligned(CACHE_LINE_SIZE)));
;

/**************************************************************************
 * Global Variables
 **************************************************************************/
/// size of the used memory.
int g_mem_size = DEFAULT_ALLOC_SIZE_KB * 1024;
/// List that will be traversed.
struct item *list = NULL;
/// List head.
struct list_head head;
/// Timers to compute the latency.
struct timespec start, end;
/// Number of iterations.
int repeat = DEFAULT_ITER;
/// Working set size
int workingset_size = 1024;
/// Output log file
FILE *bmark_output = NULL;
/// Sum of the amount of read data.
uint64_t readsum = 0;
/// Average measured latency
double avglat;
/**************************************************************************
 * Public Function Prototypes
 **************************************************************************/

/**************************************************************************
 * Implementation
 **************************************************************************/
/** @brief Compute elapsed time.
 * @param[in] start Timestamp when the benchmark started.
 * @param[in] end Timestamp when the benchmark stopped.
 * @returns Elapsed time in nanoseconds
 * */
uint64_t get_elapsed(struct timespec *start, struct timespec *end)
{
	uint64_t dur;

	dur = ((uint64_t)end->tv_sec * 1000000000 + end->tv_nsec) -
	      ((uint64_t)start->tv_sec * 1000000000 + start->tv_nsec);
	return dur;
}

/// @brief Print usage info.
void usage(int argc, char *argv[])
{
	printf("Usage: $ %s [<option>]*\n\n", argv[0]);
	printf("-m: memory size in KB. deafult=%d\n", DEFAULT_ALLOC_SIZE_KB);
	printf("-s: turn on the serial access mode\n");
	printf("-i: iterations. default=%d\n", DEFAULT_ITER);
	printf("-h: help\n");
	exit(1);
}

/**
 * @brief This handler returns the latency as the extra measurement metric.
 * @details
 * This function returns a string mentioning the metric of the benchmark
 * (here, the latency in ns). This function is only called if the benchmark has
 * been built with the `-DEXTENDED_REPORT`.
 * @returns a constant string starting with `,` that extends the report header.
 */
const char *benchmark_log_header()
{
	return ",latency(ns)";
}

/**
 * @brief Will interpret the benchmark parameters and initialize the testbed.
 * @param[in] parameters_num Number of parameters passed, should be 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The required parameters array is documented in `usage()`, and can be brought
 * up by having "-h" in `parameters`.
 * @returns `0` on success, `-1` on error, setting errno.
 */
int benchmark_init(int parameters_num, void **parameters)
{
	int i;
	int opt;
	int serial = 0;
	/*
   * get command line options
   */
	// adjust parameters list to have a dummy argument at position 0 (to fool
	// getopt)
	int opt_num = parameters_num + 1;
	char **opts = malloc(sizeof(char *) * opt_num);
	opts[0] = "latency";
	memcpy(opts + 1, parameters, sizeof(char *) * parameters_num);
	while ((opt = getopt(opt_num, opts, "m:si:h")) != -1) {
		printf("opt:%d\n", opt);
		switch (opt) {
		case 'm': /* set memory size */
			g_mem_size = 1024 * strtol(optarg, NULL, 0);
			break;
		case 's': /* set access type */
			serial = 1;
			break;
		case 'i': /* iterations */
			repeat = strtol(optarg, NULL, 0);
			break;
		case 'h':
			usage(opt_num, opts);
			break;
		}
	}
	bmark_output = open_log_file("latency.log");
	if (bmark_output == NULL) {
		return -1;
	}
	flogf(LOG_LEVEL_FILE, bmark_output, "repeat=%d\n", repeat);
	free(opts);
	workingset_size = g_mem_size / CACHE_LINE_SIZE;
	srand(0);
	INIT_LIST_HEAD(&head);

	/* allocate */
	list = (struct item *)malloc(sizeof(struct item) * workingset_size +
				     CACHE_LINE_SIZE);
	for (i = 0; i < workingset_size; i++) {
		list[i].data = i;
		list[i].in_use = 0;
		INIT_LIST_HEAD(&list[i].list);
		// printf("%d 0x%x\n", list[i].data, &list[i].data);
	}
	flogf(LOG_LEVEL_FILE, bmark_output,
	      "allocated: wokingsetsize=%d entries\n", workingset_size);

	/* initialize */

	int *perm = (int *)malloc(workingset_size * sizeof(int));
	for (i = 0; i < workingset_size; i++)
		perm[i] = i;

	if (!serial) {
		for (i = 0; i < workingset_size; i++) {
			int tmp = perm[i];
			int next = rand() % workingset_size;
			perm[i] = perm[next];
			perm[next] = tmp;
		}
	}
	for (i = 0; i < workingset_size; i++) {
		list_add(&list[perm[i]].list, &head);
		// printf("%d\n", perm[i]);
	}
	elogf(LOG_LEVEL_TRACE, "initialized.\n");
	return 0;
}

/**
 * @brief This handler is where the memory latency will be computed.
 * @param[in] parameters_num Number of passed parameters, ignored.
 * @param[in] parameters The list of passed parameters, ignored.
 * @details
 * This function will make several memory accesses (defined by `::repeat`) and
 * for each access a list will be navigated. Then it will compute and report the
 * latency.
 */
void benchmark_execution(int parameters_num, void **parameters)
{
	uint64_t nsdiff;
	int j;
	struct list_head *pos;
	int i;
	/* actual access */
	clock_gettime(CLOCK_REALTIME, &start);
	for (j = 0; j < repeat; j++) {
		pos = (&head)->next;
		for (i = 0; i < workingset_size; i++) {
			struct item *tmp = list_entry(pos, struct item, list);
			readsum += tmp->data; // READ
			pos = pos->next;
		}
	}
	clock_gettime(CLOCK_REALTIME, &end);

	nsdiff = get_elapsed(&start, &end);
	avglat = (double)nsdiff / workingset_size / repeat;
}

/**
 * @brief This handler returns the latency as the extra measurement.
 * @details
 * This function returns the experienced latency (ns) as a float after
 * each execution phase. This function is only called if the benchamrk has
 * been built with the `-DEXTENDED_REPORT`.
 * @returns The measured latency (ns)
 */
float benchmark_log_data()
{
	return (float)avglat;
}

/**
 * @brief Will revert what `benchmark_init()` has done to initialize the
 * benchmark.
 * @param[in] parameters_num Ignored.
 * @param[in] parameters Ignored.
 * @details It will free `::list`.
 */
void benchmark_teardown(int parameters_num, void **parameters)
{
	uint64_t nsdiff;
	if (repeat == 0) {
		clock_gettime(CLOCK_REALTIME, &end);

		nsdiff = get_elapsed(&start, &end);
		avglat = (double)nsdiff / workingset_size / repeat;
		flogf(LOG_LEVEL_FILE, bmark_output,
		      "duration %.0f us\naverage %.2f ns | ",
		      (double)nsdiff / 1000, avglat);
		flogf(LOG_LEVEL_FILE, bmark_output,
		      "bandwidth %.2f MB (%.2f MiB)/s\n",
		      (double)64 * 1000 / avglat,
		      (double)64 * 1000000000 / avglat / 1024 / 1024);
		flogf(LOG_LEVEL_FILE, bmark_output, "readsum  %lld\n\n",
		      (unsigned long long)readsum);
	}
	close_log_file(bmark_output);
	free(list);
}
