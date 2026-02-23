#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>

#include "data_tools.h"
#include "les.h"

void count_durs_0TO1(uint64_t *durs, uint64_t *t0s, uint64_t *t1s, uint32_t size) {
	if (size <= 0) return;
	
	for (uint32_t i = 0; i < size; i = i + 1) {
		if (t0s[i] > t1s[i]) {
			durs[i] = 0;
		} else {
			// t1 - t0
			durs[i] = cycles_to_ns(t1s[i] - t0s[i]);
		}
	}
}

void count_durs_0TO1_2TO3(uint64_t *durs, uint64_t *t0s, uint64_t *t1s, uint64_t *t2s, uint64_t *t3s, uint32_t size) {
	if (size <= 0) return;
	
	for (uint32_t i = 0; i < size; i = i + 1) {
		if (t0s[i] > t1s[i] || t1s[i] > t2s[i] || t2s[i] > t3s[i]) {
			durs[i] = 0;
		} else {
			// (t3 - t2) + (t1 - t0)
			durs[i] = cycles_to_ns(t3s[i] - t2s[i] + t1s[i] - t0s[i]);
		}
	}
}

/* 计算均值 */
uint64_t calc_avg(uint64_t *durs, uint32_t size, uint32_t cache_size) {
	if (size <= cache_size) return 0;
	
	uint64_t total = 0;
	uint32_t valid_count = 0;

	for (uint32_t i = cache_size; i < size; i = i + 1) {
		if (durs[i] > 0) {
			total = total + durs[i];
			valid_count = valid_count + 1;
		}
    }
    
    return total / (size - cache_size);
}

/* 查看数据 */
void print_datas(uint64_t *t0s, uint64_t *t1s, uint64_t *t2s, uint64_t *t3s, uint32_t size) {
    printf("Raw Data (t0-t3):\n");
    for (uint32_t i = 0; i < size; i = i + 1) {
        printf("%" PRIu64 " ", t0s[i]);
        printf("%" PRIu64 " ", t1s[i]);
        if (t2s != NULL) printf("%" PRIu64 " ", t2s[i]);
        if (t3s != NULL) printf("%" PRIu64 " ", t3s[i]);
        printf("\n");
    }
}

/* 转换为ns */
uint64_t cycles_to_ns(uint64_t cycles) {
	uint64_t freq = freqGet();
	uint64_t sec = cycles / freq;
	uint64_t nanosec = ((cycles % freq) * 1e9) / freq;
	return (sec * 1000000000ULL) + nanosec;
}

