#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* Simple EWMA (with EW variance) residual thresholding benchmark.
 * Uses only standard C/POSIX math/stdio, no dynamic allocation.
 */

typedef struct {
	double alpha;     /* smoothing factor */
	double mean;      /* EWMA estimate */
	double variance;  /* exponentially weighted variance */
} ewma_state_t;

static inline double lcg_rand01(uint64_t *state)
{
	*state = (*state * 6364136223846793005ULL) + 1ULL;
	return (double)(*state >> 11) * (1.0 / 9007199254740992.0);
}

static inline double signed_noise(uint64_t *state, double amplitude)
{
	return (lcg_rand01(state) - 0.5) * 2.0 * amplitude;
}

static inline double ewma_step(ewma_state_t *st, double x)
{
	const double prev_mean = st->mean;
	st->mean = st->alpha * x + (1.0 - st->alpha) * prev_mean;

	const double diff = x - prev_mean;
	st->variance = st->alpha * diff * diff + (1.0 - st->alpha) * st->variance;

	const double resid = x - st->mean;
	const double sigma = sqrt(st->variance + 1e-9);
	return (sigma > 0.0) ? (resid / sigma) : 0.0;
}

int ewma_bench_run(void)
{
	const size_t stream_len = 5000;
	const double alpha = 0.05;          /* slow baseline tracking */
	const double noise_amp = 0.04;
	const double spike_mag = 1.2;
	const size_t spike1 = 1200;
	const size_t spike2 = 2500;
	const size_t drop_start = 3600;
	const size_t drop_len = 40;
	const double freq = 0.01;           /* cycles per sample */
	const double threshold_z = 4.0;     /* |z| threshold */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
	const double omega = 2.0 * M_PI * freq;

	ewma_state_t st = {
		.alpha = alpha,
		.mean = 0.0,
		.variance = noise_amp * noise_amp,
	};

	uint64_t rng = 2;
	volatile double sink = 0.0;
	int alarms = 0;

	for (size_t i = 0; i < stream_len; ++i) {
		double x = sin(omega * (double)i) + signed_noise(&rng, noise_amp);

		if (i == spike1 || i == spike2) {
			x += spike_mag;
		}
		if (i >= drop_start && i < (drop_start + drop_len)) {
			x = 0.0; /* emulate dropout */
		}

		const double z = ewma_step(&st, x);
		if (fabs(z) > threshold_z) {
			alarms++;
		}
		sink += z + x;
	}

	(void)sink;

#ifndef RT_THREAD_PLATFORM
	printf("[ewma] samples=%zu spikes@%zu/%zu drop@%zu len=%zu alarms=%d\n",
	       stream_len, spike1, spike2, drop_start, drop_len, alarms);
#endif
	return alarms;
}
