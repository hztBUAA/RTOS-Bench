#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <config.h>

#define STRESS_CACHE_LINE_SIZE  64

#define UNLIKELY(x)  __builtin_expect(!!(x), 0)
#define LIKELY(x)    __builtin_expect(!!(x), 1)

#ifndef OPTIMIZE3
#define OPTIMIZE3
#endif

/*
 * 根因说明：
 * libvpmpdm.so 在 AArch64 (飞腾派 E2000) 上，memmove/memcpy 大块路径不稳定。
 * 修复：用 stress_safe_move 替代所有 memmove 调用，64 字节分块循环复制。
 * 同时限制 mem_size 上限，避免触发 VMM 大块申请路径的边界问题。
 */
#define MEMTHRASH_MAX_SAFE_SIZE  (4UL * 1024UL * 1024UL)
#define MEMTHRASH_SAFE_CHUNK     64U

typedef struct {
    void          *mem;
    size_t         mem_size;
    stress_args_t *args;
} stress_memthrash_context_t;

typedef void (*stress_memthrash_func_t)(stress_memthrash_context_t *ctxt);

static size_t s_memthrash_size = DEFAULT_MEM_SIZE;

static int stress_memthrash_opt_size(const char *opt_name, const char *opt_arg)
{
    char *endptr;
    unsigned long long val = strtoull(opt_arg, &endptr, 10);

    (void)opt_name;

    if      (*endptr == 'k' || *endptr == 'K') val *= 1024ULL;
    else if (*endptr == 'm' || *endptr == 'M') val *= 1024ULL * 1024ULL;
    else if (*endptr == 'g' || *endptr == 'G') val *= 1024ULL * 1024ULL * 1024ULL;

    if (val < MIN_MEM_SIZE) val = MIN_MEM_SIZE;
    if (val > MAX_MEM_SIZE) val = MAX_MEM_SIZE;

    s_memthrash_size = (size_t)val;
    stress_osal_print("rtos_stress: debug: memthrash-size set to %llu bytes\n",
                      (unsigned long long)s_memthrash_size);
    return 0;
}

const stress_opt_t stress_memthrash_opts[] = {
    { "memthrash-size", stress_memthrash_opt_size },
    { NULL, NULL }
};

static uint32_t stress_mwc32(void) { return (uint32_t)stress_osal_rand(); }
static uint8_t  stress_mwc8(void)  { return (uint8_t)stress_osal_rand(); }

static uint64_t stress_mwc64(void)
{
#if RAND_MAX >= 0x7fffffffL
    return ((uint64_t)(uint32_t)stress_osal_rand() << 32) |
            (uint64_t)(uint32_t)stress_osal_rand();
#else
    uint64_t v = 0;
    int i;
    for (i = 0; i < 8; i++)
        v = (v << 8) ^ (uint8_t)stress_osal_rand();
    return v;
#endif
}

static size_t stress_mwc_offset(size_t mem_size)
{
    if (mem_size == 0) return 0;
    if (mem_size <= 0xFFFFFFFFUL)
        return (size_t)(stress_mwc32() % (uint32_t)mem_size);
    return (size_t)(stress_mwc64() % (uint64_t)mem_size);
}

/* ------------------------------------------------------------------
 * 平台安全 memmove：64 字节分块，规避 libvpmpdm 大块复制路径崩溃。
 * ------------------------------------------------------------------ */
static void stress_safe_move(void *dst, const void *src, size_t len)
{
    uint8_t       *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    size_t         i;

    if (d == s || len == 0) return;

    if (d < s) {
        /* 正向分块 */
        while (len >= MEMTHRASH_SAFE_CHUNK) {
            for (i = 0; i < MEMTHRASH_SAFE_CHUNK; i++) {
                d[i] = s[i];
            }
            d   += MEMTHRASH_SAFE_CHUNK;
            s   += MEMTHRASH_SAFE_CHUNK;
            len -= MEMTHRASH_SAFE_CHUNK;
        }
        while (len--) *d++ = *s++;
    } else {
        /* 反向分块，避免重叠覆盖 */
        d += len;
        s += len;
        while (len >= MEMTHRASH_SAFE_CHUNK) {
            d   -= MEMTHRASH_SAFE_CHUNK;
            s   -= MEMTHRASH_SAFE_CHUNK;
            len -= MEMTHRASH_SAFE_CHUNK;
            for (i = 0; i < MEMTHRASH_SAFE_CHUNK; i++) {
                d[i] = s[i];
            }
        }
        while (len--) *--d = *--s;
    }
}

/* ------------------------------------------------------------------ */

