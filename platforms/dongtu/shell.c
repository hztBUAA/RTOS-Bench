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

static int rtbench_shell_cmd(const struct shell *shell, size_t argc, char **argv)
{
	(void)shell;
	return rtbench_dongtu_entry((int)argc, argv);
}

SHELL_CMD_REGISTER(rtbench, NULL,
		   "Usage: rtbench [subcommand] [options]",
		   "RTOS-Bench", rtbench_shell_cmd);
