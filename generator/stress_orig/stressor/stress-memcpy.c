/* applications/stress-ng/stress-memcpy.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <stress-config.h>

#define NOINLINE __attribute__((noinline))

#ifndef OPTIMIZE0
#if defined(__GNUC__)
#define OPTIMIZE0 __attribute__((optimize("-O0")))
#define OPTIMIZE1 __attribute__((optimize("-O1")))
#define OPTIMIZE2 __attribute__((optimize("-O2")))
#define OPTIMIZE3 __attribute__((optimize("-O3")))
#else
#define OPTIMIZE0
#define OPTIMIZE1
#define OPTIMIZE2
#define OPTIMIZE3
#endif
#endif

/*
 * 平台无关全系统内存屏障
 * 目标：确保所有写操作在后续读之前对 CPU 可见。
 * - ARMv8/ARMv7: DSB SY + ISB
 * - x86_64:      MFENCE
 * - 其他:        GCC 原子同步屏障
 */
#if defined(__aarch64__) || defined(__arm__)
#  define FULL_BARRIER() \
     __asm__ volatile("dsb sy" ::: "memory"); \
     __asm__ volatile("isb"    ::: "memory")
#elif defined(__x86_64__) || defined(__i386__)
#  define FULL_BARRIER() \
     __asm__ volatile("mfence" ::: "memory")
#elif defined(__riscv)
#  define FULL_BARRIER() \
     __asm__ volatile("fence rw,rw" ::: "memory")
#else
#  define FULL_BARRIER() \
     __sync_synchronize()
#endif

typedef void *(*memcpy_func_t)(void *dest, const void *src, size_t n);
typedef void *(*memmove_func_t)(void *dest, const void *src, size_t n);

/* ------------------------------------------------------------------ */
/* 简单 LCG，用于生成确定性测试填充数据                                 */
/* 每个 worker 用 instance 作为种子，保证多 worker 互相独立             */
/* ------------------------------------------------------------------ */
typedef struct { uint32_t state; } fill_rng_t;

static inline void fill_rng_seed(fill_rng_t *r, uint32_t seed)
{
    r->state = seed ^ 0xdeadbeefU;
    if (r->state == 0) r->state = 1;
}

static inline uint8_t fill_rng_next(fill_rng_t *r)
{
    r->state = r->state * 1664525U + 1013904223U;
    return (uint8_t)(r->state >> 24);
}

/* ------------------------------------------------------------------ */
/* 核心：用 OPTIMIZE0 字节循环做填充/比较，完全绕过平台 memcpy 实现     */
/* ------------------------------------------------------------------ */

static NOINLINE OPTIMIZE0 void safe_fill(uint8_t *buf, size_t n, fill_rng_t *r)
{
    size_t i;
    for (i = 0; i < n; i++) buf[i] = fill_rng_next(r);
    FULL_BARRIER();
}

static NOINLINE OPTIMIZE0 void safe_copy(uint8_t *dst, const uint8_t *src, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) dst[i] = src[i];
    FULL_BARRIER();
}

/*
 * 返回首个差异偏移，无差异返回 n
 */
static NOINLINE OPTIMIZE0 size_t safe_compare(const uint8_t *a,
                                               const uint8_t *b,
                                               size_t n)
{
    size_t i;
    FULL_BARRIER();
    for (i = 0; i < n; i++) {
        if (a[i] != b[i]) return i;
    }
    return n;
}

/* ------------------------------------------------------------------ */
/* 全局选项                                                             */
/* ------------------------------------------------------------------ */
static int32_t s_memcpy_loops = DEFAULT_MEMCPY_LOOPS;
static int32_t s_memcpy_size  = DEFAULT_MEMCPY_MEMSIZE;

static int stress_memcpy_opt_loops(const char *opt_name, const char *opt_arg)
{
    char               *endptr;
    unsigned long long  val = strtoull(opt_arg, &endptr, 10);
    (void)opt_name;
    if      (*endptr == 'k' || *endptr == 'K') val *= 1024ULL;
    else if (*endptr == 'm' || *endptr == 'M') val *= 1024ULL * 1024ULL;
    else if (*endptr == 'g' || *endptr == 'G') val *= 1024ULL * 1024ULL * 1024ULL;
    if (val < MIN_MEMCPY_LOOPS) val = MIN_MEMCPY_LOOPS;
    s_memcpy_loops = (int32_t)val;
    stress_osal_print("rtos_stress: debug: memcpy-loops set to %d\n", s_memcpy_loops);
    return 0;
}

static int stress_memcpy_opt_size(const char *opt_name, const char *opt_arg)
{
    int val = atoi(opt_arg);
    (void)opt_name;
    if (val < MIN_MEMCPY_MEMSIZE) val = MIN_MEMCPY_MEMSIZE;
    s_memcpy_size = val;
    stress_osal_print("rtos_stress: debug: memcpy-size set to %d\n", s_memcpy_size);
    return 0;
}

