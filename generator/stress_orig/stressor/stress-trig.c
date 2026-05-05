/* applications/stress-ng/stress-trig.c */
#include "stress-ng.h"
#include <stdlib.h>
#include <string.h>
#include "stress_osal.h"
#include <stress-config.h>

#define PI  (3.14159265358979323846264338327950288419716939937511L)

#ifndef OPTIMIZE3
#define OPTIMIZE3
#endif

static int32_t s_trig_loops = DEFAULT_TRIG_LOOPS;

static int stress_trig_opt_loops(const char *opt_name, const char *opt_arg)
{
    int val = atoi(opt_arg);
    if (val < MIN_TRIG_LOOPS) val = MIN_TRIG_LOOPS;
    s_trig_loops = val;
    stress_osal_print("rtos_stress: debug: trig-loops set to %d\n", s_trig_loops);
    return 0;
}

const stress_opt_t stress_trig_opts[] = {
    { "trig-loops", stress_trig_opt_loops },
    { NULL, NULL }
};

/* ------------------------------------------------------------------ */
/* 动态计算各测试的参考期望值                                             */
/* 用 OPTIMIZE0 + volatile 防止编译器对参考计算做与被测函数相同的优化，      */
/* 保证参考值和被测值走的是不同代码路径。                                   */
/* ------------------------------------------------------------------ */

/*
 * cos/sin 类：对等间距角度求和，理论精确值为 0。
 * 直接和 0.0 比较，无需动态计算参考值。
 */

/*
 * tan 类：起点 theta=3.0，终点趋近 PI，理论值无法简单闭合。
 * 必须动态计算参考值。
 * 用 volatile + OPTIMIZE0 确保参考路径和被测路径完全独立。
 */
static __attribute__((optimize("-O0"))) double compute_tan_ref_d(void)
{
    volatile double sum   = 0.0;
    volatile double theta = 3.0;
    const    double dtheta = ((double)PI - 3.0) / (double)s_trig_loops;
    int i;
    for (i = 0; i < s_trig_loops; i++) {
        sum   += stress_osal_tan((double)theta);
        theta += dtheta;
    }
    return (double)sum;
}

static __attribute__((optimize("-O0"))) double compute_tanf_ref_d(void)
{
    volatile double sum   = 0.0;
    volatile double theta = 3.0;
    const    double dtheta = ((double)PI - 3.0) / (double)s_trig_loops;
    int i;
    for (i = 0; i < s_trig_loops; i++) {
        sum   += (double)stress_osal_tanf((float)theta);
        theta += dtheta;
    }
    return (double)sum;
}

static __attribute__((optimize("-O0"))) long double compute_tanl_ref_ld(void)
{
    volatile long double sum   = 0.0L;
    volatile long double theta = 3.0L;
    const    long double dtheta = ((long double)PI - 3.0L) / (long double)s_trig_loops;
    int i;
    for (i = 0; i < s_trig_loops; i++) {
        sum   += stress_osal_tanl((long double)theta);
        theta += dtheta;
    }
    return (long double)sum;
}

/* ------------------------------------------------------------------ */
/* 各三角函数压测                                                         */
/* ------------------------------------------------------------------ */

static int OPTIMIZE3 stress_trig_cos(void)
{
    double sumcos = 0.0;
    double theta  = 0.0;
    const double dtheta   = (PI * 2.0) / (double)s_trig_loops;
    const double precision = 1E-7;
    int i;

    for (i = 0; i < s_trig_loops; i++) {
        sumcos += stress_osal_cos(theta);
        theta  += dtheta;
    }

    /* 理论期望：对全周期等间距采样求和 = 0 */
    if (stress_osal_fabs(sumcos) > precision) {
        stress_osal_print(
            "rtos_stress: fail: [trig] cos error detected, sum=%f, expect=0.0\n",
            sumcos);
        return -1;
    }
    return 0;
}

static int OPTIMIZE3 stress_trig_cosf(void)
{
    double sumcos = 0.0;
    double theta  = 0.0;
    const double dtheta    = (PI * 2.0) / (double)s_trig_loops;
    const double precision = 1E-4;
    int i;

    for (i = 0; i < s_trig_loops; i++) {
        sumcos += (double)stress_osal_cosf((float)theta);
        theta  += dtheta;
    }

    if (stress_osal_fabs(sumcos) > precision) {
        stress_osal_print(
            "rtos_stress: fail: [trig] cosf error detected, sum=%f, expect=0.0\n",
            sumcos);
        return -1;
    }
    return 0;
}

