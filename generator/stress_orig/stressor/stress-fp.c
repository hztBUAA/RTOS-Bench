/* applications/stress-ng/stress-fp.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stress-config.h>

#define FP_ELEMENTS     (8)

#define UNLIKELY(x)     __builtin_expect(!!(x), 0)
#define LIKELY(x)       __builtin_expect(!!(x), 1)
#define NOINLINE        __attribute__((noinline))

#ifndef OPTIMIZE3
#define OPTIMIZE3       /* __attribute__((optimize("-O3"))) */
#endif

typedef struct {
    struct {
        long double r_init;
        long double r[2];
        long double add;
        long double add_rev;
        long double mul;
        long double mul_rev;
    } ld;
    struct {
        double r_init;
        double r[2];
        double add;
        double add_rev;
        double mul;
        double mul_rev;
    } d;
    struct {
        float r_init;
        float r[2];
        float add;
        float add_rev;
        float mul;
        float mul_rev;
    } f;
} fp_data_t;

typedef void (*stress_fp_func_t)(fp_data_t *data);

static int32_t s_fp_loops = DEFAULT_FP_LOOPS;

static int stress_fp_opt_loops(const char *opt_name, const char *opt_arg)
{
    int val = atoi(opt_arg);
    if (val < MIN_FP_LOOPS) val = MIN_FP_LOOPS;

    s_fp_loops = val;
    stress_osal_print("rtos_stress: debug: fp-loops set to %d\n", s_fp_loops);
    return 0;
}

const stress_opt_t stress_fp_opts[] = {
    { "fp-loops", stress_fp_opt_loops },
    { NULL, NULL }
};

static uint32_t stress_mwc32(void) { return (uint32_t)stress_osal_rand(); }

#define STRESS_FP_ADD(field, name)                          \
static void OPTIMIZE3 name(fp_data_t *fp_data)              \
{                                                           \
    register int i;                                         \
    const int loops = s_fp_loops;                           \
    const int idx = 0;                                      \
                                                            \
    for (i = 0; i < FP_ELEMENTS; i++) {                     \
        fp_data[i].field.r[idx] = fp_data[i].field.r_init;  \
    }                                                       \
                                                            \
    for (i = 0; i < loops ; i++) {                          \
        fp_data[0].field.r[idx] += fp_data[0].field.add;    \
        fp_data[0].field.r[idx] += fp_data[0].field.add_rev;\
        fp_data[1].field.r[idx] += fp_data[1].field.add;    \
        fp_data[1].field.r[idx] += fp_data[1].field.add_rev;\
        fp_data[2].field.r[idx] += fp_data[2].field.add;    \
        fp_data[2].field.r[idx] += fp_data[2].field.add_rev;\
        fp_data[3].field.r[idx] += fp_data[3].field.add;    \
        fp_data[3].field.r[idx] += fp_data[3].field.add_rev;\
        fp_data[4].field.r[idx] += fp_data[4].field.add;    \
        fp_data[4].field.r[idx] += fp_data[4].field.add_rev;\
        fp_data[5].field.r[idx] += fp_data[5].field.add;    \
        fp_data[5].field.r[idx] += fp_data[5].field.add_rev;\
        fp_data[6].field.r[idx] += fp_data[6].field.add;    \
        fp_data[6].field.r[idx] += fp_data[6].field.add_rev;\
        fp_data[7].field.r[idx] += fp_data[7].field.add;    \
        fp_data[7].field.r[idx] += fp_data[7].field.add_rev;\
    }                                                       \
}

#define STRESS_FP_SUB(field, name)                          \
static void OPTIMIZE3 name(fp_data_t *fp_data)              \
{                                                           \
    register int i;                                         \
    const int loops = s_fp_loops;                           \
    const int idx = 0;                                      \
                                                            \
    for (i = 0; i < FP_ELEMENTS; i++) {                     \
        fp_data[i].field.r[idx] = fp_data[i].field.r_init;  \
    }                                                       \
                                                            \
    for (i = 0; i < loops ; i++) {                          \
        fp_data[0].field.r[idx] -= fp_data[0].field.add;    \
        fp_data[0].field.r[idx] -= fp_data[0].field.add_rev;\
        fp_data[1].field.r[idx] -= fp_data[1].field.add;    \
        fp_data[1].field.r[idx] -= fp_data[1].field.add_rev;\
        fp_data[2].field.r[idx] -= fp_data[2].field.add;    \
        fp_data[2].field.r[idx] -= fp_data[2].field.add_rev;\
        fp_data[3].field.r[idx] -= fp_data[3].field.add;    \
        fp_data[3].field.r[idx] -= fp_data[3].field.add_rev;\
        fp_data[4].field.r[idx] -= fp_data[4].field.add;    \
        fp_data[4].field.r[idx] -= fp_data[4].field.add_rev;\
        fp_data[5].field.r[idx] -= fp_data[5].field.add;    \
        fp_data[5].field.r[idx] -= fp_data[5].field.add_rev;\
        fp_data[6].field.r[idx] -= fp_data[6].field.add;    \
        fp_data[6].field.r[idx] -= fp_data[6].field.add_rev;\
        fp_data[7].field.r[idx] -= fp_data[7].field.add;    \
        fp_data[7].field.r[idx] -= fp_data[7].field.add_rev;\
    }                                                       \
}

