/* applications/stress-ng/stress-ptr-chase.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <stress-config.h>

#define PAGE_SIZE_4K            (4096)
#define PTRS_PER_4K_PAGE        (PAGE_SIZE_4K / sizeof(void *))
#define HEARTBEAT_INTERVAL      (1000000)

#define UNLIKELY(x)             __builtin_expect(!!(x), 0)
#define LIKELY(x)               __builtin_expect(!!(x), 1)

typedef struct stress_ptrs {
    struct stress_ptrs *next[PTRS_PER_4K_PAGE];
} stress_ptrs_t;

static uint64_t s_ptr_chase_pages = DEFAULT_PTR_CHASE_PAGES;

static int stress_ptr_chase_opt_pages(const char *opt_name, const char *opt_arg)
{
    char *endptr;
    unsigned long long val = strtoull(opt_arg, &endptr, 10);

    if (*endptr == 'k' || *endptr == 'K') val *= 1024;
    else if (*endptr == 'm' || *endptr == 'M') val *= (1024 * 1024);

    if (val < MIN_PTR_CHASE_PAGES) val = MIN_PTR_CHASE_PAGES;
    if (val > MAX_PTR_CHASE_PAGES) val = MAX_PTR_CHASE_PAGES;

    s_ptr_chase_pages = (uint64_t)val;
    stress_osal_print("rtos_stress: debug: ptr-chase-pages set to %llu\n", (unsigned long long)s_ptr_chase_pages);
    return 0;
}

const stress_opt_t stress_ptr_chase_opts[] = {
    { "ptr-chase-pages", stress_ptr_chase_opt_pages },
    { NULL, NULL }
};

static uint32_t stress_mwc32(void) { return (uint32_t)stress_osal_rand(); }

static inline uint32_t stress_mwc32modn(uint32_t n) {
    return stress_mwc32() % n;
}

void stress_ptr_chase(stress_args_t *args)
{
    uint64_t num_pages = s_ptr_chase_pages;
    stress_ptrs_t **ptr_lookup_table = NULL;
    stress_ptrs_t *page_pool = NULL;
    stress_ptrs_t *current_ptr;
    uintptr_t ptr_mask = ~(uintptr_t)1;
    uint64_t i;

    size_t visited_count = 0;
    size_t total_count = 0;
    uint64_t last_log_ops = 0;

    while (num_pages >= MIN_PTR_CHASE_PAGES) {
        size_t pool_size = num_pages * sizeof(stress_ptrs_t);
        size_t table_size = num_pages * sizeof(stress_ptrs_t *);

        page_pool = (stress_ptrs_t *)stress_osal_malloc(pool_size);
        if (page_pool) {
            ptr_lookup_table = (stress_ptrs_t **)stress_osal_malloc(table_size);
            if (ptr_lookup_table) {
                break;
            }
            stress_osal_free(page_pool);
            page_pool = NULL;
        }
        num_pages /= 2;
    }

    if (!page_pool || !ptr_lookup_table) {
        stress_osal_print("rtos_stress: error: [ptr-chase] OOM, failed to allocate memory\n");
        return;
    }

    stress_osal_print("rtos_stress: info: [ptr-chase-%d] chasing %d pages (%d KB)\n",
               args->instance, (int)num_pages, (int)(num_pages * 4));

    for (i = 0; i < num_pages; i++) {
        ptr_lookup_table[i] = &page_pool[i];
    }

    for (i = 0; i < num_pages; i++) {
        stress_ptrs_t *p = ptr_lookup_table[i];
        size_t j;

        for (j = 0; j < PTRS_PER_4K_PAGE; j++) {
            size_t target_idx;
            do {
                target_idx = stress_mwc32modn((uint32_t)num_pages);
            } while (target_idx == i && num_pages > 1);

            p->next[j] = ptr_lookup_table[target_idx];
        }
    }

    current_ptr = ptr_lookup_table[0];
    uint32_t idx_mask = PTRS_PER_4K_PAGE - 1;

    while (stress_continue(args))
    {
        int k;
        for (k = 0; k < 1024; k++) {
            size_t j = stress_mwc32() & idx_mask;

            uintptr_t next_addr_raw = (uintptr_t)current_ptr->next[j];

            current_ptr->next[j] = (struct stress_ptrs *)(next_addr_raw | 1);

            current_ptr = (stress_ptrs_t *)(next_addr_raw & ptr_mask);
        }

        args->bogo.current_ops += 1024;

        if (args->bogo.current_ops - last_log_ops >= HEARTBEAT_INTERVAL) {
            stress_osal_print("rtos_stress: [ptr-chase-%d] heartbeat: %u ops\n",
                       args->instance, (unsigned int)args->bogo.current_ops);
            last_log_ops = args->bogo.current_ops;
        }

        stress_osal_thread_yield();
    }

    total_count = num_pages * PTRS_PER_4K_PAGE;

    for (i = 0; i < num_pages; i++) {
        stress_ptrs_t *p = ptr_lookup_table[i];
        size_t j;
        for (j = 0; j < PTRS_PER_4K_PAGE; j++) {
            if ((uintptr_t)p->next[j] & 1) {
                visited_count++;
            }
        }
    }

    if (total_count > 0) {
        float percentage = (float)visited_count * 100.0f / (float)total_count;
        int p_int = (int)percentage;
        int p_dec = (int)((percentage - p_int) * 100);

        stress_osal_print("rtos_stress: info: [ptr-chase-%d] finished. visited %u of %u pointers (%d.%02d%%)\n",
                   args->instance, (unsigned int)visited_count, (unsigned int)total_count,
                   p_int, p_dec);
    }

    stress_osal_free(ptr_lookup_table);
    stress_osal_free(page_pool);
}
