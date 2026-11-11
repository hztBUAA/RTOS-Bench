#include <commonTypes.h>
#include <syscallIoctl.h>

extern int rtbench_dongtu_bind_les(void);

T_UDWORD logAddr = 0;
T_UWORD sysTimerVectorInt = 0;
int userAppInit(void)
{
	T_WORD ret;

	ret = VMK_Ioctl(LOG_ADDR_GET, &logAddr, &sysTimerVectorInt);
	if (ret != 0) {
		printk("[rtbench] LOG_ADDR_GET failed: %d\n", ret);
		return -1;
	}

	printk("[rtbench] LOG_ADDR_GET=0x%llx intnum=%u\n",
	       (unsigned long long)logAddr,
	       (unsigned int)sysTimerVectorInt);

	if (rtbench_dongtu_bind_les() != 0) {
		printk("[rtbench] Dongtu LES binding failed\n");
		return -1;
	}

	/*
	 * Binding is complete. Run benchmarks explicitly from the shell, for
	 * example: rtbench test-realtime --delay
	 */
	return 0;
}
