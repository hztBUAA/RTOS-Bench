/* applications/stress-ng/stress-trig.c */
#include "stress-ng.h"
#include <stdlib.h>
#include <string.h>
#include "stress_osal.h"
#include <config.h>

#define PI                  (3.14159265358979323846264338327950288419716939937511L)
#define TANSUM              (-710.4128636743199902703338466380955651402473L)

#ifndef OPTIMIZE3
#define OPTIMIZE3
#endif

static int32_t s_trig_loops = DEFAULT_TRIG_LOOPS;

static int stress_trig_opt_loops(const char *opt_name, const char *opt_arg)
{
    int val = atoi(opt_arg);
    if (val < 1) val = 1;

    s_trig_loops = val;
    stress_osal_print("rtos_stress: debug: trig-loops set to %d\n", s_trig_loops);
    return 0;
}

const stress_opt_t stress_trig_opts[] = {
    { "trig-loops", stress_trig_opt_loops },
    { NULL, NULL }
};

static int OPTIMIZE3 stress_trig_cos(void)
{
    double sumcos = 0.0;
    double theta = 0.0;
    const double dtheta = (PI * 2.0) / (double)s_trig_loops;
    const double precision = 1E-7;
    int i;

    for (i = 0; i < s_trig_loops; i++) {
        sumcos += stress_osal_cos(theta);
        theta += dtheta;
    }

    if (stress_osal_fabs(sumcos - (double)0.0) > precision) {
        stress_osal_print("rtos_stress: fail: [trig] cos error detected, sum=%f\n", sumcos);
        return -1;
    }
    return 0;
}

static int OPTIMIZE3 stress_trig_cosf(void)
{
    double sumcos = 0.0;
    double theta = 0.0;
    const double dtheta = (PI * 2.0) / (float)s_trig_loops;
    const double precision = 1E-4;
    int i;

    for (i = 0; i < s_trig_loops; i++) {
        sumcos += (double)stress_osal_cosf((float)theta);
        theta += dtheta;
    }

    if (stress_osal_fabs(sumcos - (float)0.0) > precision) {
        stress_osal_print("rtos_stress: fail: [trig] cosf error detected, sum=%f\n", sumcos);
        return -1;
    }
    return 0;
}

static int OPTIMIZE3 stress_trig_cosl(void)
{
    long double sumcos = 0.0L;
    long double theta = 0.0L;
    const long double dtheta = (PI * 2.0L) / (long double)s_trig_loops;
    long double precision = 1E-7;
    int i;

    for (i = 0; i < s_trig_loops; i++) {
        sumcos += stress_osal_cosl(theta);
        theta += dtheta;
    }

    if (stress_osal_fabsl(sumcos - (long double)0.0) > precision) {
        stress_osal_print("rtos_stress: fail: [trig] cosl error detected, sum=%f\n", (double)sumcos);
        return -1;
    }
    return 0;
}

static int OPTIMIZE3 stress_trig_sin(void)
{
    double sumsin = 0.0;
    double theta = 0.0;
    const double dtheta = (PI * 2.0) / (double)s_trig_loops;
    const double precision = 1E-7;
    int i;

    for (i = 0; i < s_trig_loops; i++) {
        sumsin += stress_osal_sin(theta);
        theta += dtheta;
    }

    if (stress_osal_fabs(sumsin - (double)0.0) > precision) {
        stress_osal_print("rtos_stress: fail: [trig] sin error detected\n");
        return -1;
    }
    return 0;
}

static int OPTIMIZE3 stress_trig_sinf(void)
{
    double sumsin = 0.0;
    double theta = 0.0;
    const double dtheta = (PI * 2.0) / (float)s_trig_loops;
    const double precision = 1E-4;
    int i;

    for (i = 0; i < s_trig_loops; i++) {
        sumsin += (double)stress_osal_sinf((float)theta);
        theta += dtheta;
    }

    if (stress_osal_fabs(sumsin - (float)0.0) > precision) {
        stress_osal_print("rtos_stress: fail: [trig] sinf error detected\n");
        return -1;
    }
    return 0;
}

static int OPTIMIZE3 stress_trig_sinl(void)
{
    long double sumsin = 0.0L;
    long double theta = 0.0L;
    const long double dtheta = (PI * 2.0L) / (long double)s_trig_loops;
    long double precision = 1E-7;
    int i;

    for (i = 0; i < s_trig_loops; i++) {
        sumsin += stress_osal_sinl(theta);
        theta += dtheta;
    }

    if (stress_osal_fabsl(sumsin - (long double)0.0) > precision) {
        stress_osal_print("rtos_stress: fail: [trig] sinl error detected\n");
        return -1;
    }
    return 0;
}

