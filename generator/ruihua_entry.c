/**
 * @file ruihua_entry.c
 * @brief Ruihua/ReWorks shell entry for RTOS-Bench.
 *
 * Keep ReWorks shell syntax here. Command behavior lives in rtbench_command.c
 * and platform APIs live under generator/platform/ruihua.
 */

#include "rtbench_command.h"

#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>

#define RTBENCH_RUIHUA_MAX_ARGS 32

extern int run_all_workloads(void);

/*
 * test-all 必须跑在独立的大栈线程上。ReWorks 的 shell/telnet 任务栈很小
 * (SHELL_TASK_STACKSIZE 64KB),test-all 会把 9 个负载链(含 Eigen 的
 * epnp/ekf/icp)跑在调用者栈上 → 栈溢出踩进内核调度,触发 Data Abort in EL1
 * (ucore_waitq_flush/schedule)。这里覆盖弱默认 rtbench_platform_run_test_all,
 * 改为在 4MB 栈的工作线程里执行,与 SylixOS/Dongtu 一致。
 */
#define RTBENCH_TEST_ALL_STACK_SIZE (4 * 1024 * 1024)

struct ruihua_test_all_job {
	rtbench_command_runner_fn runner;
	void *ctx;
	int result;
	sem_t done;
};

static void *ruihua_test_all_thread(void *arg)
{
	struct ruihua_test_all_job *job = (struct ruihua_test_all_job *)arg;

	job->result = job->runner ? job->runner(job->ctx) : -1;
	sem_post(&job->done);
	return NULL;
}

int rtbench_platform_run_test_all(rtbench_command_runner_fn runner, void *ctx)
{
	struct ruihua_test_all_job job;
	pthread_t tid;
	pthread_attr_t attr;
	int ret;

	if (runner == NULL) {
		return -1;
	}

	job.runner = runner;
	job.ctx = ctx;
	job.result = -1;
	if (sem_init(&job.done, 0, 0) != 0) {
		return -1;
	}

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, RTBENCH_TEST_ALL_STACK_SIZE);
	ret = pthread_create(&tid, &attr, ruihua_test_all_thread, &job);
	pthread_attr_destroy(&attr);
	if (ret != 0) {
		printf("[RTOS-Bench] Failed to create test-all worker thread\n");
		sem_destroy(&job.done);
		return -1;
	}

	sem_wait(&job.done);
	pthread_join(tid, NULL);
	sem_destroy(&job.done);
	return job.result;
}

static int split_command_line(char *buf, char **argv, int max_args)
{
	int argc = 0;
	char *p = buf;

	while (*p != '\0' && argc < max_args) {
		char quote = '\0';

		while (*p != '\0' && isspace((unsigned char)*p)) {
			p++;
		}
		if (*p == '\0') {
			break;
		}
		if (*p == '"' || *p == '\'') {
			quote = *p++;
		}
		argv[argc++] = p;
		while (*p != '\0') {
			if (quote != '\0') {
				if (*p == quote) {
					*p++ = '\0';
					break;
				}
			} else if (isspace((unsigned char)*p)) {
				*p++ = '\0';
				break;
			}
			p++;
		}
	}
	return argc;
}

static int is_standard_command_or_option(const char *arg)
{
	if (arg == NULL || arg[0] == '-') {
		return 1;
	}
	return !strcmp(arg, "test-all") ||
	       !strcmp(arg, "test-schedule") ||
	       !strcmp(arg, "test-realtime") ||
	       !strcmp(arg, "test-stress") ||
	       !strcmp(arg, "test-cmd") ||
	       !strcmp(arg, "export-result") ||
	       !strcmp(arg, "help");
}

int ruihua_rtbench_main(int argc, char **argv)
{
	return rtbench_command_main(argc, argv);
}

int ruihua_run_benchmark(const char *workload_name, double period_sec, int tasks)
{
	return rtbench_command_run_benchmark(workload_name, period_sec, tasks);
}

int rtbench(const char *command, double period, int tasks)
{
	char command_buf[512];
	char *argv[RTBENCH_RUIHUA_MAX_ARGS];
	int argc;

	if (command == NULL || command[0] == '\0') {
		rtbench_command_print_usage();
		return 0;
	}

	strncpy(command_buf, command, sizeof(command_buf) - 1);
	command_buf[sizeof(command_buf) - 1] = '\0';

	argv[0] = "rtbench";
	argc = split_command_line(command_buf, &argv[1],
				  RTBENCH_RUIHUA_MAX_ARGS - 1) + 1;

	if (argc == 2 && !is_standard_command_or_option(argv[1])) {
		return rtbench_command_run_benchmark(argv[1], period, tasks);
	}
	return rtbench_command_main(argc, argv);
}

