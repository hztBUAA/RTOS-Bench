#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <config.h>

#ifdef __GNUC__
#  define STREAM_UNLIKELY(x)  __builtin_expect(!!(x), 0)
#  define STREAM_LIKELY(x)    __builtin_expect(!!(x), 1)
#  define STREAM_RESTRICT     __restrict
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define STREAM_UNLIKELY(x)  (x)
#  define STREAM_LIKELY(x)    (x)
#  define STREAM_RESTRICT     restrict
#else
#  define STREAM_UNLIKELY(x)  (x)
#  define STREAM_LIKELY(x)    (x)
#  define STREAM_RESTRICT
#endif

#if defined(UINTPTR_MAX)
   typedef uintptr_t stream_ptr_uint_t;
#elif defined(__LP64__) || defined(__x86_64__) || defined(__aarch64__) || \
      (defined(__riscv) && defined(__riscv_xlen) && __riscv_xlen == 64) || \
      defined(__loongarch64)
   typedef uint64_t stream_ptr_uint_t;
#else
   typedef uint32_t stream_ptr_uint_t;
#endif

#define STREAM_STACK_SIZE             (64 * 1024)
#define STREAM_THREAD_NAME            "stm_wrk"
#define STREAM_WORKER_EXIT_TIMEOUT_MS 10000U
#define STREAM_METRIC_NAME_LEN        16
#define STREAM_ALIGN                  ((uint64_t)64)
#define STREAM_CHECKSUM_REL_TOL_INV   1000000

typedef struct {
    stress_args_t   *args;
    stress_sem_t     done_sem;
    uint64_t         num_elems;
    volatile int     abandoned;
    void            *raw_a;
    void            *raw_b;
    void            *raw_c;
} stream_context_t;

static uint64_t s_stream_elem = DEFAULT_STREAM_ELEM;

static uint64_t stream_parse_uint64(const char *s)
{
    uint64_t val = 0;
    const char *p = s;
    while (*p >= '0' && *p <= '9') {
        val = val * 10ULL + (uint64_t)((unsigned char)*p - (unsigned char)'0');
        p++;
    }
    if      (*p == 'k' || *p == 'K') val *= 1024ULL;
    else if (*p == 'm' || *p == 'M') val *= 1024ULL * 1024ULL;
    return val;
}

static void stream_print_u64(const char *prefix, uint64_t val, const char *suffix)
{
    char buf[22];
    int  pos = 21;
    buf[21] = '\0';
    if (val == 0ULL) {
        buf[--pos] = '0';
    } else {
        while (val > 0ULL) {
            buf[--pos] = (char)('0' + (int)(val % 10ULL));
            val /= 10ULL;
        }
    }
    stress_osal_print("%s%s%s", prefix, &buf[pos], suffix);
}

static void stream_print_fp2(const char *prefix, double val, const char *suffix)
{
    int    int_part  = (int)val;
    int    frac_part = (int)((val - (double)int_part) * 100.0);
    if (frac_part < 0) frac_part = -frac_part;
    stress_osal_print("%s%d.%02d%s", prefix, int_part, frac_part, suffix);
}

static void stream_safe_strcpy(char *dst, const char *src, size_t sz)
{
    size_t i;
    if (!dst || sz == 0u) return;
    for (i = 0u; i + 1u < sz && src[i]; i++) dst[i] = src[i];
    dst[i] = '\0';
}

static int stress_stream_opt_elem(const char *opt_name, const char *opt_arg)
{
    uint64_t val;
    (void)opt_name;

    val = stream_parse_uint64(opt_arg);
    if (val == 0ULL) {
        stress_osal_print("rtos_stress: warn: stream-elem: invalid"
                          " value '%s', using default\n", opt_arg);
        return 0;
    }
    if (val < MIN_STREAM_ELEM) val = MIN_STREAM_ELEM;
    if (val > MAX_STREAM_ELEM) val = MAX_STREAM_ELEM;
    s_stream_elem = val;
    stream_print_u64("rtos_stress: debug: stream-elem set to ", val,
                     " elements\n");
    return 0;
}

