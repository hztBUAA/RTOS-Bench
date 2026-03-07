#include <inttypes.h>
#include <limits.h>

#include "data_tools.h"
#include "les.h"

/* 转换为ns */
uint64_t cycles_to_ns(uint64_t cycles) {
	uint64_t freq = freqGet();
	uint64_t sec = cycles / freq;
	uint64_t nanosec = ((cycles % freq) * 1e9) / freq;
	return (sec * 1000000000ULL) + nanosec;
}

