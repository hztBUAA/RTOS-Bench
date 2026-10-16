/* 主机单测：验证 stress_is_fd_exhausted() 的判定集合。
 * 编译（容器内）：gcc -I../osal -o /tmp/test_fd_exhausted test_fd_exhausted.c */
#include "../osal/stress_osal.h"
#include <assert.h>
#include <stdio.h>

int main(void)
{
    /* fd 上限：三者都必须判为耗尽 */
    assert(stress_is_fd_exhausted(EMFILE) == 1);
    assert(stress_is_fd_exhausted(ENFILE) == 1);
    assert(stress_is_fd_exhausted(ENOMEM) == 1);   /* RT-Thread 的实际返回值 */

    /* 非 fd 上限：必须判为 0，否则退避会误伤 */
    assert(stress_is_fd_exhausted(ENOENT) == 0);
    assert(stress_is_fd_exhausted(ENOSPC) == 0);
    assert(stress_is_fd_exhausted(EEXIST) == 0);
    assert(stress_is_fd_exhausted(0) == 0);

    /* errno 归一化：负值（RT-Thread）与正值（POSIX）都要得到正值 */
    assert(stress_errno_posix(-ENOMEM) == ENOMEM);
    assert(stress_errno_posix(-ENOSPC) == ENOSPC);
    assert(stress_errno_posix(-ENOENT) == ENOENT);
    assert(stress_errno_posix(ENOMEM) == ENOMEM);
    assert(stress_errno_posix(ENOSPC) == ENOSPC);
    assert(stress_errno_posix(0) == 0);

    printf("test_fd_exhausted: OK\n");
    return 0;
}
