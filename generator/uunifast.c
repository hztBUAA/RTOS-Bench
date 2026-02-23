/**
 * @file uunifast.c
 * @brief UUniFast algorithm implementation
 * @details Generates uniformly distributed task utilizations for schedulability testing
 */

#include "uunifast.h"
#include <stdlib.h>
#include <math.h>
#include <time.h>

static int uunifast_seeded = 0;

void uunifast_seed(unsigned int seed)
{
	if (seed == 0) {
		srand((unsigned int)time(NULL));
	} else {
		srand(seed);
	}
	uunifast_seeded = 1;
}

void uunifast(int n, double u_total, double *output_u)
{
	double sum_u = u_total;
	double next_sum_u;
	double rand_val;
	int i;

	/* Auto-seed if not already seeded */
	if (!uunifast_seeded) {
		uunifast_seed(0);
	}

	for (i = 1; i < n; i++) {
		rand_val = (double)rand() / (double)RAND_MAX;

		/* UUniFast formula: next_sum = sum * rand^(1/(n-i)) */
		next_sum_u = sum_u * pow(rand_val, 1.0 / (double)(n - i));

		output_u[i - 1] = sum_u - next_sum_u;
		sum_u = next_sum_u;
	}
	/* Last task gets remaining utilization */
	output_u[n - 1] = sum_u;
}