static int OPTIMIZE3 stress_trig_sincos(void)
{
    double sumsin = 0.0, sumcos = 0.0;
    double theta = 0.0;
    const double dtheta = (PI * 2.0) / (double)s_trig_loops;
    const double precision = 1E-7;
    int i;

    for (i = 0; i < s_trig_loops; i++) {
        double c, s;
        s = stress_osal_sin(theta);
        c = stress_osal_cos(theta);
        sumsin += s;
        sumcos += c;
        theta += dtheta;
    }

    if ((stress_osal_fabs(sumsin - (double)0.0) > precision) ||
        (stress_osal_fabs(sumcos - (double)0.0) > precision)) {
        stress_osal_print("rtos_stress: fail: [trig] sincos error detected\n");
        return -1;
    }
    return 0;
}

static int OPTIMIZE3 stress_trig_tan(void)
{
    double sumtan = 0.0;
    double theta = 3.0;
    const double dtheta = ((double)PI - theta) / (double)s_trig_loops;
    const double precision = 1E-7;
    int i;

    for (i = 0; i < s_trig_loops; i++) {
        sumtan += stress_osal_tan(theta);
        theta += dtheta;
    }

    if (stress_osal_fabs(sumtan - (double)TANSUM) > precision) {
        stress_osal_print("rtos_stress: fail: [trig] tan error detected, sum=%f, expect=%f\n", sumtan, (double)TANSUM);
        return -1;
    }
    return 0;
}

static int OPTIMIZE3 stress_trig_tanf(void)
{
    double sumtan = 0.0;
    double theta = 3.0;
    const double dtheta = ((double)PI - theta) / (double)s_trig_loops;
    const double precision = 1E-5;
    int i;

    for (i = 0; i < s_trig_loops; i++) {
        sumtan += stress_osal_tanf((float)theta);
        theta += dtheta;
    }

    if (stress_osal_fabs(sumtan - (double)TANSUM) > precision) {
        stress_osal_print("rtos_stress: fail: [trig] tanf error detected\n");
        return -1;
    }
    return 0;
}

static int OPTIMIZE3 stress_trig_tanl(void)
{
    long double sumtan = 0.0;
    long double theta = 3.0;
    const long double dtheta = ((long double)PI - theta) / (long double)s_trig_loops;
    const long double precision = 1E-7;
    int i;

    for (i = 0; i < s_trig_loops; i++) {
        sumtan += stress_osal_tanl(theta);
        theta += dtheta;
    }

    if (stress_osal_fabsl(sumtan - (long double)TANSUM) > precision) {
        stress_osal_print("rtos_stress: fail: [trig] tanl error detected\n");
        return -1;
    }
    return 0;
}

typedef struct {
    const char *name;
    int (*func)(void);
} stress_trig_method_info_t;

static const stress_trig_method_info_t trig_methods[] = {
    { "cos",    stress_trig_cos },
    { "cosf",   stress_trig_cosf },
    { "cosl",   stress_trig_cosl },
    { "sin",    stress_trig_sin },
    { "sinf",   stress_trig_sinf },
    { "sinl",   stress_trig_sinl },
    { "sincos", stress_trig_sincos },
    { "tan",    stress_trig_tan },
    { "tanf",   stress_trig_tanf },
    { "tanl",   stress_trig_tanl },
    { NULL,     NULL }
};

void stress_trig(stress_args_t *args)
{
    int (*specific_func)(void) = NULL;
    stress_bool_t run_all = STRESS_FALSE;

    stress_osal_print("rtos_stress: info: [trig-%d] starting trigonometric stressor\n", args->instance);

    if (args->method_name == NULL || stress_osal_strcmp(args->method_name, "all") == 0) {
        run_all = STRESS_TRUE;
        stress_osal_print("rtos_stress: info: [trig-%d] using 'all' methods\n", args->instance);
    } else {
        for (int i = 0; trig_methods[i].name != NULL; i++) {
            if (stress_osal_strcmp(args->method_name, trig_methods[i].name) == 0) {
                specific_func = trig_methods[i].func;
                break;
            }
        }
        if (!specific_func) {
            stress_osal_print("rtos_stress: error: unknown method '%s', using 'all'\n", args->method_name);
            run_all = STRESS_TRUE;
        } else {
            stress_osal_print("rtos_stress: info: [trig-%d] using method '%s'\n", args->instance, args->method_name);
        }
    }

    while (stress_continue(args))
    {
        if (run_all) {
            for (int i = 0; trig_methods[i].name != NULL; i++) {
                if (!stress_continue(args)) break;
                if (trig_methods[i].func() != 0) {
                }
            }
        } else {
            specific_func();
        }

        args->bogo.current_ops++;

        stress_osal_sleep_ms(1);
    }
}
