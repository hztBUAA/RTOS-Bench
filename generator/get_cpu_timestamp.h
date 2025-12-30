#ifndef GET_TIMING_H
#define GET_TIMING_H

/** @file get_cpu_timestamp.h
 * @ingroup generator
 * @author Mattia Nicolella
 * @brief API to get the CPU timestamp value.
 *
 * @copyright (C) 2021 - 2022, Mattia Nicolella <mnico@bu.edu> and the rt-bench contributors.
 * SPDX-License-Identifier: MIT
 */

/** @brief Reads the processor timestamp counter as an unsigned long long.
 * @return Processor timestamp counter value (in clock cycles) on success, 0 on
 * error.
 */
unsigned long long get_rdtsc();

/** @brief Gets a timestamp in seconds.
 * @returns The timestamp or 0 in case of error.
 */
long double get_timestamp();

// Timing utils which depend on architecture

#ifdef GCC

#ifdef __aarch64__

#define magic_timing_begin(cycleLo, cycleHi)                                   \
  {                                                                            \
    cycleHi = 0;                                                               \
    asm volatile("mrs %0, cntvct_el0" : "=r"(cycleLo));                        \
  }

#elif defined(__arm__)

#define magic_timing_begin(cycleLo, cycleHi)                                   \
  {                                                                            \
    cycleHi = 0;                                                               \
    asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(cycleLo));                \
  }

#else

#define magic_timing_begin(cycleLo, cycleHi)                                   \
  { asm volatile("rdtsc" : "=a"(cycleLo), "=d"(cycleHi)); }

#endif /* _arm_ */

#endif /* GCC */

#ifdef METRO

#define magic_timing_begin(cycleLo, cycleHi)                                   \
  {                                                                            \
    asm volatile("mfsr $8, CYCLE_LO\n\t"                                       \
                 "mfsr $9, CYCLE_HI\n\t"                                       \
                 "addu %0, $8, $0\n\t"                                         \
                 "addu %1, $9, $0\n\t"                                         \
                 : "=r"(cycleLo), "=r"(cycleHi)                                \
                 :                                                             \
                 : "$8", "$9");                                                \
  }

#endif

#ifdef BTL

#include "/u/kvs/raw/rawlib/archlib/include/raw.h"

#define magic_timing_begin(cycleLo, cycleHi)                                   \
  { raw_magic_timing_report_begin(); }

#endif

#endif