static int OPTIMIZE3 stress_trig_cosl(void)
{
    long double sumcos = 0.0L;
    long double theta  = 0.0L;
    const long double dtheta    = (PI * 2.0L) / (long double)s_trig_loops;
    const long double precision = 1E-7L;
    int i;

    for (i = 0; i < s_trig_loops; i++) {
        sumcos += stress_osal_cosl(theta);
        theta  += dtheta;
    }

    if (stress_osal_fabsl(sumcos) > precision) {
        stress_osal_print(
            "rtos_stress: fail: [trig] cosl error detected, sum=%f, expect=0.0\n",
            (double)sumcos);
        return -1;
    }
    return 0;
}

static int OPTIMIZE3 stress_trig_sin(void)
{
    double sumsin = 0.0;
    double theta  = 0.0;
    const double dtheta    = (PI * 2.0) / (double)s_trig_loops;
    const double precision = 1E-7;
    int i;

    for (i = 0; i < s_trig_loops; i++) {
        sumsin += stress_osal_sin(theta);
        theta  += dtheta;
    }

    if (stress_osal_fabs(sumsin) > precision) {
        stress_osal_print("rtos_stress: fail: [trig] sin error detected, sum=%f, expect=0.0\n",
                          sumsin);
        return -1;
    }
    return 0;
}

static int OPTIMIZE3 stress_trig_sinf(void)
{
    double sumsin = 0.0;
    double theta  = 0.0;
    const double dtheta    = (PI * 2.0) / (double)s_trig_loops;
    const double precision = 1E-4;
    int i;

    for (i = 0; i < s_trig_loops; i++) {
        sumsin += (double)stress_osal_sinf((float)theta);
        theta  += dtheta;
    }

    if (stress_osal_fabs(sumsin) > precision) {
        stress_osal_print("rtos_stress: fail: [trig] sinf error detected, sum=%f, expect=0.0\n",
                          sumsin);
        return -1;
    }
    return 0;
}

static int OPTIMIZE3 stress_trig_sinl(void)
{
    long double sumsin = 0.0L;
    long double theta  = 0.0L;
    const long double dtheta    = (PI * 2.0L) / (long double)s_trig_loops;
    const long double precision = 1E-7L;
    int i;

    for (i = 0; i < s_trig_loops; i++) {
        sumsin += stress_osal_sinl(theta);
        theta  += dtheta;
    }

    if (stress_osal_fabsl(sumsin) > precision) {
        stress_osal_print("rtos_stress: fail: [trig] sinl error detected, sum=%f, expect=0.0\n",
                          (double)sumsin);
        return -1;
    }
    return 0;
}

static int OPTIMIZE3 stress_trig_sincos(void)
{
    double sumsin = 0.0, sumcos = 0.0;
    double theta  = 0.0;
    const double dtheta    = (PI * 2.0) / (double)s_trig_loops;
    const double precision = 1E-7;
    int i;

    for (i = 0; i < s_trig_loops; i++) {
        sumsin += stress_osal_sin(theta);
        sumcos += stress_osal_cos(theta);
        theta  += dtheta;
    }

    if ((stress_osal_fabs(sumsin) > precision) ||
        (stress_osal_fabs(sumcos) > precision)) {
        stress_osal_print(
            "rtos_stress: fail: [trig] sincos error detected, "
            "sumsin=%f, sumcos=%f, expect both=0.0\n",
            sumsin, sumcos);
        return -1;
    }
    return 0;
}

/*
 * tan 系列：
 *   期望值用 OPTIMIZE0 参考实现在运行时动态计算，
 *   然后用 OPTIMIZE3 被测实现计算，二者比较。
 *
 *   precision 用相对误差：|result - ref| / (|ref| + 1) < threshold
 *   避免在 ref 接近 0 时误报。
 */
static int stress_trig_tan(void)
{
    const double precision = 1E-6;
    double ref, result;
    double sumtan = 0.0;
    double theta  = 3.0;
    const double dtheta = ((double)PI - 3.0) / (double)s_trig_loops;
    int i;

    /* 步骤1：用 OPTIMIZE0 参考实现计算期望值 */
    ref = compute_tan_ref_d();

    /* 步骤2：用 OPTIMIZE3 被测实现计算结果 */
    for (i = 0; i < s_trig_loops; i++) {
        sumtan += stress_osal_tan(theta);
        theta  += dtheta;
    }
    result = sumtan;

    /* 步骤3：相对误差比较 */
    if (stress_osal_fabs(result - ref) >
        precision * (stress_osal_fabs(ref) + 1.0)) {
        stress_osal_print(
            "rtos_stress: fail: [trig] tan error detected, "
            "sum=%f, ref=%f, loops=%d\n",
            result, ref, s_trig_loops);
        return -1;
    }
    return 0;
}

