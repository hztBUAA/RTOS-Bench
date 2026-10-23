#include <inttypes.h>
#include <limits.h>
#include <errno.h>
#include <stddef.h>
#include "les.h"
#include "cpu_affinity.h"

#if defined(ONEOS_PLATFORM) && (defined(__loongarch__) || defined(__loongarch64) || defined(__loongarch_lp64))
// 中移系统统一都需要注释掉符号，去寻找内核镜像中的符号
/*
volatile uint64_t LES_buffer[LES_BUFFER_SIZE];
volatile uint32_t LES_offset = 0;
volatile uint64_t LES_syscall_val = 0;
volatile uint64_t LES_interrupt_start_val = 0;
volatile uint64_t LES_interrupt_end_val = 0;
volatile uint32_t LES_flag = 0;
volatile uint32_t LES_syscall_flag = 0;
volatile uint32_t LES_interrupt_flag = 0;
*/
#elif defined (SYLIXOS_PLATFORM) || defined (ONEOS_PLATFORM)
/*
volatile uint64_t LES_buffer[LES_BUFFER_SIZE];
volatile uint32_t LES_offset = 0;
volatile uint64_t LES_syscall_val = 0;
volatile uint64_t LES_interrupt_start_val = 0;
volatile uint64_t LES_interrupt_end_val = 0;
volatile uint32_t LES_flag = 0;
volatile uint32_t LES_syscall_flag = 0;
volatile uint32_t LES_interrupt_flag = 0;
*/
#elif defined(DONGTU_PLATFORM)

void LES_interrupt_end_stub(void) __attribute__((weak));
void LES_enable(void) __attribute__((weak));
void LES_disable(void) __attribute__((weak));
uint32_t LES_getOffset(void) __attribute__((weak));
int LES_getTimeVal(uint32_t loc, uint64_t *out_val) __attribute__((weak));
void LES_syscall_enable(void) __attribute__((weak));
void LES_syscall_disable(void) __attribute__((weak));
uint64_t LES_get_syscall_val(void) __attribute__((weak));
void LES_interrupt_stub_enable(void) __attribute__((weak));
void LES_interrupt_stub_disable(void) __attribute__((weak));
void LES_stub(void) __attribute__((weak));

__attribute__((weak)) volatile uint64_t LES_buffer[LES_BUFFER_SIZE];
__attribute__((weak)) volatile uint32_t LES_offset = 0;
__attribute__((weak)) volatile uint64_t *__LES_syscall_val = NULL;
__attribute__((weak)) volatile uint64_t *__LES_interrupt_start_val = NULL;
__attribute__((weak)) volatile uint64_t *__LES_interrupt_end_val = NULL;
__attribute__((weak)) volatile uint32_t LES_flag = 0;
__attribute__((weak)) volatile uint32_t *__LES_syscall_flag = NULL;
__attribute__((weak)) volatile uint32_t *__LES_interrupt_flag = NULL;

#elif defined(RUIHUA_PLATFORM)

void LES_interrupt_end_stub(void) __attribute__((weak));
void LES_enable(void) __attribute__((weak));
void LES_disable(void) __attribute__((weak));
uint32_t LES_getOffset(void) __attribute__((weak));
int LES_getTimeVal(uint32_t loc, uint64_t *out_val) __attribute__((weak));
void LES_syscall_enable(void) __attribute__((weak));
void LES_syscall_disable(void) __attribute__((weak));
uint64_t LES_get_syscall_val(void) __attribute__((weak));
void LES_interrupt_stub_enable(void) __attribute__((weak));
void LES_interrupt_stub_disable(void) __attribute__((weak));

__attribute__((weak)) volatile uint64_t LES_buffer[LES_BUFFER_SIZE];
__attribute__((weak)) volatile uint32_t LES_offset = 0;
__attribute__((weak)) volatile uint64_t LES_syscall_val = 0;
__attribute__((weak)) volatile uint64_t LES_interrupt_start_val = 0;
__attribute__((weak)) volatile uint64_t LES_interrupt_end_val = 0;
__attribute__((weak)) volatile uint32_t LES_flag = 0;
__attribute__((weak)) volatile uint32_t LES_syscall_flag = 0;
__attribute__((weak)) volatile uint32_t LES_interrupt_flag = 0;

#else
volatile uint64_t LES_buffer[LES_BUFFER_SIZE];
volatile uint32_t LES_offset = 0;
volatile uint64_t LES_syscall_val = 0;
volatile uint64_t LES_interrupt_start_val = 0;
volatile uint64_t LES_interrupt_end_val = 0;
volatile uint32_t LES_flag = 0; // 0-关闭, 1-开启
volatile uint32_t LES_syscall_flag = 0; // 0-关闭, 1-开启
volatile uint32_t LES_interrupt_flag = 0;
#endif

/* 中断插桩函数 */
void LES_interrupt_end_stub(void) {
    if (LES_interrupt_flag == 1 && bench_get_cpu() == 0) {
    	LES_interrupt_end_val = timeGet();
        LES_interrupt_flag = 0;
    }
}

#if defined(DONGTU_PLATFORM)
void LES_stub(void) {
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


/*
 * 读取本次测量窗口 [t0, t3] 内的 LES 记录。
 *
 * 内核插桩缓冲记录的是窗口内该 CPU 上的所有调度事件，其中可能混有
 * 其它任务的事件，或者出现在 t0 之前 / t3 之后的记录。这里只取
 * 落在 [t0, t3] 内的记录，并返回其中最早与最晚的时间戳，从而保证
 * t0 <= *t1 <= *t2 <= t3，调用方的无符号减法不会回绕。
 *
 * 至少需要两条记录（挂起与恢复）才视为有效样本。
 * 返回 GETTIMEVAL_EOK 表示成功，GETTIMEVAL_ERROR 表示窗口内记录不足或参数非法。
 */
int LES_getTimeSlice(uint64_t t0, uint64_t t3, uint64_t *t1, uint64_t *t2)
{
	uint32_t offset;
	uint32_t i;
	uint32_t count = 0;
	uint64_t ts;
	int have = 0;

	if (t1 == NULL || t2 == NULL || t3 < t0) {
		return GETTIMEVAL_ERROR;
	}

	offset = LES_getOffset();
	for (i = 0; i < offset; i++) {
		if (LES_getTimeVal(i, &ts) != GETTIMEVAL_EOK) {
			continue;
		}
		/* 忽略本次测量窗口之外的记录 */
		if (ts < t0 || ts > t3) {
			continue;
		}
		if (!have) {
			*t1 = ts;
			*t2 = ts;
			have = 1;
			count = 1;
			continue;
		}
		/* 防御性处理：忽略乱序记录，保证返回的首/末时间戳递增 */
		if (ts < *t1) {
			continue;
		}
		*t2 = ts;
		count++;
	}

	if (count < 2) {
		return GETTIMEVAL_ERROR;
	}

	return GETTIMEVAL_EOK;
}



