/*
 * Dongtu/Intewell shell binding for RTOS-Bench.
 *
 * The command parser lives in generator/dongtu_entry.c. This file only binds
 * it to the platform shell so Dongtu projects can keep RTOS-Bench sources in
 * the upstream repository and avoid project-local copies.
 */

#include <stddef.h>

#include <cmd.h>
#include <ttosShell.h>

extern int rtbench_dongtu_entry(int argc, char **argv);

static const struct shell *g_rtbench_shell;

const struct shell *rtbench_dongtu_current_shell(void)
{
	return g_rtbench_shell;
}

static int rtbench_shell_cmd(const struct shell *shell, size_t argc, char **argv)
{
	const struct shell *previous_shell = g_rtbench_shell;
	int ret;

	g_rtbench_shell = shell;
	ret = rtbench_dongtu_entry((int)argc, argv);
	g_rtbench_shell = previous_shell;

	return ret;
}

SHELL_CMD_REGISTER(rtbench, NULL,
		   "Usage: rtbench [subcommand] [options]",
		   "RTOS-Bench", rtbench_shell_cmd);
