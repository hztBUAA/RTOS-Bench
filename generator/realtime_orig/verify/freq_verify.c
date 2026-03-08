#include <stdio.h>
#include "les.h"

#include "bench_verify.h"

void freq_verify(void) {
    printf("[freqGet() verification]\n"
           "freq = %llu\n", (unsigned long long)freqGet());
}