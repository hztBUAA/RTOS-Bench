/* applications/stress-ng/stress-malloc.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <config.h>

#define UNLIKELY(x)         __builtin_expect(!!(x), 0)
#define LIKELY(x)           __builtin_expect(!!(x), 1)

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif
#ifndef EXIT_NO_RESOURCE
#define EXIT_NO_RESOURCE 2
#endif

typedef struct {
    void *addr;
    size_t len;
} stress_malloc_info_t;

static uint64_t s_malloc_bytes = DEFAULT_MALLOC_BYTES;
static uint32_t s_malloc_max = DEFAULT_MALLOC_MAX;

static int stress_malloc_opt_bytes(const char *opt_name, const char *opt_arg)
{
    char *endptr;
    unsigned long long val = strtoull(opt_arg, &endptr, 10);

    if (*endptr == 'k' || *endptr == 'K') val *= 1024;
    else if (*endptr == 'm' || *endptr == 'M') val *= (1024 * 1024);
    else if (*endptr == 'g' || *endptr == 'G') val *= (1024 * 1024 * 1024);

    if (val < MIN_MALLOC_BYTES) val = MIN_MALLOC_BYTES;
    if (val > MAX_MALLOC_BYTES) val = MAX_MALLOC_BYTES;

    s_malloc_bytes = (uint64_t)val;
    stress_osal_print("rtos_stress: debug: malloc-bytes set to %llu\n", (unsigned long long)s_malloc_bytes);
    return 0;
}

static int stress_malloc_opt_max(const char *opt_name, const char *opt_arg)
{
    int val = atoi(opt_arg);

    if (val < MIN_MALLOC_MAX) val = MIN_MALLOC_MAX;
    if (val > MAX_MALLOC_MAX) val = MAX_MALLOC_MAX;

    s_malloc_max = (uint32_t)val;
    stress_osal_print("rtos_stress: debug: malloc-max set to %u slots\n", s_malloc_max);
    return 0;
}

const stress_opt_t stress_malloc_opts[] = {
    { "malloc-bytes", stress_malloc_opt_bytes },
    { "malloc-max",   stress_malloc_opt_max },
    { NULL, NULL }
};

static uint32_t stress_mwc32(void) { return (uint32_t)stress_osal_rand(); }

static size_t stress_alloc_size(const size_t max_size)
{
    size_t len = stress_osal_rand() % max_size;
    if (len < sizeof(uintptr_t)) len = sizeof(uintptr_t);
    return len;
}

static void stress_malloc_touch(void *ptr, size_t size)
{
    if (ptr && size > 0) {
        volatile uint8_t *u8ptr = (volatile uint8_t *)ptr;
        u8ptr[0] = 0xA5;
        u8ptr[size - 1] = 0x5A;
        u8ptr[size / 2] = 0xFF;
    }
}

static int stress_malloc_verify(stress_args_t *args, void *ptr, size_t size)
{
    if (ptr && size > 0) {
        volatile uint8_t *u8ptr = (volatile uint8_t *)ptr;
        if (u8ptr[0] != 0xA5 || u8ptr[size - 1] != 0x5A) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

void stress_malloc(stress_args_t *args)
{
    stress_malloc_info_t *info = NULL;
    size_t malloc_max = (size_t)s_malloc_max;
    size_t malloc_bytes = (size_t)s_malloc_bytes;

    stress_osal_print("rtos_stress: info: [malloc-%d] max_allocs=%d, max_bytes_per_alloc=%d\n",
               args->instance, (int)malloc_max, (int)malloc_bytes);

    size_t info_size = malloc_max * sizeof(stress_malloc_info_t);
    info = (stress_malloc_info_t *)stress_osal_calloc(1, info_size);
    if (!info) {
        stress_osal_print("rtos_stress: error: [malloc] failed to allocate info array (%d bytes)\n", info_size);
        return;
    }

    while (stress_continue(args))
    {
        uint32_t rnd = stress_mwc32();
        uint32_t i = rnd % malloc_max;
        uint32_t action = (rnd >> 12) & 1;
        uint32_t method = (rnd >> 13) & 0x03;

        if (info[i].addr) {
            if (action) {
                if (stress_malloc_verify(args, info[i].addr, info[i].len) != EXIT_SUCCESS) {
                     stress_osal_print("rtos_stress: fail: [malloc] corruption at %p\n", info[i].addr);
                }

                stress_osal_free(info[i].addr);
                info[i].addr = NULL;
                info[i].len = 0;
            } else {
                size_t new_len = stress_alloc_size(malloc_bytes);
                void *tmp = stress_osal_realloc(info[i].addr, new_len);
                if (tmp) {
                    info[i].addr = tmp;
                    info[i].len = new_len;
                    stress_malloc_touch(info[i].addr, info[i].len);
                }
            }
        } else {
            if (action) {
                size_t len = stress_alloc_size(malloc_bytes);
                void *ptr = NULL;

                switch (method) {
                case 0:
                {
                    size_t n = (len % 16) + 1;
                    size_t sz = len / n;
                    if (sz == 0) sz = 1;
                    ptr = stress_osal_calloc(n, sz);
                    len = n * sz;
                    break;
                }
                default:
                    ptr = stress_osal_malloc(len);
                    break;
                }

                if (ptr) {
                    info[i].addr = ptr;
                    info[i].len = len;
                    stress_malloc_touch(ptr, len);
                }
            }
        }

        args->bogo.current_ops++;

        if ((args->bogo.current_ops % 1024) == 0) {
            stress_osal_sleep_ms(1);
        }
    }

    for (size_t k = 0; k < malloc_max; k++) {
        if (info[k].addr) {
            stress_osal_free(info[k].addr);
            info[k].addr = NULL;
        }
    }

    stress_osal_free(info);
}
