// Wrapper workloads to expose RT-Thread benchmark applications to rt-bench

#include "../generator/workload_registry.h"
#include <stddef.h>

/* For EPNP thread wrapper on SylixOS/POSIX */
#if defined(ENABLE_EPNP_WORKLOAD) && !defined(RT_THREAD_PLATFORM)
#include <pthread.h>
/* Large stack for Eigen JacobiSVD operations in EPNP.
 * ARM64 requires more stack than x86 due to larger stack frames.
 * Eigen's JacobiSVD with dynamic matrices needs substantial stack space. */
#define EPNP_THREAD_STACK_SIZE (4 * 1024 * 1024)
#endif

/* Built-in workloads (ensure available even if constructors are skipped) */
extern const struct rtosbench_workload rtosbench_stub_workload;
extern const struct rtosbench_workload rtosbench_busywait_workload;

extern "C" {
int fast_bench_run_once(int loops);
int modbus_bench_run(void);
int mqtt_bench_run(void);
// C++ workloads exposed as C for simplicity
#ifdef ENABLE_EPNP_WORKLOAD
int epnp_bench_run(size_t iterations);
#endif
int ekf_bench_run(void);
int icp_bench_run(void);
int pid_bench_run(void);
int cusum_bench_run(void);
int ewma_bench_run(void);
}

/* FAST */
static int fast_init(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	return 0;
}

static void fast_exec(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	fast_bench_run_once(0);
}

static void fast_teardown(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
}

const struct rtosbench_workload rtosbench_fast_workload = {
	.name = "fast",
	.description = "FAST corner detection benchmark",
	.category = "vision",
	.init = fast_init,
	.exec = fast_exec,
	.teardown = fast_teardown,
};

/* EPNP */
#ifdef ENABLE_EPNP_WORKLOAD

/* Thread arguments for EPNP execution */
static size_t epnp_iterations_arg = 1000;

#ifndef RT_THREAD_PLATFORM
/* Thread wrapper to run EPNP with large stack on SylixOS/POSIX */
static void *epnp_thread_wrapper(void *arg)
{
	size_t iters = *(size_t *)arg;
	epnp_bench_run(iters);
	return NULL;
}
#endif

static int epnp_init(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	return 0;
}

static void epnp_exec(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;

#ifdef RT_THREAD_PLATFORM
	/* RT-Thread: direct call (handled by rt_thread stack) */
	epnp_bench_run(10);
#else
	/* SylixOS/POSIX: spawn thread with large stack for Eigen operations */
	pthread_t tid;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, EPNP_THREAD_STACK_SIZE);

	/* Reduced iterations (10 instead of 1000) to avoid Eigen state issues */
	epnp_iterations_arg = 10;
	int ret = pthread_create(&tid, &attr, epnp_thread_wrapper, &epnp_iterations_arg);
	pthread_attr_destroy(&attr);

	if (ret == 0) {
		pthread_join(tid, NULL);
	} else {
		/* Fallback: direct call (may crash on small stack) */
		epnp_bench_run(10);
	}
#endif
}

static void epnp_teardown(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
}

const struct rtosbench_workload rtosbench_epnp_workload = {
	.name = "epnp",
	.description = "Perspective-n-Point solver benchmark",
	.category = "vision",
	.init = epnp_init,
	.exec = epnp_exec,
	.teardown = epnp_teardown,
};
#endif

/* EKF */
static int ekf_init(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	return 0;
}

static void ekf_exec(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	ekf_bench_run();
}

static void ekf_teardown(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
}

const struct rtosbench_workload rtosbench_ekf_workload = {
	.name = "ekf",
	.description = "Extended Kalman Filter flight dataset replay",
	.category = "estimation",
	.init = ekf_init,
	.exec = ekf_exec,
	.teardown = ekf_teardown,
};

/* ICP */
static int icp_init(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	return 0;
}

static void icp_exec(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	icp_bench_run();
}

static void icp_teardown(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
}

const struct rtosbench_workload rtosbench_icp_workload = {
	.name = "icp",
	.description = "Iterative Closest Point alignment",
	.category = "vision",
	.init = icp_init,
	.exec = icp_exec,
	.teardown = icp_teardown,
};

/* MODBUS */
static int modbus_init(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	return 0;
}

static void modbus_exec(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	modbus_bench_run();
}

static void modbus_teardown(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
}

const struct rtosbench_workload rtosbench_modbus_workload = {
	.name = "modbus",
	.description = "Modbus TCP server/client round-trip benchmark",
	.category = "network",
	.init = modbus_init,
	.exec = modbus_exec,
	.teardown = modbus_teardown,
};

/* MQTT */
static int mqtt_init(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	return 0;
}

static void mqtt_exec(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	mqtt_bench_run();
}

static void mqtt_teardown(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
}

const struct rtosbench_workload rtosbench_mqtt_workload = {
	.name = "mqtt",
	.description = "MQTT publish benchmark (GeoLife trace)",
	.category = "network",
	.init = mqtt_init,
	.exec = mqtt_exec,
	.teardown = mqtt_teardown,
};

/* PID */
static int pid_init(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	return 0;
}

static void pid_exec(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	pid_bench_run();
}

static void pid_teardown(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
}

const struct rtosbench_workload rtosbench_pid_workload = {
	.name = "pid",
	.description = "PID controller synthetic dataset benchmark",
	.category = "control",
	.init = pid_init,
	.exec = pid_exec,
	.teardown = pid_teardown,
};

/* CUSUM */
static int cusum_init(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	return 0;
}

static void cusum_exec(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	cusum_bench_run();
}

static void cusum_teardown(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
}

const struct rtosbench_workload rtosbench_cusum_workload = {
	.name = "cusum",
	.description = "CUSUM mean-shift detector (step/drift)",
	.category = "detection",
	.init = cusum_init,
	.exec = cusum_exec,
	.teardown = cusum_teardown,
};

/* EWMA */
static int ewma_init(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	return 0;
}

static void ewma_exec(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	ewma_bench_run();
}

static void ewma_teardown(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
}

const struct rtosbench_workload rtosbench_ewma_workload = {
	.name = "ewma",
	.description = "EWMA residual thresholding (spike/drop)",
	.category = "detection",
	.init = ewma_init,
	.exec = ewma_exec,
	.teardown = ewma_teardown,
};

/* Registration helpers */
static void register_all_workloads(void)
{
	/* Note: stub and busywait are registered via workload_registry.c constructor */
	rtosbench_register_workload(&rtosbench_fast_workload);
#ifdef ENABLE_EPNP_WORKLOAD
	rtosbench_register_workload(&rtosbench_epnp_workload);
#endif
	rtosbench_register_workload(&rtosbench_ekf_workload);
	rtosbench_register_workload(&rtosbench_icp_workload);
	rtosbench_register_workload(&rtosbench_modbus_workload);
	rtosbench_register_workload(&rtosbench_mqtt_workload);
	rtosbench_register_workload(&rtosbench_pid_workload);
	rtosbench_register_workload(&rtosbench_cusum_workload);
	rtosbench_register_workload(&rtosbench_ewma_workload);
}

/* Disable constructor-based auto-registration on SylixOS due to static
 * initialization order issues with position-independent code.
 * Registration happens explicitly via rtosbench_register_rtos_workloads(). */
#if defined(__GNUC__) && !defined(RT_THREAD_PLATFORM) && !defined(SYLIXOS)
__attribute__((constructor))
static void auto_register_workloads(void)
{
	register_all_workloads();
}
#endif

extern "C" void rtosbench_register_rtos_workloads(void)
{
	register_all_workloads();
}
