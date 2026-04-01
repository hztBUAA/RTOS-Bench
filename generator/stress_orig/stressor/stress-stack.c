/* applications/stress-ng/stress-stack.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <stress-config.h>

#define STACK_FRAME_DATA_SIZE   (1024)
#define STACK_SAFETY_MARGIN     (4096)

#define UNLIKELY(x)         __builtin_expect(!!(x), 0)
#define LIKELY(x)           __builtin_expect(!!(x), 1)

static int32_t s_stack_size = DEFAULT_STACK_SIZE;

static int stress_stack_opt_size(const char *opt_name, const char *opt_arg)
{
    char *endptr;
    unsigned long long val = strtoull(opt_arg, &endptr, 10);

    if (*endptr == 'k' || *endptr == 'K') val *= 1024;
    else if (*endptr == 'm' || *endptr == 'M') val *= (1024 * 1024);

    if (val < STACK_SIZE_MIN) val = STACK_SIZE_MIN;
    if (val > STACK_SIZE_MAX) val = STACK_SIZE_MAX;

    s_stack_size = (int32_t)val;
    stress_osal_print("rtos_stress: debug: stack-size set to %d bytes\n", s_stack_size);
    return 0;
}

const stress_opt_t stress_stack_opts[] = {
    { "stack-size", stress_stack_opt_size },
    { NULL, NULL }
};

static uint32_t stress_mwc32(void) { return (uint32_t)stress_osal_rand(); }

static int stress_stack_recurse(stress_args_t *args, int current_depth, int max_depth)
{
    uint8_t data[STACK_FRAME_DATA_SIZE];
    int i;
    int rc = 0;

    uint8_t pattern = (uint8_t)(current_depth ^ 0x55);
    stress_osal_memset(data, pattern, sizeof(data));

    if (current_depth % 8 == 0) {
        stress_mwc32();
    }

    if (current_depth < max_depth) {
        if (stress_continue(args)) {
            rc = stress_stack_recurse(args, current_depth + 1, max_depth);
            if (rc != 0) return rc;
        }
    }

    for (i = 0; i < STACK_FRAME_DATA_SIZE; i++) {
        if (data[i] != pattern) {
            stress_osal_print("rtos_stress: fail: [stack] corruption at depth %d, idx %d, expect %02x got %02x\n",
                       current_depth, i, pattern, data[i]);
            return -1;
        }
    }

    return 0;
}

typedef struct {
    stress_args_t *args;
    size_t stack_size;
    stress_sem_t sem_done;
} stack_ctx_t;

static void stress_stack_worker(void *parameter)
{
    stack_ctx_t *ctx = (stack_ctx_t *)parameter;

    int frame_cost = STACK_FRAME_DATA_SIZE + 64;

    int available_stack = (int)ctx->stack_size - STACK_SAFETY_MARGIN;
    if (available_stack < 0) available_stack = 0;

    int max_depth = available_stack / frame_cost;
    if (max_depth < 1) max_depth = 1;

    while (stress_continue(ctx->args))
    {
        if (stress_stack_recurse(ctx->args, 0, max_depth) != 0) {
            break;
        }
        ctx->args->bogo.current_ops++;
        stress_osal_sleep_ms(1);
    }

    stress_osal_sem_release(ctx->sem_done);
}

void stress_stack(stress_args_t *args)
{
    stack_ctx_t ctx;
    stress_tid_t tid = STRESS_INVALID_TID;
    size_t current_stack_size = (size_t)s_stack_size;
    int worker_finished = 0;

    ctx.args = args;
    ctx.sem_done = stress_osal_sem_create("stk_done", 0);

    if (!ctx.sem_done) {
        stress_osal_print("rtos_stress: error: [stack] create sem failed\n");
        return;
    }

    while (current_stack_size >= STACK_SIZE_MIN) {
        ctx.stack_size = current_stack_size;
        tid = stress_osal_thread_spawn("ng_stack",
                               stress_stack_worker,
                               &ctx,
                               (uint32_t)current_stack_size,
                               20);

        if (tid != STRESS_INVALID_TID) break;

        stress_osal_print("rtos_stress: warn: [stack] alloc %d KB stack failed, retrying...\n",
                   (int)(current_stack_size / 1024));
        current_stack_size /= 2;
    }

    if (tid) {
        stress_osal_print("rtos_stress: info: [stack-%d] started (stack: %d KB, depth: ~%d)\n",
                   args->instance,
                   (int)(current_stack_size / 1024),
                   (int)((current_stack_size - STACK_SAFETY_MARGIN) / (STACK_FRAME_DATA_SIZE + 64)));

        while (stress_continue(args)) {
            if (stress_osal_sem_take(ctx.sem_done, 100) == 0) {
                worker_finished = 1;
                break;
            }
        }

        if (!worker_finished) {
            if (stress_osal_sem_take(ctx.sem_done, 5000) == 0) {
                worker_finished = 1;
            } else {
                stress_osal_print("rtos_stress: error: [stack] worker timed out (likely crashed due to stack overflow)\n");
                stress_osal_thread_delete(tid);
            }
        }

    } else {
        stress_osal_print("rtos_stress: error: [stack] failed to create thread (OOM)\n");
    }

    stress_osal_sem_delete(ctx.sem_done);
}
