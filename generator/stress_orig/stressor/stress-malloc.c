/* applications/stress-ng/stress-malloc.c
 *
 * RTOS-safe rewrite:
 *  - zero realloc
 *  - fixed-bucket allocation
 *  - batch alloc/free phases
 *  - canary-based corruption detection
 */

#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <stress-config.h>

/* ------------------------------------------------------------------ */
/* 参数存储                                                             */
/* ------------------------------------------------------------------ */

static uint64_t s_malloc_bytes = DEFAULT_MALLOC_BYTES;
static uint32_t s_malloc_max   = DEFAULT_MALLOC_MAX;

static int stress_malloc_opt_bytes(const char *opt_name, const char *opt_arg)
{
    char               *endptr;
    unsigned long long  val = strtoull(opt_arg, &endptr, 10);

    if      (*endptr == 'k' || *endptr == 'K') val *= 1024ULL;
    else if (*endptr == 'm' || *endptr == 'M') val *= 1024ULL * 1024ULL;
    else if (*endptr == 'g' || *endptr == 'G') val *= 1024ULL * 1024ULL * 1024ULL;

    if (val < MIN_MALLOC_BYTES) val = MIN_MALLOC_BYTES;
    if (val > MAX_MALLOC_BYTES) val = MAX_MALLOC_BYTES;

    s_malloc_bytes = (uint64_t)val;
    stress_osal_print("rtos_stress: debug: malloc-bytes set to %llu\n",
                      (unsigned long long)s_malloc_bytes);
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

/* 外部接口：与原文件完全一致，其他文件无需修改 */
const stress_opt_t stress_malloc_opts[] = {
    { "malloc-bytes", stress_malloc_opt_bytes },
    { "malloc-max",   stress_malloc_opt_max   },
    { NULL, NULL }
};

/* ------------------------------------------------------------------ */
/* 内部实现                                                             */
/* ------------------------------------------------------------------ */

/* 固定 bucket 表：幂次对齐，对 ptmalloc 最友好 */
static const size_t BUCKETS[] = {
    32, 64, 128, 256, 512, 1024, 2048, 4096
};
#define BUCKET_COUNT  (sizeof(BUCKETS) / sizeof(BUCKETS[0]))

/* 每个槽的元数据 */
typedef struct {
    void   *ptr;
    size_t  size;
    uint8_t canary;   /* 写入 ptr[0] 的期望值 */
} slot_t;

/*
 * 从 bucket 表中选出不超过 max_bytes 的最大尺寸。
 * 循环选取确保整个 bucket 范围都被覆盖。
 */
static size_t pick_size(size_t max_bytes, uint32_t round)
{
    size_t limit = 0;
    size_t i;

    for (i = 0; i < BUCKET_COUNT; i++) {
        if (BUCKETS[i] <= max_bytes) {
            limit = i + 1;
        }
    }
    if (limit == 0) limit = 1;

    return BUCKETS[round % limit];
}

/* 写 canary：首字节 = canary，尾字节 = ~canary */
static void slot_touch(slot_t *s)
{
    volatile uint8_t *p = (volatile uint8_t *)s->ptr;
    p[0]          = s->canary;
    p[s->size - 1] = (uint8_t)(~s->canary);
}

/* 校验 canary，返回 0=OK，-1=损坏 */
static int slot_verify(const slot_t *s)
{
    const volatile uint8_t *p = (const volatile uint8_t *)s->ptr;

    if (p[0] != s->canary) return -1;
    if (p[s->size - 1] != (uint8_t)(~s->canary)) return -1;
    return 0;
}

/* ------------------------------------------------------------------ */
/* 主压力函数                                                           */
/* ------------------------------------------------------------------ */

void stress_malloc(stress_args_t *args)
{
    const size_t  max_slots  = (size_t)s_malloc_max;
    const size_t  max_bytes  = (size_t)s_malloc_bytes;
    slot_t       *slots      = NULL;
    size_t        live       = 0;   /* 当前存活块数 */
    uint32_t      round      = 0;   /* 轮次，用于 bucket 轮转 */

    stress_osal_print(
        "rtos_stress: info: [malloc-%d] max_slots=%zu max_bytes=%zu\n",
        args->instance, max_slots, max_bytes);

    if (max_slots == 0 || max_bytes < 32) {
        stress_osal_print("rtos_stress: error: [malloc] invalid config\n");
        return;
    }

    slots = (slot_t *)stress_osal_calloc(max_slots, sizeof(slot_t));
    if (!slots) {
        stress_osal_print("rtos_stress: error: [malloc] cannot alloc slot table\n");
        return;
    }

    /*
     * 工作模式：交替执行"填充阶段"和"释放阶段"
     *
     *  填充阶段：遍历所有空槽，逐一 malloc + touch
     *  释放阶段：遍历所有满槽，verify 后 free
     *
     * 这种批量模式比随机 interleave 对 RTOS ptmalloc 更友好，
     * 且完全可预测，不存在并发/重入风险。
     */

    while (stress_continue(args)) {

        /* ---- 填充阶段 ---- */
        size_t i;
        for (i = 0; i < max_slots && stress_continue(args); i++) {

            if (slots[i].ptr != NULL) continue;  /* 已有数据，跳过 */

            size_t sz = pick_size(max_bytes, round + (uint32_t)i);
            void  *p  = stress_osal_malloc(sz);

            if (p == NULL) {
                /* 堆压力过大，让出 CPU 后继续 */
                stress_osal_sleep_ms(2);
                continue;
            }

            slots[i].ptr    = p;
            slots[i].size   = sz;
            slots[i].canary = (uint8_t)((i ^ round) & 0xFF);
            if (slots[i].canary == 0) slots[i].canary = 0xAB; /* 避免全0 canary */

            slot_touch(&slots[i]);
            live++;
            args->bogo.current_ops++;
        }

        /* ---- 释放阶段 ---- */
        for (i = 0; i < max_slots && stress_continue(args); i++) {

            if (slots[i].ptr == NULL) continue;  /* 空槽，跳过 */

            if (slot_verify(&slots[i]) != 0) {
                /*
                 * ✅ canary 校验失败：隔离此块，不调用 free。
                 * 接受一次内存泄漏，防止把损坏传播到堆元数据。
                 */
                stress_osal_print(
                    "rtos_stress: fail: [malloc-%d] corruption "
                    "slot=%zu ptr=%p size=%zu canary=0x%02x, quarantine\n",
                    args->instance, i,
                    slots[i].ptr, slots[i].size, slots[i].canary);
                slots[i].ptr  = NULL;
                slots[i].size = 0;
                if (live > 0) live--;
                continue;
            }

            stress_osal_free(slots[i].ptr);
            slots[i].ptr    = NULL;
            slots[i].size   = 0;
            slots[i].canary = 0;
            if (live > 0) live--;
            args->bogo.current_ops++;
        }

        round++;

        /* 每轮结束后短暂退让，给其他任务和堆整理机会 */
        stress_osal_sleep_ms(1);
    }

    /* ---- 收尾清理 ---- */
    size_t k;
    for (k = 0; k < max_slots; k++) {
        if (slots[k].ptr == NULL) continue;

        if (slot_verify(&slots[k]) == 0) {
            stress_osal_free(slots[k].ptr);
        } else {
            stress_osal_print(
                "rtos_stress: warn: [malloc-%d] slot=%zu corrupted at cleanup, skip\n",
                args->instance, k);
        }
        slots[k].ptr  = NULL;
        slots[k].size = 0;
    }

    stress_osal_free(slots);
    stress_osal_print("rtos_stress: info: [malloc-%d] done\n", args->instance);
}
