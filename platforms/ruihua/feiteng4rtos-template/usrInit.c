#define CONFIGURE_INIT
#include "system.h"

extern void rtosbench_boot_smoke(void);

void UserInit(void)
{
	printf("[feiteng4rtos] UserInit enter\n");
	rtosbench_boot_smoke();
	printf("[feiteng4rtos] UserInit leave\n");
}
