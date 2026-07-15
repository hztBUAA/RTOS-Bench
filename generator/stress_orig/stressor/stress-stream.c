/* applications/stress-ng/stress-stream.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stress-config.h>

#ifndef SIZE_MAX
#define SIZE_MAX                ((size_t)-1)
#endif

#define STREAM_STACK_SIZE       (64 * 1024)
#define STREAM_WORKER_PRIORITY  (100)
#define STREAM_RAMP_MIN_ELEMS   (1024)
#define STREAM_RAMP_MAX_SHIFT   (4)
#define STREAM_RAMP_OPS_STEP    (16)
#define STREAM_RAMP_INSTANCE_OPS (4)
#define STREAM_CHUNK_ELEMS      (4096)
#define STREAM_YIELD_EVERY_OPS  (1)

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
    unsigned long long val;

    (void)opt_name;

    if (!opt_arg || *opt_arg == '\0' || *opt_arg == '-') {
        return -1;
    }

    errno = 0;
    val = strtoull(opt_arg, &endptr, 10);
    if (errno == ERANGE || endptr == opt_arg) {
        return -1;
    }

    if (*endptr == 'k' || *endptr == 'K') {
        if (val > ULLONG_MAX / 1024ULL) {
            val = ULLONG_MAX;
        } else {
            val *= 1024ULL;
        }
        endptr++;
    } else if (*endptr == 'm' || *endptr == 'M') {
        if (val > ULLONG_MAX / (1024ULL * 1024ULL)) {
            val = ULLONG_MAX;
        } else {
            val *= 1024ULL * 1024ULL;
        }
        endptr++;
    }

    if (*endptr != '\0') {
        return -1;
    }

    if (val < (unsigned long long)MIN_STREAM_ELEM) val = MIN_STREAM_ELEM;
    if (val > (unsigned long long)MAX_STREAM_ELEM) val = MAX_STREAM_ELEM;

    s_stream_elem = (uint64_t)val;
    stress_osal_print("rtos_stress: debug: stream-elem set to %llu elements\n", (unsigned long long)s_stream_elem);
    return 0;
}

const stress_opt_t stress_stream_opts[] = {
    { "stream-elem", stress_stream_opt_elem },
    { NULL, NULL }
};

static int stress_stream_size_mul(uint64_t n, size_t size, size_t *result)
{
    if (size != 0 && n > (uint64_t)(SIZE_MAX / size)) {
        return -1;
    }

    *result = (size_t)n * size;
    return 0;
}

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

static inline uint64_t stress_stream_chunk_size(const uint64_t offset, const uint64_t n)
{
    uint64_t chunk = n - offset;

    if (chunk > STREAM_CHUNK_ELEMS) {
        chunk = STREAM_CHUNK_ELEMS;
    }

    return chunk;
}

static void stress_stream_run_once(
    double *const RESTRICT a,
    double *const RESTRICT b,
    double *const RESTRICT c,
    const double q,
    const uint64_t n)
{
    uint64_t offset, chunk;

    for (offset = 0; offset < n; offset += chunk) {
        chunk = stress_stream_chunk_size(offset, n);
        stress_stream_copy(c + (size_t)offset, a + (size_t)offset, chunk);
        if (n >= STREAM_CHUNK_ELEMS) {
            stress_osal_thread_yield();
        }
    }

    for (offset = 0; offset < n; offset += chunk) {
        chunk = stress_stream_chunk_size(offset, n);
        stress_stream_scale(b + (size_t)offset, c + (size_t)offset, q, chunk);
        if (n >= STREAM_CHUNK_ELEMS) {
            stress_osal_thread_yield();
        }
    }

    for (offset = 0; offset < n; offset += chunk) {
        chunk = stress_stream_chunk_size(offset, n);
        stress_stream_add(c + (size_t)offset, a + (size_t)offset, b + (size_t)offset, chunk);
        if (n >= STREAM_CHUNK_ELEMS) {
            stress_osal_thread_yield();
        }
    }

    for (offset = 0; offset < n; offset += chunk) {
        chunk = stress_stream_chunk_size(offset, n);
        stress_stream_triad(a + (size_t)offset, b + (size_t)offset, c + (size_t)offset, q, chunk);
        if (n >= STREAM_CHUNK_ELEMS) {
            stress_osal_thread_yield();
        }
    }
}

static uint32_t stress_mwc32(void) { return (uint32_t)stress_osal_rand(); }

static void stress_stream_init_data_range(
    double *const RESTRICT a,
    double *const RESTRICT b,
    double *const RESTRICT c,
    const uint64_t start,
    const uint64_t end,
    const double base,
    const double delta)
{
    double v = base + delta * (double)start;
    uint64_t i;

    for (i = start; i < end; i++) {
        a[(size_t)i] = v;
        b[(size_t)i] = v;
        c[(size_t)i] = v;
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

static uint64_t stress_stream_ramp_elems(
    const uint64_t target_n,
    uint64_t ops,
    const uint64_t offset_ops)
{
    uint64_t min_n = STREAM_RAMP_MIN_ELEMS;
    uint64_t step;
    uint64_t active_n;

    if (min_n < (uint64_t)MIN_STREAM_ELEM) {
        min_n = (uint64_t)MIN_STREAM_ELEM;
    }

    if (target_n <= min_n) {
        return target_n;
    }

    if (ops <= offset_ops) {
        ops = 0;
    } else {
        ops -= offset_ops;
    }

    step = ops / STREAM_RAMP_OPS_STEP;
    if (step >= STREAM_RAMP_MAX_SHIFT) {
        return target_n;
    }

    active_n = target_n >> (STREAM_RAMP_MAX_SHIFT - step);

    if (active_n < min_n) {
        active_n = min_n;
    }

    if (active_n > target_n) {
        active_n = target_n;
    }

    return active_n;
}

static void stress_stream_worker(void *parameter)
{
    stream_context_t *ctx = (stream_context_t *)parameter;
    stress_args_t *args = ctx->args;

    double *a = NULL, *b = NULL, *c = NULL;
    uint64_t n = ctx->num_elems;
    uint64_t active_n = 0;
    uint64_t initialized_n = 0;
    uint64_t last_reported_n = 0;
    uint64_t ramp_offset;
    const double q = 3.0;
    const double divisor = 1.0 / 4294967296.0;
    const double init_delta = (double)stress_mwc32() * divisor;
    const double init_base = (double)stress_mwc32() * divisor;
    double old_checksum = 0.0;
    double t_start, t_end, dt;

    double total_rd_bytes = 0.0;
    double total_wr_bytes = 0.0;
    double total_fp_ops = 0.0;

    while (n >= (uint64_t)MIN_STREAM_ELEM) {
        size_t alloc_sz;

        if (stress_stream_size_mul(n, sizeof(double), &alloc_sz) != 0) {
            n /= 2;
            continue;
        }

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

    stress_osal_print("rtos_stress: info: [stream-%d] target %llu elements (approx %llu KB total)\n",
               args->instance,
               (unsigned long long)n,
               (unsigned long long)(((double)n * (double)sizeof(double) * 3.0) / 1024.0));

    ramp_offset = ((uint64_t)((unsigned int)args->instance & 7U)) * STREAM_RAMP_INSTANCE_OPS;

    active_n = stress_stream_ramp_elems(n, (uint64_t)args->bogo.current_ops, ramp_offset);
    stress_stream_init_data_range(a, b, c, 0, active_n, init_base, init_delta);
    initialized_n = active_n;
    old_checksum = stress_stream_checksum(a, b, c, active_n);

    t_start = stress_osal_time_now();

    while (stress_continue(args))
    {
        double batch_ops;
        double new_checksum;

        active_n = stress_stream_ramp_elems(n, (uint64_t)args->bogo.current_ops, ramp_offset);

        if (active_n > initialized_n) {
            stress_stream_init_data_range(a, b, c, initialized_n, active_n, init_base, init_delta);
            initialized_n = active_n;
            old_checksum = stress_stream_checksum(a, b, c, active_n);
        }

        if (active_n != last_reported_n) {
            stress_osal_print("rtos_stress: info: [stream-%d] ramp: %llu of %llu elements\n",
                       args->instance,
                       (unsigned long long)active_n,
                       (unsigned long long)n);
            last_reported_n = active_n;
            stress_osal_thread_yield();
        }

        stress_stream_run_once(a, b, c, q, active_n);

        batch_ops = (double)active_n * 4.0;
        total_rd_bytes += (double)active_n * sizeof(double) * 6.0;
        total_wr_bytes += (double)active_n * sizeof(double) * 4.0;
        total_fp_ops += batch_ops;

        if ((args->bogo.current_ops % 64) == 0) {
            new_checksum = stress_stream_checksum(a, b, c, active_n);
            if (new_checksum == 0.0 && old_checksum != 0.0) { }
        }

        args->bogo.current_ops++;

        if ((args->bogo.current_ops % STREAM_YIELD_EVERY_OPS) == 0) {
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
        stress_osal_strcpy(args->bogo.metric_name[1], "Mflop/sec");

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
                                        STREAM_WORKER_PRIORITY);

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