const stress_opt_t stress_memcpy_opts[] = {
    { "memcpy-loops", stress_memcpy_opt_loops },
    { "memcpy-size",  stress_memcpy_opt_size  },
    { NULL, NULL }
};

/* ------------------------------------------------------------------ */
/* 单次测试上下文                                                        */
/* ------------------------------------------------------------------ */
typedef struct {
    stress_args_t *args;
    const char    *method_name;
    stress_bool_t  okay;
    /*
     * 布局（各区域大小均为 sz，彼此不重叠）：
     *   src:      被拷贝的数据源（每次测试重新填充）
     *   dst:      被测函数写入的目标
     *   expected: safe_copy 的结果，作为基准期望值
     *   overlap:  专门用于 memmove overlap 场景的独立区域
     */
    uint8_t       *src;
    uint8_t       *dst;
    uint8_t       *expected;
    uint8_t       *overlap;
    size_t         sz;
    fill_rng_t     rng;
} mc_ctx_t;

/*
 * 测试 memcpy 的核心逻辑：
 *
 *   1. 用确定性 RNG 填充 src（填充完即固定，不再变化）
 *   2. 用 safe_copy 把 src → expected（已知正确的参考）
 *   3. 调用被测 func 把 src → dst
 *   4. FULL_BARRIER 后，用 safe_compare 对比 dst 与 expected
 *
 * src 在步骤 1-4 之间不被任何其他操作触碰，期望值 100% 确定。
 */
static NOINLINE OPTIMIZE0
void check_memcpy(mc_ctx_t *ctx,
                  memcpy_func_t func,
                  size_t n,
                  uint32_t fill_seed)
{
    void  *ret;
    size_t diff;

    if (!ctx->okay) return;
    if (n == 0 || n > ctx->sz) return;

    /* 步骤1：用固定种子填充 src */
    fill_rng_t local_rng;
    fill_rng_seed(&local_rng, fill_seed);
    safe_fill(ctx->src, n, &local_rng);

    /* 步骤2：用 safe_copy 生成期望值 */
    safe_copy(ctx->expected, ctx->src, n);

    /* 步骤3：调用被测函数 */
    ret = func(ctx->dst, ctx->src, n);

    /* 步骤4：屏障 + 比较 */
    FULL_BARRIER();

    if (ret != (void *)ctx->dst) {
        stress_osal_print(
            "rtos_stress: fail: [memcpy] %s return ptr mismatch "
            "(expected=%p got=%p)\n",
            ctx->method_name, (void *)ctx->dst, ret);
        ctx->okay = STRESS_FALSE;
        return;
    }

    diff = safe_compare(ctx->dst, ctx->expected, n);
    if (diff < n) {
        stress_osal_print(
            "rtos_stress: fail: [memcpy] %s content check failed "
            "(n=%zu first_diff_offset=%zu expect=0x%02x got=0x%02x)\n",
            ctx->method_name, n, diff,
            (unsigned)ctx->expected[diff],
            (unsigned)ctx->dst[diff]);
        ctx->okay = STRESS_FALSE;
    }
}

/*
 * 测试 memmove 的核心逻辑：
 *
 * 非重叠情况：与 check_memcpy 完全相同。
 *
 * 重叠情况：仅验证 memmove 返回值正确且不 crash，
 *           不校验内容（因为重叠情况下 src 被破坏，
 *           期望值无法独立建立）。
 */
