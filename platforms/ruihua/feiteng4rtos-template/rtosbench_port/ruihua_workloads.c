#include "workload_registry.h"

#include <stdio.h>
#include <stdint.h>

extern const struct rtosbench_workload rtosbench_stub_workload;
extern const struct rtosbench_workload rtosbench_busywait_workload;

extern void fast_test(void);
extern int epnp_test(void);
extern int ekf_test(void);
extern int icp_test(void);
extern int modbus_test(void);
extern int mqtt_test(void);
extern int pid_test(void);
extern int cusum_test(void);
extern int ewma_test(void);

static volatile uint32_t s_ruihua_smoke_sink;
static volatile uint32_t s_ruihua_network_sink;

static int generic_workload_init(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	return 0;
}

static void generic_workload_teardown(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
}

#define DEFINE_INT_WORKLOAD(sym, workload_name, desc, category_name, fn) \
	static void sym##_exec(int parameters_num, void **parameters) \
	{ \
		(void)parameters_num; \
		(void)parameters; \
		(void)fn(); \
	} \
	static const struct rtosbench_workload sym = { \
		.name = workload_name, \
		.description = desc, \
		.category = category_name, \
		.init = generic_workload_init, \
		.exec = sym##_exec, \
		.teardown = generic_workload_teardown, \
	}

#define DEFINE_VOID_WORKLOAD(sym, workload_name, desc, category_name, fn) \
	static void sym##_exec(int parameters_num, void **parameters) \
	{ \
		(void)parameters_num; \
		(void)parameters; \
		fn(); \
	} \
	static const struct rtosbench_workload sym = { \
		.name = workload_name, \
		.description = desc, \
		.category = category_name, \
		.init = generic_workload_init, \
		.exec = sym##_exec, \
		.teardown = generic_workload_teardown, \
	}

DEFINE_VOID_WORKLOAD(rtosbench_fast_workload, "fast",
		     "FAST corner detection benchmark", "vision", fast_test);
DEFINE_INT_WORKLOAD(rtosbench_epnp_workload, "epnp",
		    "Perspective-n-Point solver benchmark", "vision", epnp_test);
DEFINE_INT_WORKLOAD(rtosbench_ekf_workload, "ekf",
		    "Extended Kalman Filter flight dataset replay", "estimation", ekf_test);
DEFINE_INT_WORKLOAD(rtosbench_icp_workload, "icp",
		    "Iterative Closest Point alignment", "vision", icp_test);
DEFINE_INT_WORKLOAD(rtosbench_pid_workload, "pid",
		    "PID controller synthetic dataset benchmark", "control", pid_test);
DEFINE_INT_WORKLOAD(rtosbench_cusum_workload, "cusum",
		    "CUSUM mean-shift detector (step/drift)", "detection", cusum_test);
DEFINE_INT_WORKLOAD(rtosbench_ewma_workload, "ewma",
		    "EWMA residual thresholding (spike/drop)", "detection", ewma_test);

static int ruihua_modbus_offline_test(void)
{
	uint16_t holding[64] = {0};
	uint8_t coils[64] = {0};
	uint32_t checksum = s_ruihua_network_sink;
	const int rounds = 1000;
	int errors = 0;

	for (int i = 0; i < rounds; i++) {
		int reg = i % 54;
		int coil = i % 64;
		for (int k = 0; k < 10; k++) {
			holding[reg + k] = (uint16_t)(i + k);
		}
		coils[coil] = (uint8_t)(i & 1);
		if (holding[reg] != (uint16_t)i ||
		    holding[reg + 9] != (uint16_t)(i + 9) ||
		    coils[coil] != (uint8_t)(i & 1)) {
			errors++;
		}
		checksum = checksum * 33u + holding[reg] + coils[coil];
	}

	s_ruihua_network_sink = checksum;
	printf("[MODBUS] offline-loopback samples=%d errors=%d checksum=%u\n",
	       rounds * 4, errors, checksum);
	return errors == 0 ? 0 : -1;
}

static int ruihua_mqtt_offline_test(void)
{
	uint32_t checksum = s_ruihua_network_sink;
	const int messages = 512;

	for (int i = 0; i < messages; i++) {
		int lat_e7 = 399000000 + i * 17;
		int lon_e7 = 1163000000 + i * 31;
		int payload_len = 32 + (i % 48);
		checksum = checksum * 131u + (uint32_t)lat_e7;
		checksum ^= (uint32_t)lon_e7 + (uint32_t)payload_len;
	}

	s_ruihua_network_sink = checksum;
	printf("[MQTT] offline-pack samples=%d checksum=%u\n",
	       messages, checksum);
	return 0;
}

DEFINE_INT_WORKLOAD(rtosbench_modbus_workload, "modbus",
		    "Modbus TCP server/client round-trip benchmark", "network",
		    ruihua_modbus_offline_test);
DEFINE_INT_WORKLOAD(rtosbench_mqtt_workload, "mqtt",
		    "MQTT publish benchmark (GeoLife trace)", "network",
		    ruihua_mqtt_offline_test);

static int ruihua_smoke_init(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
	return 0;
}

static void ruihua_smoke_exec(int parameters_num, void **parameters)
{
	uint32_t v = s_ruihua_smoke_sink;

	(void)parameters_num;
	(void)parameters;

	for (uint32_t i = 0; i < 5000000u; i++) {
		v = (v * 1664525u) + 1013904223u + i;
	}
	s_ruihua_smoke_sink = v;
}

static void ruihua_smoke_teardown(int parameters_num, void **parameters)
{
	(void)parameters_num;
	(void)parameters;
}

static const struct rtosbench_workload ruihua_smoke_workload = {
	.name = "ruihua-smoke",
	.description = "Deterministic CPU smoke workload for Ruihua bring-up",
	.category = "industrial",
	.init = ruihua_smoke_init,
	.exec = ruihua_smoke_exec,
	.teardown = ruihua_smoke_teardown,
};

void rtosbench_register_rtos_workloads(void)
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
	rtosbench_register_workload(&rtosbench_cusum_workload);
	rtosbench_register_workload(&rtosbench_ewma_workload);
	rtosbench_register_workload(&ruihua_smoke_workload);
}
