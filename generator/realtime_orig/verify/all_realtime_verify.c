#include "bench_verify.h"

void realtime_verify_all(void) {
    freq_verify();
    //interrupt_stub_verify();
    //schedule_stub_verify();
}