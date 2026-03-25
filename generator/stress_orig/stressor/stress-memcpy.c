#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <config.h>

#define ALIGN_SIZE (64)

#if defined(__GNUC__) || defined(__clang__)
#define UNLIKELY(x)  __builtin_expect(!!(x), 0)
#define LIKELY(x)    __builtin_expect(!!(x), 1)
#define NOINLINE     __attribute__((noinline))
#ifndef OPTIMIZE0
#define OPTIMIZE0    __attribute__((optimize("-O0")))
#define OPTIMIZE1    __attribute__((optimize("-O1")))
#define OPTIMIZE2    __attribute__((optimize("-O2")))
#define OPTIMIZE3    __attribute__((optimize("-O3")))
#endif
#else
#define UNLIKELY(x)  (x)
#define LIKELY(x)    (x)
#define NOINLINE
#ifndef OPTIMIZE0
#define OPTIMIZE0
#define OPTIMIZE1
#define OPTIMIZE2
#define OPTIMIZE3
#endif
#endif

typedef struct stress_memcpy_ctx stress_memcpy_ctx_t;

typedef void *(*memcpy_func_t)(void *dest, const void *src, size_t n);
typedef void *(*memmove_func_t)(void *dest, const void *src, size_t n);

struct stress_memcpy_ctx {
    stress_bool_t   okay;
    const char     *method_name;
    void *(*memcpy_check)(stress_memcpy_ctx_t *ctx, memcpy_func_t func,
                          void *dest, const void *src, size_t n);
    void *(*memmove_check)(stress_memcpy_ctx_t *ctx, memmove_func_t func,
                           void *dest, const void *src, size_t n);
};

static int32_t s_memcpy_loops = DEFAULT_MEMCPY_LOOPS;
static int32_t s_memcpy_size  = DEFAULT_MEMCPY_MEMSIZE;

static uint64_t memcpy_parse_uint64(const char *s)
{
    uint64_t val = 0;
    const char *p = s;
    while (*p >= '0' && *p <= '9') {
        val = val * 10ULL + (uint64_t)((unsigned char)*p - (unsigned char)'0');
        p++;
    }
    if      (*p == 'k' || *p == 'K') val *= 1024ULL;
    else if (*p == 'm' || *p == 'M') val *= 1024ULL * 1024ULL;
    else if (*p == 'g' || *p == 'G') val *= 1024ULL * 1024ULL * 1024ULL;
    return val;
}

static int stress_memcpy_opt_loops(const char *opt_name, const char *opt_arg)
{
    uint64_t val;
    (void)opt_name;

    val = memcpy_parse_uint64(opt_arg);
    if (val == 0ULL) {
        stress_osal_print("rtos_stress: warn: memcpy-loops: invalid"
                          " value '%s', using default\n", opt_arg);
        return 0;
    }
    if (val < (uint64_t)MIN_MEMCPY_LOOPS) val = (uint64_t)MIN_MEMCPY_LOOPS;
    if (val > (uint64_t)MAX_MEMCPY_LOOPS) val = (uint64_t)MAX_MEMCPY_LOOPS;

    s_memcpy_loops = (int32_t)val;
    stress_osal_print("rtos_stress: debug: memcpy-loops set to %d\n",
                      (int)s_memcpy_loops);
    return 0;
}

static int stress_memcpy_opt_size(const char *opt_name, const char *opt_arg)
{
    uint64_t val;
    (void)opt_name;

    val = memcpy_parse_uint64(opt_arg);
    if (val == 0ULL) {
        stress_osal_print("rtos_stress: warn: memcpy-size: invalid"
                          " value '%s', using default\n", opt_arg);
        return 0;
    }
    if (val < (uint64_t)MIN_MEMCPY_MEMSIZE) val = (uint64_t)MIN_MEMCPY_MEMSIZE;
    if (val > (uint64_t)MAX_MEMCPY_MEMSIZE) val = (uint64_t)MAX_MEMCPY_MEMSIZE;

    s_memcpy_size = (int32_t)val;
    stress_osal_print("rtos_stress: debug: memcpy-size set to %d\n",
                      (int)s_memcpy_size);
    return 0;
}

const stress_opt_t stress_memcpy_opts[] = {
    { "memcpy-loops", stress_memcpy_opt_loops },
    { "memcpy-size",  stress_memcpy_opt_size  },
    { NULL, NULL }
};

