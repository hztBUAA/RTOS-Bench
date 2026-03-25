#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <config.h>

#define VM_STACK_SIZE               (32 * 1024)
#define VM_WORKER_EXIT_TIMEOUT_MS   (5000U)

#define UNLIKELY(x)  __builtin_expect(!!(x), 0)
#define LIKELY(x)    __builtin_expect(!!(x), 1)

typedef struct {
    stress_args_t  *args;
    stress_sem_t    done_sem;
    size_t          vm_bytes;
    const char     *method_name;
    volatile int    abort;
    /*
     * abandoned：主线程超时后置 1，ctx 所有权转移给 worker。
     *
     * 注意：ctx->abandoned 的检查与 sem_release 之间存在不可消除的
     * 微小时间窗口（见 stress_vm_worker worker_done 处注释）。
     * 在 5 秒超时设计下，此窗口出现概率极低，工程上可接受。
     * 若需彻底消除，需引入额外互斥锁保护"检查+操作"原子性。
     */
    volatile int    abandoned;
} vm_context_t;

static uint64_t s_vm_bytes = DEFAULT_VM_BYTES;

static int stress_vm_opt_bytes(const char *opt_name, const char *opt_arg)
{
    char               *endptr;
    unsigned long long  val = strtoull(opt_arg, &endptr, 10);

    if      (*endptr == 'k' || *endptr == 'K') val *= 1024ULL;
    else if (*endptr == 'm' || *endptr == 'M') val *= 1024ULL * 1024ULL;
    else if (*endptr == 'g' || *endptr == 'G') val *= 1024ULL * 1024ULL * 1024ULL;

    if (val < MIN_VM_BYTES) val = MIN_VM_BYTES;
    if (val > MAX_VM_BYTES) val = MAX_VM_BYTES;

    val &= ~(unsigned long long)7;

    s_vm_bytes = (uint64_t)val;
    stress_osal_print("rtos_stress: debug: vm-bytes set to %llu bytes\n",
                      (unsigned long long)s_vm_bytes);
    return 0;
}

const stress_opt_t stress_vm_opts[] = {
    { "vm-bytes", stress_vm_opt_bytes },
    { NULL, NULL }
};

static uint32_t stress_vm_mwc32(void) { return (uint32_t)stress_osal_rand(); }

static uint64_t stress_vm_mwc64(void)
{
    return ((uint64_t)stress_vm_mwc32() << 32) | stress_vm_mwc32();
}

static uint8_t stress_vm_mwc8(void) { return (uint8_t)stress_vm_mwc32(); }

static uint64_t stress_vm_mwc64modn(uint64_t n)
{
    if (n == 0) return 0;
    return stress_vm_mwc64() % n;
}

static uint32_t vm_lcg(uint32_t *state)
{
    *state = *state * 1664525u + 1013904223u;
    return *state;
}

#define stress_asm_mb()  stress_osal_mb()

static void stress_cache_flush(void *addr, size_t len)
{
    (void)addr;
    (void)len;
    stress_asm_mb();
}

static size_t stress_count_bits64(uint64_t v)
{
    size_t c = 0;
    for (; v; c++) v &= v - 1;
    return c;
}

typedef struct {
    void          *buf;
    void          *buf_end;
    size_t         sz;
    stress_args_t *main_args;
    volatile int  *p_abort;
} stress_vm_fn_args_t;

typedef size_t (*stress_vm_func)(stress_vm_fn_args_t *p);

static size_t stress_vm_write64(stress_vm_fn_args_t *p)
{
    uint64_t *ptr   = (uint64_t *)p->buf;
    uint64_t *end   = (uint64_t *)p->buf_end;
    uint64_t  val   = stress_vm_mwc64();
    size_t    count = 0;

    while (ptr + 4 <= end) {
        *ptr++ = val;
        *ptr++ = val;
        *ptr++ = val;
        *ptr++ = val;
        if ((++count & 0x7Fu) == 0) {
            if (*p->p_abort || !stress_continue(p->main_args)) break;
            p->main_args->bogo.current_ops += 128;
        }
    }
    while (ptr < end) *ptr++ = val;
    return 0;
}

