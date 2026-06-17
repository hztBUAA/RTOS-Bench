/*
 * Dongtu/Intewell shell command binding for RTOS-Bench.
 *
 * This file is intentionally linked directly into the final Intewell image,
 * rather than archived into librtosbench_x86.a / librtosbench_vm3588.a.
 *
 * Reason:
 *   SHELL_CMD_REGISTER creates a shell command registration object. If this
 *   file is stored only inside a static library, the linker may not extract it
 *   unless some other object has an unresolved reference to a symbol in this
 *   object. Linking shell.o directly keeps the shell command registration in
 *   the final image.
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
