/* applications/stress-ng/stress-cpu.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <config.h>

static int32_t s_cpu_load = DEFAULT_CPU_LOAD;

static int stress_cpu_opt_load(const char *opt_name, const char *opt_arg)
{
    int val = atoi(opt_arg);
    if (val < 0) val = 0;
    if (val > 100) val = 100;

    s_cpu_load = val;
    stress_osal_print("rtos_stress: debug: cpu-load set to %d%%\n", s_cpu_load);
    return 0;
}

const stress_opt_t stress_cpu_opts[] = {
    { "cpu-load", stress_cpu_opt_load },
    { NULL, NULL }
};

static void stress_cpu_sqrt(void) {
    volatile double r;
    int i;
    for (i = 0; i < 1000; i++) {
        r = sqrt((double)i);
        (void)r;
    }
}

static void stress_cpu_bitops(void) {
    volatile uint32_t r = 0;
    volatile uint32_t a = 0xAAAAAAAA;
    volatile uint32_t b = 0x55555555;
    int i;
    for (i = 0; i < 1000; i++) {
        r |= (a & b);
        r ^= (a | ~b);
        r = ~r;
        r = (r << 1) | (r >> 31);
    }
}

#define MTX_SIZE 32
static void stress_cpu_matrix_prod(void) {
    double (*a)[MTX_SIZE] = stress_osal_malloc(sizeof(double) * MTX_SIZE * MTX_SIZE);
    double (*b)[MTX_SIZE] = stress_osal_malloc(sizeof(double) * MTX_SIZE * MTX_SIZE);
    double (*r)[MTX_SIZE] = stress_osal_malloc(sizeof(double) * MTX_SIZE * MTX_SIZE);
    int i, j, k;

    if (!a || !b || !r) goto __exit;

    for (i = 0; i < MTX_SIZE; i++) {
        for (j = 0; j < MTX_SIZE; j++) {
            a[i][j] = (double)i;
            b[i][j] = (double)j;
        }
    }

    for (i = 0; i < MTX_SIZE; i++) {
        for (j = 0; j < MTX_SIZE; j++) {
            double sum = 0.0;
            for (k = 0; k < MTX_SIZE; k++) {
                sum += a[i][k] * b[k][j];
            }
            r[i][j] = sum;
        }
    }

__exit:
    if (a) stress_osal_free(a);
    if (b) stress_osal_free(b);
    if (r) stress_osal_free(r);
}

static uint32_t ackermann(uint32_t m, uint32_t n) {
    if (m == 0) return n + 1;
    if (n == 0) return ackermann(m - 1, 1);
    return ackermann(m - 1, ackermann(m, n - 1));
}
static void stress_cpu_ackermann(void) {
    volatile uint32_t r;
    r = ackermann(3, 2);
    (void)r;
}

static uint32_t fibonacci(uint32_t n) {
    if (n < 2) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}
static void stress_cpu_fibonacci(void) {
    volatile uint32_t r;
    r = fibonacci(22);
    (void)r;
}

static int is_prime(int n) {
    if (n <= 1) return 0;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}
static void stress_cpu_prime(void) {
    volatile int cnt = 0;
    int i;
    for (i = 0; i < 1000; i++) {
        if (is_prime(i)) cnt++;
    }
}

typedef void (*stress_cpu_func)(void);
typedef struct {
    const char *name;
    stress_cpu_func func;
} stress_cpu_method_info_t;

static const stress_cpu_method_info_t stress_cpu_methods[] = {
    { "sqrt",       stress_cpu_sqrt },
    { "bitops",     stress_cpu_bitops },
    { "matrixprod", stress_cpu_matrix_prod },
    { "ackermann",  stress_cpu_ackermann },
    { "fibonacci",  stress_cpu_fibonacci },
    { "prime",      stress_cpu_prime },
    { NULL,         NULL }
};

void stress_cpu(stress_args_t *args)
{
    stress_cpu_func specific_func = NULL;
    stress_bool_t run_all = STRESS_FALSE;
    int method_index = 0;

    const uint32_t period_ms = 100;
    uint32_t busy_ms;
    uint32_t sleep_ms;
    int load_pct = s_cpu_load;

    if (load_pct >= 100) {
        busy_ms = period_ms;
        sleep_ms = 0;
    } else {
        busy_ms = (period_ms * load_pct) / 100;
        sleep_ms = period_ms - busy_ms;
        if (busy_ms == 0 && load_pct > 0) {
            busy_ms = 1;
            sleep_ms = period_ms > 1 ? period_ms - 1 : 0;
        }
    }

    if (args->instance == 0) {
        stress_osal_print("rtos_stress: info: [cpu] load set to %d%% (busy: %dms, sleep: %dms)\n",
                   load_pct, busy_ms, sleep_ms);
    }

    if (args->method_name == NULL || strcmp(args->method_name, "all") == 0) {
        run_all = STRESS_TRUE;
        if (args->instance == 0)
            stress_osal_print("rtos_stress: info: [cpu] using 'all' methods (round-robin)\n");
    }
    else {
        for (int i = 0; stress_cpu_methods[i].name != NULL; i++) {
            if (strcmp(args->method_name, stress_cpu_methods[i].name) == 0) {
                specific_func = stress_cpu_methods[i].func;
                break;
            }
        }
        if (specific_func == NULL) {
            stress_osal_print("rtos_stress: error: unknown cpu method '%s', defaulting to 'all'\n", args->method_name);
            run_all = STRESS_TRUE;
        }
    }

    stress_tick_t interval_start_tick = stress_osal_tick_get();

    while (stress_continue(args))
    {
        if (run_all) {
            if (stress_cpu_methods[method_index].name == NULL) method_index = 0;
            stress_cpu_methods[method_index].func();
            method_index++;
        } else {
            if (specific_func) specific_func();
        }

        args->bogo.current_ops++;

        if (load_pct < 100) {
            stress_tick_t current_tick = stress_osal_tick_get();
            stress_tick_t elapsed_ticks = current_tick - interval_start_tick;
            uint32_t elapsed_ms = (elapsed_ticks * 1000) / stress_osal_tick_hz();

            if (elapsed_ms >= busy_ms) {
                if (sleep_ms > 0) {
                    stress_osal_sleep_ms(sleep_ms);
                }
                interval_start_tick = stress_osal_tick_get();
            }
        } else {
            if (args->bogo.current_ops % 100 == 0) {
                stress_osal_thread_yield();
            }
        }
    }
}