const stress_opt_t stress_stream_opts[] = {
    { "stream-elem", stress_stream_opt_elem },
    { NULL, NULL }
};

static void stress_stream_copy(
    double *const STREAM_RESTRICT c,
    const double *const STREAM_RESTRICT a,
    const uint64_t n)
{
    uint64_t i;
    for (i = 0; i < n; i++) c[i] = a[i];
}

static void stress_stream_scale(
    double *const STREAM_RESTRICT b,
    const double *const STREAM_RESTRICT c,
    const double q,
    const uint64_t n)
{
    uint64_t i;
    for (i = 0; i < n; i++) b[i] = q * c[i];
}

static void stress_stream_add(
    double *const STREAM_RESTRICT c,
    const double *const STREAM_RESTRICT a,
    const double *const STREAM_RESTRICT b,
    const uint64_t n)
{
    uint64_t i;
    for (i = 0; i < n; i++) c[i] = a[i] + b[i];
}

static void stress_stream_triad(
    double *const STREAM_RESTRICT a,
    const double *const STREAM_RESTRICT b,
    const double *const STREAM_RESTRICT c,
    const double q,
    const uint64_t n)
{
    uint64_t i;
    for (i = 0; i < n; i++) a[i] = b[i] + q * c[i];
}

static uint32_t stress_mwc32(void) { return (uint32_t)stress_osal_rand(); }

static void stress_stream_init_data(
    double *const STREAM_RESTRICT a,
    double *const STREAM_RESTRICT b,
    double *const STREAM_RESTRICT c,
    const uint64_t n)
{
    const double divisor = 1.0 / 4294967296.0;
    const double delta = (double)stress_mwc32() * divisor + 1e-9;
    double v = (double)stress_mwc32() * divisor + 1.0;
    uint64_t i;
    for (i = 0; i < n; i++) {
        a[i] = v;
        b[i] = v;
        c[i] = v;
        v += delta;
    }
}

static double stress_stream_checksum(
    const double *const STREAM_RESTRICT a,
    const double *const STREAM_RESTRICT b,
    const double *const STREAM_RESTRICT c,
    const uint64_t n)
{
    double cs = 0.0;
    uint64_t i;
    for (i = 0; i < n; i++) cs += a[i] + b[i] + c[i];
    return cs;
}

static int stream_is_nan_or_inf(double x)
{
    return x != x || (x > 0.0 && x + x == x) || (x < 0.0 && x + x == x);
}

static double stream_abs(double x) { return (x < 0.0) ? -x : x; }

