#include "benchmark_registry.h"
#include <string.h>

/* Forward declarations for built-in benchmarks */
extern const struct rtbench_ops rtbench_stub_ops;
extern const struct rtbench_ops rtbench_busywait_ops;

static const struct rtbench_ops *benchmarks[] = {
	&rtbench_stub_ops,
	&rtbench_busywait_ops,
};

static const struct rtbench_ops *current_bench = &rtbench_stub_ops;

int rtbench_select_benchmark(const char *name)
{
	size_t i;

	if (name == NULL) {
		return -1;
	}

	for (i = 0; i < sizeof(benchmarks) / sizeof(benchmarks[0]); i++) {
		if (benchmarks[i]->name != NULL &&
		    strcmp(benchmarks[i]->name, name) == 0) {
			current_bench = benchmarks[i];
			return 0;
		}
	}

	return -1;
}

const char *rtbench_current_benchmark(void)
{
	return current_bench ? current_bench->name : "";
}

void rtbench_list_benchmarks(void (*cb)(const char *name))
{
	size_t i;

	if (cb == NULL) {
		return;
	}

	for (i = 0; i < sizeof(benchmarks) / sizeof(benchmarks[0]); i++) {
		if (benchmarks[i]->name) {
			cb(benchmarks[i]->name);
		}
	}
}

int benchmark_init(int parameters_num, void **parameters)
{
	if (current_bench && current_bench->init) {
		return current_bench->init(parameters_num, parameters);
	}
	return 0;
}

void benchmark_execution(int parameters_num, void **parameters)
{
	if (current_bench && current_bench->exec) {
		current_bench->exec(parameters_num, parameters);
	}
}

void benchmark_teardown(int parameters_num, void **parameters)
{
	if (current_bench && current_bench->teardown) {
		current_bench->teardown(parameters_num, parameters);
	}
}

#ifdef EXTENDED_REPORT
const char *benchmark_log_header(void)
{
	if (current_bench && current_bench->log_header) {
		return current_bench->log_header();
	}
	return "";
}

float benchmark_log_data(void)
{
	if (current_bench && current_bench->log_data) {
		return current_bench->log_data();
	}
	return 0.0f;
}
#endif
