/* platforms/rtt_entry.c */
#include <rtthread.h>
#include "../common/stress-ng.h"

static void cmd_rtos_stress(int argc, char **argv)
{
    stress_ng_main(argc, argv);
}
MSH_CMD_EXPORT_ALIAS(cmd_rtos_stress, rtos_stress, stress-ng for RT-Thread);

