/* data_tools.h
 * 工具类
 */

#ifndef __DATA_TOOLS_H__
#define __DATA_TOOLS_H__

void count_durs_0TO1(uint64_t *durs, uint64_t *t0s, uint64_t *t1s, uint32_t size);
void count_durs_0TO1_2TO3(uint64_t *durs, uint64_t *t0s, uint64_t *t1s, uint64_t *t2s, uint64_t *t3s, uint32_t size);
uint64_t calc_avg(uint64_t *durs, uint32_t size, uint32_t cache_size);
void print_datas(uint64_t *t0s, uint64_t *t1s, uint64_t *t2s, uint64_t *t3s, uint32_t size);
uint64_t cycles_to_ns(uint64_t);

#endif /* __DATA_TOOLS.H__ */