static OPTIMIZE3 void *memcpy_check_func(stress_memcpy_ctx_t *ctx,
                                          memcpy_func_t func,
                                          void *dest, const void *src, size_t n)
{
    void *ptr;

    if (n == 0u) return dest;

    ptr = func(dest, src, n);

    if (UNLIKELY(memcmp(dest, src, n) != 0)) {
        stress_osal_print("rtos_stress: fail: [memcpy] %s content check"
                          " failed\n", ctx->method_name);
        ctx->okay = STRESS_FALSE;
    }
    if (UNLIKELY(ptr != dest)) {
        stress_osal_print("rtos_stress: fail: [memcpy] %s return ptr"
                          " mismatch\n", ctx->method_name);
        ctx->okay = STRESS_FALSE;
    }
    return ptr;
}

static OPTIMIZE3 void *memmove_check_func(stress_memcpy_ctx_t *ctx,
                                           memmove_func_t func,
                                           void *dest, const void *src, size_t n)
{
    uintptr_t     d;
    uintptr_t     s;
    stress_bool_t overlap;
    void         *ptr;

    if (n == 0u) return dest;

    d       = (uintptr_t)dest;
    s       = (uintptr_t)src;
    overlap = (d < s + n) && (s < d + n);
    ptr     = func(dest, src, n);

    if (!overlap) {
        if (UNLIKELY(memcmp(dest, src, n) != 0)) {
            stress_osal_print("rtos_stress: fail: [memmove] %s content"
                              " check failed\n", ctx->method_name);
            ctx->okay = STRESS_FALSE;
        }
    }
    if (UNLIKELY(ptr != dest)) {
        stress_osal_print("rtos_stress: fail: [memmove] %s return ptr"
                          " mismatch\n", ctx->method_name);
        ctx->okay = STRESS_FALSE;
    }
    return ptr;
}

#define TEST_NAIVE_MEMCPY(name, hint)                                          \
static hint void *name(void *dest, const void *src, size_t n)                 \
{                                                                              \
    size_t        i     = 0;                                                   \
    char         *cdest = (char *)dest;                                        \
    const char   *csrc  = (const char *)src;                                  \
    for (i = 0; i < n; i++)                                                    \
        *(cdest++) = *(csrc++);                                                \
    return dest;                                                               \
}

TEST_NAIVE_MEMCPY(test_naive_memcpy,    NOINLINE)
TEST_NAIVE_MEMCPY(test_naive_memcpy_o0, NOINLINE OPTIMIZE0)
TEST_NAIVE_MEMCPY(test_naive_memcpy_o1, NOINLINE OPTIMIZE1)
TEST_NAIVE_MEMCPY(test_naive_memcpy_o2, NOINLINE OPTIMIZE2)
TEST_NAIVE_MEMCPY(test_naive_memcpy_o3, NOINLINE OPTIMIZE3)

#define TEST_NAIVE_MEMMOVE(name, hint)                                         \
static hint void *name(void *dest, const void *src, size_t n)                 \
{                                                                              \
    size_t        i     = 0;                                                   \
    char         *cdest = (char *)dest;                                        \
    const char   *csrc  = (const char *)src;                                  \
    if (dest < src) {                                                          \
        for (i = 0; i < n; i++)                                                \
            *(cdest++) = *(csrc++);                                            \
    } else {                                                                   \
        csrc  += n;                                                            \
        cdest += n;                                                            \
        for (i = 0; i < n; i++)                                                \
            *(--cdest) = *(--csrc);                                            \
    }                                                                          \
    return dest;                                                               \
}

TEST_NAIVE_MEMMOVE(test_naive_memmove,    NOINLINE)
TEST_NAIVE_MEMMOVE(test_naive_memmove_o0, NOINLINE OPTIMIZE0)
TEST_NAIVE_MEMMOVE(test_naive_memmove_o1, NOINLINE OPTIMIZE1)
TEST_NAIVE_MEMMOVE(test_naive_memmove_o2, NOINLINE OPTIMIZE2)
TEST_NAIVE_MEMMOVE(test_naive_memmove_o3, NOINLINE OPTIMIZE3)

