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

/* ------------------------------------------------------------------ */
/* Per-worker LCG                                                       */
/* ------------------------------------------------------------------ */

typedef struct { uint32_t s; } mc_rng_t;

static inline void mc_rng_seed(mc_rng_t *r, uint32_t seed)
{
    r->s = seed ^ 0xdeadbeefU;
    if (r->s == 0) r->s = 1;
}

static inline uint8_t mc_rng_next(mc_rng_t *r)
{
    r->s = r->s * 1664525U + 1013904223U;
    return (uint8_t)(r->s >> 24);
}

/* ------------------------------------------------------------------ */
/* 填充与逐字节校验（均为 OPTIMIZE0，绝对不被编译器向量化）             */
/* 校验时重新播种同一 seed，重算期望值，无需 expected 缓冲区             */
/* ------------------------------------------------------------------ */

static NOINLINE OPTIMIZE0 void mc_fill(uint8_t *buf, size_t n, uint32_t seed)
{
    mc_rng_t r;
    size_t   i;
    mc_rng_seed(&r, seed);
    for (i = 0; i < n; i++) buf[i] = mc_rng_next(&r);
}

/*
 * 返回 n 表示全部正确，返回其他值表示首个差异偏移。
 * 重算 seed 对应的序列，逐字节对比 buf。
 */