int rtbench_test(void)
{
	return rtbench_command_run_benchmark("stub", 1.0, 1);
}

int rtbench_quick(const char *name)
{
	return rtbench_command_run_benchmark(name ? name : "stub", 0.5, 3);
}

int rtbench_help(void)
{
	rtbench_command_print_usage();
	return 0;
}

int rtbench_list(void)
{
	char *argv[] = { "rtbench", "--list" };
	return rtbench_command_main(2, argv);
}

int rtbench_stub(void)
{
	return rtbench_command_run_benchmark("stub", 0.1, 1);
}

int rtbench_busywait(void)
{
	return rtbench_command_run_benchmark("busywait", 0.1, 1);
}

int rtbench_ruihua_smoke(void)
{
	return rtbench_command_run_benchmark("ruihua-smoke", 0.1, 1);
}

int rtbench_test_all(void)
{
	char *argv[] = { "rtbench", "test-all" };
	return rtbench_command_main(2, argv);
}

int rtbench_test_all_multicore(void)
{
	char *argv[] = { "rtbench", "test-all", "--multicore" };
	return rtbench_command_main(3, argv);
}

int rtbench_run_workloads(void) {
	return run_all_workloads();
}

int rtbench_test_all_quick(void)
{
	char *argv[] = {
		"rtbench", "test-all", "--quick",
		"--no-realtime", "--no-stress",
	};
	return rtbench_command_main(5, argv);
}

int rtbench_test_schedule(void)
{
	char *argv[] = { "rtbench", "test-schedule" };
	return rtbench_command_main(2, argv);
}

int rtbench_test_schedule_quick(void)
{
	char *argv[] = { "rtbench", "test-schedule", "--quick" };
	return rtbench_command_main(3, argv);
}

int rtbench_test_schedule_cycles3(void)
{
	char *argv[] = { "rtbench", "test-schedule", "--cycles", "3" };
	return rtbench_command_main(4, argv);
}

int rtbench_test_realtime(void)
{
	char *argv[] = { "rtbench", "test-realtime" };
	return rtbench_command_main(2, argv);
}

int rtbench_test_realtime_multicore(void)
{
	char *argv[] = { "rtbench", "test-realtime", "--multicore" };
	return rtbench_command_main(3, argv);
}

int rtbench_test_realtime_verify(void)
{
	char *argv[] = { "rtbench", "test-realtime", "--verify" };
	return rtbench_command_main(3, argv);
}

int rtbench_test_stress_all(void)
{
	char *argv[] = { "rtbench", "test-stress", "--job", "all"};
	return rtbench_command_main(4, argv);
}

int rtbench_test_stress_all_quick(void)
{
	char *argv[] = { "rtbench", "test-stress", "--job", "all-quick"};
	return rtbench_command_main(4, argv);
}

int rtbench_test_stress_cpu(void)
{
	char *argv[] = { "rtbench", "test-stress", "--job", "cpu"};
	return rtbench_command_main(4, argv);
}

int rtbench_test_stress_memory(void)
{
	char *argv[] = { "rtbench", "test-stress", "--job", "memory"};
	return rtbench_command_main(4, argv);
}

int rtbench_test_stress_file(void)
{
	char *argv[] = { "rtbench", "test-stress", "--job", "file"};
	return rtbench_command_main(4, argv);
}
int rtbench_test_cmd(void)
{
	char *argv[] = { "rtbench", "test-cmd" };
	return rtbench_command_main(2, argv);
}

int rtbench_export_result(const char *output_path)
{
	char *argv[] = { "rtbench", "export-result", "-o", (char *)output_path };
	if (output_path == NULL || output_path[0] == '\0') {
		output_path = "/rtbench_result.json";
		argv[3] = (char *)output_path;
	}
	return rtbench_command_main(4, argv);
}

#ifndef RTBENCH_NO_STANDALONE_MAIN
int main(int argc, char **argv)
{
	return rtbench_command_main(argc, argv);
}
#endif
