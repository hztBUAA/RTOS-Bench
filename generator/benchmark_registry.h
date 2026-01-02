#ifndef RTBENCH_BENCHMARK_REGISTRY_H
#define RTBENCH_BENCHMARK_REGISTRY_H

struct rtbench_ops {
	const char *name;
	int (*init)(int parameters_num, void **parameters);
	void (*exec)(int parameters_num, void **parameters);
	void (*teardown)(int parameters_num, void **parameters);
#ifdef EXTENDED_REPORT
	const char *(*log_header)(void);
	float (*log_data)(void);
#endif
};

int rtbench_select_benchmark(const char *name);
const char *rtbench_current_benchmark(void);
void rtbench_list_benchmarks(void (*cb)(const char *name));

/* Wrappers used by periodic_benchmark */
int benchmark_init(int parameters_num, void **parameters);
void benchmark_execution(int parameters_num, void **parameters);
void benchmark_teardown(int parameters_num, void **parameters);

#ifdef EXTENDED_REPORT
const char *benchmark_log_header(void);
float benchmark_log_data(void);
#endif

#endif /* RTBENCH_BENCHMARK_REGISTRY_H */
