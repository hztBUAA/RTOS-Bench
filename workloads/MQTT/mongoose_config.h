#ifndef MONGOOSE_CONFIG_H
#define MONGOOSE_CONFIG_H

// 1. 定义架构为 RT-Thread
#define MG_ARCH MG_ARCH_RTTHREAD

// 2. 引入 RT-Thread 核心头文件
#include <rtthread.h>

// 3. --- 核心修复开始 ---
// 在包含任何其他文件之前，先包含 sys/socket.h
// 这有助于让编译环境先确定 socket 的标准定义
#include <sys/socket.h>

// 如果使用的是 LwIP，必须处理 select 函数的冲突
// 这里我们包含 netdb.h，通常它会间接包含正确的 socket 定义
#include <netdb.h>

// 欺骗编译器，假装 <sys/select.h> 已经被包含过了。
// 这样当 Mongoose 内部试图 #include <sys/select.h> 时，预处理器会直接跳过。
// 注意：不同的编译器/工具链宏名称可能不同，Newlib 通常是 _SYS_SELECT_H
#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H
#endif

// 处理 FD_SET 重定义警告 (可选，让输出更干净)
#if defined(FD_SET)
#undef FD_SET
#undef FD_CLR
#undef FD_ISSET
#undef FD_ZERO
#endif
// 3. --- 核心修复结束 ---

#endif // MONGOOSE_CONFIG_H