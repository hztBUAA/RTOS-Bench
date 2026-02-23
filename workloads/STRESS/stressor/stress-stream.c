/* applications/stress-ng/stress-stream.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <config.h>

#define STREAM_STACK_SIZE       (64 * 1024)

#define UNLIKELY(x)         __builtin_expect(!!(x), 0)
#define LIKELY(x)           __builtin_expect(!!(x), 1)
#define RESTRICT            __restrict

typedef struct {
    stress_args_t *args;
    stress_sem_t done_sem;
    uint64_t num_elems;
} stream_context_t;

static uint64_t s_stream_elem = DEFAULT_STREAM_ELEM;

static int stress_stream_opt_elem(const char *opt_name, const char *opt_arg)
{
    char *endptr;
    unsigned long long val = strtoull(opt_arg, &endptr, 10);

    if (*endptr == 'k' || *endptr == 'K') val *= 1024;
    else if (*endptr == 'm' || *endptr == 'M') val *= (1024 * 1024);

    if (val < MIN_STREAM_ELEM) val = MIN_STREAM_ELEM;
    if (val > MAX_STREAM_ELEM) val = MAX_STREAM_ELEM;

    s_stream_elem = (uint64_t)val;
    stress_osal_print("rtos_stress: debug: stream-elem set to %llu elements\n", (unsigned long long)s_stream_elem);
    return 0;
}

const stress_opt_t stress_stream_opts[] = {
    { "stream-elem", stress_stream_opt_elem },
    { NULL, NULL }
};

static void stress_stream_copy(
    double *const RESTRICT c,
    const double *const RESTRICT a,
    const uint64_t n)
{
    uint64_t i;
    for (i = 0; i < n; i++) c[i] = a[i];
}

static void stress_stream_scale(
    double *const RESTRICT b,
    const double *const RESTRICT c,
    const double q,
    const uint64_t n)
{
    uint64_t i;
    for (i = 0; i < n; i++) b[i] = q * c[i];
}

static void stress_stream_add(
    double *const RESTRICT c,
    const double *const RESTRICT a,
    const double *const RESTRICT b,
    const uint64_t n)
{
    uint64_t i;
    for (i = 0; i < n; i++) c[i] = a[i] + b[i];
}

static void stress_stream_triad(
    double *const RESTRICT a,
    const double *const RESTRICT b,
    const double *const RESTRICT c,
    const double q,
    const uint64_t n)
{
    uint64_t i;
    for (i = 0; i < n; i++) a[i] = b[i] + q * c[i];
}

static uint32_t stress_mwc32(void) { return (uint32_t)stress_osal_rand(); }

static void stress_stream_init_data(
    double *const RESTRICT a,
    double *const RESTRICT b,
    double *const RESTRICT c,
    const uint64_t n)
{
    const double divisor = 1.0 / 4294967296.0;
    const double delta = (double)stress_mwc32() * divisor;
    double v = (double)stress_mwc32() * divisor;
    uint64_t i;

    for (i = 0; i < n; i++) {
        a[i] = v;
        b[i] = v;
        c[i] = v;
        v += delta;
    }
}

static double stress_stream_checksum(
    const double *const RESTRICT a,
    const double *const RESTRICT b,
    const double *const RESTRICT c,
    const uint64_t n)
{
    double checksum = 0.0;
    uint64_t i;
    for (i = 0; i < n; i++) {
        checksum += a[i] + b[i] + c[i];
    }
    return checksum;
}

static void stress_stream_worker(void *parameter)
{
    stream_context_t *ctx = (stream_context_t *)parameter;
    stress_args_t *args = ctx->args;
    
    double *a = NULL, *b = NULL, *c = NULL;
    uint64_t n = ctx->num_elems;
    const double q = 3.0;
    double old_checksum = 0.0;
    double t_start, t_end, dt;
    
    double total_rd_bytes = 0.0;
    double total_wr_bytes = 0.0;
    double total_fp_ops = 0.0;

    while (n >= 128) {
        size_t alloc_sz = n * sizeof(double);
        a = (double *)stress_osal_malloc(alloc_sz);
        if (a) b = (double *)stress_osal_malloc(alloc_sz);
        if (b) c = (double *)stress_osal_malloc(alloc_sz);

        if (a && b && c) break;

        if (a) stress_osal_free(a);
        if (b) stress_osal_free(b);
        if (c) stress_osal_free(c);
        a = b = c = NULL;
        n /= 2;
    }

    if (!a || !b || !c) {
        stress_osal_print("rtos_stress: error: [stream] OOM, failed to allocate buffers\n");
        goto worker_done;
    }

    stress_osal_print("rtos_stress: info: [stream-%d] using %d elements (approx %d KB total)\n",
               args->instance, (int)n, (int)(n * sizeof(double) * 3 / 1024));

    stress_stream_init_data(a, b, c, n);
    old_checksum = stress_stream_checksum(a, b, c, n);
    t_start = stress_osal_time_now();

    while (stress_continue(args))
    {
        stress_stream_copy(c, a, n);
        stress_stream_scale(b, c, q, n);
        stress_stream_add(c, a, b, n);
        stress_stream_triad(a, b, c, q, n);

        double batch_ops = (double)n * 4.0;
        total_rd_bytes += (double)n * sizeof(double) * 6.0;
        total_wr_bytes += (double)n * sizeof(double) * 4.0;
        total_fp_ops += batch_ops;

        if ((args->bogo.current_ops % 64) == 0) {
            double new_checksum = stress_stream_checksum(a, b, c, n);
            if (new_checksum == 0.0 && old_checksum != 0.0) { }
        }

        args->bogo.current_ops++;

        if (args->bogo.current_ops % 10 == 0) {
            stress_osal_thread_yield();
        }
    }

    t_end = stress_osal_time_now();
    dt = t_end - t_start;

    if (dt > 0.001) {
        double mb_rate = (total_rd_bytes + total_wr_bytes) / (1024.0 * 1024.0) / dt;
        double mflops = (total_fp_ops / 1000000.0) / dt;

        args->bogo.metric_val[0] = mb_rate;
        stress_osal_strcpy(args->bogo.metric_name[0], "MB/sec");
        args->bogo.metric_val[1] = mflops;
        stress_osal_strcpy(args->bogo.metric_name[0], "Mflop/sec");

        stress_osal_print("rtos_stress: info: [stream-%d] bandwidth: %.2f MB/sec, compute: %.2f Mflop/sec\n",
                   args->instance, mb_rate, mflops);
    }

    if (a) stress_osal_free(a);
    if (b) stress_osal_free(b);
    if (c) stress_osal_free(c);

worker_done:
    stress_osal_sem_release(ctx->done_sem);
}

void stress_stream(stress_args_t *args)
{
    stream_context_t *ctx;
    stress_tid_t t_worker = NULL;

    ctx = (stream_context_t *)stress_osal_malloc(sizeof(stream_context_t));
    if (!ctx) {
        stress_osal_print("rtos_stress: error: [stream] OOM allocating context\n");
        return;
    }

    ctx->args = args;
    ctx->num_elems = s_stream_elem;
    ctx->done_sem = stress_osal_sem_create("stream_done", 0);

    if (!ctx->done_sem) {
        stress_osal_print("rtos_stress: error: [stream] Failed to create semaphore\n");
        stress_osal_free(ctx);
        return;
    }

    t_worker = stress_osal_thread_spawn("ng_stream", 
                                        stress_stream_worker, 
                                        ctx, 
                                        STREAM_STACK_SIZE,
                                        20);

    if (!t_worker) {
        stress_osal_print("rtos_stress: error: [stream] Failed to spawn worker thread\n");
        stress_osal_sem_delete(ctx->done_sem);
        stress_osal_free(ctx);
        return;
    }

    stress_osal_sem_take(ctx->done_sem, STRESS_WAIT_FOREVER);

    stress_osal_sem_delete(ctx->done_sem);
    stress_osal_free(ctx);
}
