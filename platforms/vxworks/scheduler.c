/**
 * @file scheduler.c
 * @brief VxWorks 调度属性抽象（RTOS-Bench 参考实现脚手架）
 *
 * 契约（见 generator/platform_abstraction.h）：
 *   rtbench_set_priority(priority)               -> 0 / -1
 *   rtbench_set_deadline(runtime, deadline, period) -> 0 / -1（不支持时返回 -1）
 *   rtbench_set_affinity(cpu_mask)               -> 0 / -1
 *
 * 契约允许 rtbench_set_deadline() 返回 -1 表示"本平台无 deadline 调度"，
 * 统计层仍会用 deadline 判定 miss，因此 VxWorks 上只配优先级即可出分。
 *
 * 注意：本文件在仓库的现有构建中不会被编译（见 platforms/vxworks/README.md）。
 */

#include "platform_abstraction.h"

#if defined(RTBENCH_PLATFORM_VXWORKS)

#include <vxWorks.h>
#include <taskLib.h>
#include <stdint.h>

int rtbench_set_priority(unsigned int priority)
{
    /* VxWorks 与 POSIX 相反：数值越小优先级越高（默认 256 级，0 最高 255 最低）。
     * rt-bench 传入的 priority 直接透传；若你的入口按 POSIX 语义给值，
     * 需要在此处做一次取反映射。 */
    return (taskPrioritySet(taskIdSelf(), (int)priority) == OK) ? 0 : -1;
}

int rtbench_set_deadline(uint64_t runtime, uint64_t deadline, uint64_t period)
{
    /* VxWorks 没有原生 SCHED_DEADLINE 等价物（optional deadline scheduling）。 */
    (void)runtime;
    (void)deadline;
    (void)period;
    return -1;
}

int rtbench_set_affinity(uint32_t cpu_mask)
{
#if defined(VX_SMP)
    /* SMP 内核专用：cpuset_t 的构造方式在不同 VxWorks 版本间有差异
     * （VxWorks 7 用 CPUSET_* 宏，6.9 部分版本仍在用旧的 int 掩码），
     * 请按你的 SDK 替换下面两行。 */
    int tid = taskIdSelf();
    cpuset_t cpuset;

    CPUSET_ZERO(cpuset);
    for (int cpu = 0; cpu < 32; cpu++) {
        if (cpu_mask & (1u << cpu)) {
            CPUSET_ADD(cpuset, cpu);
        }
    }
    return (taskCpuAffinitySet(tid, cpuset) == OK) ? 0 : -1;
#else
    /* 单核内核：无亲和性概念。返回 0 让上层继续，不打断测试流程。 */
    (void)cpu_mask;
    return 0;
#endif
}

#endif /* RTBENCH_PLATFORM_VXWORKS */
