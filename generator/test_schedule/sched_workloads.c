/**
 * @file sched_workloads.c
 * @brief Wrapper registration and lookup for test-schedule
 */

#include "sched_workloads.h"
#include <string.h>
#include <stddef.h>

/**
 * @brief Static table of workload wrappers for test-schedule
 *
 * Add new wrappers here. The wrapper name must exactly match
 * the original workload name in the registry.
 */
static const struct sched_workload_wrapper g_sched_wrappers[] = {
	{
		.name = "mqtt",
		.init = sched_mqtt_init,
		.quick_exec = sched_mqtt_quick_exec,
		.teardown = sched_mqtt_teardown,
		.max_wcet_ms = 2000,
		.needs_state_reset = 0,
	},
	{
		.name = "modbus",
		.init = sched_modbus_init,
		.quick_exec = sched_modbus_quick_exec,
		.teardown = sched_modbus_teardown,
		.max_wcet_ms = 2000,
		.needs_state_reset = 0,
	},
};

#define SCHED_WRAPPER_COUNT (sizeof(g_sched_wrappers) / sizeof(g_sched_wrappers[0]))

const struct sched_workload_wrapper *sched_get_wrapper(const char *name)
{
	if (!name) {
		return NULL;
	}

	for (size_t i = 0; i < SCHED_WRAPPER_COUNT; i++) {
		if (strcmp(g_sched_wrappers[i].name, name) == 0) {
			return &g_sched_wrappers[i];
		}
	}

	return NULL;
}

int sched_wrapper_count(void)
{
	return (int)SCHED_WRAPPER_COUNT;
}

const struct sched_workload_wrapper *sched_get_wrapper_by_index(int idx)
{
	if (idx < 0 || idx >= (int)SCHED_WRAPPER_COUNT) {
		return NULL;
	}
	return &g_sched_wrappers[idx];
}
