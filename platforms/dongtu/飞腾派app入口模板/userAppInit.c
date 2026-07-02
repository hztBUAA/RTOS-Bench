/*
* @file：userAppInit.c
* @brief：
*	    <li>用户入口文件，userAppInit()是用户的入口函数。</li>
* @implements：
*/

/************************头 文 件******************************/
#include <commonTypes.h>
#include <sysCallIoctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <vbspEx.h>
#include <debugInfo.h>
#include <ttosShell.h>
#include <les_common.h>

/************************宏 定 义******************************/
/************************类型定义******************************/
/************************外部声明******************************/
/************************前向声明******************************/
/************************模块变量******************************/
/************************全局变量******************************/
/************************实   现*******************************/


T_UDWORD logAddr;
unsigned int sysTimerVectorInt;
extern void rttest(void);
extern void realtime_print(void);
extern void realtime_init(void);

extern void multicore_init(void);
extern void multicore_print(void);

extern volatile uint64_t *__LES_syscall_val;
extern volatile uint64_t *__LES_interrupt_start_val;
extern volatile uint64_t *__LES_interrupt_end_val;
extern volatile uint32_t *__LES_syscall_flag;
extern volatile uint32_t *__LES_interrupt_flag;

static void rtinit()
{
    T_UWORD intnum;
    T_WORD ret;
    T_UWORD vmID;
    ADDRESS tmp;
    T_UDWORD phyaddr;
     debugEventHeader_t *stDEHeaderPtr = NULL;
    ret = VMK_Ioctl(LOG_ADDR_GET, &logAddr, &intnum);
    if(ret != 0)
    {
      printk("%s %d ret:%d\n",__func__,__LINE__,ret);
      return;
    }
	VMK_GetVMID(NULL, &vmID);
    VMK_GetVMPhysicalAddress (vmID, ( ADDRESS )logAddr, (T_UDWORD *)&phyaddr);

    stDEHeaderPtr = (debugEventHeader_t *)logAddr;//这里我不知道是啥情况，这里我改掉的。add for test
    __LES_interrupt_start_val = (volatile uint64_t *)&stDEHeaderPtr->allirq_val;
    __LES_interrupt_end_val =  (volatile uint64_t *)&stDEHeaderPtr->systimeirq_val;
    __LES_interrupt_flag = (volatile uint32_t *)&stDEHeaderPtr->interrupt_flag;
    __LES_syscall_val =(volatile uint64_t *)&stDEHeaderPtr->LES_syscall_val;
    __LES_syscall_flag = (volatile uint32_t *)&stDEHeaderPtr->LES_syscall_flag;
    //realtime_init();
    //realtime_print();
}

int userAppInit(void)
{
	printf("hello world, kyland.\n");
	//ssk运行第一个线程，进入osInit函数，此时已经建立好共享内存，通过该系统的Ioctl接口，可以获得共享内存mem的地址，以及定时器tick心跳的向量。
	/* 获取logaddr */
	VMK_Ioctl(LOG_ADDR_GET, &logAddr, &sysTimerVectorInt);
	printk("%s[%d]: logAddr = 0x%x sysTimerVectorInt=%d\n", __func__, __LINE__, logAddr,sysTimerVectorInt);//add for test, add by lin
	/* 在TTOS中初始化 */
	debugEventInit((T_VOID *)logAddr, 0x8000*4, 0xffffffff);
	/* 初始化 */
	rtinit();
    return 0;
}

