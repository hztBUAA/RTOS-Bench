/* applications/stress-ng/stress-bitops.c */
#include "stress-ng.h"
#include "stress_osal.h" /* 引入 OSAL 头文件 */
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <stress-config.h>

/* ==================================================================
 * 宏定义与兼容性层
 * ================================================================== */

/* 编译器内建函数支持检测 (主要针对 GCC/Clang) */
#if defined(__GNUC__) || defined(__clang__)
    #define HAVE_BUILTIN_CLZ
    #define HAVE_BUILTIN_CTZ
    #define HAVE_BUILTIN_POPCOUNT
    #define HAVE_BUILTIN_PARITY
    #define HAVE_BUILTIN_BITREVERSE
#endif

#define UNLIKELY(x)     __builtin_expect(!!(x), 0)
#define ALIGN64         __attribute__ ((aligned(64)))


/* ==================================================================
 * 命令行选项解析 (Internal Options)
 * ================================================================== */

/* 静态变量存储配置，初始化为默认值 */
static int32_t s_bitops_loops = DEFAULT_BITOPS_LOOPS;

/*
 * 选项解析回调：--bitops-loops
 */
static int stress_bitops_opt_loops(const char *opt_name, const char *opt_arg)
{
    /* atoi 不是 OSAL 接口，保留标准调用 */
    int val = atoi(opt_arg);
    /* 限制最小 1，防止除零或空转 */
    if (val < MIN_BITOPS_LOOPS) val = MIN_BITOPS_LOOPS;

    s_bitops_loops = val;
    stress_osal_print("rtos_stress: debug: bitops-loops set to %d\n", s_bitops_loops);
    return 0;
}

/* 导出给 stress-ng.c 使用的选项表 */
const stress_opt_t stress_bitops_opts[] = {
    { "bitops-loops", stress_bitops_opt_loops },
    { NULL, NULL } /* 哨兵 */
};

/* ==================================================================
 * 辅助函数
 * ================================================================== */

/* 简单的防优化输出 */
static void stress_uint32_put(uint32_t val) {
    volatile uint32_t sink;
    sink = val;
    (void)sink;
}

/* 简单的随机数生成器模拟 (使用 OSAL) */
static uint32_t stress_mwc32(void) {
    return (uint32_t)stress_osal_rand();
}

static uint16_t stress_mwc16(void) {
    return (uint16_t)stress_osal_rand();
}

/* ==================================================================
 * 具体 Bitops 算法实现
 * ================================================================== */

typedef int (*stress_bitops_func)(const char *name, uint32_t *count);

/*
 * 1. Sign (符号判断)
 */
static int stress_bitops_sign(const char *name, uint32_t *count)
{
    int32_t i;
    int32_t v = (int32_t)stress_mwc32();
    const uint32_t d = (~0U) >> 1;
    uint32_t sum = 0;

    /* 使用配置的循环次数 */
    for (i = 0; i < s_bitops_loops; i++) {
        register int32_t sign1, sign2;

        /* 方法1: 比较 */
        sign1 = -(v < 0);
        sum += sign1;

        /* 方法2: 移位 */
        sign2 = -(int)((unsigned int)((int)v) >> 31);
        sum += sign2;

        if (UNLIKELY(sign1 != sign2)) {
            stress_osal_print("%s: sign failure v=%d s1=%d s2=%d\n", name, v, sign1, sign2);
            return -1;
        }
        v += d;
    }
    stress_uint32_put(sum);
    *count += (2 * i);
    return 0;
}

/*
 * 2. Abs (绝对值)
 */
static int stress_bitops_abs(const char *name, uint32_t *count)
{
    int32_t i;
    int32_t v = (int32_t)stress_mwc32();
    const uint32_t d = (~0U) >> 1;
    uint32_t sum = 0;

    for (i = 0; i < s_bitops_loops; i++) {
        register const int32_t mask = v >> 31;
        register int32_t abs1, abs2;

        abs1 = (v + mask) ^ mask;
        sum += abs1;

        abs2 = (v ^ mask) - mask;
        sum += abs2;

        if (UNLIKELY(abs1 != abs2)) {
            stress_osal_print("%s: abs failure v=%d\n", name, v);
            return -1;
        }
        v += d;
    }
    stress_uint32_put(sum);
    *count += (2 * i);
    return 0;
}

/*
 * 3. Count Bits (统计置位位数)
 */
