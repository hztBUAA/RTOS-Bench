#ifdef RT_THREAD_PLATFORM
#include <rtthread.h>
#include <finsh.h>
#else
#define MSH_CMD_EXPORT(cmd, desc)
#endif

#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* POSIX - required for clock_gettime */
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <sched.h>

/* Minimal CUSUM implementation for step/drift mean shifts.
 * Keeps state small and uses only standard C/POSIX math/stdio APIs.
 */

static uint64_t get_time_ns(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

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

	int loops = 100;
	/* Start timing */
	uint64_t start_time = get_time_ns();

	for (int j = 0; j < loops; j ++) {
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
	}

	uint64_t end_time = get_time_ns();
	uint64_t total_ns = end_time - start_time;
	double avg_ns = (double)total_ns / stream_len;

	printf("[CUSUM] samples=%llu total_time=%.3f ms avg_latency=%.3f us/sample\n",
	       (unsigned long long)stream_len,
	       (double)total_ns / 1000000.0, avg_ns / 1000.0);

	return alarms;
}

static void* cusum_thread_entry(void *parameter) {
    (void)parameter;
    cusum_bench_run();
    return NULL;
}

int cusum_test(void) {
    pthread_t tid;
    pthread_attr_t attr;
    int ret;

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 64 * 1024);

    struct sched_param param;
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = 20;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    ret = pthread_create(&tid, &attr, cusum_thread_entry, NULL);

    pthread_attr_destroy(&attr);

    if (ret != 0) {
        printf("Failed to create pthread. Error: %d\n", ret);
    } else {
        pthread_join(tid, NULL); 
    }

    return 0;
}
MSH_CMD_EXPORT(cusum_test, Run CUSUM benchmark);
