/*
* @file��userAppInit.cpp
* @brief��
*	    <li>�û�����ļ���userAppInit()���û�����ں�����</li>
* @implements��
*/

/************************ͷ �� ��******************************/
#include <commonTypes.h>
#include <syscallIoctl.h>
#include "dongtu_entry.c"

/************************�� �� ��******************************/
/************************���Ͷ���******************************/
/************************�ⲿ����******************************/
/************************ǰ������******************************/
/************************ģ�����******************************/
/************************ȫ�ֱ���******************************/
/************************ʵ   ��*******************************/
T_UWORD logAddr = 0;
T_UWORD sysTimerVectorInt=0;
int userAppInit(void)
{
	VMK_Ioctl(LOG_ADDR_GET, &logAddr, &sysTimerVectorInt);
	debugEventInit((T_VOID *)logAddr, 0x8000*4, 0xffffffff);
	/* argv: argv[0] should be program name, argv[1] the subcommand */
	char *argv[] = { "rtbench", "test-schedule", NULL };
	int argc = 0;
	while (argv[argc] != NULL) {
		argc++;
	}
	rtbench_dongtu_entry(argc, argv);
    return 0;
}
/*
void intewell_stub() {

}
*/