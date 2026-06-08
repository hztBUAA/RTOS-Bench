#ifndef RTOS_RUST_BENCH_H
#define RTOS_RUST_BENCH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*rtos_bench_thread_entry_t)(void *arg);

int32_t rtos_bench_run_all(void);

int32_t rtos_bench_thread_spawn(
    rtos_bench_thread_entry_t entry,
    void *arg,
    uint32_t priority,
    uint32_t stack_bytes,
    const uint8_t *name,
    size_t name_len
);
void rtos_bench_thread_exit(void);

int32_t rtos_bench_sem_create(uint32_t initial, uint32_t max, void **out_sem);
int32_t rtos_bench_sem_take(void *sem, uint64_t timeout_us);
int32_t rtos_bench_sem_give(void *sem);
void rtos_bench_sem_destroy(void *sem);

uint64_t rtos_bench_now_us(void);
void rtos_bench_write(const uint8_t *ptr, size_t len);
void *rtos_bench_alloc(size_t size);
void rtos_bench_free(void *ptr);
void rtos_bench_yield(void);
void rtos_bench_busy_hint(void);

#ifdef __cplusplus
}
#endif

#endif
