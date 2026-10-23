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

#elif defined(DONGTU_PLATFORM)

#define BENCHMARK_HIGH_PRIO		250
#define BENCHMARK_MIDDLE_PRIO	120
#define BENCHMARK_LOW_PRIO		10

#else

#ifndef BENCHMARK_HIGH_PRIO
#define BENCHMARK_HIGH_PRIO		16
#endif
#ifndef BENCHMARK_MIDDLE_PRIO
#define BENCHMARK_MIDDLE_PRIO	15
#endif
#ifndef BENCHMARK_LOW_PRIO
#define BENCHMARK_LOW_PRIO		14
#endif

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
#if defined(LES_KERNEL_DEBUGFS)

/*
 * 内核插桩（openEuler）：插桩数据在内核侧，用户态既不定义 LES_* 变量，
 * 也没有可 extern 的符号，只能通过 debugfs 节点读取。下面这几个访问函数
 * 在 les.c 中实现，读取节点时对外仍以"变量"形式暴露，调用点无需改动。
 */
uint64_t LES_kernel_interrupt_start_val(void);
uint64_t LES_kernel_interrupt_end_val(void);
uint64_t LES_kernel_syscall_val(void);

#define LES_interrupt_start_val LES_kernel_interrupt_start_val()
#define LES_interrupt_end_val   LES_kernel_interrupt_end_val()
#define LES_syscall_val         LES_kernel_syscall_val()

#elif defined(DONGTU_PLATFORM)

extern volatile uint64_t LES_buffer[LES_BUFFER_SIZE];
extern volatile uint32_t LES_offset;
extern volatile uint64_t *__LES_syscall_val;
extern volatile uint64_t *__LES_interrupt_start_val;
extern volatile uint64_t *__LES_interrupt_end_val;
extern volatile uint32_t LES_flag;
extern volatile uint32_t *__LES_syscall_flag;
extern volatile uint32_t *__LES_interrupt_flag;

#define LES_syscall_val (*__LES_syscall_val)
#define LES_interrupt_start_val (*__LES_interrupt_start_val)
#define LES_interrupt_end_val (*__LES_interrupt_end_val)
#define LES_syscall_flag (*__LES_syscall_flag)
#define LES_interrupt_flag (*__LES_interrupt_flag)

#else

extern volatile uint64_t LES_buffer[LES_BUFFER_SIZE];
extern volatile uint32_t LES_offset;
extern volatile uint64_t LES_syscall_val;
extern volatile uint64_t LES_interrupt_start_val;
extern volatile uint64_t LES_interrupt_end_val;
extern volatile uint32_t LES_flag;
extern volatile uint32_t LES_syscall_flag;
extern volatile uint32_t LES_interrupt_flag;

#endif

/* 启动计时器 */
/* 此函数默认为空，除非特定架构下的计时器有手动开启的需要 */
static inline void LES_start_timer(void) {
#if defined(__aarch64__)
    /* ... */

#elif defined(_M_X64) || defined(__x86_64__) || defined(_X86_)
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

#elif defined(_M_X64) || defined(__x86_64__) || defined(_X86_)
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
#if defined(LES_KERNEL_DEBUGFS)
    /*
     * openEuler 内核不允许 EL0 读物理计数器 CNTPCT_EL0
     * （CNTKCTL_EL1.EL0PCTEN=0，读到就 UNDEFINED -> SIGILL），
     * 只能用虚拟计数器 CNTVCT_EL0。两者同频（本板 CNTFRQ_EL0=50MHz），
     * 且内核插桩记录的时间戳同样取自 arch timer，可直接相减。
     */
    __asm__ volatile("mrs %0, cntvct_el0" : "=r"(val));
#else
    __asm__ volatile("mrs %0, cntpct_el0" : "=r"(val));
#endif

#elif defined(_M_X64) || defined(__x86_64__) || defined(_X86_)
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
#elif defined(LES_KERNEL_DEBUGFS)
/*
 * 内核插桩：系统服务入口的时间戳由内核记录，用户态不再需要插桩桩函数，
 * 这里给出空实现，保证调用点（若被调用）仍可编译链接。
 */
static inline void LES_stub(void)
{
}
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
#if defined(LES_KERNEL_DEBUGFS)
    /*
     * 内核插桩：系统调用出口的时间戳由内核记录（syscall_val 节点），
     * 用户态这里不再重复记录，也不再需要 LES_syscall_flag。
     */
#else
    if (LES_syscall_flag == 1) {
        LES_syscall_val = timeGet();
    }
#endif
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
