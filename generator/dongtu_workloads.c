/**
 * @file dongtu_workloads.c
 * @brief Dongtu C workload registration for RTOS-Bench.
 *
 * Dongtu vm_3588 projects are C-only by default, so this file registers the
 * C workloads without requiring the C++ wrapper used by richer builds.
 */

#include "workload_registry.h"

#include <stddef.h>

extern const struct rtosbench_workload rtosbench_stub_workload;
extern const struct rtosbench_workload rtosbench_busywait_workload;

int fast_bench_run_once(int loops);
int modbus_bench_run(void);
int mqtt_bench_run(void);
int cusum_bench_run(void);
int ewma_bench_run(void);

static int default_init(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	return 0;
}

static void default_teardown(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
}

static void fast_exec(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	fast_bench_run_once(0);
}

static void modbus_exec(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	modbus_bench_run();
}

static void mqtt_exec(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	mqtt_bench_run();
}

static void cusum_exec(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	cusum_bench_run();
}

static void ewma_exec(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	ewma_bench_run();
}

static const struct rtosbench_workload rtosbench_fast_workload = {
	.name = "fast",
	.description = "FAST corner detection benchmark",
	.category = "vision",
	.init = default_init,
	.exec = fast_exec,
	.teardown = default_teardown,
};

static const struct rtosbench_workload rtosbench_modbus_workload = {
	.name = "modbus",
	.description = "Modbus TCP server/client round-trip benchmark",
	.category = "network",
	.init = default_init,
	.exec = modbus_exec,
	.teardown = default_teardown,
};

static const struct rtosbench_workload rtosbench_mqtt_workload = {
	.name = "mqtt",
	.description = "MQTT publish benchmark (GeoLife trace)",
	.category = "network",
	.init = default_init,
	.exec = mqtt_exec,
	.teardown = default_teardown,
};

static const struct rtosbench_workload rtosbench_cusum_workload = {
	.name = "cusum",
	.description = "CUSUM anomaly detection benchmark",
	.category = "signal",
	.init = default_init,
	.exec = cusum_exec,
	.teardown = default_teardown,
};

static const struct rtosbench_workload rtosbench_ewma_workload = {
	.name = "ewma",
	.description = "EWMA residual thresholding benchmark",
	.category = "signal",
	.init = default_init,
	.exec = ewma_exec,
	.teardown = default_teardown,
};

void rtosbench_register_rtos_workloads(void)
{
	rtosbench_register_workload(&rtosbench_stub_workload);
	rtosbench_register_workload(&rtosbench_busywait_workload);
	rtosbench_register_workload(&rtosbench_fast_workload);
	rtosbench_register_workload(&rtosbench_modbus_workload);
	rtosbench_register_workload(&rtosbench_mqtt_workload);
	rtosbench_register_workload(&rtosbench_cusum_workload);
	rtosbench_register_workload(&rtosbench_ewma_workload);
}
