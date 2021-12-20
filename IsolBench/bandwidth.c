/**
 * @file bandwidth.c
 * @ingroup IsolBench
 * @brief Functions used to run the bandwidth benchmark periodically.
 * @details
 * The original script has been broken down in three components:
 * - init: benchmark_init();
 * - execution: benchmark_execution();
 * - teardown: benchmark_teardown();
 *
 * This allows the benchmark to be run periodically, by re-running only the
 * execution portion.
 *
 * Copyright (C) 2012 @author Heechul Yun <heechul@illinois.edu>
 *               2012 @author Zheng <zpwu@uwaterloo.ca>
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

/* clang -S -mllvm --x86-asm-syntax=intel ./bandwidth.c */

/**************************************************************************
 * Conditional Compilation Options
 **************************************************************************/

/**************************************************************************
 * Included Files
 **************************************************************************/
#define _GNU_SOURCE /* See feature_test_macros(7) */
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

// Libraries used by rt-bench
#include "logging.h"
#include "periodic_benchmark.h"

/**************************************************************************
 * Public Definitions
 **************************************************************************/
#define CACHE_LINE_SIZE 64 /* cache Line size is 64 byte */
#ifdef __arm__
#define DEFAULT_ALLOC_SIZE_KB 4096
#else
#define DEFAULT_ALLOC_SIZE_KB 16384
#endif

/**************************************************************************
 * Public Types
 **************************************************************************/
enum access_type { READ, WRITE };

/**************************************************************************
 * Global Variables
 **************************************************************************/
int g_mem_size = DEFAULT_ALLOC_SIZE_KB * 1024; /* memory size */
int *g_mem_ptr = 0; /* pointer to allocated memory region */

volatile uint64_t g_nread = 0; /* number of bytes read */
volatile unsigned int g_start; /* starting time */
int cpuid = 0;
/// Memory access type
int acc_type = READ;
/// Number of iterations
int iterations = 0;

/**************************************************************************
 * Public Functions
 **************************************************************************/
unsigned int get_usecs() {
  struct timeval time;
  gettimeofday(&time, NULL);
  return (time.tv_sec * 1000000 + time.tv_usec);
}

void quit(int param) {
  float dur_in_sec;
  float bw;
  float dur = get_usecs() - g_start;
  dur_in_sec = (float)dur / 1000000;
  printf("g_nread(bytes read) = %lld\n", (long long)g_nread);
  printf("elapsed = %.2f sec ( %.0f usec )\n", dur_in_sec, dur);
  bw = (float)g_nread / dur_in_sec / 1024 / 1024;
  printf("CPU%d: B/W = %.2f MB/s | ", cpuid, bw);
  printf("CPU%d: average = %.2f ns\n", cpuid,
         (dur * 1000) / (g_nread / CACHE_LINE_SIZE));
}

int64_t bench_read() {
  int i;
  int64_t sum = 0;
  for (i = 0; i < g_mem_size / 4; i += (CACHE_LINE_SIZE / 4)) {
    sum += g_mem_ptr[i];
  }
  g_nread += g_mem_size;
  return sum;
}

int bench_write() {
  register int i;
  for (i = 0; i < g_mem_size / 4; i += (CACHE_LINE_SIZE / 4)) {
    g_mem_ptr[i] = i;
  }
  g_nread += g_mem_size;
  return 1;
}

void usage(int argc, char *argv[]) {
  printf("Usage: $ %s [<option>]*\n\n", argv[0]);
  printf("-m: memory size in KB. deafult=8192\n");
  printf("-a: access type - read, write. default=read\n");
  printf("-t: time to run in sec. 0 means indefinite. default=5. \n");
  printf("-i: iterations. 0 means intefinite. default=0\n");
  printf("-h: help\n");
  printf("\nExamples: \n$ bandwidth -m 8192 -a read -t 1  <- 8MB read "
         "for 1 second\n");
  exit(1);
}

/**
 * @brief Will interpret the benchmark parameters and initialize the testbed.
 * @param[in] parameters_num Number of parameters passed, should be 1.
 * @param[in] parameters The list of passed parameters.
 * @details
 * The required parameters array is documented in `usage()`, and can be brought
 * up by havin "-h" in `parameters`.
 * @returns `0` on success, `-1` on error, setting errno.
 */
int benchmark_init(int parameters_num, void **parameters) {
  unsigned finish = 5;
  int prio = 0;
  int num_processors;
  int opt;
  cpu_set_t cmask;
  int i;
  struct sched_param param;

  /*
   * get command line options
   */
  while ((opt = getopt(parameters_num, (char *const *)parameters,
                       "m:a:t:i:h")) != -1) {
    switch (opt) {
    case 'm': /* set memory size */
      g_mem_size = 1024 * strtol(optarg, NULL, 0);
      break;
    case 'a': /* set access type */
      if (!strcmp(optarg, "read"))
        acc_type = READ;
      else if (!strcmp(optarg, "write"))
        acc_type = WRITE;
      else
        exit(1);
      break;
    case 't': /* set time in secs to run */
      finish = strtol(optarg, NULL, 0);
      break;
    case 'i': /* iterations */
      iterations = strtol(optarg, NULL, 0);
      break;
    case 'h':
      usage(parameters_num, (char **)parameters);
      break;
    }
  }

  /*
   * allocate contiguous region of memory
   */
  g_mem_ptr = (int *)malloc(g_mem_size);

  memset((char *)g_mem_ptr, 1, g_mem_size);

  for (i = 0; i < g_mem_size / sizeof(int); i++)
    g_mem_ptr[i] = i;

  /* print experiment info before starting */
  printf("memsize=%d KB, type=%s, cpuid=%d\n", g_mem_size / 1024,
         ((acc_type == READ) ? "read" : "write"), cpuid);
  printf("stop at %d\n", finish);

  /* set signals to terminate once time has been reached */
  signal(SIGINT, &quit);
  if (finish > 0) {
    signal(SIGALRM, &quit);
    alarm(finish);
  }
}

/**
 * @brief This handler is where the memory bandwidth will be computed.
 * @param[in] parameters_num Number of passed parameters, ignored.
 * @param[in] parameters The list of passed parameters, ignored.
 * @details
 * This function will make several memory accesses (defined by `::iterations`,
 * then it will call `quit()` to compute and report the bandwidth.
 */
void benchmark_execution(int parameters_num, void **parameters) {
  int64_t sum = 0;
  int i = 0;
  /*
   * actual memory access
   */
  g_start = get_usecs();
  for (i = 0;; i++) {
    switch (acc_type) {
    case READ:
      sum += bench_read();
      break;
    case WRITE:
      sum += bench_write();
      break;
    }

    if (iterations > 0 && i + 1 >= iterations)
      break;
  }
  printf("total sum = %ld\n", (long)sum);
  quit(0);
}

/**
 * @brief Will revert what `benchmark_init()` has done to initialize the
 * benchmark.
 * @param[in] parameters_num Ignored.
 * @param[in] parameters Ignored.
 * @details It will free `::g_mem_ptr`.
 */
void benchmark_teardown(int parameters_num, void **parameters) {
  free(g_mem_ptr);
}
