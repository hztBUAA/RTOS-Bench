/*
 * RTOS-Bench entry point for RT-Thread
 * Registers the rtos_bench command with msh
 */

#ifdef RT_THREAD_PLATFORM

#include <rtthread.h>

/* RT-Thread entry from generator */
extern int rtosbench_rtthread_entry(int argc, char **argv);

/* MSH command wrapper */
static int cmd_rtbench(int argc, char **argv)
{
    return rtosbench_rtthread_entry(argc, argv);
}

/* Register command with msh */
MSH_CMD_EXPORT_ALIAS(cmd_rtbench, rtos_bench, RTOS-Bench workload runner);

#endif