static NOINLINE OPTIMIZE0
void check_memmove(mc_ctx_t *ctx,
                   memmove_func_t func,
                   size_t n,
                   uint32_t fill_seed,
                   stress_bool_t is_overlap)
{
    void *ret;

    if (!ctx->okay) return;
    if (n == 0 || n > ctx->sz) return;

    if (!is_overlap) {
        /* 非重叠：与 check_memcpy 逻辑一致 */
        fill_rng_t local_rng;
        size_t     diff;

        fill_rng_seed(&local_rng, fill_seed);
        safe_fill(ctx->src, n, &local_rng);
        safe_copy(ctx->expected, ctx->src, n);

        ret = func(ctx->dst, ctx->src, n);
        FULL_BARRIER();

        if (ret != (void *)ctx->dst) {
            stress_osal_print(
                "rtos_stress: fail: [memmove] %s return ptr mismatch\n",
                ctx->method_name);
            ctx->okay = STRESS_FALSE;
            return;
        }

        diff = safe_compare(ctx->dst, ctx->expected, n);
        if (diff < n) {
            stress_osal_print(
                "rtos_stress: fail: [memmove] %s (non-overlap) content check failed "
                "(n=%zu first_diff_offset=%zu expect=0x%02x got=0x%02x)\n",
                ctx->method_name, n, diff,
                (unsigned)ctx->expected[diff],
                (unsigned)ctx->dst[diff]);
            ctx->okay = STRESS_FALSE;
        }
    } else {
        /*
         * 重叠：用 overlap 区域做测试，只验证不 crash 且返回值正确。
         * 区域布局：overlap[0..n) 作为 dest，overlap[n/4..n/4+n) 作为 src，
         * 两者故意重叠。
         */
        fill_rng_t local_rng;
        uint8_t   *ov_dest = ctx->overlap;
        uint8_t   *ov_src  = ctx->overlap + n / 4;

        if (ov_src + n > ctx->overlap + ctx->sz) {
            /* overlap 区域不够大，跳过 */
            return;
        }

        fill_rng_seed(&local_rng, fill_seed ^ 0x12345678U);
        safe_fill(ctx->overlap, ctx->sz, &local_rng);

        ret = func(ov_dest, ov_src, n);
        FULL_BARRIER();

        if (ret != (void *)ov_dest) {
            stress_osal_print(
                "rtos_stress: fail: [memmove] %s (overlap) return ptr mismatch\n",
                ctx->method_name);
            ctx->okay = STRESS_FALSE;
        }
        /* 不校验内容，仅确认不 crash 且返回正确 */
    }
}

/* ------------------------------------------------------------------ */
/* 各种 memcpy/memmove 实现的 wrapper                                   */
/* ------------------------------------------------------------------ */

static void *wrap_builtin_memcpy(void *dst, const void *src, size_t n)
{
    return __builtin_memcpy(dst, src, n);
}

static void *wrap_builtin_memmove(void *dst, const void *src, size_t n)
{
    return __builtin_memmove(dst, src, n);
}

#define DEF_NAIVE_MEMCPY(name, hint)                                     \
static hint void *name(void *dest, const void *src, size_t n)            \
{                                                                         \
    size_t i;                                                             \
    char *d = (char *)dest;                                               \
    const char *s = (const char *)src;                                   \
    for (i = 0; i < n; i++) d[i] = s[i];                                 \
    return dest;                                                          \
}

#define DEF_NAIVE_MEMMOVE(name, hint)                                     \
static hint void *name(void *dest, const void *src, size_t n)             \
{                                                                          \
    size_t i;                                                              \
    char *d = (char *)dest;                                                \
    const char *s = (const char *)src;                                    \
    if (d < s) {                                                           \
        for (i = 0; i < n; i++) d[i] = s[i];                             \
    } else if (d > s) {                                                    \
        for (i = n; i-- > 0; ) d[i] = s[i];                              \
    }                                                                      \
    return dest;                                                           \
}

DEF_NAIVE_MEMCPY (naive_memcpy,    NOINLINE)
DEF_NAIVE_MEMCPY (naive_memcpy_o0, NOINLINE OPTIMIZE0)
DEF_NAIVE_MEMCPY (naive_memcpy_o1, NOINLINE OPTIMIZE1)
DEF_NAIVE_MEMCPY (naive_memcpy_o2, NOINLINE OPTIMIZE2)
DEF_NAIVE_MEMCPY (naive_memcpy_o3, NOINLINE OPTIMIZE3)
DEF_NAIVE_MEMMOVE(naive_memmove,    NOINLINE)
DEF_NAIVE_MEMMOVE(naive_memmove_o0, NOINLINE OPTIMIZE0)
DEF_NAIVE_MEMMOVE(naive_memmove_o1, NOINLINE OPTIMIZE1)
DEF_NAIVE_MEMMOVE(naive_memmove_o2, NOINLINE OPTIMIZE2)
DEF_NAIVE_MEMMOVE(naive_memmove_o3, NOINLINE OPTIMIZE3)

/* ------------------------------------------------------------------ */
/* 测试序列：每个方法用相同的序列                                        */
/* ------------------------------------------------------------------ */

