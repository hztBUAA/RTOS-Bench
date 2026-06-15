#include "test_schedule/sched_workloads.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static volatile uint32_t g_sched_sink;

static int sched_spin(unsigned int rounds)
{
	uint32_t v = g_sched_sink;

	for (unsigned int i = 0; i < rounds; i++) {
		v = (v * 1664525u) + 1013904223u + i;
	}
	g_sched_sink = v;
	return 0;
}

static int sched_fast_quick_exec(void) { return sched_spin(20000u); }
static int sched_epnp_quick_exec(void) { return sched_spin(26000u); }
static int sched_ekf_quick_exec(void) { return sched_spin(30000u); }
static int sched_icp_quick_exec(void) { return sched_spin(24000u); }
static int sched_modbus_quick_exec_local(void) { return sched_spin(18000u); }
static int sched_mqtt_quick_exec_local(void) { return sched_spin(18000u); }
static int sched_pid_quick_exec(void) { return sched_spin(16000u); }
static int sched_cusum_quick_exec(void) { return sched_spin(12000u); }
static int sched_ewma_quick_exec(void) { return sched_spin(12000u); }
static int sched_ruihua_smoke_quick_exec(void) { return sched_spin(14000u); }

static const struct sched_workload_wrapper g_wrappers[] = {
	{ "fast", NULL, sched_fast_quick_exec, NULL, 5, 0 },
	{ "epnp", NULL, sched_epnp_quick_exec, NULL, 5, 0 },
	{ "ekf", NULL, sched_ekf_quick_exec, NULL, 5, 0 },
	{ "icp", NULL, sched_icp_quick_exec, NULL, 5, 0 },
	{ "modbus", NULL, sched_modbus_quick_exec_local, NULL, 5, 0 },
	{ "mqtt", NULL, sched_mqtt_quick_exec_local, NULL, 5, 0 },
	{ "pid", NULL, sched_pid_quick_exec, NULL, 5, 0 },
	{ "cusum", NULL, sched_cusum_quick_exec, NULL, 5, 0 },
	{ "ewma", NULL, sched_ewma_quick_exec, NULL, 5, 0 },
	{ "ruihua-smoke", NULL, sched_ruihua_smoke_quick_exec, NULL, 5, 0 },
};

const struct sched_workload_wrapper *sched_get_wrapper(const char *name)
{
	if (name == NULL) {
		return NULL;
	}
	for (int i = 0; i < sched_wrapper_count(); i++) {
		if (strcmp(g_wrappers[i].name, name) == 0) {
			return &g_wrappers[i];
		}
	}
	return NULL;
}

int sched_wrapper_count(void)
{
	return (int)(sizeof(g_wrappers) / sizeof(g_wrappers[0]));
}

const struct sched_workload_wrapper *sched_get_wrapper_by_index(int idx)
{
	if (idx < 0 || idx >= sched_wrapper_count()) {
		return NULL;
	}
	return &g_wrappers[idx];
}
