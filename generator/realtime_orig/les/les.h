/* les.h
 * 包括精确时间获取功能、插桩功能。
 */

#ifndef __LES_H__
#define __LES_H__

#include "platform_macro.h"
#include "cpu_affinity.h"

#include "data_tools.h"
#include "safe_sleep.h"
#include "test_list.h"

/* 根据POSIX标准和定义优先级，
 * 根据具体实现进行调整
 */
#if defined(RT_THREAD_PLATFORM)

#define BENCHMARK_HIGH_PRIO		14
#define BENCHMARK_MIDDLE_PRIO	15
#define BENCHMARK_LOW_PRIO		16

#else

#define BENCHMARK_HIGH_PRIO		16
#define BENCHMARK_MIDDLE_PRIO	15
#define BENCHMARK_LOW_PRIO		14

#endif

/* 系统调用延迟开关：1-关闭，0-开启 */
#if defined(ONEOS_PLATFORM)

#define LES_NO_GETPID 1

#else

#define LES_NO_GETPID 0

#endif



/* 默认缓冲区大小 */
#define LES_BUFFER_SIZE 1024

#define GETTIMEVAL_EOK 0
#define GETTIMEVAL_ERROR -1

/* 声明全局变量 */
extern volatile uint64_t LES_buffer[LES_BUFFER_SIZE];
extern volatile uint32_t LES_offset;
extern volatile uint64_t LES_syscall_val;
extern volatile uint64_t LES_interrupt_start_val;
extern volatile uint64_t LES_interrupt_end_val;

extern volatile uint32_t LES_flag;
extern volatile uint32_t LES_syscall_flag;
extern volatile uint32_t LES_interrupt_flag;

/* 启动计时器 */
/* 此函数默认为空，除非特定架构下的计时器有手动开启的需要 */
static inline void LES_start_timer(void) {
#if defined(__aarch64__)
    /* ... */

#elif defined(_M_X64) || defined(__x86_64__)
    /* ... */

#elif defined(__riscv)
    /* ... */

#elif defined(__loongarch__)
    /* ... */

#else
    /* ... */
#endif
}

/* 计时器频率获取函数 */
static inline uint64_t freqGet(void) {
    uint64_t freq = 100000000;
#if defined(__aarch64__)
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(freq));

#elif defined(_M_X64) || defined(__x86_64__)
    #if defined(SYLIXOS_PLATFORM)
        freq = 3599453234;
    #else
        freq = 3599453234;
    #endif

#elif defined(__riscv)
    /* ... */

#elif defined(__loongarch__)
    uint32_t val;
    __asm__ volatile("cpucfg %0, %1" : "=r"(val) : "r"(0x4));
    freq = (uint64_t)val; 

#else
    #warning "未知架构，请确保正确地获取了计时器频率"
    /* freq = ... */
#endif
    return freq;
}

/* 计时器时间获取函数 */
static inline uint64_t timeGet(void) {
    uint64_t val = 0;

#if defined(__aarch64__)
    __asm__ volatile("mrs %0, cntpct_el0" : "=r"(val));

#elif defined(_M_X64) || defined(__x86_64__)
    #if defined(_M_X64)
        #include <intrin.h>
        val = __rdtsc();
    #else
        uint32_t low, high;
        __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
        val = ((uint64_t)high << 32) | low;
    #endif

#elif defined(__riscv)
    #if __riscv_xlen == 64
        __asm__ volatile("rdtime %0" : "=r"(val));
    #else
        uint32_t low, high, temp;
        do {
            __asm__ volatile("rdtimeh %0" : "=r"(high));
            __asm__ volatile("rdtime %0" : "=r"(low));
            __asm__ volatile("rdtimeh %0" : "=r"(temp));
        } while (temp != high);
        val = ((uint64_t)high << 32) | low;
    #endif

#elif defined(__loongarch__)
    __asm__ volatile("rdtime.d %0, $zero" : "=r"(val));

#else
    #warning "未知架构，请确保正确地获取了计时器值"
    /* val = ... */
#endif
    return val;
}

/* 系统服务插桩函数 */
#if defined(DONGTU_PLATFORM)
void LES_stub(void);
#else
static inline void LES_stub(void) {
    if (LES_flag == 0 || bench_get_cpu() != 0) {
        return;
    }
    if (LES_offset >= LES_BUFFER_SIZE) {
        return;
    }
    LES_buffer[LES_offset] = timeGet();
    LES_offset = LES_offset + 1;
}
#endif

/* 系统调用插桩函数 */
static inline void LES_syscall_stub(void) {
    if (LES_syscall_flag == 1) {
        LES_syscall_val = timeGet();
    }
}

/* 中断插桩函数 */
/* LES_interrupt_start_stub 的功能（通常）在汇编代码中完成 */
void LES_interrupt_end_stub(void);

/* 系统服务插桩控制API */
void LES_enable(void);
void LES_disable(void);
uint32_t LES_getOffset(void);
int LES_getTimeVal(uint32_t loc, uint64_t *out_val);

/* 系统调用插桩控制API */
void LES_syscall_enable(void);
void LES_syscall_disable(void);
uint64_t LES_get_syscall_val(void);

/* 中断插桩控制API */
void LES_interrupt_stub_enable(void);
void LES_interrupt_stub_disable(void);

#endif /* __LES_H__ */