static void OPTIMIZE3 stress_memthrash_random_chunk(
    stress_memthrash_context_t *ctxt,
    const size_t chunk_size)
{
    uint32_t i;
    const uint32_t max = 256;
    size_t chunks = ctxt->mem_size / chunk_size;

    if (chunks < 1) chunks = 1;

    for (i = 0; i < max; i++) {
        size_t   chunk  = stress_mwc_offset(chunks);
        size_t   offset = chunk * chunk_size;
        uint8_t *ptr    = (uint8_t *)ctxt->mem + offset;
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
    if (ctxt->mem_size < 2) return;
    char *dst = ((char *)ctxt->mem) + 1;
    /* 用 stress_safe_move 替代 memmove，规避 libvpmpdm 大块路径崩溃 */
    stress_safe_move(dst, ctxt->mem, ctxt->mem_size - 1);
}

static void OPTIMIZE3 stress_memthrash_memset64(stress_memthrash_context_t *ctxt)
{
    if ((uintptr_t)ctxt->mem & (sizeof(uint64_t) - 1)) {
        stress_osal_memset(ctxt->mem, (int)stress_mwc8(), ctxt->mem_size);
        return;
    }

    const size_t n64         = ctxt->mem_size / sizeof(uint64_t);
    const size_t n64_aligned = n64 & ~(size_t)3;
    uint64_t    *ptr         = (uint64_t *)ctxt->mem;
    uint64_t    *end         = ptr + n64_aligned;
    uint64_t     val         = stress_mwc64();

    while (ptr < end) {
        *ptr++ = val;
        *ptr++ = val;
        *ptr++ = val;
        *ptr++ = val;
    }

    end = (uint64_t *)ctxt->mem + n64;
    while (ptr < end)
        *ptr++ = val;
}

static void OPTIMIZE3 stress_memthrash_matrix(stress_memthrash_context_t *ctxt)
{
    size_t n = 0;

    while (n < (size_t)(-1) - 1) {
        size_t next = n + 1;
        if (next > ctxt->mem_size / (next ? next : 1)) break;
        if (next * next > ctxt->mem_size) break;
        n = next;
    }
    if (n < 2) return;

    volatile uint8_t *vmem = (volatile uint8_t *)ctxt->mem;
    size_t i, j;
    size_t step = (size_t)(stress_mwc8() & 0xF) + 1;

    for (i = 0; i < n; i += step) {
        for (j = 0; j < n; j += 16) {
            size_t idx1 = i * n + j;
            size_t idx2 = j * n + i;
            if (idx1 < ctxt->mem_size && idx2 < ctxt->mem_size) {
                uint8_t tmp = vmem[idx1];
                vmem[idx1]  = vmem[idx2];
                vmem[idx2]  = tmp;
            }
        }
    }
}

static void OPTIMIZE3 stress_memthrash_prefetch(stress_memthrash_context_t *ctxt)
{
    uint32_t i;
    const uint32_t max = 1024;

    for (i = 0; i < max; i++) {
        size_t   offset = stress_mwc_offset(ctxt->mem_size);
        uint8_t *ptr    = (uint8_t *)ctxt->mem + offset;
#if defined(__GNUC__) && !defined(__riscv)
        __builtin_prefetch(ptr, 1, 3);
#else
        (void)ptr;
#endif
        *((volatile uint8_t *)ptr) = (uint8_t)i;
    }
}

static void OPTIMIZE3 stress_memthrash_swap(stress_memthrash_context_t *ctxt)
{
    size_t   i;
    size_t   sz   = ctxt->mem_size;
    size_t   off1 = stress_mwc_offset(sz);
    size_t   off2 = stress_mwc_offset(sz);
    uint8_t *u8   = (uint8_t *)ctxt->mem;

    for (i = 0; i < 4096; i++) {
        uint8_t tmp = u8[off1];
        u8[off1] = u8[off2];
        u8[off2] = tmp;

        off1 += 129;
        if (off1 >= sz) off1 %= sz;
        off2 += 65;
        if (off2 >= sz) off2 %= sz;
    }
}

#if defined(STRESS_HAVE_ATOMIC)
#  define HAVE_SYNC_FETCH_AND_ADD STRESS_HAVE_ATOMIC
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__)  || \
    defined(__aarch64__) || defined(__arm__)                           || \
    defined(__riscv)     || defined(__mips__)  || defined(__loongarch__))
#  define HAVE_SYNC_FETCH_AND_ADD 1
#else
#  define HAVE_SYNC_FETCH_AND_ADD 0
#endif

static void OPTIMIZE3 stress_memthrash_lock(stress_memthrash_context_t *ctxt)
{
    uint32_t i;

    if (ctxt->mem_size < sizeof(uint32_t)) return;

    for (i = 0; i < 256; i++) {
        size_t offset = stress_mwc_offset(ctxt->mem_size);
        offset &= ~(size_t)3;
        if (offset + sizeof(uint32_t) > ctxt->mem_size) continue;

        volatile uint32_t *ptr =
            (volatile uint32_t *)((uint8_t *)ctxt->mem + offset);
#if HAVE_SYNC_FETCH_AND_ADD
        __sync_fetch_and_add(ptr, 1u);
#else
        (*ptr)++;
#endif
    }
}

static void OPTIMIZE3 stress_memthrash_spinread(stress_memthrash_context_t *ctxt)
{
    uint32_t i;
    uint32_t sink = 0;

    if (ctxt->mem_size < sizeof(uint32_t)) return;

    size_t offset = stress_mwc_offset(ctxt->mem_size);
    offset &= ~(size_t)3;
    if (offset + sizeof(uint32_t) > ctxt->mem_size)
        offset = 0;

    volatile uint32_t *ptr =
        (volatile uint32_t *)((uint8_t *)ctxt->mem + offset);

    for (i = 0; i < 4096; i++) {
        sink += *ptr;
        sink += *ptr;
        sink += *ptr;
        sink += *ptr;
#if !defined(__riscv) || defined(CONFIG_SMP)
        stress_osal_mb();
#endif
    }
    (void)sink;
}

