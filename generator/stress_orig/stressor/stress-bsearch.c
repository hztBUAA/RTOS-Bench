/* applications/stress-ng/stress-bsearch.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <config.h>

/* ==================================================================
 * 配置与宏定义
 * ================================================================== */



#define LIKELY(x)           __builtin_expect(!!(x), 1)
#define UNLIKELY(x)         __builtin_expect(!!(x), 0)

typedef void * (*bsearch_func_t)(const void *key, const void *base, size_t nmemb, size_t size,
               int (*compare)(const void *p1, const void *p2));

/* ==================================================================
 * 命令行选项解析 (Internal Options)
 * ================================================================== */

static uint32_t s_bsearch_size = DEFAULT_BSEARCH_SIZE;

static int stress_bsearch_opt_size(const char *opt_name, const char *opt_arg)
{
    int val = atoi(opt_arg);

    if (val < MIN_BSEARCH_SIZE) val = MIN_BSEARCH_SIZE;
    if (val > MAX_BSEARCH_SIZE) val = MAX_BSEARCH_SIZE;

    s_bsearch_size = (uint32_t)val;
    stress_osal_print("rtos_stress: debug: bsearch-size set to %u elements\n", s_bsearch_size);
    return 0;
}

const stress_opt_t stress_bsearch_opts[] = {
    { "bsearch-size", stress_bsearch_opt_size },
    { NULL, NULL }
};

/* ==================================================================
 * 辅助函数
 * ================================================================== */

static int cmp_int32(const void *p1, const void *p2)
{
    const int32_t *i1 = (const int32_t *)p1;
    const int32_t *i2 = (const int32_t *)p2;

    if (*i1 < *i2) return -1;
    if (*i1 > *i2) return 1;
    return 0;
}

#define UNCONSTIFY(ptr) ((void *)(uintptr_t)(ptr))

/* ==================================================================
 * 查找算法实现
 * ================================================================== */

/* 1. 手动实现的二分查找 (Non-Libc) */
static void *stress_bsearch_nonlibc(
    const void *key, const void *base, size_t nmemb, size_t size,
    int (*compare)(const void *p1, const void *p2))
{
    size_t lower = 0;
    size_t upper = nmemb;

    while (LIKELY(lower < upper)) {
        const size_t idx = (lower + upper) >> 1;
        const void *ptr = (const char *)base + (idx * size);
        const int cmp = compare(key, ptr);

        if (cmp < 0) {
            upper = idx;
        } else if (cmp > 0) {
            lower = idx + 1;
        } else {
            return UNCONSTIFY(ptr);
        }
    }
    return NULL;
}

/* 2. 三分查找 (Ternary Search) */
static void *stress_bsearch_ternary(
    const void *key, const void *base, size_t nmemb, size_t size,
    int (*compare)(const void *p1, const void *p2))
{
    size_t lower = 0;
    size_t upper = nmemb;

    while (LIKELY(upper >= lower)) {
        if (upper == lower) {
             if (upper < nmemb) {
                const void *ptr = (const char *)base + (upper * size);
                if (compare(key, ptr) == 0) return UNCONSTIFY(ptr);
             }
             break;
        }

        const size_t diff = upper - lower;
        const size_t mid1 = lower + (diff / 3);
        const size_t mid2 = upper - (diff / 3);
        const void *ptr1, *ptr2;
        int cmp1, cmp2;

        ptr1 = (const void *)((const char *)base + (mid1 * size));
        cmp1 = compare(key, ptr1);
        if (cmp1 == 0) return UNCONSTIFY(ptr1);

        ptr2 = (const void *)((const char *)base + (mid2 * size));
        cmp2 = compare(key, ptr2);
        if (cmp2 == 0) return UNCONSTIFY(ptr2);

        if (cmp1 < 0) {
            if (mid1 == 0) break;
            upper = mid1 - 1;
        } else if (cmp2 > 0) {
            lower = mid2 + 1;
        } else {
            lower = mid1 + 1;
            upper = mid2 - 1;
        }
    }
    return NULL;
}

/* ==================================================================
 * 注册表
 * ================================================================== */

typedef struct {
    const char *name;
    bsearch_func_t func;
} stress_bsearch_method_t;

static const stress_bsearch_method_t bsearch_methods[] = {
    { "bsearch-libc",    bsearch },              /* 标准库 bsearch, 在 stdlib.h 中 */
    { "bsearch-nonlibc", stress_bsearch_nonlibc },
    { "ternary",         stress_bsearch_ternary },
    { NULL,              NULL }
};

/* ==================================================================
 * 主入口
 * ================================================================== */

void stress_bsearch(stress_args_t *args)
{
    int32_t *data = NULL;
    size_t n = (size_t)s_bsearch_size;
    size_t mem_size = 0;
    bsearch_func_t bsearch_func = NULL;
    stress_bool_t run_all = STRESS_FALSE;

    /* 内存分配 */
    while (n >= MIN_BSEARCH_SIZE) {
        mem_size = n * sizeof(int32_t);
        data = stress_osal_malloc(mem_size);
        if (data) break;
        n /= 2;
    }

    if (!data) {
        stress_osal_print("rtos_stress: error: [bsearch] OOM allocating memory\n");
        return;
    }

    stress_osal_print("rtos_stress: info: [bsearch-%d] using %d elements (%d KB)\n",
               args->instance, n, mem_size / 1024);

    /* 初始化有序数据 */
    for (size_t i = 0; i < n; i++) {
        data[i] = (int32_t)i;
    }

    /* 方法选择 */
    if (args->method_name == NULL || stress_osal_strcmp(args->method_name, "all") == 0) {
        run_all = STRESS_TRUE;
        stress_osal_print("rtos_stress: info: [bsearch-%d] using 'all' methods\n", args->instance);
    } else {
        for (int i = 0; bsearch_methods[i].name != NULL; i++) {
            if (stress_osal_strcmp(args->method_name, bsearch_methods[i].name) == 0) {
                bsearch_func = bsearch_methods[i].func;
                break;
            }
        }
        if (!bsearch_func) {
            stress_osal_print("rtos_stress: error: unknown method '%s', using 'all'\n", args->method_name);
            run_all = STRESS_TRUE;
        } else {
            stress_osal_print("rtos_stress: info: [bsearch-%d] using method '%s'\n", args->instance, args->method_name);
        }
    }

    /* 核心循环 */
    while (stress_continue(args))
    {
        int m_start = 0;
        int m_end = 0;

        if (run_all) {
            while (bsearch_methods[m_end].name != NULL) m_end++;
        } else {
            for (int i = 0; bsearch_methods[i].name != NULL; i++) {
                if (bsearch_methods[i].func == bsearch_func) {
                    m_start = i;
                    m_end = i + 1;
                    break;
                }
            }
        }

        for (int m = m_start; m < m_end; m++) {
            bsearch_func_t curr_func = bsearch_methods[m].func;

            if (!stress_continue(args)) break;

            for (size_t i = 0; i < n; i++) {
                int32_t *result = curr_func(&data[i], data, n, sizeof(int32_t), cmp_int32);

                if (UNLIKELY(result == NULL || *result != data[i])) {
                    stress_osal_print("rtos_stress: fail: [bsearch] %s failed to find element %d\n",
                               bsearch_methods[m].name, i);
                }

                if (i % 1000 == 0) {
                     if (!stress_continue(args)) break;
                }
            }

            args->bogo.current_ops++;
            stress_osal_sleep_ms(1);
        }
    }

    stress_osal_free(data);
}
