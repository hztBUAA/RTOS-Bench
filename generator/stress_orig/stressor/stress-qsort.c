/* applications/stress-ng/stress-qsort.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <stress-config.h>

#define UNLIKELY(x)     __builtin_expect(!!(x), 0)
#define LIKELY(x)       __builtin_expect(!!(x), 1)
#define RESTRICT        __restrict__

#define ALWAYS_INLINE inline
#define PRAGMA_UNROLL_N(x)

typedef int (*comp_func_t)(const void *v1, const void *v2);
typedef void (*qsort_func_t)(void *base, size_t nmemb, size_t size, comp_func_t cmp);

static int32_t s_qsort_size = DEFAULT_QSORT_SIZE;

static int stress_qsort_opt_size(const char *opt_name, const char *opt_arg)
{
    char *endptr;
    unsigned long long val = strtoull(opt_arg, &endptr, 10);

    if (*endptr == 'k' || *endptr == 'K') val *= 1024;
    else if (*endptr == 'm' || *endptr == 'M') val *= (1024 * 1024);

    if (val < MIN_QSORT_SIZE) val = MIN_QSORT_SIZE;
    if (val > MAX_QSORT_SIZE) val = MAX_QSORT_SIZE;

    s_qsort_size = (int32_t)val;
    stress_osal_print("rtos_stress: debug: qsort-size set to %d elements\n", s_qsort_size);
    return 0;
}

const stress_opt_t stress_qsort_opts[] = {
    { "qsort-size", stress_qsort_opt_size },
    { NULL, NULL }
};

static int cmp_fwd_int32(const void *v1, const void *v2)
{
    const int32_t i1 = *(const int32_t *)v1;
    const int32_t i2 = *(const int32_t *)v2;
    if (i1 < i2) return -1;
    return (i1 > i2);
}

static int cmp_rev_int32(const void *v1, const void *v2)
{
    const int32_t i1 = *(const int32_t *)v1;
    const int32_t i2 = *(const int32_t *)v2;
    if (i1 > i2) return -1;
    return (i1 < i2);
}

static void stress_sort_shuffle(int32_t *data, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) {
        size_t j = (size_t)stress_osal_rand() % n;
        int32_t tmp = data[i];
        data[i] = data[j];
        data[j] = tmp;
    }
}

static void stress_sort_init(int32_t *data, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) {
        data[i] = (int32_t)i;
    }
}

static int stress_qsort_verify_forward(stress_args_t *args, int32_t *data, size_t n)
{
    size_t i;
    for (i = 0; i < n - 1; i++) {
        if (data[i] > data[i+1]) {
            stress_osal_print("rtos_stress: %s: forward sort error at index %d (%d > %d)\n",
                       args->name, i, data[i], data[i+1]);
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

static int stress_qsort_verify_reverse(stress_args_t *args, int32_t *data, size_t n)
{
    size_t i;
    for (i = 0; i < n - 1; i++) {
        if (data[i] < data[i+1]) {
            stress_osal_print("rtos_stress: %s: reverse sort error at index %d (%d < %d)\n",
                       args->name, i, data[i], data[i+1]);
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

typedef uint32_t qsort_swap_type_t;

static inline size_t qsort_bm_minimum(const size_t x, const size_t y)
{
    return x <= y ? x : y;
}

static inline uint8_t ALWAYS_INLINE *qsort_bm_med3(uint8_t *a, uint8_t *b, uint8_t *c, comp_func_t cmp)
{
    return (cmp(a, b) < 0) ?
        ((cmp(b, c) < 0) ? b : (cmp(a, c) < 0) ? c : a) :
        ((cmp(b, c) > 0) ? b : (cmp(a, c) > 0) ? c : a);
}

static inline void ALWAYS_INLINE qsort_bm_swapfunc(uint8_t *a, uint8_t *b, size_t n, int swaptype)
{
    if (swaptype <= 1) {
        register qsort_swap_type_t * RESTRICT pi = (qsort_swap_type_t *)a;
        register qsort_swap_type_t * RESTRICT pj = (qsort_swap_type_t *)b;
        do {
            register qsort_swap_type_t tmp;
            tmp = *pi; *pi++ = *pj; *pj++ = tmp;
        } while ((n -= sizeof(qsort_swap_type_t)) > 0);
    } else {
        register uint8_t * RESTRICT pi = (uint8_t *)a;
        register uint8_t * RESTRICT pj = (uint8_t *)b;
        do {
            register uint8_t tmp;
            tmp = *pi; *pi++ = *pj; *pj++ = tmp;
        } while ((n -= sizeof(uint8_t)) > 0);
    }
}

static inline void ALWAYS_INLINE qsort_bm_swap(uint8_t *a, uint8_t *b, const size_t es, const int swaptype)
{
    if (swaptype == 0) {
        register qsort_swap_type_t tmp;
        tmp = *(qsort_swap_type_t *)a;
        *(qsort_swap_type_t *)a = *(qsort_swap_type_t *)b;
        *(qsort_swap_type_t *)b = tmp;
    } else {
        qsort_bm_swapfunc(a, b, es, swaptype);
    }
}

#define THRESH 63

static void qsort_bm(void *base, size_t n, size_t es, comp_func_t cmp)
{
    uint8_t *a = (uint8_t *)base;
    const int swaptype = (((uintptr_t)a | (uintptr_t)es) % sizeof(qsort_swap_type_t)) ?
        2 : (es > sizeof(qsort_swap_type_t));
    uint8_t *pa, *pb, *pc, *pd, *pm, *pn, *pv;
    size_t s;
    qsort_swap_type_t v;

    if (n < THRESH) {
        for (pm = a + es; pm < a + (n * es); pm += es) {
            register uint8_t *p;
            for (p = pm; (p > a) && (cmp(p - es, p) > 0); p -= es) {
                qsort_bm_swap(p, p - es, es, swaptype);
            }
        }
        return;
    }
    pm = a + (n >> 1) * es;
    if (n > THRESH) {
        register uint8_t *p = a;
        pn = a + (n - 1) * es;
        if (n > 63) {
            s = (n >> 3) * es;
            p = qsort_bm_med3(p, p + s, p + (s << 1), cmp);
            pm = qsort_bm_med3(pm - s, pm, pm + s, cmp);
            pn = qsort_bm_med3(pn - (s << 1), pn - s, pn, cmp);
        }
        pm = qsort_bm_med3(p, pm, pn, cmp);
    }

    if (swaptype != 0) {
        pv = a;
        qsort_bm_swap(pv, pm, es, swaptype);
    } else {
        pv = (uint8_t *)&v;
        *(qsort_swap_type_t *)pv = *(qsort_swap_type_t *)pm;
    }

    pa = pb = a;
    pc = pd = a + (n - 1) * es;
    for (;;) {
        int r;
        while ((pb <= pc) && (r = cmp(pb, pv)) <= 0) {
            if (r == 0) {
                qsort_bm_swap(pa, pb, es, swaptype);
                pa += es;
            }
            pb += es;
        }
        while ((pb <= pc) && (r = cmp(pc, pv)) >= 0) {
            if (r == 0) {
                qsort_bm_swap(pc, pd, es, swaptype);
                pd -= es;
            }
            pc -= es;
        }
        if (pb > pc) break;
        qsort_bm_swap(pb, pc, es, swaptype);
        pb += es;
        pc -= es;
    }
    pn = a + (n * es);
    s = qsort_bm_minimum(pa - a, pb - pa);
    if (s > 0) qsort_bm_swapfunc(a, pb - s, s, swaptype);
    s = qsort_bm_minimum(pd - pc, pn - pd - es);
    if (s > 0) qsort_bm_swapfunc(pb, pn-s, s, swaptype);
    s = pb - pa;
    if (s > es) qsort_bm(a, s / es, es, cmp);
    s = pd - pc;
    if (s > es) qsort_bm(pn - s, s / es, es, cmp);
}

typedef struct {
    const char *name;
    qsort_func_t qsort_func;
} stress_qsort_method_t;

static const stress_qsort_method_t stress_qsort_methods[] = {
    { "qsort-libc",     stress_osal_qsort },
    { "qsort-bm",       qsort_bm },
    { NULL,             NULL }
};

void stress_qsort(stress_args_t *args)
{
    size_t n = (size_t)s_qsort_size;
    int32_t *data;
    qsort_func_t qsort_func = NULL;
    stress_bool_t run_all = STRESS_FALSE;

    size_t mem_size = n * sizeof(int32_t);
    data = stress_osal_malloc(mem_size);
    if (!data) {
        stress_osal_print("rtos_stress: error: [qsort] OOM allocating %d bytes\n", mem_size);
        return;
    }
    stress_osal_print("rtos_stress: info: [qsort-%d] sorting %d elements (%d KB)\n",
               args->instance, n, mem_size/1024);

    if (args->method_name == NULL || stress_osal_strcmp(args->method_name, "all") == 0) {
        run_all = STRESS_TRUE;
        stress_osal_print("rtos_stress: info: [qsort-%d] using 'all' methods\n", args->instance);
    } else {
        for (int i = 0; stress_qsort_methods[i].name != NULL; i++) {
            if (stress_osal_strcmp(args->method_name, stress_qsort_methods[i].name) == 0) {
                qsort_func = stress_qsort_methods[i].qsort_func;
                break;
            }
        }
        if (!qsort_func) {
            stress_osal_print("rtos_stress: error: unknown method '%s', using 'all'\n", args->method_name);
            run_all = STRESS_TRUE;
        } else {
            stress_osal_print("rtos_stress: info: [qsort-%d] using method '%s'\n", args->instance, args->method_name);
        }
    }

    stress_sort_init(data, n);

    while (stress_continue(args))
    {
        int m_start = 0;
        int m_end = 0;

        if (run_all) {
            while (stress_qsort_methods[m_end].name != NULL) m_end++;
        } else {
            for (int i = 0; stress_qsort_methods[i].name != NULL; i++) {
                if (stress_qsort_methods[i].qsort_func == qsort_func) {
                    m_start = i;
                    m_end = i + 1;
                    break;
                }
            }
        }

        for (int i = m_start; i < m_end; i++) {
            qsort_func_t curr_func = stress_qsort_methods[i].qsort_func;

            if (!stress_continue(args)) break;

            stress_sort_shuffle(data, n);

            curr_func(data, n, sizeof(int32_t), cmp_fwd_int32);
            if (stress_qsort_verify_forward(args, data, n) != EXIT_SUCCESS) goto cleanup;

            curr_func(data, n, sizeof(int32_t), cmp_rev_int32);
            if (stress_qsort_verify_reverse(args, data, n) != EXIT_SUCCESS) goto cleanup;

            args->bogo.current_ops++;

            stress_osal_sleep_ms(1);
        }
    }

cleanup:
    stress_osal_free(data);
}
