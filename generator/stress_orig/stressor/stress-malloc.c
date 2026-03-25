#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <config.h>

#define UNLIKELY(x)  __builtin_expect(!!(x), 0)
#define LIKELY(x)    __builtin_expect(!!(x), 1)

#define MALLOC_REALLOC_FAIL_LIMIT   8
#define MALLOC_MAX_TOTAL_HARD_CAP   (64ULL * 1024ULL * 1024ULL)
#define MALLOC_TOTAL_MULTIPLIER     2ULL
#define MALLOC_ALIGN                16U
#define MALLOC_MIN_SIZE             (MALLOC_ALIGN * 2)
#define MALLOC_SAFE_COPY_CHUNK      64U

typedef struct {
    void   *addr;
    size_t  len;
    int     realloc_fail_count;
} stress_malloc_info_t;

static uint64_t s_malloc_bytes = DEFAULT_MALLOC_BYTES;
static uint32_t s_malloc_max   = DEFAULT_MALLOC_MAX;

/* ------------------------------------------------------------------
 * 平台安全复制：64 字节分块，规避 libvpmpdm memcpy 大块路径崩溃。
 * ------------------------------------------------------------------ */
static void stress_safe_copy(void *dst, const void *src, size_t len)
{
    uint8_t       *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    size_t         i;

    while (len >= MALLOC_SAFE_COPY_CHUNK) {
        for (i = 0; i < MALLOC_SAFE_COPY_CHUNK; i++) {
            d[i] = s[i];
        }
        d   += MALLOC_SAFE_COPY_CHUNK;
        s   += MALLOC_SAFE_COPY_CHUNK;
        len -= MALLOC_SAFE_COPY_CHUNK;
    }
    while (len > 0) {
        *d++ = *s++;
        len--;
    }
}

/* ------------------------------------------------------------------
 * 平台安全 realloc 模拟。
 *
 * 语义与标准 realloc 不同，调用方必须遵守：
 *   - 返回非 NULL：new_ptr 有效，old_ptr 已被 free，调用方更新 slot。
 *   - 返回 NULL  ：malloc 失败，old_ptr 依然有效，调用方保留原 slot。
 *     （不同于标准 realloc：失败时 old_ptr 不会被 free。）
 * ------------------------------------------------------------------ */
static void *stress_safe_realloc(void *old_ptr, size_t old_len,
                                 size_t new_len)
{
    void  *new_ptr;
    size_t copy_len;

    if (new_len == 0) {
        stress_osal_free(old_ptr);
        return NULL;
    }

    new_ptr = stress_osal_malloc(new_len);
    if (!new_ptr) {
        /* malloc 失败：old_ptr 保持有效，由调用方决定后续处理 */
        return NULL;
    }

    if (old_ptr && old_len > 0) {
        copy_len = (new_len < old_len) ? new_len : old_len;
        stress_safe_copy(new_ptr, old_ptr, copy_len);
    }

    stress_osal_free(old_ptr);
    return new_ptr;
}

/* ------------------------------------------------------------------ */

static int stress_malloc_opt_bytes(const char *opt_name, const char *opt_arg)
{
    char               *endptr;
    unsigned long long  val = strtoull(opt_arg, &endptr, 10);

    (void)opt_name;

    if      (*endptr == 'k' || *endptr == 'K') val *= 1024ULL;
    else if (*endptr == 'm' || *endptr == 'M') val *= 1024ULL * 1024ULL;
    else if (*endptr == 'g' || *endptr == 'G') val *= 1024ULL * 1024ULL * 1024ULL;

    if (val < MIN_MALLOC_BYTES) val = MIN_MALLOC_BYTES;
    if (val > MAX_MALLOC_BYTES) val = MAX_MALLOC_BYTES;

    s_malloc_bytes = (uint64_t)val;
    stress_osal_print("rtos_stress: debug: malloc-bytes set to %llu\n",
                      (unsigned long long)s_malloc_bytes);
    return 0;
}

static int stress_malloc_opt_max(const char *opt_name, const char *opt_arg)
{
    int val = atoi(opt_arg);

    (void)opt_name;

    if (val < MIN_MALLOC_MAX) val = MIN_MALLOC_MAX;
    if (val > MAX_MALLOC_MAX) val = MAX_MALLOC_MAX;

    s_malloc_max = (uint32_t)val;
    stress_osal_print("rtos_stress: debug: malloc-max set to %u slots\n",
                      s_malloc_max);
    return 0;
}

const stress_opt_t stress_malloc_opts[] = {
    { "malloc-bytes", stress_malloc_opt_bytes },
    { "malloc-max",   stress_malloc_opt_max   },
    { NULL, NULL }
};

static uint32_t instance_rand(uint32_t *seed)
{
    *seed = *seed * 1664525U + 1013904223U;
    return *seed;
}

static size_t stress_alloc_size(uint32_t *seed, size_t max_size)
{
    uint32_t effective_max;
    size_t   len;

    if (UNLIKELY(max_size <= MALLOC_MIN_SIZE)) {
        return MALLOC_MIN_SIZE;
    }

    effective_max = (max_size > (size_t)UINT32_MAX)
                    ? UINT32_MAX
                    : (uint32_t)max_size;

    len = (size_t)(instance_rand(seed) % effective_max);
    if (len < MALLOC_MIN_SIZE) {
        len = MALLOC_MIN_SIZE;
    }

    len = (len + (size_t)(MALLOC_ALIGN - 1)) & ~(size_t)(MALLOC_ALIGN - 1);
    return len;
}

static void total_bytes_sub(size_t *total, size_t sub)
{
    if (*total >= sub) {
        *total -= sub;
    } else {
        *total = 0;
    }
}

