/* applications/stress-ng/stress-ptr-chase.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <stress-config.h>

#define PAGE_SIZE_4K            (4096)
#define PTRS_PER_4K_PAGE        (PAGE_SIZE_4K / sizeof(void *))
#define HEARTBEAT_INTERVAL      (10000000)

#define UNLIKELY(x)             __builtin_expect(!!(x), 0)
#define LIKELY(x)               __builtin_expect(!!(x), 1)

typedef struct stress_ptrs {
    uintptr_t next[PTRS_PER_4K_PAGE];
} stress_ptrs_t;

enum {
    STRESS_PTR_CHASE_SIZE_CHECK = 1 / (int)(sizeof(stress_ptrs_t) == PAGE_SIZE_4K),
    STRESS_PTR_CHASE_PAGE_POWER_CHECK = 1 / (int)((PAGE_SIZE_4K & (PAGE_SIZE_4K - 1)) == 0),
    STRESS_PTR_CHASE_PTRS_POWER_CHECK = 1 / (int)((PTRS_PER_4K_PAGE & (PTRS_PER_4K_PAGE - 1)) == 0)
};

static uint64_t s_ptr_chase_pages = DEFAULT_PTR_CHASE_PAGES;

static int stress_ptr_chase_opt_pages(const char *opt_name, const char *opt_arg)
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

    if (val < (unsigned long long)MIN_PTR_CHASE_PAGES) val = MIN_PTR_CHASE_PAGES;
    if (val > (unsigned long long)MAX_PTR_CHASE_PAGES) val = MAX_PTR_CHASE_PAGES;

    s_ptr_chase_pages = (uint64_t)val;
    stress_osal_print("rtos_stress: debug: ptr-chase-pages set to %llu\n", (unsigned long long)s_ptr_chase_pages);
    return 0;
}

const stress_opt_t stress_ptr_chase_opts[] = {
    { "ptr-chase-pages", stress_ptr_chase_opt_pages },
    { NULL, NULL }
};

static int stress_ptr_chase_size_mul(uint64_t n, size_t size, size_t *result)
{
    if (size != 0 && n > (uint64_t)(SIZE_MAX / size)) {
        return -1;
    }

    *result = (size_t)n * size;
    return 0;
}

static void *stress_ptr_chase_aligned_alloc(size_t alignment, size_t size)
{
    void *raw;
    uintptr_t raw_ptr;
    uintptr_t raw_addr;
    uintptr_t aligned_addr;
    uintptr_t mask;
    size_t alloc_size;

    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        return NULL;
    }

    if ((alignment - 1) > SIZE_MAX - sizeof(void *)) {
        return NULL;
    }

    if (size > SIZE_MAX - (alignment - 1) - sizeof(void *)) {
        return NULL;
    }

    alloc_size = size + alignment - 1 + sizeof(void *);

    raw = stress_osal_malloc(alloc_size);
    if (!raw) {
        return NULL;
    }

    raw_ptr = (uintptr_t)raw;

    if (raw_ptr > UINTPTR_MAX - sizeof(void *)) {
        stress_osal_free(raw);
        return NULL;
    }

    raw_addr = raw_ptr + sizeof(void *);
    mask = (uintptr_t)alignment - 1;

    if (raw_addr > UINTPTR_MAX - mask) {
        stress_osal_free(raw);
        return NULL;
    }

    aligned_addr = (raw_addr + mask) & ~mask;
    ((void **)aligned_addr)[-1] = raw;

    return (void *)aligned_addr;
}

static void stress_ptr_chase_aligned_free(void *ptr)
{
    if (ptr) {
        stress_osal_free(((void **)ptr)[-1]);
    }
}

static uint64_t stress_ptr_chase_seed(stress_args_t *args)
{
    uint64_t seed;

    seed = ((uint64_t)(uint32_t)stress_osal_rand() << 32) |
           (uint64_t)(uint32_t)stress_osal_rand();
    seed ^= (uint64_t)(uintptr_t)&seed;
    seed ^= (uint64_t)(uintptr_t)args;
    seed ^= ((uint64_t)(uint32_t)args->instance << 32);

    if (seed == 0) {
        seed = 0x9e3779b97f4a7c15ULL;
    }

    return seed;
}

static inline uint32_t stress_ptr_chase_rand32(uint64_t *state)
{
    uint64_t x = *state;

    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;

    if (x == 0) {
        x = 0x9e3779b97f4a7c15ULL;
    }

    *state = x;

    return (uint32_t)((x >> 32) ^ x);
}

static inline uint64_t stress_ptr_chase_rand64modn(uint64_t *state, uint64_t n)
{
    uint64_t r;

    if (n == 0) {
        return 0;
    }

    r = ((uint64_t)stress_ptr_chase_rand32(state) << 32) |
        (uint64_t)stress_ptr_chase_rand32(state);

    return r % n;
}

static inline int stress_ptr_chase_ptr_in_pool(uintptr_t pool_start, uintptr_t pool_end, uintptr_t ptr)
{
    if (ptr < pool_start || ptr >= pool_end) {
        return 0;
    }

    if (((ptr - pool_start) & (uintptr_t)(PAGE_SIZE_4K - 1)) != 0) {
        return 0;
    }

    return 1;
}

void stress_ptr_chase(stress_args_t *args)
{
    uint64_t num_pages = s_ptr_chase_pages;
    stress_ptrs_t **ptr_lookup_table = NULL;
    stress_ptrs_t *page_pool = NULL;
    stress_ptrs_t *current_ptr;
    uintptr_t ptr_mask = ~(uintptr_t)1;
    uintptr_t pool_start;
    uintptr_t pool_end;
    uint64_t rng_state;
    uint64_t i;
    size_t pool_size = 0;

    size_t visited_count = 0;
    size_t total_count = 0;
    uint64_t last_log_ops = 0;
    int failed = 0;

    while (num_pages >= MIN_PTR_CHASE_PAGES) {
        size_t try_pool_size;
        size_t try_table_size;

        if (stress_ptr_chase_size_mul(num_pages, sizeof(stress_ptrs_t), &try_pool_size) != 0 ||
            stress_ptr_chase_size_mul(num_pages, sizeof(stress_ptrs_t *), &try_table_size) != 0) {
            num_pages /= 2;
            continue;
        }

        page_pool = (stress_ptrs_t *)stress_ptr_chase_aligned_alloc(PAGE_SIZE_4K, try_pool_size);
        if (page_pool) {
            ptr_lookup_table = (stress_ptrs_t **)stress_osal_malloc(try_table_size);
            if (ptr_lookup_table) {
                pool_size = try_pool_size;
                break;
            }
            stress_ptr_chase_aligned_free(page_pool);
            page_pool = NULL;
        }
        num_pages /= 2;
    }

    if (!page_pool || !ptr_lookup_table) {
        stress_osal_print("rtos_stress: error: [ptr-chase] OOM, failed to allocate memory\n");
        return;
    }

    pool_start = (uintptr_t)page_pool;

    if (pool_size > (size_t)UINTPTR_MAX || pool_start > UINTPTR_MAX - (uintptr_t)pool_size) {
        stress_osal_print("rtos_stress: error: [ptr-chase] address range overflow\n");
        stress_osal_free(ptr_lookup_table);
        stress_ptr_chase_aligned_free(page_pool);
        return;
    }

    if ((pool_start & (uintptr_t)(PAGE_SIZE_4K - 1)) != 0) {
        stress_osal_print("rtos_stress: error: [ptr-chase] page pool is not 4K aligned\n");
        stress_osal_free(ptr_lookup_table);
        stress_ptr_chase_aligned_free(page_pool);
        return;
    }

    pool_end = pool_start + (uintptr_t)pool_size;

    stress_osal_print("rtos_stress: info: [ptr-chase-%d] chasing %llu pages (%llu KB)\n",
               args->instance, (unsigned long long)num_pages, (unsigned long long)(num_pages * 4ULL));

    rng_state = stress_ptr_chase_seed(args);

    for (i = 0; i < num_pages; i++) {
        ptr_lookup_table[i] = &page_pool[i];
    }

    for (i = 0; i < num_pages; i++) {
        stress_ptrs_t *p = ptr_lookup_table[i];
        size_t j;

        for (j = 0; j < PTRS_PER_4K_PAGE; j++) {
            uint64_t target_idx;
            do {
                target_idx = stress_ptr_chase_rand64modn(&rng_state, num_pages);
            } while (target_idx == i && num_pages > 1);

            p->next[j] = (uintptr_t)ptr_lookup_table[(size_t)target_idx];
        }
    }

    current_ptr = ptr_lookup_table[0];
    uint32_t idx_mask = (uint32_t)(PTRS_PER_4K_PAGE - 1);

    while (stress_continue(args))
    {
        int k;
        uint32_t ops_done = 0;

        for (k = 0; k < 1024; k++) {
            size_t j = (size_t)(stress_ptr_chase_rand32(&rng_state) & idx_mask);
            uintptr_t next_addr_raw = current_ptr->next[j];
            uintptr_t next_addr = next_addr_raw & ptr_mask;

            if (UNLIKELY(!stress_ptr_chase_ptr_in_pool(pool_start, pool_end, next_addr))) {
                stress_osal_print("rtos_stress: error: [ptr-chase-%d] invalid next pointer 0x%llx at current 0x%llx index %u\n",
                           args->instance,
                           (unsigned long long)next_addr_raw,
                           (unsigned long long)(uintptr_t)current_ptr,
                           (unsigned int)j);
                failed = 1;
                break;
            }

            current_ptr->next[j] = next_addr | (uintptr_t)1;
            current_ptr = (stress_ptrs_t *)next_addr;
            ops_done++;
        }

        args->bogo.current_ops += ops_done;

        if (failed) {
            break;
        }

        if ((uint64_t)args->bogo.current_ops - last_log_ops >= HEARTBEAT_INTERVAL) {
            stress_osal_print("rtos_stress: [ptr-chase-%d] heartbeat: %llu ops\n",
                       args->instance, (unsigned long long)args->bogo.current_ops);
            last_log_ops = (uint64_t)args->bogo.current_ops;
        }

        stress_osal_thread_yield();
    }

    total_count = (size_t)num_pages * PTRS_PER_4K_PAGE;

    for (i = 0; i < num_pages; i++) {
        stress_ptrs_t *p = ptr_lookup_table[i];
        size_t j;
        for (j = 0; j < PTRS_PER_4K_PAGE; j++) {
            if (p->next[j] & (uintptr_t)1) {
                visited_count++;
            }
        }
    }

    if (total_count > 0) {
        float percentage = (float)visited_count * 100.0f / (float)total_count;
        int p_int = (int)percentage;
        int p_dec = (int)((percentage - p_int) * 100);

        stress_osal_print("rtos_stress: info: [ptr-chase-%d] finished. visited %llu of %llu pointers (%d.%02d%%)\n",
                   args->instance, (unsigned long long)visited_count, (unsigned long long)total_count,
                   p_int, p_dec);
    }

    stress_osal_free(ptr_lookup_table);
    stress_ptr_chase_aligned_free(page_pool);
}