#if defined(__GNUC__) || defined(__clang__)
static void *stress_builtin_memcpy_wrapper(void *dst, const void *src, size_t n)
{
    return __builtin_memcpy(dst, src, n);
}
static void *stress_builtin_memmove_wrapper(void *dst, const void *src, size_t n)
{
    return __builtin_memmove(dst, src, n);
}
#else
static void *stress_builtin_memcpy_wrapper(void *dst, const void *src, size_t n)
{
    return memcpy(dst, src, n);
}
static void *stress_builtin_memmove_wrapper(void *dst, const void *src, size_t n)
{
    return memmove(dst, src, n);
}
#endif

#define STRESS_MEMCPY_BODY(cpy, move)                                          \
    int32_t i;                                                                 \
    size_t  sz = (size_t)s_memcpy_size;                                        \
    for (i = 0; ctx->okay && (i < s_memcpy_loops); i++) {                     \
        if (!stress_continue(args)) break;                                     \
        if (sz > (size_t)ALIGN_SIZE) {                                         \
            ctx->memcpy_check (ctx, cpy,  str3,      str2,      sz);          \
            ctx->memcpy_check (ctx, cpy,  str2,      str3,      sz / 2u);     \
            ctx->memmove_check(ctx, move, str3,      str3 + 64, sz - 64u);    \
            ctx->memcpy_check (ctx, cpy,  str1,      str2,      sz);          \
            ctx->memmove_check(ctx, move, str3 + 64, str3,      sz - 64u);    \
            ctx->memcpy_check (ctx, cpy,  str3,      str1,      sz);          \
        }                                                                      \
        if (sz > 1u) {                                                         \
            ctx->memmove_check(ctx, move, str3 + 1,  str3,      sz - 1u);     \
            ctx->memmove_check(ctx, move, str3,      str3 + 1,  sz - 1u);     \
        }                                                                      \
        args->bogo.current_ops++;                                              \
    }

static NOINLINE void stress_memcpy_libc(stress_args_t *args,
                                         stress_memcpy_ctx_t *ctx,
                                         uint8_t *str1, uint8_t *str2,
                                         uint8_t *str3)
{
    ctx->method_name = "libc";
    STRESS_MEMCPY_BODY(memcpy, memmove)
}

static NOINLINE void stress_memcpy_builtin(stress_args_t *args,
                                            stress_memcpy_ctx_t *ctx,
                                            uint8_t *str1, uint8_t *str2,
                                            uint8_t *str3)
{
    ctx->method_name = "builtin";
    STRESS_MEMCPY_BODY(stress_builtin_memcpy_wrapper,
                       stress_builtin_memmove_wrapper)
}

#define STRESS_MEMCPY_NAIVE(method, fname, cpy, move)                          \
static NOINLINE void fname(stress_args_t *args,                                \
                            stress_memcpy_ctx_t *ctx,                          \
                            uint8_t *str1, uint8_t *str2, uint8_t *str3)       \
{                                                                              \
    ctx->method_name = method;                                                 \
    STRESS_MEMCPY_BODY(cpy, move)                                              \
}

STRESS_MEMCPY_NAIVE("naive",    stress_memcpy_naive,    test_naive_memcpy,    test_naive_memmove)
STRESS_MEMCPY_NAIVE("naive_o0", stress_memcpy_naive_o0, test_naive_memcpy_o0, test_naive_memmove_o0)
STRESS_MEMCPY_NAIVE("naive_o1", stress_memcpy_naive_o1, test_naive_memcpy_o1, test_naive_memmove_o1)
STRESS_MEMCPY_NAIVE("naive_o2", stress_memcpy_naive_o2, test_naive_memcpy_o2, test_naive_memmove_o2)
STRESS_MEMCPY_NAIVE("naive_o3", stress_memcpy_naive_o3, test_naive_memcpy_o3, test_naive_memmove_o3)

typedef void (*stress_memcpy_func_t)(stress_args_t *args,
                                      stress_memcpy_ctx_t *ctx,
                                      uint8_t *str1, uint8_t *str2,
                                      uint8_t *str3);

typedef struct {
    const char              *name;
    stress_memcpy_func_t     func;
} stress_memcpy_method_info_t;

