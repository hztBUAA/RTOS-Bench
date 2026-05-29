/**
 * @file ruihua_entry.c
 * @brief Ruihua/ReWorks shell entry for RTOS-Bench.
 *
 * Keep ReWorks shell syntax here. Command behavior lives in rtbench_command.c
 * and platform APIs live under generator/platform/ruihua.
 */

#include "rtbench_command.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define RTBENCH_RUIHUA_MAX_ARGS 32

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

int rtbench_test_stress(void)
{
	char *argv[] = { "rtbench", "test-stress" };
	return rtbench_command_main(2, argv);
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
