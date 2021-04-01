#ifndef GET_TIMING_H
#define GET_TIMING_H

/** @file get_cpu_timestamp.h
 * @brief API to get the CPU timestamp value.
 */

/** @brief Reads the processor timestamp counter as an unsigned long long.
 * @return Processor timestamp counter value (in clock cycles) on success, 0 on error.
*/
unsigned long long get_cpu_timestamp();

#endif
