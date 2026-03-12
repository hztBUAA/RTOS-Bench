#include <stdio.h>
#include "bench_verify.h"

void realtime_verify_all(void) {
    printf("\n"
           "=============================================================\n"
           "[test-realtime] Verification\n"
           "=============================================================\n");
    freq_verify();
    printf("\n");
    time_verify();
    printf("\n");
    interrupt_stub_verify();
    printf("\n");
    schedule_stub_verify();
    printf("\n");
    cpu_bind_verify();
    printf("=============================================================\n");
}