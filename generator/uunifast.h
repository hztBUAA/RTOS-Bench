/**
 * @file uunifast.h
 * @brief UUniFast algorithm for generating task utilization distributions
 * @details Implementation of the UUniFast algorithm from:
 *          Bini & Buttazzo, "Measuring the Performance of Schedulability Tests"
 *          Real-Time Systems, 2005
 */

#ifndef UUNIFAST_H
#define UUNIFAST_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Generate N utilization values that sum to u_total using UUniFast algorithm
 * @param n Number of tasks
 * @param u_total Target total utilization (0.0 to 1.0)
 * @param output_u Output array of size n for generated utilizations
 *
 * The UUniFast algorithm generates uniformly distributed utilizations
 * for n tasks that sum to u_total. This ensures unbiased task set generation
 * for schedulability testing.
 */
void uunifast(int n, double u_total, double *output_u);

/**
 * @brief Seed the random number generator for UUniFast
 * @param seed Random seed value (use 0 for time-based seed)
 */
void uunifast_seed(unsigned int seed);

#ifdef __cplusplus
}
#endif

#endif /* UUNIFAST_H */
