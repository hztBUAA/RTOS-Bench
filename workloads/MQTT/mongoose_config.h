#ifndef MONGOOSE_CONFIG_H
#define MONGOOSE_CONFIG_H

// 包含必要的网络头文件
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

// 启用必要的特性
#define MG_ENABLE_SOCKET 1

#ifdef ONEOS_PLATFORM
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

#endif // MONGOOSE_CONFIG_H