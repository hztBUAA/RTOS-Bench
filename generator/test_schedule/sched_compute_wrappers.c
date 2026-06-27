/**
 * @file sched_compute_wrappers.c
 * @brief Synchronous compute workload wrappers for test-schedule.
 */

#include "sched_workloads.h"

#include <stddef.h>

extern int fast_bench_run_once(int loops);
extern int epnp_bench_run(size_t iterations);
extern int ekf_bench_run_quick(size_t max_imu_samples);
extern int icp_bench_run(void);
extern int pid_bench_run(void);
extern int cusum_bench_run(void);
extern int ewma_bench_run(void);

int sched_fast_init(void) { return 0; }
int sched_fast_quick_exec(void) { return fast_bench_run_once(1); }
void sched_fast_teardown(void) {}

int sched_epnp_init(void) { return 0; }
int sched_epnp_quick_exec(void) { return epnp_bench_run(1); }
void sched_epnp_teardown(void) {}

int sched_ekf_init(void) { return 0; }
int sched_ekf_quick_exec(void) { return ekf_bench_run_quick(256); }
void sched_ekf_teardown(void) {}

int sched_icp_init(void) { return 0; }
int sched_icp_quick_exec(void) { return icp_bench_run(); }
void sched_icp_teardown(void) {}

int sched_pid_init(void) { return 0; }
int sched_pid_quick_exec(void) { return pid_bench_run(); }
void sched_pid_teardown(void) {}

int sched_cusum_init(void) { return 0; }
/* cusum_bench_run() returns a CUSUM result count (not an error code); for
 * schedulability we only care that the job completed, so normalize to 0. */
int sched_cusum_quick_exec(void) { cusum_bench_run(); return 0; }
void sched_cusum_teardown(void) {}

int sched_ewma_init(void) { return 0; }
int sched_ewma_quick_exec(void) { return ewma_bench_run(); }
void sched_ewma_teardown(void) {}