static int stress_bitops_countbits(const char *name, uint32_t *count)
{
    int32_t i;
    uint32_t v = stress_mwc32();
    const uint32_t dv = stress_mwc16();
    uint32_t sum = 0;

    for (i = 0; i < s_bitops_loops; i++) {
        uint32_t c1 = 0, c2 = 0, tmp;

        /* 方法1: Brian Kernighan 算法 */
        for (tmp = v, c1 = 0; tmp; c1++)
            tmp &= (tmp - 1);
        sum += c1;

        /* 方法2: 并行位计算 */
        tmp = v - ((v >> 1) & 0x55555555);
        tmp = (tmp & 0x33333333) + ((tmp >> 2) & 0x33333333);
        c2 = (((tmp + (tmp >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
        sum += c2;

        if (UNLIKELY(c1 != c2)) {
            stress_osal_print("%s: countbits failure v=%x c1=%d c2=%d\n", name, v, c1, c2);
            return -1;
        }

#if defined(HAVE_BUILTIN_POPCOUNT)
        c2 = __builtin_popcount(v);
        if (UNLIKELY(c1 != c2)) {
            stress_osal_print("%s: popcount failure\n", name);
            return -1;
        }
#endif
        v += dv;
    }
    stress_uint32_put(sum);
    *count += i;
    return 0;
}

/*
 * 4. CLZ (前导零计数)
 */
static int stress_bitops_clz(const char *name, uint32_t *count)
{
    int32_t i;
    uint32_t v = stress_mwc32();
    const uint32_t dv = stress_mwc16();
    uint32_t sum = 0;

    for (i = 0; i < s_bitops_loops; i++) {
        uint32_t c1, c2, tmp, n;

        if (v == 0) c1 = 32;
        else {
            /* 朴素方法 */
            for (c1 = 0, tmp = v; tmp && ((tmp & 0x80000000) == 0); tmp <<= 1)
                c1++;
        }
        sum += c1;

        /* 对数移位法 */
        n = 32;
        c2 = v;
        if ((tmp = c2 >> 16) != 0) { n -= 16; c2 = tmp; }
        if ((tmp = c2 >> 8) != 0)  { n -= 8;  c2 = tmp; }
        if ((tmp = c2 >> 4) != 0)  { n -= 4;  c2 = tmp; }
        if ((tmp = c2 >> 2) != 0)  { n -= 2;  c2 = tmp; }
        if ((tmp = c2 >> 1) != 0)  c2 = n - 2;
        else c2 = n - c2;

        sum += c2;

        if (UNLIKELY(c1 != c2)) {
            stress_osal_print("%s: clz failure v=%x c1=%d c2=%d\n", name, v, c1, c2);
            return -1;
        }
#if defined(HAVE_BUILTIN_CLZ)
        if (v != 0 && c1 != (uint32_t)__builtin_clz(v)) {
            stress_osal_print("%s: builtin_clz failure\n", name);
            return -1;
        }
#endif
        v += dv;
    }
    stress_uint32_put(sum);
    *count += i;
    return 0;
}

/*
 * 5. Parity (奇偶校验)
 */
static int stress_bitops_parity(const char *name, uint32_t *count)
{
    int32_t i;
    uint32_t v = stress_mwc32();
    const uint32_t dv = stress_mwc16();
    uint32_t sum = 0;

    for (i = 0; i < s_bitops_loops; i++) {
        int p1, p2;
        uint32_t tmp;

        /* 朴素方法 */
        for (p1 = 0, tmp = v; tmp; tmp = tmp & (tmp - 1))
            p1 = !p1;
        sum += p1;

        /* 异或移位法 */
        tmp = v ^ (v >> 16);
        tmp ^= tmp >> 8;
        tmp ^= tmp >> 4;
        tmp &= 0xf;
        p2 = (0x6996 >> tmp) & 1;
        sum += p2;

        if (UNLIKELY(p1 != p2)) {
            stress_osal_print("%s: parity failure v=%x\n", name, v);
            return -1;
        }
        v += dv;
    }
    stress_uint32_put(sum);
    *count += i;
    return 0;
}

/*
 * 6. Reverse (位翻转)
 */
static int stress_bitops_reverse(const char *name, uint32_t *count)
{
    int32_t i;
    uint32_t v = stress_mwc32();
    const uint32_t dv = stress_mwc16();
    uint32_t sum = 0;

    for (i = 0; i < s_bitops_loops; i++) {
        uint32_t tmp, r1, r2, s, mask;

        /* lg(N) method */
        mask = ~0;
        s = 32;
        r2 = v;
        while ((s >>= 1) > 0) {
            mask ^= (mask << s);
            r2 = ((r2 >> s) & mask) | ((r2 << s) & ~mask);
        }
        sum += r2;

        /* Parallel method */
        tmp = v;
        tmp = (((tmp & 0xaaaaaaaaUL) >> 1)  | ((tmp & 0x55555555UL) << 1));
        tmp = (((tmp & 0xccccccccUL) >> 2)  | ((tmp & 0x33333333UL) << 2));
        tmp = (((tmp & 0xf0f0f0f0UL) >> 4)  | ((tmp & 0x0f0f0f0fUL) << 4));
        tmp = (((tmp & 0xff00ff00UL) >> 8)  | ((tmp & 0x00ff00ffUL) << 8));
        r1 =  (((tmp & 0xffff0000UL) >> 16) | ((tmp & 0x0000ffffUL) << 16));
        sum += r1;

        if (UNLIKELY(r1 != r2)) {
            stress_osal_print("%s: reverse failure v=%x r1=%x r2=%x\n", name, v, r1, r2);
            return -1;
        }
        v += dv;
    }
    stress_uint32_put(sum);
    *count += i;
    return 0;
}

/*
 * 7. Gray Code (格雷码 - 额外补充的简单位运算)
 */
static int stress_bitops_gray(const char *name, uint32_t *count)
{
    int32_t i;
    uint32_t v = stress_mwc32();
    uint32_t sum = 0;

    for (i = 0; i < s_bitops_loops; i++) {
        uint32_t gray = (v >> 1) ^ v;
        /* 逆格雷码 */
        uint32_t bin = gray;
        bin ^= (bin >> 16);
        bin ^= (bin >> 8);
        bin ^= (bin >> 4);
        bin ^= (bin >> 2);
        bin ^= (bin >> 1);

        if (UNLIKELY(bin != v)) {
            stress_osal_print("%s: gray code failure v=%x bin=%x\n", name, v, bin);
            return -1;
        }
        sum += gray;
        v++;
    }
    stress_uint32_put(sum);
    *count += i;
    return 0;
}

/* ==================================================================
 * 注册表与分发逻辑
 * ================================================================== */

typedef struct {
    const char *name;
    stress_bitops_func func;
} stress_bitops_method_info_t;

static const stress_bitops_method_info_t bitops_methods[] = {
    { "sign",      stress_bitops_sign },
    { "abs",       stress_bitops_abs },
    { "countbits", stress_bitops_countbits },
    { "clz",       stress_bitops_clz },
    { "parity",    stress_bitops_parity },
    { "reverse",   stress_bitops_reverse },
    { "gray",      stress_bitops_gray },
    { NULL,        NULL }
};

/*
 * 主入口函数: stress_bitops
 */
void stress_bitops(stress_args_t *args)
{
    uint32_t dummy_count = 0;
    stress_bitops_func specific_func = NULL;
    stress_bool_t run_all = STRESS_FALSE;

    /* 1. 解析方法 (使用 OSAL strcmp) */
    if (args->method_name == NULL || stress_osal_strcmp(args->method_name, "all") == 0) {
        run_all = STRESS_TRUE;
        stress_osal_print("rtos_stress: info: [bitops-%d] using 'all' methods\n", args->instance);
    } else {
        for (int i = 0; bitops_methods[i].name != NULL; i++) {
            if (stress_osal_strcmp(args->method_name, bitops_methods[i].name) == 0) {
                specific_func = bitops_methods[i].func;
                break;
            }
        }
        if (!specific_func) {
            stress_osal_print("rtos_stress: error: unknown method '%s', using 'all'\n", args->method_name);
            run_all = STRESS_TRUE;
        } else {
            stress_osal_print("rtos_stress: info: [bitops-%d] using method '%s'\n", args->instance, args->method_name);
        }
    }

    /* 2. 核心循环 */
    while (stress_continue(args))
    {
        if (run_all) {
            /* 轮询所有算法 */
            for (int i = 0; bitops_methods[i].name != NULL; i++) {
                if (!stress_continue(args)) break;

                /* 执行算法 */
                int ret = bitops_methods[i].func(bitops_methods[i].name, &dummy_count);
                if (ret != 0) {
                    stress_osal_print("rtos_stress: fatal error in %s\n", bitops_methods[i].name);
                    return;
                }

                /* 累加操作计数 */
                args->bogo.current_ops++;
            }
        } else {
            /* 运行指定算法 */
            int ret = specific_func(args->method_name, &dummy_count);
             if (ret != 0) return;

            args->bogo.current_ops++;
        }

        /* Bitops 属于纯 CPU 计算，非常密集，必须让渡 (使用 OSAL) */
        stress_osal_sleep_ms(1);
    }
}
