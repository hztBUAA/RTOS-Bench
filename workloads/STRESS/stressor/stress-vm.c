/* applications/stress-ng/stress-vm.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <config.h>

#define VM_STACK_SIZE       (16 * 1024)

#define UNLIKELY(x)         __builtin_expect(!!(x), 0)
#define LIKELY(x)           __builtin_expect(!!(x), 1)

typedef struct {
    stress_args_t *args;
    stress_sem_t done_sem;
    size_t vm_bytes;
    const char *method_name;
} vm_context_t;

static uint64_t s_vm_bytes = DEFAULT_VM_BYTES;

static int stress_vm_opt_bytes(const char *opt_name, const char *opt_arg)
{
    char *endptr;
    unsigned long long val = strtoull(opt_arg, &endptr, 10);

    if (*endptr == 'k' || *endptr == 'K') val *= 1024;
    else if (*endptr == 'm' || *endptr == 'M') val *= (1024 * 1024);
    else if (*endptr == 'g' || *endptr == 'G') val *= (1024 * 1024 * 1024);

    if (val < MIN_VM_BYTES) val = MIN_VM_BYTES;
    if (val > MAX_VM_BYTES) val = MAX_VM_BYTES;

    val &= ~(unsigned long long)7;

    s_vm_bytes = (uint64_t)val;
    stress_osal_print("rtos_stress: debug: vm-bytes set to %llu bytes\n", (unsigned long long)s_vm_bytes);
    return 0;
}

const stress_opt_t stress_vm_opts[] = {
    { "vm-bytes", stress_vm_opt_bytes },
    { NULL, NULL }
};

static void stress_mwc_reseed(void) {
    stress_osal_srand(stress_osal_tick_get());
}

static uint32_t stress_mwc32(void) {
    return (uint32_t)stress_osal_rand();
}

static uint64_t stress_mwc64(void) {
    return ((uint64_t)stress_mwc32() << 32) | stress_mwc32();
}

static uint8_t stress_mwc8(void) {
    return (uint8_t)stress_mwc32();
}

static uint64_t stress_mwc64modn(uint64_t n) {
    if (n == 0) return 0;
    return stress_mwc64() % n;
}

#define stress_asm_mb()     stress_osal_mb()

static inline void stress_cache_flush(void *addr, size_t len) {
    (void)addr;
    (void)len;
    stress_asm_mb();
}

static inline size_t stress_count_bits64(uint64_t v) {
    size_t c = 0;
    for (; v; c++) v &= v - 1;
    return c;
}

typedef struct stress_vm_args {
    void *buf;
    void *buf_end;
    size_t sz;
    stress_args_t *main_args;
} stress_vm_fn_args_t;

typedef size_t (*stress_vm_func)(stress_vm_fn_args_t *p);

static size_t stress_vm_write64(stress_vm_fn_args_t *p)
{
    uint64_t *ptr = (uint64_t *)p->buf;
    uint64_t *end = (uint64_t *)((uintptr_t)p->buf_end & ~(uintptr_t)7);
    static uint64_t val = 0;
    size_t count = 0;

    while (ptr + 4 <= end) {
        *ptr++ = val;
        *ptr++ = val;
        *ptr++ = val;
        *ptr++ = val;
        if ((++count & 0x7F) == 0) {
            if (!stress_continue(p->main_args)) break;
            p->main_args->bogo.current_ops += 128;
        }
    }
    while (ptr < end) {
        *ptr++ = val;
    }
    val++;
    return 0;
}

static size_t stress_vm_read64(stress_vm_fn_args_t *p)
{
    volatile uint64_t *ptr = (uint64_t *)p->buf;
    volatile uint64_t *end = (uint64_t *)((uintptr_t)p->buf_end & ~(uintptr_t)7);
    size_t count = 0;

    while (ptr + 4 <= end) {
        (void)*ptr++;
        (void)*ptr++;
        (void)*ptr++;
        (void)*ptr++;
        if ((++count & 0x7F) == 0) {
            if (!stress_continue(p->main_args)) break;
            p->main_args->bogo.current_ops += 128;
        }
    }
    while (ptr < end) {
        (void)*ptr++;
    }
    return 0;
}

static size_t stress_vm_rand_set(stress_vm_fn_args_t *p)
{
    uint8_t *ptr;
    size_t bit_errors = 0;
    uint32_t seed = stress_osal_rand();
    size_t count = 0;

    stress_osal_srand(seed);
    for (ptr = (uint8_t *)p->buf; ptr < (uint8_t *)p->buf_end; ptr++) {
        *ptr = (uint8_t)stress_osal_rand();
        if ((++count & 0x3FF) == 0) {
            if (!stress_continue(p->main_args)) break;
            p->main_args->bogo.current_ops += 1024;
        }
    }

    stress_cache_flush(p->buf, p->sz);

    stress_osal_srand(seed);
    for (ptr = (uint8_t *)p->buf; ptr < (uint8_t *)p->buf_end; ptr++) {
        if (*ptr != (uint8_t)stress_osal_rand()) bit_errors++;
    }
    return bit_errors;
}

static size_t stress_vm_toggle(stress_vm_fn_args_t *p)
{
    uint64_t *ptr;
    size_t bit_errors = 0;
    uint64_t *end = (uint64_t *)((uintptr_t)p->buf_end & ~(uintptr_t)7);
    size_t count = 0;

    stress_osal_memset(p->buf, 0x00, p->sz);
    stress_cache_flush(p->buf, p->sz);
    for (ptr = (uint64_t *)p->buf; ptr < end; ptr++) {
        if (*ptr != 0x0000000000000000ULL) bit_errors++;
        if ((++count & 0x3FF) == 0) {
            if (!stress_continue(p->main_args)) break;
            p->main_args->bogo.current_ops += 1024;
        }
    }

    count = 0;
    stress_osal_memset(p->buf, 0xFF, p->sz);
    stress_cache_flush(p->buf, p->sz);
    for (ptr = (uint64_t *)p->buf; ptr < end; ptr++) {
        if (*ptr != 0xFFFFFFFFFFFFFFFFULL) bit_errors++;
        if ((++count & 0x3FF) == 0) {
            if (!stress_continue(p->main_args)) break;
            p->main_args->bogo.current_ops += 1024;
        }
    }

    return bit_errors;
}

static size_t stress_vm_walking_one(stress_vm_fn_args_t *p)
{
    uint8_t *ptr;
    size_t bit_errors = 0;
    size_t count = 0;

    for (ptr = (uint8_t *)p->buf; ptr < (uint8_t *)p->buf_end; ptr++) {
        uint8_t val = 0x01;
        int i;
        for (i = 0; i < 8; i++) {
            *ptr = val;
            stress_asm_mb();
            if (*ptr != val) bit_errors++;
            val <<= 1;
        }
        if ((++count & 0xFF) == 0) {
            if (!stress_continue(p->main_args)) break;
            p->main_args->bogo.current_ops += 256;
        }
    }
    return bit_errors;
}

static size_t stress_vm_galpat_one(stress_vm_fn_args_t *p)
{
    uint64_t *ptr;
    size_t bit_errors = 0;
    size_t bits_set = 0;
    size_t bits_flipped = 0;
    size_t count = 0;

    size_t bits_bad = p->sz / 4096;
    if (bits_bad == 0) bits_bad = 1;

    stress_osal_memset(p->buf, 0xff, p->sz);

    for (size_t i = 0; i < bits_bad; i++) {
        size_t offset = stress_mwc64modn(p->sz);
        uint8_t *u8p = (uint8_t *)p->buf + offset;
        uint8_t mask = (1 << (stress_osal_rand() & 7));

        if (*u8p & mask) {
            *u8p &= ~mask;
            bits_flipped++;
        }
        if ((i & 0xFF) == 0) {
            if (!stress_continue(p->main_args)) break;
            p->main_args->bogo.current_ops += 256;
        }
    }

    stress_cache_flush(p->buf, p->sz);

    uint64_t *end = (uint64_t *)((uintptr_t)p->buf_end & ~(uintptr_t)7);
    for (ptr = (uint64_t *)p->buf; ptr < end; ptr++) {
        bits_set += stress_count_bits64(~(*ptr));
        if ((++count & 0x3FF) == 0) {
            if (!stress_continue(p->main_args)) break;
        }
    }

    if (bits_set != bits_flipped) {
        bit_errors = 1;
    }

    return bit_errors;
}

static size_t stress_vm_gray(stress_vm_fn_args_t *p)
{
    static uint8_t val = 0;
    uint8_t v = val;
    uint8_t *ptr;
    size_t bit_errors = 0;
    size_t count = 0;

    for (ptr = (uint8_t *)p->buf; ptr < (uint8_t *)p->buf_end; ) {
        uint8_t mask = (v >> 1) ^ v;
        *ptr++ = mask;
        v++;
        if ((++count & 0x3FF) == 0) {
            if (!stress_continue(p->main_args)) break;
            p->main_args->bogo.current_ops += 1024;
        }
    }

    stress_cache_flush(p->buf, p->sz);

    v = val;
    count = 0;
    for (ptr = (uint8_t *)p->buf; ptr < (uint8_t *)p->buf_end; ) {
        uint8_t mask = (v >> 1) ^ v;
        if (*ptr++ != mask) bit_errors++;
        v++;
        if ((++count & 0x3FF) == 0) {
            if (!stress_continue(p->main_args)) break;
        }
    }
    val++;
    return bit_errors;
}

#define VM_ROWHAMMER_LOOPS 1000
static size_t stress_vm_rowhammer(stress_vm_fn_args_t *p)
{
    uint32_t *buf32 = (uint32_t *)p->buf;
    size_t n = p->sz / sizeof(uint32_t);
    volatile uint32_t *addr0, *addr1;
    static uint32_t val = 0xff5a00a5;
    size_t bit_errors = 0;

    if (n < 2) return 0;

    for (size_t j = 0; j < n; j++) buf32[j] = val;

    addr0 = &buf32[stress_mwc64modn(n)];
    addr1 = &buf32[stress_mwc64modn(n)];

    for (int j = 0; j < VM_ROWHAMMER_LOOPS; j++) {
        (void)*addr0; stress_asm_mb();
        (void)*addr1; stress_asm_mb();
        *addr0 = val;
        *addr1 = val;
        if ((j & 0x3F) == 0) {
            if (!stress_continue(p->main_args)) break;
            p->main_args->bogo.current_ops += 64;
        }
    }

    for (size_t j = 0; j < n; j++) {
        if (buf32[j] != val) bit_errors++;
    }

    val = (val >> 31) | (val << 1);
    return bit_errors;
}

static size_t stress_vm_modulo_x(stress_vm_fn_args_t *p)
{
    uint32_t stride = 23;
    uint8_t pattern = stress_mwc8();
    uint8_t comp = ~pattern;
    uint8_t *ptr;
    size_t bit_errors = 0;
    size_t count = 0;

    for (ptr = (uint8_t *)p->buf; ptr < (uint8_t *)p->buf_end; ptr += stride) {
        *ptr = pattern;
        if ((++count & 0x3FF) == 0) {
            if (!stress_continue(p->main_args)) break;
            p->main_args->bogo.current_ops += 1024;
        }
    }

    uint8_t *base = (uint8_t *)p->buf;
    size_t len = p->sz;
    for (size_t k = 0; k < len; k++) {
        if (k % stride != 0) base[k] = comp;
    }

    stress_cache_flush(p->buf, p->sz);

    for (ptr = (uint8_t *)p->buf; ptr < (uint8_t *)p->buf_end; ptr += stride) {
        if (*ptr != pattern) bit_errors++;
    }

    return bit_errors;
}

typedef struct {
    const char *name;
    stress_vm_func func;
} stress_vm_method_info_t;

static const stress_vm_method_info_t vm_methods[] = {
    { "write64",     stress_vm_write64 },
    { "read64",      stress_vm_read64 },
    { "rand-set",    stress_vm_rand_set },
    { "toggle",      stress_vm_toggle },
    { "walk-1",      stress_vm_walking_one },
    { "galpat-1",    stress_vm_galpat_one },
    { "gray",        stress_vm_gray },
    { "rowhammer",   stress_vm_rowhammer },
    { "modulo-x",    stress_vm_modulo_x },
    { NULL,          NULL }
};

static void stress_vm_worker(void *parameter)
{
    vm_context_t *ctx = (vm_context_t *)parameter;
    stress_args_t *args = ctx->args;
    void *buf = NULL;
    size_t vm_bytes = ctx->vm_bytes;
    stress_vm_func specific_func = NULL;
    stress_bool_t run_all = STRESS_FALSE;

    while (vm_bytes >= MIN_VM_BYTES) {
        buf = stress_osal_malloc(vm_bytes);
        if (buf) break;
        vm_bytes /= 2;
    }

    if (!buf) {
        stress_osal_print("rtos_stress: error: [vm] failed to allocate memory (min %d bytes)\n", MIN_VM_BYTES);
        goto worker_done;
    }

    size_t aligned_sz = vm_bytes & ~(size_t)7;
    
    stress_osal_print("rtos_stress: info: [vm-%d] allocated %d KB buffer\n", args->instance, (int)(vm_bytes / 1024));

    if (ctx->method_name == NULL || stress_osal_strcmp(ctx->method_name, "all") == 0) {
        run_all = STRESS_TRUE;
        stress_osal_print("rtos_stress: info: [vm-%d] using 'all' methods\n", args->instance);
    } else {
        for (int i = 0; vm_methods[i].name != NULL; i++) {
            if (stress_osal_strcmp(ctx->method_name, vm_methods[i].name) == 0) {
                specific_func = vm_methods[i].func;
                break;
            }
        }
        if (!specific_func) {
            stress_osal_print("rtos_stress: error: unknown method '%s', using 'all'\n", ctx->method_name);
            run_all = STRESS_TRUE;
        } else {
            stress_osal_print("rtos_stress: info: [vm-%d] using method '%s'\n", args->instance, ctx->method_name);
        }
    }

    stress_vm_fn_args_t fn_args;
    fn_args.buf = buf;
    fn_args.sz = aligned_sz;
    fn_args.buf_end = (void *)((uint8_t *)buf + aligned_sz);
    fn_args.main_args = args;

    while (stress_continue(args))
    {
        if (run_all) {
            for (int i = 0; vm_methods[i].name != NULL; i++) {
                if (!stress_continue(args)) break;

                size_t errors = vm_methods[i].func(&fn_args);
                if (errors > 0) {
                    stress_osal_print("rtos_stress: fail: [vm-%d] %s detected %d errors!\n",
                               args->instance, vm_methods[i].name, (int)errors);
                }
                stress_osal_sleep_ms(1);
            }
        } else {
            size_t errors = specific_func(&fn_args);
            if (errors > 0) {
                 stress_osal_print("rtos_stress: fail: [vm-%d] detected %d errors!\n", args->instance, (int)errors);
            }
            stress_osal_sleep_ms(1);
        }
    }

    stress_osal_free(buf);

worker_done:
    stress_osal_sem_release(ctx->done_sem);
}

void stress_vm(stress_args_t *args)
{
    vm_context_t *ctx;
    stress_tid_t t_worker = NULL;

    stress_mwc_reseed();

    ctx = (vm_context_t *)stress_osal_malloc(sizeof(vm_context_t));
    if (!ctx) {
        stress_osal_print("rtos_stress: error: [vm] OOM allocating context\n");
        return;
    }

    ctx->args = args;
    ctx->vm_bytes = (size_t)s_vm_bytes;
    ctx->method_name = args->method_name;
    ctx->done_sem = stress_osal_sem_create("vm_done", 0);

    if (!ctx->done_sem) {
        stress_osal_free(ctx);
        return;
    }

    t_worker = stress_osal_thread_spawn("ng_vm", 
                                        stress_vm_worker, 
                                        ctx, 
                                        VM_STACK_SIZE, 
                                        20);

    if (!t_worker) {
        stress_osal_print("rtos_stress: error: [vm] Failed to spawn worker\n");
        stress_osal_sem_delete(ctx->done_sem);
        stress_osal_free(ctx);
        return;
    }

    stress_osal_sem_take(ctx->done_sem, STRESS_WAIT_FOREVER);

    stress_osal_sem_delete(ctx->done_sem);
    stress_osal_free(ctx);
}
