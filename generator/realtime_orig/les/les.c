#include <inttypes.h>
#include <limits.h>
#include <errno.h>
#include <stddef.h>
#include "les.h"
#include "cpu_affinity.h"

#if defined(LES_KERNEL_DEBUGFS)
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

#if defined(LES_KERNEL_DEBUGFS)

/*
 * 内核插桩（openEuler，见 platform_macro.h）：
 * 插桩数据由内核镜像维护，用户态不定义任何 LES_* 变量，
 * 读写都通过 debugfs 节点完成，因此这里没有变量定义。
 */

#elif defined(ONEOS_PLATFORM) && (defined(__loongarch__) || defined(__loongarch64) || defined(__loongarch_lp64))
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

#if defined(LES_KERNEL_DEBUGFS)

/* =========================================================================
 * openEuler 内核插桩后端
 *
 * 插桩数据由内核镜像维护，用户态通过 debugfs 节点读写。节点含义见
 * D:\OS\OpenEuler\插桩使用说明.md：
 *
 *   reset               清空数据并关闭所有插桩开关
 *   sched_enable        调度插桩开关，写 1 开启，写 0 关闭
 *   syscall_enable      系统调用出口插桩开关
 *   interrupt_enable    中断插桩开关，内核记录到中断终点后自动清 0
 *   offset              调度插桩已记录数量
 *   buffer              调度插桩时间戳，每行 "index counter_cycles"
 *   syscall_val         系统调用出口时间戳
 *   interrupt_start_val 中断起点时间戳
 *   interrupt_end_val   中断终点时间戳
 *   counter_freq        计数器频率（Hz）
 *
 * 注：本后端与 DONGTU_PLATFORM 的 LES_stub() 互斥（内核已在系统服务入口
 *     插桩），如同时定义以 LES_KERNEL_DEBUGFS 为准。
 * ========================================================================= */

static void les_kernel_warn(const char *node)
{
	static int warned = 0;

	if (warned != 0) {
		return;
	}
	warned = 1;
	fprintf(stderr,
		"[LES] 无法访问内核插桩节点 %s/%s\n"
		"[LES] 请确认已引导插桩内核，并已执行 mount -t debugfs none /sys/kernel/debug\n",
		LES_KERNEL_DEBUGFS_DIR, node);
}

static int les_kernel_open(const char *node, int flags)
{
	char path[256];
	int n = snprintf(path, sizeof(path), "%s/%s", LES_KERNEL_DEBUGFS_DIR, node);
	int fd;

	if (n <= 0 || (size_t)n >= sizeof(path)) {
		return -1;
	}

	fd = open(path, flags);
	if (fd < 0) {
		les_kernel_warn(node);
	}
	return fd;
}

/* 读单值节点：offset / syscall_val / interrupt_start_val / interrupt_end_val */
static uint64_t les_kernel_read_u64(const char *node)
{
	char buf[64];
	ssize_t n;
	int fd = les_kernel_open(node, O_RDONLY);

	if (fd < 0) {
		return 0;
	}

	n = read(fd, buf, sizeof(buf) - 1);
	close(fd);
	if (n <= 0) {
		return 0;
	}
	buf[n] = '\0';

	return (uint64_t)strtoull(buf, NULL, 10);
}

/* 写开关节点，返回 0 表示成功 */
static int les_kernel_write_u32(const char *node, uint32_t value)
{
	char buf[16];
	int len;
	ssize_t n;
	int fd = les_kernel_open(node, O_WRONLY);

	if (fd < 0) {
		return -1;
	}

	len = snprintf(buf, sizeof(buf), "%u", value);
	n = write(fd, buf, (size_t)len);
	close(fd);

	return (n == len) ? 0 : -1;
}

/* 匹配 buffer 节点里的一行："index counter_cycles" */
static int les_kernel_line_match(const char *line, uint32_t want, uint64_t *out_val)
{
	char *end;
	unsigned long index;
	unsigned long long cycles;

	index = strtoul(line, &end, 10);
	if (end == line) {
		return -1;
	}
	while (*end == ' ' || *end == '\t') {
		end++;
	}
	cycles = strtoull(end, NULL, 10);

	if (index != (unsigned long)want) {
		return -1;
	}

	*out_val = (uint64_t)cycles;
	return 0;
}

uint64_t LES_kernel_interrupt_start_val(void)
{
	return les_kernel_read_u64("interrupt_start_val");
}

uint64_t LES_kernel_interrupt_end_val(void)
{
	return les_kernel_read_u64("interrupt_end_val");
}

uint64_t LES_kernel_syscall_val(void)
{
	return les_kernel_read_u64("syscall_val");
}

/* 中断终点由内核插桩记录，用户态桩函数不需要（也无法）做事 */
void LES_interrupt_end_stub(void)
{
}

/* 打开调度插桩：先 reset 清空上一轮数据并复位所有开关，再打开 sched_enable */
void LES_enable(void)
{
	(void)les_kernel_write_u32("reset", 1);
	(void)les_kernel_write_u32("sched_enable", 1);
}

void LES_disable(void)
{
	(void)les_kernel_write_u32("sched_enable", 0);
}

uint32_t LES_getOffset(void)
{
	return (uint32_t)les_kernel_read_u64("offset");
}

/*
 * 从 buffer 节点解析出 index == loc 的时间戳。数据最多 LES_BUFFER_SIZE 行，
 * 这里逐块/逐行解析，避免在小栈线程（如 4KB 栈）里放大数组。
 */
int LES_getTimeVal(uint32_t loc, uint64_t *out_val)
{
	char chunk[256];
	char line[128];
	size_t line_len = 0;
	ssize_t n;
	ssize_t i;
	int fd;

	if (out_val == NULL) {
		return GETTIMEVAL_ERROR;
	}
	if (loc >= LES_BUFFER_SIZE) {
		return GETTIMEVAL_ERROR;
	}

	fd = les_kernel_open("buffer", O_RDONLY);
	if (fd < 0) {
		return GETTIMEVAL_ERROR;
	}

	while ((n = read(fd, chunk, sizeof(chunk))) > 0) {
		for (i = 0; i < n; i++) {
			if (chunk[i] == '\n') {
				line[line_len] = '\0';
				if (line_len > 0 &&
				    les_kernel_line_match(line, loc, out_val) == 0) {
					close(fd);
					return GETTIMEVAL_EOK;
				}
				line_len = 0;
			} else if (line_len < sizeof(line) - 1) {
				line[line_len++] = chunk[i];
			}
		}
	}

	if (line_len > 0) {
		line[line_len] = '\0';
		if (les_kernel_line_match(line, loc, out_val) == 0) {
			close(fd);
			return GETTIMEVAL_EOK;
		}
	}

	close(fd);
	return GETTIMEVAL_ERROR;
}

void LES_syscall_enable(void)
{
	(void)les_kernel_write_u32("reset", 1);
	(void)les_kernel_write_u32("syscall_enable", 1);
}

void LES_syscall_disable(void)
{
	(void)les_kernel_write_u32("syscall_enable", 0);
}

uint64_t LES_get_syscall_val(void)
{
	return les_kernel_read_u64("syscall_val");
}

void LES_interrupt_stub_enable(void)
{
	(void)les_kernel_write_u32("reset", 1);
	(void)les_kernel_write_u32("interrupt_enable", 1);
}

void LES_interrupt_stub_disable(void)
{
	(void)les_kernel_write_u32("interrupt_enable", 0);
}

#else /* !LES_KERNEL_DEBUGFS：其它平台由用户态自己维护 LES 变量 */

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

#endif /* LES_KERNEL_DEBUGFS */

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



