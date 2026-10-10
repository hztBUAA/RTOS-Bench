#ifndef MONGOOSE_CONFIG_H
#define MONGOOSE_CONFIG_H

// 包含必要的网络头文件
#include <sys/socket.h>
#include <netinet/in.h>

/* Dongtu Intewell builds link lwip. Dongtu Linux/musl builds (LoongArch) do
 * not have lwip headers, so they set RTBENCH_DONGTU_LINUX and use POSIX. */
#if defined(DONGTU_PLATFORM) && !defined(RTBENCH_DONGTU_LINUX)
#include <lwip/tcp.h>
#else
#include <netinet/tcp.h>
#endif

#if defined(DONGTU_PLATFORM) && !defined(RTBENCH_DONGTU_LINUX)
#include <lwip/inet.h>
#else
#include <arpa/inet.h>
#endif

#include <netdb.h>
#include <unistd.h>

// 启用必要的特性
#define MG_ENABLE_SOCKET 1

#if defined(ONEOS_PLATFORM) || (defined(DONGTU_PLATFORM) && !defined(RTBENCH_DONGTU_LINUX)) || defined(RUIHUA_PLATFORM)
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#define MG_ENABLE_EPOLL 0
#else
#define MG_ARCH MG_ARCH_UNIX
#define MG_ENABLE_EPOLL 1
#endif
#endif // MONGOOSE_CONFIG_H