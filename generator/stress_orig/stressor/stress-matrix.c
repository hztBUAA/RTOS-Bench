/* applications/stress-ng/stress-matrix.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <string.h>
#include <stdlib.h>
#include <config.h>

typedef float stress_matrix_type_t;

#if defined(__GNUC__)
    #define RESTRICT __restrict__
#else
    #define RESTRICT
#endif

typedef void (*stress_matrix_func_t)(
    const size_t n,
    stress_matrix_type_t a[RESTRICT n][n],
    stress_matrix_type_t b[RESTRICT n][n],
    stress_matrix_type_t r[RESTRICT n][n]);

typedef struct {
    const char *name;
    stress_matrix_func_t func;
} stress_matrix_method_info_t;

static int32_t s_matrix_size = DEFAULT_MATRIX_SIZE;

static int stress_matrix_opt_size(const char *opt_name, const char *opt_arg)
{
    int val = atoi(opt_arg);

    if (val < MIN_MATRIX_SIZE) val = MIN_MATRIX_SIZE;
    if (val > MAX_MATRIX_SIZE) val = MAX_MATRIX_SIZE;

    s_matrix_size = val;
    stress_osal_print("rtos_stress: debug: matrix-size set to %d\n", s_matrix_size);
    return 0;
}

const stress_opt_t stress_matrix_opts[] = {
    { "matrix-size", stress_matrix_opt_size },
    { NULL, NULL }
};

static void stress_matrix_prod(
    const size_t n,
    stress_matrix_type_t a[RESTRICT n][n],
    stress_matrix_type_t b[RESTRICT n][n],
    stress_matrix_type_t r[RESTRICT n][n])
{
    size_t i, j, k;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            stress_matrix_type_t sum = 0.0f;
            for (k = 0; k < n; k++) {
                sum += a[i][k] * b[k][j];
            }
            r[i][j] = sum;
        }
    }
}

static void stress_matrix_add(
    const size_t n,
    stress_matrix_type_t a[RESTRICT n][n],
    stress_matrix_type_t b[RESTRICT n][n],
    stress_matrix_type_t r[RESTRICT n][n])
{
    size_t i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            r[i][j] = a[i][j] + b[i][j];
        }
    }
}

static void stress_matrix_sub(
    const size_t n,
    stress_matrix_type_t a[RESTRICT n][n],
    stress_matrix_type_t b[RESTRICT n][n],
    stress_matrix_type_t r[RESTRICT n][n])
{
    size_t i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            r[i][j] = a[i][j] - b[i][j];
        }
    }
}

static void stress_matrix_trans(
    const size_t n,
    stress_matrix_type_t a[RESTRICT n][n],
    stress_matrix_type_t b[RESTRICT n][n],
    stress_matrix_type_t r[RESTRICT n][n])
{
    size_t i, j;
    (void)b;

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            r[i][j] = a[j][i];
        }
    }
}

static void stress_matrix_mean(
    const size_t n,
    stress_matrix_type_t a[RESTRICT n][n],
    stress_matrix_type_t b[RESTRICT n][n],
    stress_matrix_type_t r[RESTRICT n][n])
{
    size_t i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            r[i][j] = (a[i][j] + b[i][j]) * 0.5f;
        }
    }
}

static void stress_matrix_identity(
    const size_t n,
    stress_matrix_type_t a[RESTRICT n][n],
    stress_matrix_type_t b[RESTRICT n][n],
    stress_matrix_type_t r[RESTRICT n][n])
{
    size_t i, j;
    (void)a; (void)b;

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            r[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
}

static const stress_matrix_method_info_t matrix_methods[] = {
    { "prod",     stress_matrix_prod },
    { "add",      stress_matrix_add },
    { "sub",      stress_matrix_sub },
    { "trans",    stress_matrix_trans },
    { "mean",     stress_matrix_mean },
    { "identity", stress_matrix_identity },
    { NULL,       NULL }
};

static void stress_matrix_init_data(size_t n, float *raw_data)
{
    size_t i;
    for (i = 0; i < n * n; i++) {
        raw_data[i] = ((float)stress_osal_rand() / (float)RAND_MAX) * 200.0f - 100.0f;
    }
}

void stress_matrix(stress_args_t *args)
{
    size_t n = (size_t)s_matrix_size;

    size_t mem_size = n * n * sizeof(stress_matrix_type_t);

    stress_matrix_type_t *raw_a = (stress_matrix_type_t *)stress_osal_malloc(mem_size);
    stress_matrix_type_t *raw_b = (stress_matrix_type_t *)stress_osal_malloc(mem_size);
    stress_matrix_type_t *raw_r = (stress_matrix_type_t *)stress_osal_malloc(mem_size);

    if (!raw_a || !raw_b || !raw_r) {
        stress_osal_print("rtos_stress: error: [matrix] OOM! Failed to alloc %d bytes x 3\n", mem_size);
        if (raw_a) stress_osal_free(raw_a);
        if (raw_b) stress_osal_free(raw_b);
        if (raw_r) stress_osal_free(raw_r);
        return;
    }

    typedef stress_matrix_type_t (*matrix_ptr_t)[n];

    matrix_ptr_t ptr_a = (matrix_ptr_t)raw_a;
    matrix_ptr_t ptr_b = (matrix_ptr_t)raw_b;
    matrix_ptr_t ptr_r = (matrix_ptr_t)raw_r;

    stress_matrix_init_data(n, raw_a);
    stress_matrix_init_data(n, raw_b);

    stress_matrix_func_t specific_func = NULL;
    stress_bool_t run_all = STRESS_FALSE;

    if (args->method_name == NULL || stress_osal_strcmp(args->method_name, "all") == 0) {
        run_all = STRESS_TRUE;
        stress_osal_print("rtos_stress: info: [matrix-%d] size=%d, method=all\n", args->instance, n);
    } else {
        for (int i = 0; matrix_methods[i].name != NULL; i++) {
            if (stress_osal_strcmp(args->method_name, matrix_methods[i].name) == 0) {
                specific_func = matrix_methods[i].func;
                break;
            }
        }
        if (!specific_func) {
            stress_osal_print("rtos_stress: error: unknown method '%s', using 'all'\n", args->method_name);
            run_all = STRESS_TRUE;
        } else {
            stress_osal_print("rtos_stress: info: [matrix-%d] size=%d, method=%s\n", args->instance, n, args->method_name);
        }
    }

    while (stress_continue(args))
    {
        if (run_all) {
            for (int i = 0; matrix_methods[i].name != NULL; i++) {
                if (!stress_continue(args)) break;

                matrix_methods[i].func(n, ptr_a, ptr_b, ptr_r);
                args->bogo.current_ops++;
                stress_osal_sleep_ms(1);
            }
        } else {
            specific_func(n, ptr_a, ptr_b, ptr_r);
            args->bogo.current_ops++;
            stress_osal_sleep_ms(1);
        }
    }

    stress_osal_free(raw_a);
    stress_osal_free(raw_b);
    stress_osal_free(raw_r);
}
