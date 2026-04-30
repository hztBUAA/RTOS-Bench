/**
 * @file workload_registry.c
 * @brief Unified workload registry implementation for RTOS-Bench
 */

#include "workload_registry.h"
#include <string.h>
#include <stddef.h>

/* ============================================================================
 * Registry Storage
 * ============================================================================ */

static const struct rtosbench_workload *registered_workloads[RTOSBENCH_MAX_WORKLOADS];
static int workload_count = 0;
static const struct rtosbench_workload *current_workload = NULL;

/* ============================================================================
 * Built-in Workloads (stub and busywait)
 * ============================================================================ */

/* Forward declarations */
extern const struct rtosbench_workload rtosbench_stub_workload;
extern const struct rtosbench_workload rtosbench_busywait_workload;

/* Auto-register built-in workloads only for generic POSIX/Linux builds. */
#ifndef RTOSBENCH_USE_MANUAL_WORKLOAD_REGISTRATION
__attribute__((constructor(101)))
static void rtosbench_register_builtins(void)
{
	rtosbench_register_workload(&rtosbench_stub_workload);
	rtosbench_register_workload(&rtosbench_busywait_workload);
}
#endif

/* ============================================================================
 * Registry API Implementation
 * ============================================================================ */

int rtosbench_register_workload(const struct rtosbench_workload *workload)
{
	if (workload == NULL || workload->name == NULL) {
		return -1;
	}

	if (workload_count >= RTOSBENCH_MAX_WORKLOADS) {
		return -1;
	}

	/* Check for duplicate registration */
	for (int i = 0; i < workload_count; i++) {
		if (registered_workloads[i] == workload ||
		    (registered_workloads[i]->name &&
		     strcmp(registered_workloads[i]->name, workload->name) == 0)) {
			return 0; /* Already registered */
		}
	}

	registered_workloads[workload_count++] = workload;

	/* Set first registered workload as default */
	if (current_workload == NULL) {
		current_workload = workload;
	}

	return 0;
}

int rtosbench_select_workload(const char *name)
{
	if (name == NULL) {
		return -1;
	}

	for (int i = 0; i < workload_count; i++) {
		if (registered_workloads[i]->name &&
		    strcmp(registered_workloads[i]->name, name) == 0) {
			current_workload = registered_workloads[i];
			return 0;
		}
	}

	return -1;
}

const char *rtosbench_current_workload(void)
{
	return current_workload ? current_workload->name : "";
}

void rtosbench_list_workloads(void (*cb)(const char *name, const char *desc))
{
	if (cb == NULL) {
		return;
	}

	for (int i = 0; i < workload_count; i++) {
		const char *name = registered_workloads[i]->name;
		const char *desc = registered_workloads[i]->description;
		cb(name ? name : "", desc ? desc : "");
	}
}

int rtosbench_workload_count(void)
{
	return workload_count;
}

const struct rtosbench_workload *rtosbench_get_workload(int idx)
{
	if (idx < 0 || idx >= workload_count) {
		return NULL;
	}
	return registered_workloads[idx];
}

/* ============================================================================
 * Workload Execution Interface
 * ============================================================================ */

int workload_init(int parameters_num, void **parameters)
{
	if (current_workload && current_workload->init) {
		return current_workload->init(parameters_num, parameters);
	}
	return 0;
}

void workload_exec(int parameters_num, void **parameters)
{
	if (current_workload && current_workload->exec) {
		current_workload->exec(parameters_num, parameters);
	}
}

void workload_teardown(int parameters_num, void **parameters)
{
	if (current_workload && current_workload->teardown) {
		current_workload->teardown(parameters_num, parameters);
	}
}

#ifdef EXTENDED_REPORT
const char *workload_log_header(void)
{
	if (current_workload && current_workload->log_header) {
		return current_workload->log_header();
	}
	return "";
}

float workload_log_data(void)
{
	if (current_workload && current_workload->log_data) {
		return current_workload->log_data();
	}
	return 0.0f;
}
#endif

/* ============================================================================
 * Legacy API Compatibility
 *
 * The benchmark_* names are kept for existing code and always route through the
 * workload registry (no overrides), ensuring a single maintenance path.
 * ============================================================================ */
#define RTOSBENCH_WEAK

RTOSBENCH_WEAK
int benchmark_init(int parameters_num, void **parameters)
{
	return workload_init(parameters_num, parameters);
}

RTOSBENCH_WEAK
void benchmark_execution(int parameters_num, void **parameters)
{
	workload_exec(parameters_num, parameters);
}

RTOSBENCH_WEAK
void benchmark_teardown(int parameters_num, void **parameters)
{
	workload_teardown(parameters_num, parameters);
}

#ifdef EXTENDED_REPORT
RTOSBENCH_WEAK
const char *benchmark_log_header(void)
{
	return workload_log_header();
}

RTOSBENCH_WEAK
float benchmark_log_data(void)
{
	return workload_log_data();
}
#endif
