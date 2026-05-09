/**
 * @file posixlite_entry.c
 * @brief Thin POSIX-lite entry for the shared RTOS-Bench dispatcher.
 */

#include "rtbench_command.h"

int main(int argc, char **argv)
{
	return rtbench_command_main(argc, argv);
}
