/**
 * @file linux_entry.c
 * @brief Linux/openEuler entry for the shared RTOS-Bench command dispatcher.
 */

#include "rtbench_command.h"

#include <stddef.h>

const char *rtbench_platform_default_output_path(void)
{
	return "./rtbench_result.json";
}

void rtbench_platform_get_env(struct rtbench_platform_env *env)
{
	if (env == NULL) {
		return;
	}

	env->os_name = "openEuler/Linux";
	env->os_version = "unknown";

#if defined(OPENEULER_FEITENG)
	env->board = "Phytium Pi / E2000Q";
#elif defined(OPENEULER_ORANGEPI)
	env->board = "Orange Pi / RK3588";
#elif defined(__aarch64__)
	env->board = "openEuler AArch64";
#elif defined(__x86_64__)
	env->board = "openEuler x86_64";
#elif defined(__riscv)
	env->board = "openEuler RISC-V";
#elif defined(__loongarch__)
	env->board = "openEuler LoongArch";
#else
	env->board = "openEuler board";
#endif

#if defined(__aarch64__)
	env->cpu_type = "aarch64";
#elif defined(__x86_64__)
	env->cpu_type = "x86_64";
#elif defined(__riscv)
	env->cpu_type = "riscv";
#elif defined(__loongarch__)
	env->cpu_type = "loongarch";
#else
	env->cpu_type = "unknown";
#endif

	env->cpu_freq_mhz = 0;
	env->cpu_core_num = 0;
}

int main(int argc, char **argv)
{
	return rtbench_command_main(argc, argv);
}
