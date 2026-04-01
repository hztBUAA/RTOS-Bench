/* applications/stress-ng/stress-memthrash.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <stress-config.h>

#define STRESS_CACHE_LINE_SIZE  64

#define UNLIKELY(x)         __builtin_expect(!!(x), 0)
#define LIKELY(x)           __builtin_expect(!!(x), 1)

#ifndef OPTIMIZE3
#define OPTIMIZE3
#endif

#define stress_asm_mb()     __asm__ volatile("" ::: "memory")

typedef struct {
    void *mem;
    size_t mem_size;
    stress_args_t *args;
} stress_memthrash_context_t;

typedef void (*stress_memthrash_func_t)(stress_memthrash_context_t *ctxt);

static size_t s_memthrash_size = DEFAULT_MEM_SIZE;

static int stress_memthrash_opt_size(const char *opt_name, const char *opt_arg)
{
    char *endptr;
    unsigned long long val = strtoull(opt_arg, &endptr, 10);

    if (*endptr == 'k' || *endptr == 'K') val *= 1024;
    else if (*endptr == 'm' || *endptr == 'M') val *= (1024 * 1024);
    else if (*endptr == 'g' || *endptr == 'G') val *= (1024 * 1024 * 1024);

    if (val < MIN_MEM_SIZE) val = MIN_MEM_SIZE;
    if (val > MAX_MEM_SIZE) val = MAX_MEM_SIZE;

    s_memthrash_size = (size_t)val;
    stress_osal_print("rtos_stress: debug: memthrash-size set to %lu bytes\n", (unsigned long)s_memthrash_size);
    return 0;
}

const stress_opt_t stress_memthrash_opts[] = {
    { "memthrash-size", stress_memthrash_opt_size },
    { NULL, NULL }
};

static uint32_t stress_mwc32(void) { return (uint32_t)stress_osal_rand(); }
static uint8_t  stress_mwc8(void)  { return (uint8_t)stress_osal_rand(); }
static uint64_t stress_mwc64(void) { return ((uint64_t)stress_osal_rand() << 32) | stress_osal_rand(); }
static uint32_t stress_mwc32modn(uint32_t n) { return (n > 0) ? stress_mwc32() % n : 0; }

static void OPTIMIZE3 stress_memthrash_random_chunk(
    stress_memthrash_context_t *ctxt,
    const size_t chunk_size)
{
    uint32_t i;
    const uint32_t max = 256;
    size_t chunks = ctxt->mem_size / chunk_size;

    if (chunks < 1) chunks = 1;

    for (i = 0; i < max; i++) {
        const size_t chunk = stress_mwc32modn((uint32_t)chunks);
        const size_t offset = chunk * chunk_size;
        uint8_t *ptr = (uint8_t *)ctxt->mem + offset;

        stress_osal_memset(ptr, stress_mwc8(), chunk_size);
    }
}

static void stress_memthrash_chunk1(stress_memthrash_context_t *ctxt) {
    stress_memthrash_random_chunk(ctxt, 1);
}
static void stress_memthrash_chunk8(stress_memthrash_context_t *ctxt) {
    stress_memthrash_random_chunk(ctxt, 8);
}
static void stress_memthrash_chunk64(stress_memthrash_context_t *ctxt) {
    stress_memthrash_random_chunk(ctxt, 64);
}
static void stress_memthrash_chunk256(stress_memthrash_context_t *ctxt) {
    stress_memthrash_random_chunk(ctxt, 256);
}

static void stress_memthrash_memset(stress_memthrash_context_t *ctxt) {
    stress_osal_memset(ctxt->mem, stress_mwc8(), ctxt->mem_size);
}

static void stress_memthrash_memmove(stress_memthrash_context_t *ctxt) {
    char *dst = ((char *)ctxt->mem) + 1;
    memmove(dst, ctxt->mem, ctxt->mem_size - 1);
}

static void OPTIMIZE3 stress_memthrash_memset64(stress_memthrash_context_t *ctxt) {
    uint64_t *ptr = (uint64_t *)ctxt->mem;
    uint64_t *end = (uint64_t *)((uint8_t *)ctxt->mem + ctxt->mem_size);
    uint64_t val = stress_mwc64();

    end = ptr + (ctxt->mem_size / sizeof(uint64_t));

    while (ptr < end) {
        *ptr++ = val;
        *ptr++ = val;
        *ptr++ = val;
        *ptr++ = val;
    }
}

static void OPTIMIZE3 stress_memthrash_matrix(stress_memthrash_context_t *ctxt) {
    size_t n = 0;
    while ((n + 1) * (n + 1) <= ctxt->mem_size) n++;
    if (n < 2) return;

    volatile uint8_t *vmem = (volatile uint8_t *)ctxt->mem;
    size_t i, j;

    size_t step = (stress_mwc8() & 0xF) + 1;

    for (i = 0; i < n; i += step) {
        for (j = 0; j < n; j += 16) {
            size_t idx1 = i * n + j;
            size_t idx2 = j * n + i;
            if (idx1 < ctxt->mem_size && idx2 < ctxt->mem_size) {
                uint8_t tmp = vmem[idx1];
                vmem[idx1] = vmem[idx2];
                vmem[idx2] = tmp;
            }
        }
    }
}

static void OPTIMIZE3 stress_memthrash_prefetch(stress_memthrash_context_t *ctxt) {
    uint32_t i;
    const uint32_t max = 1024;

    for (i = 0; i < max; i++) {
        size_t offset = stress_mwc32modn((uint32_t)ctxt->mem_size);
        uint8_t *ptr = (uint8_t *)ctxt->mem + offset;

        __builtin_prefetch(ptr, 1, 3);

        *ptr = (uint8_t)i;
    }
}

static void OPTIMIZE3 stress_memthrash_swap(stress_memthrash_context_t *ctxt) {
    size_t i;
    size_t off1 = stress_mwc32modn((uint32_t)ctxt->mem_size);
    size_t off2 = stress_mwc32modn((uint32_t)ctxt->mem_size);
    uint8_t *u8 = (uint8_t *)ctxt->mem;

    for (i = 0; i < 4096; i++) {
        uint8_t tmp = u8[off1];
        u8[off1] = u8[off2];
        u8[off2] = tmp;

        off1 = (off1 + 129) % ctxt->mem_size;
        off2 = (off2 + 65) % ctxt->mem_size;
    }
}

static void OPTIMIZE3 stress_memthrash_lock(stress_memthrash_context_t *ctxt) {
    uint32_t i;
    for (i = 0; i < 256; i++) {
        size_t offset = stress_mwc32modn((uint32_t)ctxt->mem_size);
        offset &= ~(size_t)3;

        volatile uint32_t *ptr = (volatile uint32_t *)((uint8_t *)ctxt->mem + offset);

        __sync_fetch_and_add(ptr, 1);
    }
}

static void OPTIMIZE3 stress_memthrash_spinread(stress_memthrash_context_t *ctxt) {
    uint32_t i;
    size_t offset = stress_mwc32modn((uint32_t)ctxt->mem_size) & ~(size_t)3;
    volatile uint32_t *ptr = (volatile uint32_t *)((uint8_t *)ctxt->mem + offset);

    for (i = 0; i < 4096; i++) {
        (void)*ptr; (void)*ptr; (void)*ptr; (void)*ptr;
    }
}

static void OPTIMIZE3 stress_memthrash_tlb(stress_memthrash_context_t *ctxt) {
    size_t cache_lines = ctxt->mem_size / STRESS_CACHE_LINE_SIZE;
    size_t stride = 17 * STRESS_CACHE_LINE_SIZE;
    size_t k = 0;
    volatile uint8_t *ptr = (volatile uint8_t *)ctxt->mem;

    if (cache_lines == 0) return;

    for (size_t j = 0; j < cache_lines; j++) {
        ptr[k] = (uint8_t)j;
        k = (k + stride) % ctxt->mem_size;
    }
}

typedef struct {
    const char *name;
    stress_memthrash_func_t func;
} stress_memthrash_method_info_t;

static const stress_memthrash_method_info_t memthrash_methods[] = {
    { "chunk1",     stress_memthrash_chunk1 },
    { "chunk8",     stress_memthrash_chunk8 },
    { "chunk64",    stress_memthrash_chunk64 },
    { "chunk256",   stress_memthrash_chunk256 },
    { "memset",     stress_memthrash_memset },
    { "memset64",   stress_memthrash_memset64 },
    { "memmove",    stress_memthrash_memmove },
    { "matrix",     stress_memthrash_matrix },
    { "prefetch",   stress_memthrash_prefetch },
    { "swap",       stress_memthrash_swap },
    { "lock",       stress_memthrash_lock },
    { "spinread",   stress_memthrash_spinread },
    { "tlb",        stress_memthrash_tlb },
    { NULL,         NULL }
};

void stress_memthrash(stress_args_t *args)
{
    stress_memthrash_context_t ctxt;
    stress_memthrash_func_t specific_func = NULL;
    stress_bool_t run_all = STRESS_FALSE;

    ctxt.args = args;
    ctxt.mem_size = s_memthrash_size;

    while (ctxt.mem_size >= MIN_MEM_SIZE) {
        ctxt.mem = stress_osal_malloc(ctxt.mem_size);
        if (ctxt.mem) break;
        ctxt.mem_size /= 2;
    }

    if (!ctxt.mem) {
        stress_osal_print("rtos_stress: error: [memthrash] OOM allocating memory (min %d bytes)\n", MIN_MEM_SIZE);
        return;
    }

    stress_osal_print("rtos_stress: info: [memthrash-%d] allocated %d KB\n", args->instance, ctxt.mem_size / 1024);

    if (args->method_name == NULL || stress_osal_strcmp(args->method_name, "all") == 0) {
        run_all = STRESS_TRUE;
        stress_osal_print("rtos_stress: info: [memthrash-%d] using 'all' methods\n", args->instance);
    } else {
        for (int i = 0; memthrash_methods[i].name != NULL; i++) {
            if (stress_osal_strcmp(args->method_name, memthrash_methods[i].name) == 0) {
                specific_func = memthrash_methods[i].func;
                break;
            }
        }
        if (!specific_func) {
            stress_osal_print("rtos_stress: error: unknown method '%s', using 'all'\n", args->method_name);
            run_all = STRESS_TRUE;
        } else {
            stress_osal_print("rtos_stress: info: [memthrash-%d] using method '%s'\n", args->instance, args->method_name);
        }
    }

    while (stress_continue(args))
    {
        if (run_all) {
            for (int i = 0; memthrash_methods[i].name != NULL; i++) {
                if (!stress_continue(args)) break;
                memthrash_methods[i].func(&ctxt);

                if (i % 4 == 0) stress_osal_sleep_ms(1);
            }
        } else {
            specific_func(&ctxt);
            stress_osal_sleep_ms(1);
        }

        args->bogo.current_ops++;
    }

    stress_osal_free(ctxt.mem);
}