#define STRESS_FP_MUL(field, name)                          \
static void OPTIMIZE3 name(fp_data_t *fp_data)              \
{                                                           \
    register int i;                                         \
    const int loops = s_fp_loops;                           \
    const int idx = 0;                                      \
                                                            \
    for (i = 0; i < FP_ELEMENTS; i++) {                     \
        fp_data[i].field.r[idx] = fp_data[i].field.r_init;  \
    }                                                       \
                                                            \
    for (i = 0; i < loops ; i++) {                          \
        fp_data[0].field.r[idx] *= fp_data[0].field.mul;    \
        fp_data[0].field.r[idx] *= fp_data[0].field.mul_rev;\
        fp_data[1].field.r[idx] *= fp_data[1].field.mul;    \
        fp_data[1].field.r[idx] *= fp_data[1].field.mul_rev;\
        fp_data[2].field.r[idx] *= fp_data[2].field.mul;    \
        fp_data[2].field.r[idx] *= fp_data[2].field.mul_rev;\
        fp_data[3].field.r[idx] *= fp_data[3].field.mul;    \
        fp_data[3].field.r[idx] *= fp_data[3].field.mul_rev;\
        fp_data[4].field.r[idx] *= fp_data[4].field.mul;    \
        fp_data[4].field.r[idx] *= fp_data[4].field.mul_rev;\
        fp_data[5].field.r[idx] *= fp_data[5].field.mul;    \
        fp_data[5].field.r[idx] *= fp_data[5].field.mul_rev;\
        fp_data[6].field.r[idx] *= fp_data[6].field.mul;    \
        fp_data[6].field.r[idx] *= fp_data[6].field.mul_rev;\
        fp_data[7].field.r[idx] *= fp_data[7].field.mul;    \
        fp_data[7].field.r[idx] *= fp_data[7].field.mul_rev;\
    }                                                       \
}

#define STRESS_FP_DIV(field, name)                          \
static void OPTIMIZE3 name(fp_data_t *fp_data)              \
{                                                           \
    register int i;                                         \
    const int loops = s_fp_loops;                           \
    const int idx = 0;                                      \
                                                            \
    for (i = 0; i < FP_ELEMENTS; i++) {                     \
        fp_data[i].field.r[idx] = fp_data[i].field.r_init;  \
    }                                                       \
                                                            \
    for (i = 0; i < loops ; i++) {                          \
        fp_data[0].field.r[idx] /= fp_data[0].field.mul;    \
        fp_data[0].field.r[idx] /= fp_data[0].field.mul_rev;\
        fp_data[1].field.r[idx] /= fp_data[1].field.mul;    \
        fp_data[1].field.r[idx] /= fp_data[1].field.mul_rev;\
        fp_data[2].field.r[idx] /= fp_data[2].field.mul;    \
        fp_data[2].field.r[idx] /= fp_data[2].field.mul_rev;\
        fp_data[3].field.r[idx] /= fp_data[3].field.mul;    \
        fp_data[3].field.r[idx] /= fp_data[3].field.mul_rev;\
        fp_data[4].field.r[idx] /= fp_data[4].field.mul;    \
        fp_data[4].field.r[idx] /= fp_data[4].field.mul_rev;\
        fp_data[5].field.r[idx] /= fp_data[5].field.mul;    \
        fp_data[5].field.r[idx] /= fp_data[5].field.mul_rev;\
        fp_data[6].field.r[idx] /= fp_data[6].field.mul;    \
        fp_data[6].field.r[idx] /= fp_data[6].field.mul_rev;\
        fp_data[7].field.r[idx] /= fp_data[7].field.mul;    \
        fp_data[7].field.r[idx] /= fp_data[7].field.mul_rev;\
    }                                                       \
}

STRESS_FP_ADD(f, stress_fp_float_add)
STRESS_FP_SUB(f, stress_fp_float_sub)
STRESS_FP_MUL(f, stress_fp_float_mul)
STRESS_FP_DIV(f, stress_fp_float_div)

STRESS_FP_ADD(d, stress_fp_double_add)
STRESS_FP_SUB(d, stress_fp_double_sub)
STRESS_FP_MUL(d, stress_fp_double_mul)
STRESS_FP_DIV(d, stress_fp_double_div)

