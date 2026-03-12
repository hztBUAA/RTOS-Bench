#include <stdio.h>
#include "les.h"

#include "bench_verify.h"

#define ITERATION 4

void time_verify(void) {
    printf("[timeGet() verification]\n");
    for (int i = 0; i < ITERATION; i++) {
        printf("time_%d = %llu cycles\n", i+1, (unsigned long long)timeGet());
    }
}