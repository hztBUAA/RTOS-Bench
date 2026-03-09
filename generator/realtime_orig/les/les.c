#include <inttypes.h>
#include <limits.h>
#include <errno.h>
#include "les.h"
#include "cpu_affinity.h"

volatile uint64_t LES_buffer[LES_BUFFER_SIZE];
volatile uint32_t LES_offset = 0;
volatile uint64_t LES_syscall_val = 0;

volatile uint64_t LES_interrupt_start_val = 0;
volatile uint64_t LES_interrupt_end_val = 0;
volatile uint32_t LES_flag = 0; // 0-关闭, 1-开启
volatile uint32_t LES_syscall_flag = 0; // 0-关闭, 1-开启
volatile uint32_t LES_interrupt_flag = 0;

/* 中断插桩函数 */
void LES_interrupt_end_stub(void) {
    if (LES_interrupt_flag == 1 && bench_get_cpu() == 0) {
    	LES_interrupt_end_val = timeGet();
    }
    LES_interrupt_flag = 0;
}

/* 开启插桩 */
void LES_enable(void) {
    LES_offset = 0; // 重置偏移量
    LES_flag = 1;   // 打开开关
}

/* 关闭插桩 */
void LES_disable(void) {
    LES_flag = 0;
}

/* 获取当前的偏移量 */
uint32_t LES_getOffset(void) {
    return LES_offset;
}

/**
 * 读取指定位置的时间戳
 * @param loc 索引位置
 * @param out_val 输出指针
 * @return GETTIMEVAL_EOK 成功, GETTIMEVAL_ERROR 失败
 */
int LES_getTimeVal(uint32_t loc, uint64_t *out_val) {
    if (out_val == NULL) {
        return GETTIMEVAL_ERROR;
    }
    if (loc >= LES_BUFFER_SIZE || loc >= LES_offset) {
        return GETTIMEVAL_ERROR;
    }
    *out_val = LES_buffer[loc];
    return GETTIMEVAL_EOK;
}

// 系统调用
/* 开启系统调用插桩 */
void LES_syscall_enable(void) {
    LES_syscall_val = 0;
    LES_syscall_flag = 1;
}

/* 关闭系统调用插桩 */
void LES_syscall_disable(void) {
    LES_syscall_flag = 0;
}

/* 获取系统调用记录的值 (t1) */
uint64_t LES_get_syscall_val(void) {
    return LES_syscall_val;
}

/* 开启中断插桩 */
void LES_interrupt_stub_enable(void) {
	LES_interrupt_start_val = 0;
	LES_interrupt_end_val = 0;
    LES_interrupt_flag = 1;
}

/* 关闭中断插桩 */
void LES_interrupt_stub_disable(void) {
    LES_interrupt_flag = 0;
}




