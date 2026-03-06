#ifndef MONGOOSE_CONFIG_H
#define MONGOOSE_CONFIG_H

// 定义架构为 Unix/Linux (使用musl工具链)
#define MG_ARCH MG_ARCH_UNIX

// 包含必要的网络头文件
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

// 启用必要的特性
#define MG_ENABLE_SOCKET 1
#define MG_ENABLE_EPOLL 1

#endif // MONGOOSE_CONFIG_H