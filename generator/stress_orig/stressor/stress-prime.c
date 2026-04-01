/* applications/stress-ng/stress-prime.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <string.h>
#include <stdlib.h>
#include <stress-config.h>

#define SIEVE_BUFFER_SIZE 8192

typedef struct {
    uint64_t inc_current;
    uint64_t fact_val;
    uint64_t fact_n;
    uint8_t *sieve_buffer;
} stress_prime_context_t;

static uint64_t s_prime_start = DEFAULT_PRIME_START;

static int stress_prime_opt_start(const char *opt_name, const char *opt_arg)
{
    char *endptr;
    unsigned long long val = strtoull(opt_arg, &endptr, 10);

    if (val < MIN_PRIME_START) val = MIN_PRIME_START;

    s_prime_start = (uint64_t)val;
    stress_osal_print("rtos_stress: debug: prime-start set to %llu\n", (unsigned long long)s_prime_start);
    return 0;
}

const stress_opt_t stress_prime_opts[] = {
    { "prime-start", stress_prime_opt_start },
    { NULL, NULL }
};

static int is_prime_trial(uint64_t n)
{
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if ((n % 2 == 0) || (n % 3 == 0)) return 0;

    for (uint64_t i = 5; i * i <= n; i += 6) {
        if ((n % i == 0) || (n % (i + 2) == 0)) return 0;
    }
    return 1;
}

static void stress_prime_op_inc(stress_args_t *args, stress_prime_context_t *ctx)
{
    for (int i = 0; i < 50; i++) {
        if (ctx->inc_current < 3) ctx->inc_current = s_prime_start + (args->instance * 12345);

        if (is_prime_trial(ctx->inc_current)) {
        }
        ctx->inc_current += 2;
    }
    args->bogo.current_ops += 50;
}

static void stress_prime_op_sieve(stress_args_t *args, stress_prime_context_t *ctx)
{
    if (!ctx->sieve_buffer) return;

    uint8_t *flags = ctx->sieve_buffer;

    stress_osal_memset(flags, 1, SIEVE_BUFFER_SIZE);

    uint32_t count = 0;
    for (uint32_t i = 2; i < SIEVE_BUFFER_SIZE; i++) {
        if (flags[i]) {
            count++;
            for (uint32_t j = i * 2; j < SIEVE_BUFFER_SIZE; j += i) {
                flags[j] = 0;
            }
        }
    }

    args->bogo.current_ops += count;
}

static void stress_prime_op_factorial(stress_args_t *args, stress_prime_context_t *ctx)
{
    const uint64_t limit = 0xFFFFFFFFFFFFFF00ULL;

    for (int i = 0; i < 100; i++) {
        ctx->fact_val *= ctx->fact_n;
        ctx->fact_n++;

        if (ctx->fact_val == 0 || ctx->fact_val > limit) {
            ctx->fact_val = 1;
            ctx->fact_n = 1;
        }
    }
    args->bogo.current_ops += 100;
}

typedef void (*stress_prime_func)(stress_args_t *args, stress_prime_context_t *ctx);

typedef struct {
    const char *name;
    stress_prime_func func;
} stress_prime_method_info_t;

static const stress_prime_method_info_t prime_methods[] = {
    { "inc",       stress_prime_op_inc },
    { "sieve",     stress_prime_op_sieve },
    { "factorial", stress_prime_op_factorial },
    { NULL,        NULL }
};

void stress_prime(stress_args_t *args)
{
    stress_prime_context_t ctx;
    stress_bool_t run_all = STRESS_FALSE;
    stress_prime_func specific_func = NULL;

    ctx.inc_current = s_prime_start + (args->instance * 999);
    ctx.fact_val = 1;
    ctx.fact_n = 1;
    ctx.sieve_buffer = NULL;

    ctx.sieve_buffer = stress_osal_malloc(SIEVE_BUFFER_SIZE);
    if (!ctx.sieve_buffer) {
        stress_osal_print("rtos_stress: warning: [prime] OOM, sieve method disabled\n");
    }

    if (args->method_name == NULL || stress_osal_strcmp(args->method_name, "all") == 0) {
        run_all = STRESS_TRUE;
        stress_osal_print("rtos_stress: info: [prime-%d] using 'all' methods\n", args->instance);
    } else {
        for (int i = 0; prime_methods[i].name != NULL; i++) {
            if (stress_osal_strcmp(args->method_name, prime_methods[i].name) == 0) {
                specific_func = prime_methods[i].func;
                break;
            }
        }
        if (!specific_func) {
            stress_osal_print("rtos_stress: error: unknown method '%s', using 'all'\n", args->method_name);
            run_all = STRESS_TRUE;
        } else {
            stress_osal_print("rtos_stress: info: [prime-%d] using method '%s'\n", args->instance, args->method_name);
        }
    }

    while (stress_continue(args))
    {
        if (run_all) {
            for (int i = 0; prime_methods[i].name != NULL; i++) {
                if (!stress_continue(args)) break;

                if (prime_methods[i].func == stress_prime_op_sieve && !ctx.sieve_buffer) {
                    continue;
                }

                prime_methods[i].func(args, &ctx);

                stress_osal_sleep_ms(1);
            }
        } else {
            if (specific_func == stress_prime_op_sieve && !ctx.sieve_buffer) {
                stress_osal_print("rtos_stress: error: sieve buffer not allocated\n");
                break;
            }

            specific_func(args, &ctx);

            stress_osal_sleep_ms(1);
        }
    }

    if (ctx.sieve_buffer) {
        stress_osal_free(ctx.sieve_buffer);
    }
}
