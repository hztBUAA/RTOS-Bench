/* applications/stress-ng/stress-memcpy.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <stress-config.h>

#define ALIGN_SIZE                  (64)

#define UNLIKELY(x)         __builtin_expect(!!(x), 0)
#define LIKELY(x)           __builtin_expect(!!(x), 1)
#define NOINLINE            __attribute__((noinline))

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

typedef void * (*memcpy_func_t)(void *dest, const void *src, size_t n);
typedef void * (*memmove_func_t)(void *dest, const void *src, size_t n);
typedef void * (*memcpy_check_func_t)(memcpy_func_t func, void *dest, const void *src, size_t n);
typedef void * (*memmove_check_func_t)(memmove_func_t func, void *dest, const void *src, size_t n);

static memcpy_check_func_t memcpy_check;
static memmove_check_func_t memmove_check;
static stress_bool_t memcpy_okay = STRESS_TRUE;
static const char *s_method_name = "";

static int32_t s_memcpy_loops = DEFAULT_MEMCPY_LOOPS;
static int32_t s_memcpy_size = DEFAULT_MEMCPY_MEMSIZE;

static int stress_memcpy_opt_loops(const char *opt_name, const char *opt_arg)
{
    char *endptr;
    unsigned long long val = strtoull(opt_arg, &endptr, 10);

    if (*endptr == 'k' || *endptr == 'K') val *= 1024;
    else if (*endptr == 'm' || *endptr == 'M') val *= (1024 * 1024);
    else if (*endptr == 'g' || *endptr == 'G') val *= (1024 * 1024 * 1024);
    if (val < MIN_MEMCPY_LOOPS) val = MIN_MEMCPY_LOOPS;
    s_memcpy_loops = val;
    stress_osal_print("rtos_stress: debug: memcpy-loops set to %d\n", s_memcpy_loops);
    return 0;
}

static int stress_memcpy_opt_size(const char *opt_name, const char *opt_arg)
{
    int val = atoi(opt_arg);
    if (val < MIN_MEMCPY_MEMSIZE) val = MIN_MEMCPY_MEMSIZE;
    s_memcpy_size = val;
    stress_osal_print("rtos_stress: debug: memcpy-size set to %d\n", s_memcpy_size);
    return 0;
}

const stress_opt_t stress_memcpy_opts[] = {
    { "memcpy-loops", stress_memcpy_opt_loops },
    { "memcpy-size",  stress_memcpy_opt_size },
    { NULL, NULL }
};

static OPTIMIZE3 void *memcpy_check_func(memcpy_func_t func, void *dest, const void *src, size_t n)
{
    void *ptr = func(dest, src, n);

    if (UNLIKELY(memcmp(dest, src, n) != 0)) {
        stress_osal_print("rtos_stress: fail: [memcpy] %s content check failed\n", s_method_name);
        memcpy_okay = STRESS_FALSE;
    }
    if (UNLIKELY(ptr != dest)) {
        stress_osal_print("rtos_stress: fail: [memcpy] %s return ptr mismatch\n", s_method_name);
        memcpy_okay = STRESS_FALSE;
    }
    return ptr;
}

static OPTIMIZE3 void *memmove_check_func(memmove_func_t func, void *dest, const void *src, size_t n)
{
    uintptr_t d = (uintptr_t)dest;
    uintptr_t s = (uintptr_t)src;
    stress_bool_t overlap = (d < s + n) && (s < d + n);

    void *ptr = func(dest, src, n);

    if (!overlap) {
        if (UNLIKELY(memcmp(dest, src, n) != 0)) {
            stress_osal_print("rtos_stress: fail: [memmove] %s content check failed\n", s_method_name);
            memcpy_okay = STRESS_FALSE;
        }
    }

    if (UNLIKELY(ptr != dest)) {
        stress_osal_print("rtos_stress: fail: [memmove] %s return ptr mismatch\n", s_method_name);
        memcpy_okay = STRESS_FALSE;
    }
    return ptr;
}

#define TEST_NAIVE_MEMCPY(name, hint)                   \
static hint void *name(void *dest, const void *src, size_t n)       \
{                                   \
    register size_t i;                      \
    register char *cdest = (char *)dest;                \
    register const char *csrc = (const char *)src;          \
                                    \
    for (i = 0; i < n; i++)                     \
        *(cdest++) = *(csrc++);                 \
    return dest;                            \
}                                   \

TEST_NAIVE_MEMCPY(test_naive_memcpy, NOINLINE)
TEST_NAIVE_MEMCPY(test_naive_memcpy_o0, NOINLINE OPTIMIZE0)
TEST_NAIVE_MEMCPY(test_naive_memcpy_o1, NOINLINE OPTIMIZE1)
TEST_NAIVE_MEMCPY(test_naive_memcpy_o2, NOINLINE OPTIMIZE2)
TEST_NAIVE_MEMCPY(test_naive_memcpy_o3, NOINLINE OPTIMIZE3)

#define TEST_NAIVE_MEMMOVE(name, hint)                  \
static hint void *name(void *dest, const void *src, size_t n)       \
{                                   \
    register size_t i;                      \
    register char *cdest = (char *)dest;                \
    register const char *csrc = (const char *)src;          \
                                    \
    if (dest < src) {                       \
        for (i = 0; i < n; i++)                 \
            *(cdest++) = *(csrc++);             \
    } else {                            \
        csrc += n;                      \
        cdest += n;                     \
                                    \
        for (i = 0; i < n; i++)                 \
            *(--cdest) = *(--csrc);             \
    }                               \
    return dest;                            \
}

TEST_NAIVE_MEMMOVE(test_naive_memmove, NOINLINE)
TEST_NAIVE_MEMMOVE(test_naive_memmove_o0, NOINLINE OPTIMIZE0)
TEST_NAIVE_MEMMOVE(test_naive_memmove_o1, NOINLINE OPTIMIZE1)
TEST_NAIVE_MEMMOVE(test_naive_memmove_o2, NOINLINE OPTIMIZE2)
TEST_NAIVE_MEMMOVE(test_naive_memmove_o3, NOINLINE OPTIMIZE3)

static NOINLINE void stress_memcpy_libc(stress_args_t *args, uint8_t *str1, uint8_t *str2, uint8_t *str3)
{
    int i;
    s_method_name = "libc";
    size_t sz = (size_t)s_memcpy_size;

    for (i = 0; memcpy_okay && (i < s_memcpy_loops); i++) {
        if (!stress_continue(args)) break;
        memcpy_check(memcpy, str3, str2, sz);
        memcpy_check(memcpy, str2, str3, sz / 2);
        memmove_check(memmove, str3, str3 + 64, sz - 64);
        memcpy_check(memcpy, str1, str2, sz);
        memmove_check(memmove, str3 + 64, str3, sz - 64);
        memcpy_check(memcpy, str3, str1, sz);
        memmove_check(memmove, str3 + 1, str3, sz - 1);
        memmove_check(memmove, str3, str3 + 1, sz - 1);
        args->bogo.current_ops++;
    }
}

static void *stress_builtin_memcpy_wrapper(void *dst, const void *src, size_t n)
{
    return __builtin_memcpy(dst, src, n);
}

static void *stress_builtin_memmove_wrapper(void *dst, const void *src, size_t n)
{
    return __builtin_memmove(dst, src, n);
}

static NOINLINE void stress_memcpy_builtin(stress_args_t *args, uint8_t *str1, uint8_t *str2, uint8_t *str3)
{
    int i;
    s_method_name = "builtin";
    size_t sz = (size_t)s_memcpy_size;

    for (i = 0; memcpy_okay && (i < s_memcpy_loops); i++) {
        if (!stress_continue(args)) break;
        memcpy_check(stress_builtin_memcpy_wrapper, str3, str2, sz);
        memcpy_check(stress_builtin_memcpy_wrapper, str2, str3, sz / 2);
        memmove_check(stress_builtin_memmove_wrapper, str3, str3 + 64, sz - 64);
        memcpy_check(stress_builtin_memcpy_wrapper, str1, str2, sz);
        memmove_check(stress_builtin_memmove_wrapper, str3 + 64, str3, sz - 64);
        memcpy_check(stress_builtin_memcpy_wrapper, str3, str1, sz);
        memmove_check(stress_builtin_memmove_wrapper, str3 + 1, str3, sz - 1);
        memmove_check(stress_builtin_memmove_wrapper, str3, str3 + 1, sz - 1);
        args->bogo.current_ops++;
    }
}

#define STRESS_MEMCPY_NAIVE(method, name, cpy, move)                \
static NOINLINE void name(                          \
    stress_args_t *args,                        \
    uint8_t *str1,                              \
    uint8_t *str2,                              \
    uint8_t *str3)                              \
{                                       \
    int i;                                  \
    size_t sz = (size_t)s_memcpy_size;      \
    s_method_name = method;                         \
                                        \
    for (i = 0; memcpy_okay && (i < s_memcpy_loops); i++) {           \
        if (!stress_continue(args)) break;      \
        memcpy_check(cpy, str3, str2, sz);      \
        memcpy_check(cpy, str2, str3, sz / 2);  \
        memmove_check(move, str3, str3 + 64, sz - 64);\
        memcpy_check(cpy, str1, str2, sz);      \
        memmove_check(move, str3 + 64, str3, sz - 64);\
        memcpy_check(cpy, str3, str1, sz);      \
        memmove_check(move, str3 + 1, str3, sz - 1);    \
        memmove_check(move, str3, str3 + 1, sz - 1);    \
        args->bogo.current_ops++;               \
    }                                   \
}

STRESS_MEMCPY_NAIVE("naive", stress_memcpy_naive, test_naive_memcpy, test_naive_memmove)
STRESS_MEMCPY_NAIVE("naive_o0", stress_memcpy_naive_o0, test_naive_memcpy_o0, test_naive_memmove_o0)
STRESS_MEMCPY_NAIVE("naive_o1", stress_memcpy_naive_o1, test_naive_memcpy_o1, test_naive_memmove_o1)
STRESS_MEMCPY_NAIVE("naive_o2", stress_memcpy_naive_o2, test_naive_memcpy_o2, test_naive_memmove_o2)
STRESS_MEMCPY_NAIVE("naive_o3", stress_memcpy_naive_o3, test_naive_memcpy_o3, test_naive_memmove_o3)

typedef void (*stress_memcpy_func_t)(stress_args_t *args, uint8_t *str1, uint8_t *str2, uint8_t *str3);

typedef struct {
    const char *name;
    const stress_memcpy_func_t func;
} stress_memcpy_method_info_t;

static const stress_memcpy_method_info_t stress_memcpy_methods[] = {
    { "libc",       stress_memcpy_libc },
    { "builtin",    stress_memcpy_builtin },
    { "naive",      stress_memcpy_naive },
    { "naive_o0",   stress_memcpy_naive_o0 },
    { "naive_o1",   stress_memcpy_naive_o1 },
    { "naive_o2",   stress_memcpy_naive_o2 },
    { "naive_o3",   stress_memcpy_naive_o3 },
    { NULL,         NULL }
};

void stress_memcpy(stress_args_t *args)
{
    uint8_t *buf, *str1, *str2, *str3;
    stress_memcpy_func_t specific_func = NULL;
    stress_bool_t run_all = STRESS_FALSE;
    size_t sz = (size_t)s_memcpy_size;

    memcpy_okay = STRESS_TRUE;

    size_t alloc_size = 3 * sz;
    buf = (uint8_t *)stress_osal_malloc(alloc_size);
    if (!buf) {
        stress_osal_print("rtos_stress: error: [memcpy] OOM allocating %d bytes\n", alloc_size);
        return;
    }

    str1 = buf;
    str2 = str1 + sz;
    str3 = str2 + sz;

    for (size_t i = 0; i < sz; i++) {
        uint8_t v = (uint8_t)stress_osal_rand();
        str1[i] = v;
        str2[i] = v;
        str3[i] = v;
    }

    stress_osal_print("rtos_stress: info: [memcpy-%d] buffer size: 3 x %d bytes\n", args->instance, sz);

    memcpy_check = memcpy_check_func;
    memmove_check = memmove_check_func;

    if (args->method_name == NULL || stress_osal_strcmp(args->method_name, "all") == 0) {
        run_all = STRESS_TRUE;
        stress_osal_print("rtos_stress: info: [memcpy-%d] using 'all' methods\n", args->instance);
    } else {
        for (int i = 0; stress_memcpy_methods[i].name != NULL; i++) {
            if (stress_osal_strcmp(args->method_name, stress_memcpy_methods[i].name) == 0) {
                specific_func = stress_memcpy_methods[i].func;
                break;
            }
        }
        if (!specific_func) {
            stress_osal_print("rtos_stress: error: unknown method '%s', using 'all'\n", args->method_name);
            run_all = STRESS_TRUE;
        } else {
            stress_osal_print("rtos_stress: info: [memcpy-%d] using method '%s'\n", args->instance, args->method_name);
        }
    }

    while (memcpy_okay && stress_continue(args))
    {
        if (run_all) {
            for (int i = 0; stress_memcpy_methods[i].name != NULL; i++) {
                if (!memcpy_okay || !stress_continue(args)) break;

                stress_memcpy_methods[i].func(args, str1, str2, str3);
            }
        } else {
            specific_func(args, str1, str2, str3);
        }

        stress_osal_sleep_ms(1);
    }

    stress_osal_free(buf);
}