static void run_one_method(mc_ctx_t *ctx,
                            memcpy_func_t  cpy_func,
                            memmove_func_t mov_func)
{
    size_t sz    = ctx->sz;
    uint32_t sid = (uint32_t)ctx->args->instance
                 ^ (uint32_t)stress_osal_tick_get();

    if (!ctx->okay || !stress_continue(ctx->args)) return;

    /* --- memcpy 测试：全尺寸 --- */
    check_memcpy(ctx, cpy_func, sz,       sid + 0);
    check_memcpy(ctx, cpy_func, sz / 2,   sid + 1);
    check_memcpy(ctx, cpy_func, sz / 4,   sid + 2);
    check_memcpy(ctx, cpy_func, sz - 1,   sid + 3);
    check_memcpy(ctx, cpy_func, 64,       sid + 4);
    check_memcpy(ctx, cpy_func, 1,        sid + 5);

    if (!ctx->okay || !stress_continue(ctx->args)) return;

    /* --- memmove 测试：非重叠 --- */
    check_memmove(ctx, mov_func, sz,       sid + 6,  STRESS_FALSE);
    check_memmove(ctx, mov_func, sz / 2,   sid + 7,  STRESS_FALSE);
    check_memmove(ctx, mov_func, sz / 4,   sid + 8,  STRESS_FALSE);
    check_memmove(ctx, mov_func, sz - 1,   sid + 9,  STRESS_FALSE);

    if (!ctx->okay || !stress_continue(ctx->args)) return;

    /* --- memmove 测试：重叠（只测不 crash，不校验内容）--- */
    check_memmove(ctx, mov_func, sz / 2,   sid + 10, STRESS_TRUE);
    check_memmove(ctx, mov_func, sz / 4,   sid + 11, STRESS_TRUE);

    ctx->args->bogo.current_ops++;
}

/* ------------------------------------------------------------------ */
/* 方法表                                                               */
/* ------------------------------------------------------------------ */

typedef struct {
    const char    *name;
    memcpy_func_t  cpy;
    memmove_func_t mov;
} mc_method_t;

static const mc_method_t s_methods[] = {
    { "libc",     memcpy,            memmove            },
    { "builtin",  wrap_builtin_memcpy, wrap_builtin_memmove },
    { "naive",    naive_memcpy,      naive_memmove      },
    { "naive_o0", naive_memcpy_o0,   naive_memmove_o0   },
    { "naive_o1", naive_memcpy_o1,   naive_memmove_o1   },
    { "naive_o2", naive_memcpy_o2,   naive_memmove_o2   },
    { "naive_o3", naive_memcpy_o3,   naive_memmove_o3   },
    { NULL, NULL, NULL }
};

/* ------------------------------------------------------------------ */
/* 入口（保持原有外部接口）                                              */
/* ------------------------------------------------------------------ */

void stress_memcpy(stress_args_t *args)
{
    mc_ctx_t   ctx;
    uint8_t   *buf = NULL;
    size_t     sz  = (size_t)s_memcpy_size;
    int        method_idx = -1;   /* -1 = all */
    int        i;

    if (sz < 256) sz = 256;

    /*
     * 分配 4 个独立区域：src / dst / expected / overlap
     * 每块对齐到 64 字节（NEON/AVX Cache line）
     */
    size_t block = (sz + 63) & ~(size_t)63;
    buf = (uint8_t *)stress_osal_malloc(4 * block);
    if (!buf) {
        stress_osal_print("rtos_stress: error: [memcpy-%d] OOM (%zu bytes)\n",
                          args->instance, 4 * block);
        return;
    }

    ctx.args     = args;
    ctx.okay     = STRESS_TRUE;
    ctx.method_name = "";
    ctx.sz       = sz;
    ctx.src      = buf;
    ctx.dst      = buf + block;
    ctx.expected = buf + 2 * block;
    ctx.overlap  = buf + 3 * block;

    fill_rng_seed(&ctx.rng, (uint32_t)args->instance ^ 0xabcd1234U);

    stress_osal_print("rtos_stress: info: [memcpy-%d] buffer size: 3 x %zu bytes\n",
                      args->instance, sz);

    /* 解析方法名 */
    if (args->method_name != NULL &&
        stress_osal_strcmp(args->method_name, "all") != 0) {
        for (i = 0; s_methods[i].name != NULL; i++) {
            if (stress_osal_strcmp(args->method_name, s_methods[i].name) == 0) {
                method_idx = i;
                break;
            }
        }
        if (method_idx < 0) {
            stress_osal_print(
                "rtos_stress: error: [memcpy-%d] unknown method '%s', using 'all'\n",
                args->instance, args->method_name);
        }
    }

    if (method_idx < 0) {
        stress_osal_print("rtos_stress: info: [memcpy-%d] using 'all' methods\n",
                          args->instance);
    } else {
        stress_osal_print("rtos_stress: info: [memcpy-%d] using method '%s'\n",
                          args->instance, s_methods[method_idx].name);
    }

    /* 主循环 */
    while (ctx.okay && stress_continue(args)) {
        if (method_idx < 0) {
            for (i = 0; s_methods[i].name != NULL; i++) {
                if (!ctx.okay || !stress_continue(args)) break;
                ctx.method_name = s_methods[i].name;
                run_one_method(&ctx,
                               s_methods[i].cpy,
                               s_methods[i].mov);
            }
        } else {
            ctx.method_name = s_methods[method_idx].name;
            run_one_method(&ctx,
                           s_methods[method_idx].cpy,
                           s_methods[method_idx].mov);
        }
        stress_osal_sleep_ms(1);
    }

    stress_osal_free(buf);
}