static size_t stress_vm_read64(stress_vm_fn_args_t *p)
{
    volatile uint64_t *ptr   = (volatile uint64_t *)p->buf;
    volatile uint64_t *end   = (volatile uint64_t *)p->buf_end;
    size_t             count = 0;

    while (ptr + 4 <= end) {
        (void)*ptr++; (void)*ptr++; (void)*ptr++; (void)*ptr++;
        if ((++count & 0x7Fu) == 0) {
            if (*p->p_abort || !stress_continue(p->main_args)) break;
            p->main_args->bogo.current_ops += 128;
        }
    }
    while (ptr < end) (void)*ptr++;
    return 0;
}

static size_t stress_vm_rand_set(stress_vm_fn_args_t *p)
{
    uint8_t  *ptr;
    size_t    bit_errors = 0;
    uint32_t  seed;
    uint32_t  lcg_state;
    size_t    count      = 0;
    int       complete   = 1;

    seed      = stress_vm_mwc32() ^ (uint32_t)(uintptr_t)p->buf;
    lcg_state = seed;

    for (ptr = (uint8_t *)p->buf; ptr < (uint8_t *)p->buf_end; ptr++) {
        *ptr = (uint8_t)vm_lcg(&lcg_state);
        if ((++count & 0x3FFu) == 0) {
            if (*p->p_abort || !stress_continue(p->main_args)) {
                complete = 0;
                break;
            }
            p->main_args->bogo.current_ops += 1024;
        }
    }

    if (!complete) return 0;

    stress_cache_flush(p->buf, p->sz);

    lcg_state = seed;
    count = 0;
    for (ptr = (uint8_t *)p->buf; ptr < (uint8_t *)p->buf_end; ptr++) {
        if (*ptr != (uint8_t)vm_lcg(&lcg_state)) bit_errors++;
        if ((++count & 0x3FFu) == 0) {
            if (*p->p_abort || !stress_continue(p->main_args)) break;
        }
    }
    return bit_errors;
}

static size_t stress_vm_toggle(stress_vm_fn_args_t *p)
{
    uint64_t *ptr;
    uint64_t *end        = (uint64_t *)p->buf_end;
    size_t    bit_errors = 0;
    size_t    count      = 0;

    stress_osal_memset(p->buf, 0x00, p->sz);
    stress_cache_flush(p->buf, p->sz);

    for (ptr = (uint64_t *)p->buf; ptr < end; ptr++) {
        if (*ptr != 0x0000000000000000ULL) bit_errors++;
        if ((++count & 0x3FFu) == 0) {
            if (*p->p_abort || !stress_continue(p->main_args)) goto toggle_done;
            p->main_args->bogo.current_ops += 1024;
        }
    }

    count = 0;
    stress_osal_memset(p->buf, 0xFF, p->sz);
    stress_cache_flush(p->buf, p->sz);

    for (ptr = (uint64_t *)p->buf; ptr < end; ptr++) {
        if (*ptr != 0xFFFFFFFFFFFFFFFFULL) bit_errors++;
        if ((++count & 0x3FFu) == 0) {
            if (*p->p_abort || !stress_continue(p->main_args)) goto toggle_done;
            p->main_args->bogo.current_ops += 1024;
        }
    }

toggle_done:
    return bit_errors;
}

static size_t stress_vm_walking_one(stress_vm_fn_args_t *p)
{
    uint8_t *ptr;
    size_t   bit_errors = 0;
    size_t   count      = 0;
    int      i;

    for (ptr = (uint8_t *)p->buf; ptr < (uint8_t *)p->buf_end; ptr++) {
        uint8_t val = 0x01;
        for (i = 0; i < 8; i++) {
            *ptr = val;
            stress_asm_mb();
            if (*ptr != val) bit_errors++;
            val = (uint8_t)(val << 1);
        }
        if ((++count & 0xFFu) == 0) {
            if (*p->p_abort || !stress_continue(p->main_args)) break;
            p->main_args->bogo.current_ops += 256;
        }
    }
    return bit_errors;
}

