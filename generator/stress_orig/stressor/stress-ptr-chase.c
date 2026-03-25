#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <config.h>

#ifndef PRIu64
#  ifdef __LP64__
#    define PRIu64 "lu"
#  else
#    define PRIu64 "llu"
#  endif
#endif

#define PAGE_SIZE_4K            (4096U)
#define PTRS_PER_4K_PAGE        (PAGE_SIZE_4K / sizeof(void *))
#define HEARTBEAT_INTERVAL      (1000000ULL)
#define INNER_YIELD_STRIDE      (64)

#define UNLIKELY(x)             __builtin_expect(!!(x), 0)
#define LIKELY(x)               __builtin_expect(!!(x), 1)

typedef struct stress_ptrs {
    struct stress_ptrs *next[PTRS_PER_4K_PAGE];
} stress_ptrs_t;

static uint64_t s_ptr_chase_pages = DEFAULT_PTR_CHASE_PAGES;

static int stress_ptr_chase_opt_pages(const char *opt_name, const char *opt_arg)
{
    char          *endptr;
    unsigned long  val = strtoul(opt_arg, &endptr, 10);
    unsigned long  max = (unsigned long)MAX_PTR_CHASE_PAGES;
    unsigned long  min = (unsigned long)MIN_PTR_CHASE_PAGES;

    if (*endptr == 'k' || *endptr == 'K') {
        if (val > max / 1024UL) val = max;
        else val *= 1024UL;
    } else if (*endptr == 'm' || *endptr == 'M') {
        if (val > max / (1024UL * 1024UL)) val = max;
        else val *= (1024UL * 1024UL);
    }

    if (val < min) val = min;
    if (val > max) val = max;

    s_ptr_chase_pages = (uint64_t)val;
    stress_osal_print("rtos_stress: debug: ptr-chase-pages set to %"
                      PRIu64 "\n", s_ptr_chase_pages);
    return 0;
}

const stress_opt_t stress_ptr_chase_opts[] = {
    { "ptr-chase-pages", stress_ptr_chase_opt_pages },
    { NULL, NULL }
};

static uint32_t stress_mwc32(void)
{
    return (uint32_t)stress_osal_rand();
}

static uint64_t stress_mwc64modn(uint64_t n)
{
    uint64_t v;
    if (n == 0) return 0;
    v = ((uint64_t)(stress_mwc32() & 0xFFFFU))
      | ((uint64_t)(stress_mwc32() & 0xFFFFU) << 16)
      | ((uint64_t)(stress_mwc32() & 0xFFFFU) << 32)
      | ((uint64_t)(stress_mwc32() & 0xFFFFU) << 48);
    return v % n;
}

static int pool_fits_size_t(uint64_t num_pages)
{
    uint64_t size_max = (uint64_t)(size_t)-1;
    uint64_t page_sz  = (uint64_t)sizeof(stress_ptrs_t);
    uint64_t ptr_sz   = (uint64_t)sizeof(stress_ptrs_t *);

    if (num_pages == 0)                 return 0;
    if (num_pages > size_max / page_sz) return 0;
    if (num_pages > size_max / ptr_sz)  return 0;
    return 1;
}

