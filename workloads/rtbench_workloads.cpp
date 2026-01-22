// Wrapper workloads to expose RT-Thread benchmark applications to rt-bench

#include "../generator/workload_registry.h"
#include <stddef.h>

/* Built-in workloads (ensure available even if constructors are skipped) */
extern const struct rtosbench_workload rtosbench_stub_workload;
extern const struct rtosbench_workload rtosbench_busywait_workload;

extern "C" {
int fast_bench_run_once(int loops);
int modbus_bench_run(void);
int mqtt_bench_run(void);
// C++ workloads exposed as C for simplicity
int epnp_bench_run(size_t iterations);
int ekf_bench_run(void);
int icp_bench_run(void);
int pid_bench_run(void);
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
	.init = fast_init,
	.exec = fast_exec,
	.teardown = fast_teardown,
};

/* EPNP */
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
	epnp_bench_run(1000);
}

static void epnp_teardown(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
}

const struct rtosbench_workload rtosbench_epnp_workload = {
	.name = "epnp",
	.description = "Perspective-n-Point solver benchmark",
	.init = epnp_init,
	.exec = epnp_exec,
	.teardown = epnp_teardown,
};

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
	.init = pid_init,
	.exec = pid_exec,
	.teardown = pid_teardown,
};

/* Registration helpers */
static void register_all_workloads(void)
{
	rtosbench_register_workload(&rtosbench_stub_workload);
	rtosbench_register_workload(&rtosbench_busywait_workload);
	rtosbench_register_workload(&rtosbench_fast_workload);
	rtosbench_register_workload(&rtosbench_epnp_workload);
	rtosbench_register_workload(&rtosbench_ekf_workload);
	rtosbench_register_workload(&rtosbench_icp_workload);
	rtosbench_register_workload(&rtosbench_modbus_workload);
	rtosbench_register_workload(&rtosbench_mqtt_workload);
	rtosbench_register_workload(&rtosbench_pid_workload);
}

#if defined(__GNUC__) && !defined(RT_THREAD_PLATFORM)
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
