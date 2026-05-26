/*
* @file閿熸枻鎷穟serAppInit.cpp
* @brief閿熸枻鎷�
*	    <li>閿熺煫浼欐嫹閿熸枻鎷烽敓鏂ゆ嫹鍕熼敓鏂ゆ嫹閿熺惮serAppInit()閿熸枻鎷烽敓鐭紮鎷烽敓鏂ゆ嫹閿熸枻鎷疯閿熸枻鎷烽敓鏂ゆ嫹閿燂拷</li>
* @implements閿熸枻鎷�
*/

/************************澶� 閿熸枻鎷� 閿熸枻鎷�******************************/
#include <commonTypes.h>
#include <syscallIoctl.h>
#include "dongtu_entry.c"

/************************閿熸枻鎷� 閿熸枻鎷� 閿熸枻鎷�******************************/
/************************閿熸枻鎷烽敓閰佃鎷烽敓鏂ゆ嫹******************************/
/************************閿熻В閮ㄩ敓鏂ゆ嫹閿熸枻鎷�******************************/
/************************鍓嶉敓鏂ゆ嫹閿熸枻鎷烽敓鏂ゆ嫹******************************/
/************************妯￠敓鏂ゆ嫹閿熸枻鎷烽敓锟�******************************/
/************************鍏ㄩ敓琛楁唻鎷烽敓鏂ゆ嫹******************************/
/************************瀹�   閿熸枻鎷�*******************************/
T_UWORD logAddr = 0;
T_UWORD sysTimerVectorInt=0;
int userAppInit(void)
{
	VMK_Ioctl(LOG_ADDR_GET, &logAddr, &sysTimerVectorInt);
	debugEventInit((T_VOID *)logAddr, 0x8000*4, 0xffffffff);
	/* argv: argv[0] should be program name, argv[1] the subcommand */
	char *argv[] = { "rtbench", "test-stress", "--job", "file", NULL };
	int argc = 0;
	while (argv[argc] != NULL) {
		argc++;
	}
	rtbench_dongtu_entry(argc, argv);
    return 0;
}
void intewell_stub() {

}