static void stress_stream_worker(void *parameter)
{
    stream_context_t *ctx  = (stream_context_t *)parameter;
    stress_args_t    *args = ctx->args;

    double   *a = NULL, *b = NULL, *c = NULL;
    uint64_t  n = ctx->num_elems;
    const double q = 3.0;
    double old_checksum  = 0.0;
    int    checksum_valid = 0;
    double t_start, t_end, dt;
    double total_rd_bytes = 0.0;
    double total_wr_bytes = 0.0;
    double total_fp_ops   = 0.0;

    while (n >= 128ULL) {
        uint64_t elem_bytes = n * (uint64_t)sizeof(double);
        uint64_t alloc_u64;
        size_t   alloc_sz;

        if (elem_bytes > (uint64_t)(size_t)(-1) - STREAM_ALIGN) {
            n /= 2ULL;
            continue;
        }
        alloc_u64 = elem_bytes + STREAM_ALIGN;
        alloc_sz  = (size_t)alloc_u64;

        ctx->raw_a = stress_osal_malloc(alloc_sz);
        ctx->raw_b = ctx->raw_a ? stress_osal_malloc(alloc_sz) : NULL;
        ctx->raw_c = ctx->raw_b ? stress_osal_malloc(alloc_sz) : NULL;

        if (ctx->raw_a && ctx->raw_b && ctx->raw_c) {
            stream_ptr_uint_t mask = (stream_ptr_uint_t)(STREAM_ALIGN - 1ULL);
            a = (double *)(((stream_ptr_uint_t)ctx->raw_a + mask) & ~mask);
            b = (double *)(((stream_ptr_uint_t)ctx->raw_b + mask) & ~mask);
            c = (double *)(((stream_ptr_uint_t)ctx->raw_c + mask) & ~mask);
            break;
        }

        if (ctx->raw_c) { stress_osal_free(ctx->raw_c); ctx->raw_c = NULL; }
        if (ctx->raw_b) { stress_osal_free(ctx->raw_b); ctx->raw_b = NULL; }
        if (ctx->raw_a) { stress_osal_free(ctx->raw_a); ctx->raw_a = NULL; }
        n /= 2ULL;
    }

    if (!a || !b || !c) {
        stress_osal_print("rtos_stress: error: [stream-%d] OOM,"
                          " failed to allocate buffers\n",
                          args->instance);
        goto worker_done;
    }

    stream_print_u64("rtos_stress: info: [stream] using ", n, " elements\n");

    stress_stream_init_data(a, b, c, n);
    old_checksum = stress_stream_checksum(a, b, c, n);

    if (stream_is_nan_or_inf(old_checksum) || old_checksum == 0.0) {
        stress_osal_print("rtos_stress: warn: [stream-%d] initial"
                          " checksum is zero or invalid,"
                          " checksum validation disabled\n",
                          args->instance);
        checksum_valid = 0;
    } else {
        checksum_valid = 1;
    }

    t_start = stress_osal_time_now();

    while (stress_continue(args)) {
        stress_stream_copy(c, a, n);
        stress_stream_scale(b, c, q, n);
        stress_stream_add(c, a, b, n);
        stress_stream_triad(a, b, c, q, n);

        total_rd_bytes += (double)n * (double)sizeof(double) * 6.0;
        total_wr_bytes += (double)n * (double)sizeof(double) * 4.0;
        total_fp_ops   += (double)n * 4.0;

        if (((uint32_t)args->bogo.current_ops & 63u) == 0u) {
            if (checksum_valid) {
                double new_cs = stress_stream_checksum(a, b, c, n);
                if (stream_is_nan_or_inf(new_cs)) {
                    stress_osal_print("rtos_stress: fail: [stream-%d]"
                                      " checksum is NaN or Inf,"
                                      " possible memory corruption\n",
                                      args->instance);
                    checksum_valid = 0;
                } else {
                    double ref  = stream_abs(old_checksum);
                    double diff = stream_abs(new_cs - old_checksum);
                    if (ref > 0.0 &&
                        (diff * (double)STREAM_CHECKSUM_REL_TOL_INV) > ref) {
                        stress_osal_print("rtos_stress: fail: [stream-%d]"
                                          " checksum drift exceeds 1/%d"
                                          " relative tolerance,"
                                          " possible memory corruption\n",
                                          args->instance,
                                          STREAM_CHECKSUM_REL_TOL_INV);
                    }
                    old_checksum = new_cs;
                }
            }
        }

        args->bogo.current_ops++;

        if (((uint32_t)args->bogo.current_ops % 10u) == 0u) {
            stress_osal_thread_yield();
        }
    }

    t_end = stress_osal_time_now();
    dt = t_end - t_start;

    if (dt > 0.001 && !stream_is_nan_or_inf(dt)) {
        double mb_rate = (total_rd_bytes + total_wr_bytes)
                         / (1024.0 * 1024.0) / dt;
        double mflops  = (total_fp_ops / 1000000.0) / dt;

        args->bogo.metric_val[0] = mb_rate;
        stream_safe_strcpy(args->bogo.metric_name[0], "MB/sec",
                           STREAM_METRIC_NAME_LEN);
        args->bogo.metric_val[1] = mflops;
        stream_safe_strcpy(args->bogo.metric_name[1], "Mflop/sec",
                           STREAM_METRIC_NAME_LEN);

        stress_osal_print("rtos_stress: info: [stream-%d] bandwidth: ",
                          args->instance);
        stream_print_fp2("", mb_rate, " MB/sec, compute: ");
        stream_print_fp2("", mflops,  " Mflop/sec\n");
    } else if (dt <= 0.0 || stream_is_nan_or_inf(dt)) {
        stress_osal_print("rtos_stress: warn: [stream-%d] invalid"
                          " elapsed time (clock may have wrapped"
                          " or gone backwards)\n",
                          args->instance);
    }

    if (ctx->raw_a) { stress_osal_free(ctx->raw_a); ctx->raw_a = NULL; }
    if (ctx->raw_b) { stress_osal_free(ctx->raw_b); ctx->raw_b = NULL; }
    if (ctx->raw_c) { stress_osal_free(ctx->raw_c); ctx->raw_c = NULL; }

worker_done:
    stress_osal_mb();
    if (!ctx->abandoned) {
        stress_osal_sem_release(ctx->done_sem);
    } else {
        stress_osal_print("rtos_stress: info: [stream-%d] worker"
                          " freeing abandoned ctx\n",
                          args->instance);
        if (ctx->raw_a) stress_osal_free(ctx->raw_a);
        if (ctx->raw_b) stress_osal_free(ctx->raw_b);
        if (ctx->raw_c) stress_osal_free(ctx->raw_c);
        stress_osal_free(ctx);
    }
}

