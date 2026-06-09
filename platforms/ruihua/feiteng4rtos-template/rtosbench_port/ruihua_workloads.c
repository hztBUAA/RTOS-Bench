#include "workload_registry.h"

#include <stdint.h>

extern const struct rtosbench_workload rtosbench_stub_workload;
extern const struct rtosbench_workload rtosbench_busywait_workload;

static volatile uint32_t s_ruihua_smoke_sink;

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
	rtosbench_register_workload(&ruihua_smoke_workload);
}