static int stress_trig_tanf(void)
{
    const double precision = 1E-3;
    double ref, result;
    double sumtan = 0.0;
    double theta  = 3.0;
    const double dtheta = ((double)PI - 3.0) / (double)s_trig_loops;
    int i;

    ref = compute_tanf_ref_d();

    for (i = 0; i < s_trig_loops; i++) {
        sumtan += (double)stress_osal_tanf((float)theta);
        theta  += dtheta;
    }
    result = sumtan;

    if (stress_osal_fabs(result - ref) >
        precision * (stress_osal_fabs(ref) + 1.0)) {
        stress_osal_print(
            "rtos_stress: fail: [trig] tanf error detected, "
            "sum=%f, ref=%f, loops=%d\n",
            result, ref, s_trig_loops);
        return -1;
    }
    return 0;
}

static int stress_trig_tanl(void)
{
    const long double precision = 1E-6L;
    long double ref, result;
    long double sumtan = 0.0L;
    long double theta  = 3.0L;
    const long double dtheta = ((long double)PI - 3.0L) / (long double)s_trig_loops;
    int i;

    ref = compute_tanl_ref_ld();

    for (i = 0; i < s_trig_loops; i++) {
        sumtan += stress_osal_tanl(theta);
        theta  += dtheta;
    }
    result = sumtan;

    if (stress_osal_fabsl(result - ref) >
        precision * (stress_osal_fabsl(ref) + 1.0L)) {
        stress_osal_print(
            "rtos_stress: fail: [trig] tanl error detected, "
            "sum=%f, ref=%f, loops=%d\n",
            (double)result, (double)ref, s_trig_loops);
        return -1;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* 方法表 & 入口（保持原有外部接口不变）                                   */
/* ------------------------------------------------------------------ */

typedef struct {
    const char *name;
    int (*func)(void);
} stress_trig_method_info_t;

static const stress_trig_method_info_t trig_methods[] = {
    { "cos",    stress_trig_cos    },
    { "cosf",   stress_trig_cosf   },
    { "cosl",   stress_trig_cosl   },
    { "sin",    stress_trig_sin    },
    { "sinf",   stress_trig_sinf   },
    { "sinl",   stress_trig_sinl   },
    { "sincos", stress_trig_sincos },
    { "tan",    stress_trig_tan    },
    { "tanf",   stress_trig_tanf   },
    { "tanl",   stress_trig_tanl   },
    { NULL,     NULL               }
};

void stress_trig(stress_args_t *args)
{
    int (*specific_func)(void) = NULL;
    stress_bool_t run_all = STRESS_FALSE;

    stress_osal_print("rtos_stress: info: [trig-%d] starting trigonometric stressor\n",
                      args->instance);

    if (args->method_name == NULL ||
        stress_osal_strcmp(args->method_name, "all") == 0) {
        run_all = STRESS_TRUE;
        stress_osal_print("rtos_stress: info: [trig-%d] using 'all' methods\n",
                          args->instance);
    } else {
        int i;
        for (i = 0; trig_methods[i].name != NULL; i++) {
            if (stress_osal_strcmp(args->method_name, trig_methods[i].name) == 0) {
                specific_func = trig_methods[i].func;
                break;
            }
        }
        if (!specific_func) {
            stress_osal_print(
                "rtos_stress: error: [trig-%d] unknown method '%s', using 'all'\n",
                args->instance, args->method_name);
            run_all = STRESS_TRUE;
        } else {
            stress_osal_print("rtos_stress: info: [trig-%d] using method '%s'\n",
                              args->instance, args->method_name);
        }
    }

    while (stress_continue(args)) {
        if (run_all) {
            int i;
            for (i = 0; trig_methods[i].name != NULL; i++) {
                if (!stress_continue(args)) break;
                trig_methods[i].func();
            }
        } else {
            specific_func();
        }

        args->bogo.current_ops++;
        stress_osal_sleep_ms(1);
    }
}