static int cap_check(size_t total_bytes, size_t max_total, size_t need)
{
    if (total_bytes >= max_total) {
        return 0;
    }
    return need <= (max_total - total_bytes);
}

static void slot_free(stress_malloc_info_t *slot, size_t *total_bytes)
{
    total_bytes_sub(total_bytes, slot->len);
    stress_osal_free(slot->addr);
    slot->addr               = NULL;
    slot->len                = 0;
    slot->realloc_fail_count = 0;
}

void stress_malloc(stress_args_t *args)
{
    stress_malloc_info_t *info         = NULL;
    size_t                malloc_max   = (size_t)s_malloc_max;
    size_t                malloc_bytes = (size_t)s_malloc_bytes;
    size_t                info_size;
    size_t                total_bytes  = 0;
    size_t                max_total;
    uint64_t              dynamic_cap;
    uint32_t              seed;
    size_t                k;

    if (malloc_max == 0) {
        stress_osal_print("rtos_stress: error: [malloc-%d] malloc_max is 0\n",
                          args->instance);
        return;
    }

    if (malloc_max > SIZE_MAX / sizeof(stress_malloc_info_t)) {
        stress_osal_print("rtos_stress: error: [malloc-%d] malloc_max overflow\n",
                          args->instance);
        return;
    }

    info_size = malloc_max * sizeof(stress_malloc_info_t);

    dynamic_cap = (uint64_t)malloc_max * (uint64_t)malloc_bytes
                  * MALLOC_TOTAL_MULTIPLIER;
    if (dynamic_cap > MALLOC_MAX_TOTAL_HARD_CAP) {
        dynamic_cap = MALLOC_MAX_TOTAL_HARD_CAP;
    }
    if (dynamic_cap > (uint64_t)SIZE_MAX) {
        dynamic_cap = (uint64_t)SIZE_MAX;
    }
    max_total = (size_t)dynamic_cap;

    seed = (uint32_t)(
        (uintptr_t)args
        ^ ((uint32_t)args->instance * 2654435761U)
        ^ (uint32_t)stress_osal_tick_get()
    );
    instance_rand(&seed);
    seed ^= (uint32_t)stress_osal_tick_get();
    if (seed == 0U) seed = 0xDEADBEEFU;

    stress_osal_print("rtos_stress: info: [malloc-%d] max_allocs=%llu,"
                      " max_bytes_per_alloc=%llu, total_cap=%llu\n",
                      args->instance,
                      (unsigned long long)malloc_max,
                      (unsigned long long)malloc_bytes,
                      (unsigned long long)max_total);

    info = (stress_malloc_info_t *)stress_osal_calloc(1, info_size);
    if (!info) {
        stress_osal_print("rtos_stress: error: [malloc-%d] info alloc failed"
                          " (%llu bytes)\n",
                          args->instance,
                          (unsigned long long)info_size);
        return;
    }

    while (stress_continue(args)) {
        uint32_t rnd    = instance_rand(&seed);
        uint32_t i      = rnd % (uint32_t)malloc_max;
        uint32_t action = (rnd >> 12) & 1;

        if (info[i].addr) {
            if (action) {
                /* --- free 路径 --- */
                slot_free(&info[i], &total_bytes);
                args->bogo.current_ops++;
            } else {
                /* --- realloc 路径（用 stress_safe_realloc 替代原生 realloc） --- */
                size_t new_len = stress_alloc_size(&seed, malloc_bytes);
                void  *tmp;

                if (new_len > info[i].len) {
                    size_t delta = new_len - info[i].len;
                    if (!cap_check(total_bytes, max_total, delta)) {
                        stress_osal_thread_yield();
                        continue;
                    }
                }

                tmp = stress_safe_realloc(info[i].addr, info[i].len, new_len);

                if (tmp) {
                    /*
                     * 成功：old_ptr 已被 stress_safe_realloc 内部 free，
                     * 更新 slot 到新块。
                     */
                    if (new_len >= info[i].len) {
                        total_bytes += new_len - info[i].len;
                    } else {
                        total_bytes_sub(&total_bytes, info[i].len - new_len);
                    }
                    info[i].addr               = tmp;
                    info[i].len                = new_len;
                    info[i].realloc_fail_count = 0;
                    args->bogo.current_ops++;
                } else {
                    /*
                     * 失败：old_ptr 依然有效（stress_safe_realloc 失败时不 free）。
                     * 仅计数，超限后主动 free 旧块释放压力。
                     */
                    info[i].realloc_fail_count++;
                    if (info[i].realloc_fail_count >= MALLOC_REALLOC_FAIL_LIMIT) {
                        slot_free(&info[i], &total_bytes);
                    }
                    stress_osal_thread_yield();
                }
            }
        } else {
            if (action) {
                /* --- malloc 路径 --- */
                size_t len = stress_alloc_size(&seed, malloc_bytes);
                void  *ptr;

                if (!cap_check(total_bytes, max_total, len)) {
                    stress_osal_thread_yield();
                    continue;
                }

                ptr = stress_osal_malloc(len);
                if (ptr) {
                    info[i].addr               = ptr;
                    info[i].len                = len;
                    info[i].realloc_fail_count = 0;
                    total_bytes               += len;
                    args->bogo.current_ops++;
                } else {
                    stress_osal_thread_yield();
                }
            }
            /* action == 0 且 slot 为空：跳过，等下一轮 */
        }

        if ((args->bogo.current_ops % 512ULL) == 0ULL) {
            stress_osal_thread_yield();
        }
    }

    /* 清理所有存活的 slot */
    for (k = 0; k < malloc_max; k++) {
        if (info[k].addr) {
            stress_osal_free(info[k].addr);
            info[k].addr = NULL;
            info[k].len  = 0;
        }
    }

    stress_osal_free(info);
}