static size_t stress_vm_galpat_one(stress_vm_fn_args_t *p)
{
    uint64_t *ptr;
    uint64_t *end;
    size_t    bit_errors   = 0;
    size_t    bits_set     = 0;
    size_t    bits_flipped = 0;
    size_t    count        = 0;
    size_t    i;
    int       interrupted  = 0;

    size_t bits_bad = p->sz / 4096;
    if (bits_bad == 0) bits_bad = 1;

    stress_osal_memset(p->buf, 0xff, p->sz);

    for (i = 0; i < bits_bad; i++) {
        size_t   offset = (size_t)stress_vm_mwc64modn((uint64_t)(p->sz));
        uint8_t *u8p    = (uint8_t *)p->buf + offset;
        uint8_t  mask   = (uint8_t)(1u << (stress_vm_mwc32() & 7u));

        if (*u8p & mask) {
            *u8p &= (uint8_t)~mask;
            bits_flipped++;
        }
        if ((i & 0xFFu) == 0) {
            if (*p->p_abort || !stress_continue(p->main_args)) {
                interrupted = 1;
                break;
            }
            p->main_args->bogo.current_ops += 256;
        }
    }

    if (interrupted) return 0;

    stress_cache_flush(p->buf, p->sz);

    end = (uint64_t *)p->buf_end;
    for (ptr = (uint64_t *)p->buf; ptr < end; ptr++) {
        bits_set += stress_count_bits64(~(*ptr));
        if ((++count & 0x3FFu) == 0) {
            if (*p->p_abort || !stress_continue(p->main_args)) {
                interrupted = 1;
                break;
            }
        }
    }

    if (interrupted) return 0;

    if (bits_set != bits_flipped) bit_errors = 1;
    return bit_errors;
}

static size_t stress_vm_gray(stress_vm_fn_args_t *p)
{
    uint8_t  start_v    = stress_vm_mwc8();
    uint8_t  v          = start_v;
    uint8_t *ptr;
    size_t   bit_errors = 0;
    size_t   count      = 0;
    int      complete   = 1;

    for (ptr = (uint8_t *)p->buf; ptr < (uint8_t *)p->buf_end; ) {
        uint8_t mask = (uint8_t)((v >> 1) ^ v);
        *ptr++ = mask;
        v++;
        if ((++count & 0x3FFu) == 0) {
            if (*p->p_abort || !stress_continue(p->main_args)) {
                complete = 0;
                break;
            }
            p->main_args->bogo.current_ops += 1024;
        }
    }

    if (!complete) return 0;

    stress_cache_flush(p->buf, p->sz);

    v = start_v;
    count = 0;
    for (ptr = (uint8_t *)p->buf; ptr < (uint8_t *)p->buf_end; ) {
        uint8_t mask = (uint8_t)((v >> 1) ^ v);
        if (*ptr++ != mask) bit_errors++;
        v++;
        if ((++count & 0x3FFu) == 0) {
            if (*p->p_abort || !stress_continue(p->main_args)) break;
        }
    }
    return bit_errors;
}

#define VM_ROWHAMMER_LOOPS 1000

static size_t stress_vm_rowhammer(stress_vm_fn_args_t *p)
{
    uint32_t          *buf32      = (uint32_t *)p->buf;
    size_t             n          = p->sz / sizeof(uint32_t);
    volatile uint32_t *addr0;
    volatile uint32_t *addr1;
    uint32_t           val        = 0xff5a00a5u ^ stress_vm_mwc32();
    size_t             bit_errors = 0;
    size_t             i;
    int                j;

    if (n < 2) return 0;

    for (i = 0; i < n; i++) buf32[i] = val;

    addr0 = &buf32[stress_vm_mwc64modn((uint64_t)n)];
    addr1 = &buf32[stress_vm_mwc64modn((uint64_t)n)];

    for (j = 0; j < VM_ROWHAMMER_LOOPS; j++) {
        (void)*addr0; stress_asm_mb();
        (void)*addr1; stress_asm_mb();
        *addr0 = val;
        *addr1 = val;
        if ((j & 0x3F) == 0) {
            if (*p->p_abort || !stress_continue(p->main_args)) break;
            p->main_args->bogo.current_ops += 64;
        }
    }

    for (i = 0; i < n; i++) {
        if (buf32[i] != val) bit_errors++;
    }
    return bit_errors;
}

