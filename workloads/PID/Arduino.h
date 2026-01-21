#ifndef ARDUINO_MOCK_H
#define ARDUINO_MOCK_H

#include <stdint.h>
#include <string.h>
#include <math.h>

// 1. 基础类型模拟
typedef uint8_t byte;

// 2. 伪造时间变量 (声明外部变量，告诉编译器它在别的地方定义了)
// 使用 volatile 至关重要，防止 PID 库读取时被编译器优化成常量
extern volatile unsigned long g_mock_millis;

// 3. 声明 millis() 函数 (不要在这里写实现！)
// 加上 extern "C" 是为了防止 C++ Name Mangling，确保符号名洁净
#ifdef __cplusplus
extern "C" {
#endif

    unsigned long millis(void);

#ifdef __cplusplus
}
#endif

// 4. 常用宏
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define abs(x) ((x)>0?(x):-(x))

#endif