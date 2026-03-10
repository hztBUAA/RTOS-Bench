#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>
#include "safe_sleep.h"

/* 安全睡眠 */
void safe_usleep(uint64_t us) {
    struct timespec req, rem;

    // 转换微秒到秒和纳秒
    req.tv_sec = us / 1000000;
    req.tv_nsec = (us % 1000000) * 1000L;

    while (nanosleep(&req, &rem) == -1 && errno == EINTR) {
        req = rem;
    }
}
