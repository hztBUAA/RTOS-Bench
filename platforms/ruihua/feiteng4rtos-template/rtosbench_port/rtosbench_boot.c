#include <stdio.h>

extern int rtbench_help(void);
extern int rtbench_list(void);
extern int rtbench_test_schedule_quick(void);

void rtosbench_boot_smoke(void)
{
	printf("[rtos-bench] Ruihua boot integration active\n");
	rtbench_help();
	rtbench_list();
	rtbench_test_schedule_quick();
}