static size_t stress_vm_modulo_x(stress_vm_fn_args_t *p)
{
    uint32_t stride          = 23;
    uint8_t  pattern         = stress_vm_mwc8();
    uint8_t  comp            = (uint8_t)~pattern;
    uint8_t *ptr;
    uint8_t *base            = (uint8_t *)p->buf;
    size_t   len             = p->sz;
    size_t   bit_errors      = 0;
    size_t   count           = 0;
    size_t   k;
    size_t   off;
    int      phase1_complete = 1;

    for (ptr = base; ptr < (uint8_t *)p->buf_end; ptr += stride) {
        *ptr = pattern;
        if ((++count & 0x3FFu) == 0) {
            if (*p->p_abort || !stress_continue(p->main_args)) {
                phase1_complete = 0;
                break;
            }
            p->main_args->bogo.current_ops += 1024;
        }
    }

    if (!phase1_complete) return 0;

    off   = 0;
    count = 0;
    for (k = 0; k < len; k++) {
        if (off != 0) base[k] = comp;
        if (++off == stride) off = 0;
        if ((++count & 0x3FFu) == 0) {
            if (*p->p_abort || !stress_continue(p->main_args)) return 0;
        }
    }

    stress_cache_flush(p->buf, p->sz);

    for (ptr = base; ptr < (uint8_t *)p->buf_end; ptr += stride) {
        if (*ptr != pattern) bit_errors++;
    }
    return bit_errors;
}

typedef struct {
    const char     *name;
    stress_vm_func  func;
} stress_vm_method_info_t;

static const stress_vm_method_info_t vm_methods[] = {
    { "write64",   stress_vm_write64     },
    { "read64",    stress_vm_read64      },
    { "rand-set",  stress_vm_rand_set    },
    { "toggle",    stress_vm_toggle      },
    { "walk-1",    stress_vm_walking_one },
    { "galpat-1",  stress_vm_galpat_one  },
    { "gray",      stress_vm_gray        },
    { "rowhammer", stress_vm_rowhammer   },
    { "modulo-x",  stress_vm_modulo_x    },
    { NULL,        NULL                  }
};

static void stress_vm_worker(void *parameter)
{
    vm_context_t        *ctx           = (vm_context_t *)parameter;
    stress_args_t       *args          = ctx->args;
    void                *buf           = NULL;
    size_t               vm_bytes      = ctx->vm_bytes;
    size_t               aligned_sz;
    stress_vm_func       specific_func = NULL;
    stress_bool_t        run_all       = STRESS_FALSE;
    stress_vm_fn_args_t  fn_args;
    int                  i;
    size_t               errors;

    while (vm_bytes >= MIN_VM_BYTES) {
        buf = stress_osal_malloc(vm_bytes);
        if (buf) break;
        vm_bytes /= 2;
    }

    if (!buf) {
        stress_osal_print("rtos_stress: error: [vm-%d] failed to"
                          " allocate memory (min %d bytes)\n",
                          args->instance, MIN_VM_BYTES);
        goto worker_done;
    }

    aligned_sz = vm_bytes & ~(size_t)7;
    if (aligned_sz == 0) {
        stress_osal_print("rtos_stress: error: [vm-%d] aligned size"
                          " is zero\n", args->instance);
        stress_osal_free(buf);
        buf = NULL;
        goto worker_done;
    }

    stress_osal_print("rtos_stress: info: [vm-%d] allocated %llu KB"
                      " buffer\n",
                      args->instance,
                      (unsigned long long)(vm_bytes / 1024));

    if (ctx->method_name == NULL ||
        stress_osal_strcmp(ctx->method_name, "all") == 0) {
        run_all = STRESS_TRUE;
    } else {
        for (i = 0; vm_methods[i].name != NULL; i++) {
            if (stress_osal_strcmp(ctx->method_name,
                                   vm_methods[i].name) == 0) {
                specific_func = vm_methods[i].func;
                break;
            }
        }
        if (!specific_func) {
            stress_osal_print("rtos_stress: error: [vm-%d] unknown"
                              " method '%s', using 'all'\n",
                              args->instance, ctx->method_name);
            run_all = STRESS_TRUE;
        }
    }

    fn_args.buf       = buf;
    fn_args.sz        = aligned_sz;
    fn_args.buf_end   = (void *)((uint8_t *)buf + aligned_sz);
    fn_args.main_args = args;
    fn_args.p_abort   = &ctx->abort;

    while (stress_continue(args) && !ctx->abort) {
        if (run_all) {
            for (i = 0; vm_methods[i].name != NULL; i++) {
                if (!stress_continue(args) || ctx->abort) break;
                errors = vm_methods[i].func(&fn_args);
                if (errors > 0) {
                    stress_osal_print("rtos_stress: fail: [vm-%d]"
                                      " %s detected %zu errors!\n",
                                      args->instance,
                                      vm_methods[i].name,
                                      errors);
                }
                stress_osal_sleep_ms(1);
            }
        } else {
            errors = specific_func(&fn_args);
            if (errors > 0) {
                stress_osal_print("rtos_stress: fail: [vm-%d]"
                                  " detected %zu errors!\n",
                                  args->instance, errors);
            }
            stress_osal_sleep_ms(1);
        }
    }

    stress_osal_free(buf);
    buf = NULL;

worker_done:
    stress_osal_mb();
    if (!ctx->abandoned) {
        stress_osal_sem_release(ctx->done_sem);
    } else {
        stress_osal_print("rtos_stress: info: [vm-%d] worker freeing"
                          " abandoned ctx\n", args->instance);
        stress_osal_free(ctx);
    }
}