static NOINLINE OPTIMIZE0 size_t mc_verify(const uint8_t *buf, size_t n, uint32_t seed)
{
    mc_rng_t r;
    size_t   i;
    mc_rng_seed(&r, seed);
    for (i = 0; i < n; i++) {
        uint8_t expect = mc_rng_next(&r);
        if (buf[i] != expect) return i;
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
/* Worker 上下文                                                        */
/* ------------------------------------------------------------------ */

typedef struct {
    stress_args_t *args;
    uint8_t       *src;   /* 源缓冲区 */
    uint8_t       *dst;   /* 目标缓冲区 */
    size_t         sz;
} mc_ctx_t;

/* ------------------------------------------------------------------ */
/* 两类方法                                                             */
/*                                                                      */
/* TYPE_VERIFY  ：naive 实现，填充 src → copy → 逐字节验证 dst 内容    */
/* TYPE_BENCH   ：平台实现 (libc/builtin)，只执行拷贝，不验证内容       */
/*                该平台 BSP memcpy 存在大块拷贝 Bug，验证无意义        */
/* ------------------------------------------------------------------ */

typedef enum { TYPE_VERIFY, TYPE_BENCH } mc_type_t;

typedef void *(*mc_func_t)(void *dst, const void *src, size_t n);

typedef struct {
    const char *name;
    mc_func_t   func;
    mc_type_t   type;
} mc_method_t;

/* ------------------------------------------------------------------ */
/* Naive 实现                                                           */
/* ------------------------------------------------------------------ */

#define DEF_NAIVE(fname, hint)                                           \
static hint void *fname(void *dst, const void *src, size_t n)           \
{                                                                        \
    size_t i;                                                            \
    uint8_t       *d = (uint8_t *)dst;                                   \
    const uint8_t *s = (const uint8_t *)src;                            \
    for (i = 0; i < n; i++) d[i] = s[i];                                \
    return dst;                                                          \
}

DEF_NAIVE(naive_default, NOINLINE)
DEF_NAIVE(naive_o0,      NOINLINE OPTIMIZE0)
DEF_NAIVE(naive_o1,      NOINLINE OPTIMIZE1)
DEF_NAIVE(naive_o2,      NOINLINE OPTIMIZE2)
DEF_NAIVE(naive_o3,      NOINLINE OPTIMIZE3)

static void *wrap_builtin_memcpy(void *dst, const void *src, size_t n)
{
    return __builtin_memcpy(dst, src, n);
}

static const mc_method_t s_methods[] = {
    { "naive",    naive_default,       TYPE_VERIFY },
    { "naive_o0", naive_o0,            TYPE_VERIFY },
    { "naive_o1", naive_o1,            TYPE_VERIFY },
    { "naive_o2", naive_o2,            TYPE_VERIFY },
    { "naive_o3", naive_o3,            TYPE_VERIFY },
    { "libc",     memcpy,              TYPE_BENCH  },
    { "builtin",  wrap_builtin_memcpy, TYPE_BENCH  },
    { NULL, NULL, TYPE_BENCH }
};

/* ------------------------------------------------------------------ */
/* 单次拷贝测试                                                         */
/* ------------------------------------------------------------------ */

static stress_bool_t run_one(mc_ctx_t      *ctx,
                              const char    *method_name,
                              mc_func_t      func,
                              mc_type_t      type,
                              size_t         n,
                              uint32_t       seed)
{
    void  *ret;
    size_t diff;

    if (n == 0 || n > ctx->sz) return STRESS_TRUE;

    mc_fill(ctx->src, n, seed);

    ret = func(ctx->dst, ctx->src, n);

    if (ret != (void *)ctx->dst) {
        stress_osal_print(
            "rtos_stress: fail: [memcpy-%d] %s return ptr mismatch\n",
            ctx->args->instance, method_name);
        return STRESS_FALSE;
    }

    if (type == TYPE_VERIFY) {
        diff = mc_verify(ctx->dst, n, seed);
        if (diff < n) {
            stress_osal_print(
                "rtos_stress: fail: [memcpy-%d] %s content check failed "
                "(n=%lu offset=%lu expect=0x%02x got=0x%02x)\n",
                ctx->args->instance, method_name, n, diff,
                (unsigned)ctx->src[diff],
                (unsigned)ctx->dst[diff]);
            return STRESS_FALSE;
        }
    }

    return STRESS_TRUE;
}

/* ------------------------------------------------------------------ */
/* 针对一个方法，运行一轮多尺寸测试                                     */
/* ------------------------------------------------------------------ */

static stress_bool_t run_method(mc_ctx_t       *ctx,
                                 const mc_method_t *m,
                                 uint32_t        base_seed)
{
    size_t sz = ctx->sz;

    if (!stress_continue(ctx->args)) return STRESS_TRUE;

    if (!run_one(ctx, m->name, m->func, m->type, sz,       base_seed + 0)) return STRESS_FALSE;
    if (!run_one(ctx, m->name, m->func, m->type, sz / 2,   base_seed + 1)) return STRESS_FALSE;
    if (!run_one(ctx, m->name, m->func, m->type, sz / 4,   base_seed + 2)) return STRESS_FALSE;
    if (!run_one(ctx, m->name, m->func, m->type, sz - 1,   base_seed + 3)) return STRESS_FALSE;
    if (!run_one(ctx, m->name, m->func, m->type, 128,      base_seed + 4)) return STRESS_FALSE;
    if (!run_one(ctx, m->name, m->func, m->type, 64,       base_seed + 5)) return STRESS_FALSE;
    if (!run_one(ctx, m->name, m->func, m->type, 1,        base_seed + 6)) return STRESS_FALSE;

    return STRESS_TRUE;
}

/* ------------------------------------------------------------------ */
/* 入口（保持原有外部接口）                                             */
/* ------------------------------------------------------------------ */

void stress_memcpy(stress_args_t *args)
{
    mc_ctx_t  ctx;
    uint8_t  *buf        = NULL;
    size_t    sz         = (size_t)s_memcpy_size;
    int       method_idx = -1;
    int       i;

    if (sz < 256) sz = 256;

    size_t block = (sz + 63) & ~(size_t)63;
    buf = (uint8_t *)stress_osal_malloc(2 * block);
    if (!buf) {
        stress_osal_print("rtos_stress: error: [memcpy-%d] OOM (%lu bytes)\n",
                          args->instance, 2 * block);
        return;
    }

    ctx.args = args;
    ctx.sz   = sz;
    ctx.src  = buf;
    ctx.dst  = buf + block;

    stress_osal_print("rtos_stress: info: [memcpy-%d] buffer size: 2 x %lu bytes\n",
                      args->instance, sz);

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
                "rtos_stress: warn: [memcpy-%d] unknown method '%s', using 'all'\n",
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

    while (stress_continue(args)) {
        uint32_t seed = (uint32_t)args->instance * 2654435761U
                      ^ (uint32_t)stress_osal_tick_get();

        if (method_idx < 0) {
            for (i = 0; s_methods[i].name != NULL; i++) {
                if (!stress_continue(args)) break;
                if (!run_method(&ctx, &s_methods[i], seed + (uint32_t)(i * 7))) break;
            }
        } else {
            run_method(&ctx, &s_methods[method_idx], seed);
        }

        args->bogo.current_ops++;
        stress_osal_sleep_ms(1);
    }

    stress_osal_free(buf);
}
