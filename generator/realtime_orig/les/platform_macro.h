#ifndef __PLATFORM_MACRO_H__
#define __PLATFORM_MACRO_H__

/*
 * Platform Macro Definination
 * If RT-Thread, define RT_THREAD_PLATFORM
 * If SylixOS, define SYLIXOS_PLATFORM
 * If OneOS, define ONEOS_PLATFORM
 * If ReWorks, define RUIHUA_PLATFORM
 * If Intewell, define DONGTU_PLATFORM
 * If openEuler board, define OPENEULER_PLATFORM
 *
 */
//#define RT_THREAD_PLATFORM

#if !defined(OPENEULER_PLATFORM) && \
	(defined(OPENEULER_FEITENG) || defined(OPENEULER_ORANGEPI))
#define OPENEULER_PLATFORM
#endif

/*
 * openEuler 镜像的内核自带 LES 插桩：插桩数据不在用户态进程里，而是由内核
 * 维护并通过 debugfs 暴露出来（见 D:\OS\OpenEuler\插桩使用说明.md）：
 *
 *     /sys/kernel/debug/les_latency/
 *         reset  sched_enable  syscall_enable  interrupt_enable
 *         offset  buffer  syscall_val  interrupt_start_val
 *         interrupt_end_val  counter_freq
 *
 * 所以在本平台上：
 *   1. 用户态不能再定义或查找 LES 变量（其它 RT 板卡"用 extern 找符号"的做法
 *      在这里不适用），要改成读写上面的调试节点；
 *   2. EL0 不允许读物理计数器 CNTPCT_EL0（内核 CNTKCTL_EL1.EL0PCTEN=0，
 *      读了会 UNDEFINED -> SIGILL），计时必须改用虚拟计数器 CNTVCT_EL0。
 */
#if defined(OPENEULER_PLATFORM) && !defined(LES_KERNEL_DEBUGFS)
#define LES_KERNEL_DEBUGFS 1

/* 内核插桩节点目录（如换位置可用 -DLES_KERNEL_DEBUGFS_DIR=\"...\" 覆盖） */
#ifndef LES_KERNEL_DEBUGFS_DIR
#define LES_KERNEL_DEBUGFS_DIR "/sys/kernel/debug/les_latency"
#endif
#endif

#endif