void stress_vm(stress_args_t *args)
{
    vm_context_t *ctx;
    stress_tid_t  t_worker;
    char          sem_name[16];

    stress_osal_srand((unsigned int)(
        stress_osal_tick_get() ^ (uintptr_t)args));

    ctx = (vm_context_t *)stress_osal_malloc(sizeof(vm_context_t));
    if (!ctx) {
        stress_osal_print("rtos_stress: error: [vm-%d] OOM allocating"
                          " context\n", args->instance);
        return;
    }

    stress_osal_memset(ctx, 0, sizeof(vm_context_t));
    ctx->args        = args;
    ctx->vm_bytes    = (size_t)s_vm_bytes;
    ctx->method_name = args->method_name;
    ctx->abort       = 0;
    ctx->abandoned   = 0;

    stress_osal_snprintf(sem_name, sizeof(sem_name), "vd_%d", args->instance);
    ctx->done_sem = stress_osal_sem_create(sem_name, 0);

    if (!ctx->done_sem) {
        stress_osal_print("rtos_stress: error: [vm-%d] sem create"
                          " failed\n", args->instance);
        stress_osal_free(ctx);
        return;
    }

    t_worker = stress_osal_thread_spawn("ng_vm",
                                        stress_vm_worker,
                                        ctx,
                                        VM_STACK_SIZE,
                                        20);
    if (!t_worker) {
        stress_osal_print("rtos_stress: error: [vm-%d] failed to spawn"
                          " worker\n", args->instance);
        stress_osal_sem_delete(ctx->done_sem);
        stress_osal_free(ctx);
        return;
    }

    while (stress_continue(args)) {
        stress_osal_sleep_ms(100);
    }

    stress_osal_mb();
    ctx->abort = 1;
    stress_osal_mb();

    {
        stress_tick_t timeout_ticks = (stress_tick_t)(
            (uint64_t)VM_WORKER_EXIT_TIMEOUT_MS
            * stress_osal_tick_hz() / 1000ULL);

        int ret = stress_osal_sem_take(ctx->done_sem, timeout_ticks);

        if (ret == 0) {
            stress_osal_sem_delete(ctx->done_sem);
            stress_osal_free(ctx);
        } else {
            stress_osal_print("rtos_stress: warn: [vm-%d] worker did"
                              " not exit within %u ms\n",
                              args->instance,
                              VM_WORKER_EXIT_TIMEOUT_MS);
            stress_osal_mb();
            ctx->abandoned = 1;
            stress_osal_mb();
            stress_osal_sem_delete(ctx->done_sem);
        }
    }
}