void stress_stream(stress_args_t *args)
{
    stream_context_t *ctx;
    stress_tid_t      t_worker;

    ctx = (stream_context_t *)stress_osal_malloc(sizeof(stream_context_t));
    if (!ctx) {
        stress_osal_print("rtos_stress: error: [stream] OOM allocating"
                          " context\n");
        return;
    }

    ctx->args      = args;
    ctx->num_elems = s_stream_elem;
    ctx->abandoned = 0;
    ctx->raw_a     = NULL;
    ctx->raw_b     = NULL;
    ctx->raw_c     = NULL;
    ctx->done_sem  = stress_osal_sem_create("stm_sem", 0);

    if (!ctx->done_sem) {
        stress_osal_print("rtos_stress: error: [stream] failed to"
                          " create semaphore\n");
        stress_osal_free(ctx);
        return;
    }

    t_worker = stress_osal_thread_spawn(STREAM_THREAD_NAME,
                                        stress_stream_worker,
                                        ctx,
                                        STREAM_STACK_SIZE,
                                        20);
    if (!t_worker) {
        stress_osal_print("rtos_stress: error: [stream] failed to"
                          " spawn worker thread\n");
        stress_osal_sem_delete(ctx->done_sem);
        stress_osal_free(ctx);
        return;
    }

    while (stress_continue(args)) {
        stress_osal_sleep_ms(100);
    }

    {
        stress_tick_t timeout_ticks = (stress_tick_t)(
            (uint64_t)STREAM_WORKER_EXIT_TIMEOUT_MS
            * stress_osal_tick_hz() / 1000ULL);

        int ret = stress_osal_sem_take(ctx->done_sem, timeout_ticks);
        if (ret != 0) {
            stress_osal_print("rtos_stress: warn: [stream] worker did"
                              " not exit within %u ms, abandoning\n",
                              STREAM_WORKER_EXIT_TIMEOUT_MS);
            stress_osal_sem_delete(ctx->done_sem);
            stress_osal_mb();
            ctx->abandoned = 1;
            stress_osal_mb();
            return;
        }
    }

    stress_osal_sem_delete(ctx->done_sem);
    stress_osal_free(ctx);
}
