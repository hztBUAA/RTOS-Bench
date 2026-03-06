#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* Minimal CUSUM implementation for step/drift mean shifts.
 * Keeps state small and uses only standard C/POSIX math/stdio APIs.
 */

typedef struct {
	double k;       /* reference offset (half expected shift) */
	double h;       /* decision threshold */
	double mean0;   /* nominal mean */
	double s_pos;   /* positive cumulative sum */
	double s_neg;   /* negative cumulative sum */
} cusum_state_t;

static inline double lcg_rand01(uint64_t *state)
{
	/* 64-bit LCG; returns double in [0,1). */
	*state = (*state * 6364136223846793005ULL) + 1ULL;
	return (double)(*state >> 11) * (1.0 / 9007199254740992.0); /* 53 bits */
}

static inline double signed_noise(uint64_t *state, double amplitude)
{
	/* Uniform noise in [-amplitude, amplitude]. */
	return (lcg_rand01(state) - 0.5) * 2.0 * amplitude;
}

static inline int cusum_step(cusum_state_t *st, double x)
{
	const double centered = x - st->mean0;
	st->s_pos = fmax(0.0, st->s_pos + centered - st->k);
	st->s_neg = fmax(0.0, st->s_neg - centered - st->k);

	if (st->s_pos > st->h || st->s_neg > st->h) {
		st->s_pos = 0.0;
		st->s_neg = 0.0;
		return 1;
	}
	return 0;
}

int cusum_bench_run(void)
{
	const size_t stream_len = 6000;
	const size_t step_idx = 2000;
	const size_t drift_idx = 4000;
	const double mean_step = 0.35;      /* step jump */
	const double drift_slope = 0.00045; /* slow drift per sample */
	const double noise_amp = 0.05;

	cusum_state_t st = {
		.k = mean_step * 0.5,     /* typical reference (half shift) */
		.h = 5.0 * noise_amp,     /* coarse threshold tuned to noise */
		.mean0 = 0.0,
		.s_pos = 0.0,
		.s_neg = 0.0,
	};

	double mu = st.mean0;
	int alarms = 0;
	volatile double sink = 0.0; /* prevents optimizer from removing work */
	uint64_t rng = 1;           /* deterministic noise */

	for (size_t i = 0; i < stream_len; ++i) {
		if (i == step_idx) {
			mu += mean_step;
		}
		if (i >= drift_idx) {
			mu += drift_slope;
		}

		const double x = mu + signed_noise(&rng, noise_amp);
		alarms += cusum_step(&st, x);
		sink += x + st.s_pos - st.s_neg;
	}

	(void)sink; /* keep compiler quiet */

	printf("[cusum] samples=%zu step@%zu drift@%zu alarms=%d final_mean=%.4f\n",
	       stream_len, step_idx, drift_idx, alarms, mu);
           
	return alarms;
}