static const stress_memcpy_method_info_t stress_memcpy_methods[] = {
    { "libc",     stress_memcpy_libc      },
    { "builtin",  stress_memcpy_builtin   },
    { "naive",    stress_memcpy_naive     },
    { "naive_o0", stress_memcpy_naive_o0  },
    { "naive_o1", stress_memcpy_naive_o1  },
    { "naive_o2", stress_memcpy_naive_o2  },
    { "naive_o3", stress_memcpy_naive_o3  },
    { NULL,       NULL                    }
};

void stress_memcpy(stress_args_t *args)
{
    uint8_t                      *raw_buf = NULL;
    uint8_t                      *buf;
    uint8_t                      *str1, *str2, *str3;
    stress_memcpy_func_t          specific_func = NULL;
    stress_bool_t                 run_all       = STRESS_FALSE;
    stress_memcpy_ctx_t           ctx;
    size_t                        sz, alloc_size, raw_alloc_size;
    size_t                        i;
    uintptr_t                     raw_addr, aligned_addr;

    sz = (size_t)s_memcpy_size;

    if (sz < (size_t)ALIGN_SIZE) {
        stress_osal_print("rtos_stress: warn: [memcpy-%d] memcpy-size"
                          " %d < %d, clamping to %d\n",
                          args->instance,
                          (int)sz, ALIGN_SIZE, ALIGN_SIZE);
        sz = (size_t)ALIGN_SIZE;
    }

    if (sz > (SIZE_MAX - (size_t)ALIGN_SIZE) / 3u) {
        stress_osal_print("rtos_stress: error: [memcpy-%d] memcpy-size"
                          " %d too large, would overflow alloc\n",
                          args->instance, (int)sz);
        return;
    }

    alloc_size     = 3u * sz;
    raw_alloc_size = alloc_size + (size_t)ALIGN_SIZE;

    raw_buf = (uint8_t *)stress_osal_malloc(raw_alloc_size);
    if (!raw_buf) {
        stress_osal_print("rtos_stress: error: [memcpy-%d] OOM"
                          " allocating %d bytes\n",
                          args->instance, (int)raw_alloc_size);
        return;
    }

    raw_addr     = (uintptr_t)raw_buf;
    aligned_addr = (raw_addr + (uintptr_t)(ALIGN_SIZE - 1))
                   & ~(uintptr_t)(ALIGN_SIZE - 1);
    buf  = (uint8_t *)aligned_addr;

    str1 = buf;
    str2 = str1 + sz;
    str3 = str2 + sz;

    for (i = 0u; i < sz; i++) {
        uint8_t v = (uint8_t)stress_osal_rand();
        str1[i] = v;
        str2[i] = v;
        str3[i] = v;
    }

    stress_osal_print("rtos_stress: info: [memcpy-%d] buffer size:"
                      " 3 x %d bytes\n",
                      args->instance, (int)sz);

    ctx.okay          = STRESS_TRUE;
    ctx.method_name   = "";
    ctx.memcpy_check  = memcpy_check_func;
    ctx.memmove_check = memmove_check_func;

    if (args->method_name == NULL ||
        stress_osal_strcmp(args->method_name, "all") == 0) {
        run_all = STRESS_TRUE;
        stress_osal_print("rtos_stress: info: [memcpy-%d] using 'all'"
                          " methods\n", args->instance);
    } else {
        for (i = 0u; stress_memcpy_methods[i].name != NULL; i++) {
            if (stress_osal_strcmp(args->method_name,
                                   stress_memcpy_methods[i].name) == 0) {
                specific_func = stress_memcpy_methods[i].func;
                break;
            }
        }
        if (!specific_func) {
            stress_osal_print("rtos_stress: error: [memcpy-%d] unknown"
                              " method '%s', using 'all'\n",
                              args->instance, args->method_name);
            run_all = STRESS_TRUE;
        } else {
            stress_osal_print("rtos_stress: info: [memcpy-%d] using"
                              " method '%s'\n",
                              args->instance, args->method_name);
        }
    }

    while (ctx.okay && stress_continue(args)) {
        if (run_all) {
            for (i = 0u; stress_memcpy_methods[i].name != NULL; i++) {
                if (!ctx.okay || !stress_continue(args)) break;
                stress_memcpy_methods[i].func(args, &ctx, str1, str2, str3);
            }
        } else {
            specific_func(args, &ctx, str1, str2, str3);
        }
    }

    stress_osal_free(raw_buf);
}