static void OPTIMIZE3 stress_memthrash_tlb(stress_memthrash_context_t *ctxt)
{
    const size_t stride      = 17 * STRESS_CACHE_LINE_SIZE;
    size_t       cache_lines = ctxt->mem_size / STRESS_CACHE_LINE_SIZE;
    size_t       k           = 0;
    volatile uint8_t *ptr    = (volatile uint8_t *)ctxt->mem;
    size_t j;

    if (cache_lines == 0) return;

    for (j = 0; j < cache_lines; j++) {
        ptr[k] = (uint8_t)j;
        k += stride;
        if (k >= ctxt->mem_size) k %= ctxt->mem_size;
    }
}

typedef struct {
    const char              *name;
    stress_memthrash_func_t  func;
} stress_memthrash_method_info_t;

static const stress_memthrash_method_info_t memthrash_methods[] = {
    { "chunk1",   stress_memthrash_chunk1   },
    { "chunk8",   stress_memthrash_chunk8   },
    { "chunk64",  stress_memthrash_chunk64  },
    { "chunk256", stress_memthrash_chunk256 },
    { "memset",   stress_memthrash_memset   },
    { "memset64", stress_memthrash_memset64 },
    { "memmove",  stress_memthrash_memmove  },
    { "matrix",   stress_memthrash_matrix   },
    { "prefetch", stress_memthrash_prefetch },
    { "swap",     stress_memthrash_swap     },
    { "lock",     stress_memthrash_lock     },
    { "spinread", stress_memthrash_spinread },
    { "tlb",      stress_memthrash_tlb      },
    { NULL,       NULL                      }
};

void stress_memthrash(stress_args_t *args)
{
    stress_memthrash_context_t ctxt;
    stress_memthrash_func_t    specific_func = NULL;
    stress_bool_t              run_all       = STRESS_FALSE;

    ctxt.args     = args;
    ctxt.mem_size = s_memthrash_size;

    /* 限制上限，避免触发 VMM 大块申请路径的边界问题 */
    if (ctxt.mem_size > MEMTHRASH_MAX_SAFE_SIZE) {
        ctxt.mem_size = MEMTHRASH_MAX_SAFE_SIZE;
        stress_osal_print("rtos_stress: info: [memthrash-%d] mem_size"
                          " capped to %lu bytes\n",
                          args->instance,
                          (unsigned long)MEMTHRASH_MAX_SAFE_SIZE);
    }

    while (ctxt.mem_size >= MIN_MEM_SIZE) {
        ctxt.mem = stress_osal_malloc(ctxt.mem_size);
        if (ctxt.mem) break;
        ctxt.mem_size /= 2;
    }

    if (!ctxt.mem) {
        stress_osal_print("rtos_stress: error: [memthrash] OOM (min %d bytes)\n",
                          MIN_MEM_SIZE);
        return;
    }

    if ((uintptr_t)ctxt.mem & (sizeof(uint64_t) - 1)) {
        stress_osal_print("rtos_stress: warn: [memthrash-%d] mem not"
                          " 8-byte aligned, memset64 will use memset fallback\n",
                          args->instance);
    }

    stress_osal_print("rtos_stress: info: [memthrash-%d] allocated %llu KB\n",
                      args->instance,
                      (unsigned long long)(ctxt.mem_size / 1024));

    if (args->method_name == NULL ||
        stress_osal_strcmp(args->method_name, "all") == 0) {
        run_all = STRESS_TRUE;
        stress_osal_print("rtos_stress: info: [memthrash-%d] using 'all' methods\n",
                          args->instance);
    } else {
        int i;
        for (i = 0; memthrash_methods[i].name != NULL; i++) {
            if (stress_osal_strcmp(args->method_name,
                                   memthrash_methods[i].name) == 0) {
                specific_func = memthrash_methods[i].func;
                break;
            }
        }
        if (!specific_func) {
            stress_osal_print("rtos_stress: warn: unknown method '%s',"
                              " using 'all'\n", args->method_name);
            run_all = STRESS_TRUE;
        } else {
            stress_osal_print("rtos_stress: info: [memthrash-%d]"
                              " using method '%s'\n",
                              args->instance, args->method_name);
        }
    }

    while (stress_continue(args)) {
        if (run_all) {
            int i;
            for (i = 0; memthrash_methods[i].name != NULL; i++) {
                if (!stress_continue(args)) break;
                memthrash_methods[i].func(&ctxt);
                args->bogo.current_ops++;
                if (i > 0 && i % 4 == 0) stress_osal_sleep_ms(1);
            }
        } else {
            specific_func(&ctxt);
            args->bogo.current_ops++;
            stress_osal_sleep_ms(1);
        }
    }

    stress_osal_free(ctxt.mem);
}
