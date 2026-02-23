/*
 * RTOS-Bench entry point for RT-Thread
 * Registers the rtbench command with msh
 */

#include <rtthread.h>

/* RT-Thread entry from generator */
extern int rtosbench_rtthread_entry(int argc, char **argv);

/* MSH command wrapper */
static int cmd_rtbench(int argc, char **argv)
{
    return rtosbench_rtthread_entry(argc, argv);
}

/* Register command with msh */
MSH_CMD_EXPORT_ALIAS(cmd_rtbench, rtbench, RTOS-Bench workload runner);
