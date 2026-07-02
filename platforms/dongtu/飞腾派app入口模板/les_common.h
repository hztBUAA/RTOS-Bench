#ifndef LES_COMMON_H
#define LES_COMMON_H

/* 事件记录块 */
typedef struct {
	/* 时间戳 */
    T_UDWORD timeStamp;
    /* 事件ID */
    T_ULONG  event;
    /* 事件记录参数 */
    T_ULONG  parameter[3];
} debugEvent_t;


/* 事件记录控制头，需要保证8字节对齐 */
typedef struct {
	/* 总事件记录个数 */
    T_ULONG  totalEvent;
	/* 当前事件记录个数 */
    T_ULONG  eventCnt;
	/* 多核系统下允许记录事件的CPU掩码 */
    T_ULONG  recordCpuMask;
    /* 当前事件记录索引号 */
    T_ULONG  writeIdx;
    /* 事件记录开关 */
    T_ULONG  recordFlag;
    /* 事件块数组指针 */
    debugEvent_t *event;

	T_ULONG interrupt_flag; //开关中断延迟测试的标志位
	T_ULONG LES_syscall_flag; //系统调用测试的标志位
	T_UDWORD LES_syscall_val; //用来记录系统调用插桩点的count值
	T_UDWORD allirq_val; //用来记录tick中断来临时，中断总入口记录的count值 ? ?
	T_UDWORD systimeirq_val; //用来记录tick中断执行到c函数入口的count值
} debugEventHeader_t;
/************************外部声明******************************/
/************************前向声明******************************/
/************************模块变量******************************/

#endif
