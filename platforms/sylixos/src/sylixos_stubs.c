/**
 * @file sylixos_stubs.c
 * @brief Stub implementations for SylixOS
 * @details Some modules require RT-Thread specific APIs.
 *          This provides stubs until proper implementation is available.
 */

#ifdef SYLIXOS_PLATFORM

#include <stdio.h>
#include <stdint.h>

/* =========================================================================
 * Realtime benchmark stubs
 * ========================================================================= */

int realtime_benchmark_run_all(int run_multicore)
{
    printf("\n");
    printf("=============================================================\n");
    printf("[test-realtime] WARNING: Not available on SylixOS\n");
    printf("=============================================================\n");
    printf("\n");
    printf("The realtime_orig module requires RT-Thread specific APIs.\n");
    printf("This feature needs to be ported to SylixOS.\n");
    printf("\n");
    (void)run_multicore;
    return -1;
}

#endif /* SYLIXOS_PLATFORM */
