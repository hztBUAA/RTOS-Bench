/**
 * @file signal.c
 * @brief VxWorks 信号/事件抽象（RTOS-Bench 参考实现脚手架）
 *
 * 契约（见 generator/platform_abstraction.h）：
 *   rtbench_signal_register(signal, handler) -> 0 / -1
 *   另有内部辅助 rtbench_signal_trigger()，供定时器回调把事件分发给已注册处理器。
 *
 * 启用 POSIX 信号路径（VXWORKS_POSIX/INCLUDE_POSIX_SIGNALS）时，把
 * RTBENCH_SIG_DEADLINE/PERIOD/QUIT 映射到 SIGALRM/SIGUSR1/SIGINT 即可，
 * 参考 generator/platform/posix-lite/signal.c。
 * 若目标工程是内核态模块（DKM）且未启用 POSIX 信号，请改用
 * sigqueue / 任务通知（taskLib 的事件）实现，并把 RTBENCH_VXWORKS_POSIX_SIGNALS
 * 关掉 —— 此时 register 返回 -1，rt-bench 会自动退化为不使用信号。
 *
 * 注意：本文件在仓库的现有构建中不会被编译（见 platforms/vxworks/README.md）。
 */

#include "platform_abstraction.h"

#if defined(RTBENCH_PLATFORM_VXWORKS)

#include <stddef.h>

static rtbench_signal_handler_t g_handlers[3] = { NULL, NULL, NULL };

int rtbench_signal_register(rtbench_signal_t signal,
                            rtbench_signal_handler_t handler)
{
    if (signal > RTBENCH_SIG_QUIT) {
        return -1;
    }
    g_handlers[signal] = handler;
    return 0;
}

/* 供定时器回调调用；把事件转发给注册的处理器。 */
void rtbench_signal_trigger(rtbench_signal_t signal, void *context)
{
    if (signal <= RTBENCH_SIG_QUIT && g_handlers[signal] != NULL) {
        g_handlers[signal](signal, context);
    }
}

#endif /* RTBENCH_PLATFORM_VXWORKS */