void stress_ptr_chase(stress_args_t *args)
{
    uint64_t         num_pages        = s_ptr_chase_pages;
    stress_ptrs_t  **ptr_lookup_table = NULL;
    stress_ptrs_t   *page_pool        = NULL;
    stress_ptrs_t   *current_ptr;
    uintptr_t        ptr_mask         = ~(uintptr_t)1U;
    uint32_t         idx_mask         = (uint32_t)(PTRS_PER_4K_PAGE - 1U);
    uint64_t         i;
    uint64_t         visited_count    = 0;
    uint64_t         total_count      = 0;
    uint64_t         ops_accum        = 0;
    uint64_t         last_log_ops     = 0;

    while (num_pages >= (uint64_t)MIN_PTR_CHASE_PAGES) {
        if (!pool_fits_size_t(num_pages)) {
            num_pages /= 2;
            continue;
        }

        page_pool = (stress_ptrs_t *)stress_osal_calloc(
            (size_t)num_pages, sizeof(stress_ptrs_t));

        if (page_pool) {
            ptr_lookup_table = (stress_ptrs_t **)stress_osal_malloc(
                (size_t)(num_pages * (uint64_t)sizeof(stress_ptrs_t *)));

            if (ptr_lookup_table) {
                break;
            }
            stress_osal_free(page_pool);
            page_pool = NULL;
        }
        num_pages /= 2;
    }

    if (!page_pool || !ptr_lookup_table) {
        stress_osal_print("rtos_stress: error: [ptr-chase-%d] OOM\n",
                          args->instance);
        if (ptr_lookup_table) stress_osal_free(ptr_lookup_table);
        if (page_pool)        stress_osal_free(page_pool);
        return;
    }

    if (num_pages == 1) {
        stress_osal_print("rtos_stress: warn: [ptr-chase-%d] only 1 page,"
                          " coverage stats will be trivial\n",
                          args->instance);
    }

    stress_osal_print("rtos_stress: info: [ptr-chase-%d] chasing %"
                      PRIu64 " pages (%" PRIu64 " KB)\n",
                      args->instance, num_pages, num_pages * 4ULL);

    for (i = 0; i < num_pages; i++) {
        ptr_lookup_table[(size_t)i] = &page_pool[(size_t)i];
    }

    for (i = 0; i < num_pages; i++) {
        stress_ptrs_t *p = ptr_lookup_table[(size_t)i];
        size_t j;

        for (j = 0; j < PTRS_PER_4K_PAGE; j++) {
            uint64_t target_idx;
            do {
                target_idx = stress_mwc64modn(num_pages);
            } while (target_idx == i && num_pages > 1);

            p->next[j] = ptr_lookup_table[(size_t)target_idx];
        }
    }

    current_ptr  = ptr_lookup_table[0];
    last_log_ops = 0;

    while (stress_continue(args)) {
        int k;
        for (k = 0; k < 1024; k++) {
            uint32_t  ridx          = stress_mwc32() & idx_mask;
            size_t    j             = (size_t)ridx;
            uintptr_t next_addr_raw = (uintptr_t)current_ptr->next[j];

            current_ptr->next[j] = (struct stress_ptrs *)(next_addr_raw | 1U);
            current_ptr          = (stress_ptrs_t *)(next_addr_raw & ptr_mask);

            if ((k & (INNER_YIELD_STRIDE - 1)) == (INNER_YIELD_STRIDE - 1)) {
                stress_osal_thread_yield();
            }
        }

        ops_accum += 1024ULL;
        args->bogo.current_ops += 1024;

        if (ops_accum - last_log_ops >= HEARTBEAT_INTERVAL) {
            stress_osal_print("rtos_stress: [ptr-chase-%d] heartbeat: %"
                              PRIu64 " ops\n",
                              args->instance, ops_accum);
            last_log_ops = ops_accum;
        }
    }

    total_count = num_pages * (uint64_t)PTRS_PER_4K_PAGE;

    for (i = 0; i < num_pages; i++) {
        stress_ptrs_t *p = ptr_lookup_table[(size_t)i];
        size_t j;
        for (j = 0; j < PTRS_PER_4K_PAGE; j++) {
            if ((uintptr_t)p->next[j] & 1U) {
                visited_count++;
            }
        }
    }

    if (total_count > 0) {
        uint64_t p_int = visited_count * 100ULL / total_count;
        uint64_t p_dec = (visited_count * 10000ULL / total_count) % 100ULL;

        stress_osal_print("rtos_stress: info: [ptr-chase-%d] finished."
                          " visited %" PRIu64 " of %" PRIu64
                          " pointers (%" PRIu64 ".%02" PRIu64 "%%)\n",
                          args->instance,
                          visited_count, total_count,
                          p_int, p_dec);
    }

    stress_osal_free(ptr_lookup_table);
    stress_osal_free(page_pool);
}
