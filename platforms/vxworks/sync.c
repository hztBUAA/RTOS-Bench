/**
 * @file sync.c
 * @brief VxWorks 信号量抽象（RTOS-Bench 参考实现脚手架）
 *
 * 契约（见 generator/platform_abstraction.h）：
 *   rtbench_sem_create(initial_value) -> rtbench_sem_t
 *   rtbench_sem_wait(sem)             -> 0 / -1   （无限等待）
 *   rtbench_sem_post(sem)             -> 0 / -1
 *   rtbench_sem_destroy(sem)          -> 0 / -1
 *
 * VxWorks 映射：semBCreate / semTake / semGive / semDelete（二进制信号量）。
 * 契约里的 initial_value 只取 0/非 0 两种语义，映射到 SEM_EMPTY / SEM_FULL。
 *
 * 注意：本文件在仓库的现有构建中不会被编译（见 platforms/vxworks/README.md）。
 */

#include "platform_abstraction.h"

#if defined(RTBENCH_PLATFORM_VXWORKS)

#include <vxWorks.h>
#include <semLib.h>
#include <stdlib.h>
#include <stddef.h>

struct rtbench_sem_internal {
    SEM_ID sem;
};

rtbench_sem_t rtbench_sem_create(unsigned int initial_value)
{
    struct rtbench_sem_internal *s;

    s = (struct rtbench_sem_internal *)calloc(1, sizeof(*s));
    if (s == NULL) {
        return NULL;
    }

    /* SEM_Q_FIFO：等待队列先进先出，最贴近 POSIX 信号量语义；
     * 若需要优先级继承（避免优先级反转）请改用 SEM_Q_PRIORITY。 */
    s->sem = semBCreate(SEM_Q_FIFO, initial_value ? SEM_FULL : SEM_EMPTY);
    if (s->sem == NULL) {
        free(s);
        return NULL;
    }

    return (rtbench_sem_t)s;
}

int rtbench_sem_wait(rtbench_sem_t sem)
{
    struct rtbench_sem_internal *s = (struct rtbench_sem_internal *)sem;

    if (s == NULL || s->sem == NULL) {
        return -1;
    }

    return (semTake(s->sem, WAIT_FOREVER) == OK) ? 0 : -1;
}

int rtbench_sem_post(rtbench_sem_t sem)
{
    struct rtbench_sem_internal *s = (struct rtbench_sem_internal *)sem;

    if (s == NULL || s->sem == NULL) {
        return -1;
    }

    return (semGive(s->sem) == OK) ? 0 : -1;
}

int rtbench_sem_destroy(rtbench_sem_t sem)
{
    struct rtbench_sem_internal *s = (struct rtbench_sem_internal *)sem;

    if (s == NULL) {
        return -1;
    }

    if (s->sem != NULL) {
        semDelete(s->sem);
    }

    free(s);
    return 0;
}

#endif /* RTBENCH_PLATFORM_VXWORKS */