STRESS_FP_ADD(ld, stress_fp_ldouble_add)
STRESS_FP_SUB(ld, stress_fp_ldouble_sub)
STRESS_FP_MUL(ld, stress_fp_ldouble_mul)
STRESS_FP_DIV(ld, stress_fp_ldouble_div)

typedef struct {
    const char *name;
    stress_fp_func_t func;
} stress_fp_method_info_t;

static const stress_fp_method_info_t stress_fp_methods[] = {
    { "floatadd",   stress_fp_float_add },
    { "floatsub",   stress_fp_float_sub },
    { "floatmul",   stress_fp_float_mul },
    { "floatdiv",   stress_fp_float_div },
    { "doubleadd",  stress_fp_double_add },
    { "doublesub",  stress_fp_double_sub },
    { "doublemul",  stress_fp_double_mul },
    { "doublediv",  stress_fp_double_div },
    { "ldoubleadd", stress_fp_ldouble_add },
    { "ldoublesub", stress_fp_ldouble_sub },
    { "ldoublemul", stress_fp_ldouble_mul },
    { "ldoublediv", stress_fp_ldouble_div },
    { NULL,         NULL }
};

static void stress_fp_init_data(fp_data_t *data)
{
    for (int i = 0; i < FP_ELEMENTS; i++) {
        long double ld;
        uint32_t r;

        r = stress_mwc32();
        ld = (long double)i + (long double)r / ((long double)(1ULL << 30));
        data[i].ld.r_init = ld;
        data[i].ld.r[0] = ld;
        data[i].ld.r[1] = ld;

        data[i].d.r_init = (double)ld;
        data[i].d.r[0] = (double)ld;
        data[i].d.r[1] = (double)ld;

        data[i].f.r_init = (float)ld;
        data[i].f.r[0] = (float)ld;
        data[i].f.r[1] = (float)ld;

        r = stress_mwc32();
        ld = (long double)r / ((long double)(1ULL << 20));
        data[i].ld.add = ld;
        data[i].d.add = (double)ld;
        data[i].f.add = (float)ld;

        ld = -(ld * 0.992);
        data[i].ld.add_rev = ld;
        data[i].d.add_rev = (double)ld;
        data[i].f.add_rev = (float)ld;

        r = stress_mwc32();
        ld = (long double)i + (long double)r / ((long double)(1ULL << 25));
        if (ld < 0.1) ld += 1.0;

        data[i].ld.mul = ld;
        data[i].d.mul = (double)ld;
        data[i].f.mul = (float)ld;

        ld = 0.9995 / ld;
        data[i].ld.mul_rev = ld;
        data[i].d.mul_rev = (double)ld;
        data[i].f.mul_rev = (float)ld;
    }
}

void stress_fp(stress_args_t *args)
{
    fp_data_t *fp_data = NULL;
    stress_fp_func_t specific_func = NULL;
    stress_bool_t run_all = STRESS_FALSE;

    size_t alloc_size = FP_ELEMENTS * sizeof(fp_data_t);
    fp_data = (fp_data_t *)stress_osal_malloc(alloc_size);
    if (!fp_data) {
        stress_osal_print("rtos_stress: error: [fp] OOM allocating fp data\n");
        return;
    }

    stress_fp_init_data(fp_data);
    stress_osal_print("rtos_stress: info: [fp-%d] float/double/long double math stressor\n", args->instance);

    if (args->method_name == NULL || stress_osal_strcmp(args->method_name, "all") == 0) {
        run_all = STRESS_TRUE;
        if (args->instance == 0)
            stress_osal_print("rtos_stress: info: [fp-%d] using 'all' methods\n", args->instance);
    } else {
        for (int i = 0; stress_fp_methods[i].name != NULL; i++) {
            if (stress_osal_strcmp(args->method_name, stress_fp_methods[i].name) == 0) {
                specific_func = stress_fp_methods[i].func;
                break;
            }
        }
        if (!specific_func) {
            stress_osal_print("rtos_stress: error: unknown method '%s', using 'all'\n", args->method_name);
            run_all = STRESS_TRUE;
        } else {
            stress_osal_print("rtos_stress: info: [fp-%d] using method '%s'\n", args->instance, args->method_name);
        }
    }

    while (stress_continue(args))
    {
        if (run_all) {
            for (int i = 0; stress_fp_methods[i].name != NULL; i++) {
                if (!stress_continue(args)) break;
                stress_fp_methods[i].func(fp_data);
            }
        } else {
            specific_func(fp_data);
        }

        args->bogo.current_ops++;

        stress_osal_sleep_ms(1);
    }

    stress_osal_free(fp_data);
}
