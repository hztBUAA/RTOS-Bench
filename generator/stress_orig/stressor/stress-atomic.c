/* applications/stress-ng/stress-atomic.c */
#include "stress_osal.h"
#include "stress-ng.h"
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <config.h>

/*
 * 原子操作宽度能力检测。
 *
 * 使用 GCC 预定义宏 __GCC_ATOMIC_*_LOCK_FREE：
 *   - 这是 __atomic_always_lock_free() 的预处理器等价物
 *   - 由 GCC 根据 -march 参数在预处理阶段自动计算
 *   - 可以直接用于 #if 表达式（不同于 __atomic_always_lock_free()
 *     只能在运行期/编译期函数中使用）
 *   - 任何基于 GCC 的工具链均支持（riscv-sylixos-elf-gcc 等）
 *
 * 值含义：
 *   0 = 该宽度的原子操作从不无锁（需要 libatomic 或 mutex 模拟）
 *   1 = 有时无锁（取决于对齐和具体实现，不可靠）
 *   2 = 始终无锁（GCC 可直接内联 AMO/LOCK/LDXR 等指令）
 *
 * 仅当值为 2 时才启用对应宽度的测试，值为 0 或 1 时跳过，
 * 避免产生对 __atomic_fetch_add_1 / __atomic_fetch_xor_2 等
 * libatomic 符号的外部调用，解决 BSP 未部署 libatomic 时的链接失败。
 *
 * 实测（哪吒派 XuanTie C906, -march=rv64imafdc）：
 *   __GCC_ATOMIC_CHAR_LOCK_FREE  = 1 → HAVE_8BIT  不定义
 *   __GCC_ATOMIC_SHORT_LOCK_FREE = 1 → HAVE_16BIT 不定义
 *   __GCC_ATOMIC_INT_LOCK_FREE   = 2 → HAVE_32BIT 定义
 *   __GCC_ATOMIC_LLONG_LOCK_FREE = 2 → HAVE_64BIT 定义
 *
 * 实测（飞腾派 E2000Q AArch64）：
 *   全部为 2 → 四个宏均定义
 *
 * 实测（标准 PC x86-64）：
 *   全部为 2 → 四个宏均定义
 */

#if defined(__GCC_ATOMIC_CHAR_LOCK_FREE) && (__GCC_ATOMIC_CHAR_LOCK_FREE == 2)
    #define STRESS_ATOMIC_HAVE_8BIT
#endif

#if defined(__GCC_ATOMIC_SHORT_LOCK_FREE) && (__GCC_ATOMIC_SHORT_LOCK_FREE == 2)
    #define STRESS_ATOMIC_HAVE_16BIT
#endif

#if defined(__GCC_ATOMIC_INT_LOCK_FREE) && (__GCC_ATOMIC_INT_LOCK_FREE == 2)
    #define STRESS_ATOMIC_HAVE_32BIT
#endif

#if defined(__GCC_ATOMIC_LLONG_LOCK_FREE) && (__GCC_ATOMIC_LLONG_LOCK_FREE == 2)
    #define STRESS_ATOMIC_HAVE_64BIT
#endif
#if !defined(STRESS_ATOMIC_HAVE_32BIT)
    #warning "stress-atomic: no always-lock-free 32-bit atomics on this target, stressor will be a no-op"
#endif

/* 数组大小必须为 2 的幂，用于位掩码取模 */
#define ATOMIC_ARRAY_SIZE   (64 * 4)

#define UNLIKELY(x)         __builtin_expect(!!(x), 0)
#define LIKELY(x)           __builtin_expect(!!(x), 1)

/* ------------------------------------------------------------------ */
/* 原子操作宏封装                                                       */
/* ------------------------------------------------------------------ */

#define SHIM_ATOMIC_ADD_FETCH(p, v, mo)  __atomic_add_fetch(p, v, mo)
#define SHIM_ATOMIC_SUB_FETCH(p, v, mo)  __atomic_sub_fetch(p, v, mo)
#define SHIM_ATOMIC_AND_FETCH(p, v, mo)  __atomic_and_fetch(p, v, mo)
#define SHIM_ATOMIC_XOR_FETCH(p, v, mo)  __atomic_xor_fetch(p, v, mo)
#define SHIM_ATOMIC_OR_FETCH(p, v, mo)   __atomic_or_fetch(p, v, mo)
#define SHIM_ATOMIC_NAND_FETCH(p, v, mo) __atomic_nand_fetch(p, v, mo)

#define SHIM_ATOMIC_FETCH_ADD(p, v, mo)  __atomic_fetch_add(p, v, mo)
#define SHIM_ATOMIC_FETCH_SUB(p, v, mo)  __atomic_fetch_sub(p, v, mo)
#define SHIM_ATOMIC_FETCH_AND(p, v, mo)  __atomic_fetch_and(p, v, mo)
#define SHIM_ATOMIC_FETCH_XOR(p, v, mo)  __atomic_fetch_xor(p, v, mo)
#define SHIM_ATOMIC_FETCH_OR(p, v, mo)   __atomic_fetch_or(p, v, mo)
#define SHIM_ATOMIC_FETCH_NAND(p, v, mo) __atomic_fetch_nand(p, v, mo)

#define SHIM_ATOMIC_LOAD(p, v, mo)       __atomic_load(p, v, mo)
#define SHIM_ATOMIC_STORE(p, v, mo)      __atomic_store(p, v, mo)

#define SHIM_ATOMIC_CLEAR(ptr, mo)                          \
    do {                                                    \
        __typeof__(*(ptr)) _z = (__typeof__(*(ptr)))0;      \
        __atomic_store(ptr, &_z, mo);                       \
    } while (0)

/* ------------------------------------------------------------------ */
/* 共享数据结构：仅包含当前平台可以无 libatomic 内联的宽度             */
/* ------------------------------------------------------------------ */

typedef struct {
#if defined(STRESS_ATOMIC_HAVE_8BIT)
    uint8_t  val8[ATOMIC_ARRAY_SIZE];
#endif
#if defined(STRESS_ATOMIC_HAVE_16BIT)
    uint16_t val16[ATOMIC_ARRAY_SIZE];
#endif
#if defined(STRESS_ATOMIC_HAVE_32BIT)
    uint32_t val32[ATOMIC_ARRAY_SIZE];
#endif
#if defined(STRESS_ATOMIC_HAVE_64BIT)
    uint64_t val64[ATOMIC_ARRAY_SIZE];
#endif
} atomic_shared_data_t;

typedef struct {
    stress_args_t        *args;
    atomic_shared_data_t *data;
    int                   thread_id;
    stress_sem_t          sem_done;
} atomic_thread_ctx_t;

static int32_t s_atomic_threads = DEFAULT_ATOMIC_THREADS;

static int stress_atomic_opt_threads(const char *opt_name, const char *opt_arg)
{
    int val = atoi(opt_arg);
    (void)opt_name;
    if (val < MIN_ATOMIC_THREADS) val = MIN_ATOMIC_THREADS;
    if (val > MAX_ATOMIC_THREADS) val = MAX_ATOMIC_THREADS;
    s_atomic_threads = val - 1;
    stress_osal_print("rtos_stress: debug: atomic-threads set to %d\n",
                      s_atomic_threads);
    return 0;
}

const stress_opt_t stress_atomic_opts[] = {
    { "atomic-threads", stress_atomic_opt_threads },
    { NULL, NULL }
};

static uint64_t stress_mwc64(void)
{
    return ((uint64_t)stress_osal_rand() << 32) | (uint64_t)stress_osal_rand();
}

/* ------------------------------------------------------------------ */
/* 核心压测宏                                                           */
/* ------------------------------------------------------------------ */


#define DO_ATOMIC_OPS(type, var, rc)                                \
do {                                                                \
    type tmp    = (type)stress_mwc64();                             \
    type unshared;                                                  \
    type check1 = tmp;                                              \
    type check2 = (type)(~tmp);                                     \
                                                                    \
    SHIM_ATOMIC_STORE(&unshared, &check1, __ATOMIC_RELAXED);        \
    SHIM_ATOMIC_ADD_FETCH(&unshared, (type)2, __ATOMIC_RELAXED);    \
    SHIM_ATOMIC_SUB_FETCH(&unshared, (type)1, __ATOMIC_RELAXED);    \
    SHIM_ATOMIC_LOAD(&unshared, &check2, __ATOMIC_RELAXED);         \
                                                                    \
    SHIM_ATOMIC_STORE(var, &tmp, __ATOMIC_RELAXED);                 \
    SHIM_ATOMIC_LOAD(var, &tmp, __ATOMIC_RELAXED);                  \
    SHIM_ATOMIC_LOAD(var, &tmp, __ATOMIC_ACQUIRE);                  \
    SHIM_ATOMIC_ADD_FETCH(var, (type)1,  __ATOMIC_RELAXED);         \
    SHIM_ATOMIC_ADD_FETCH(var, (type)2,  __ATOMIC_ACQUIRE);         \
    SHIM_ATOMIC_SUB_FETCH(var, (type)3,  __ATOMIC_RELAXED);         \
    SHIM_ATOMIC_SUB_FETCH(var, (type)4,  __ATOMIC_ACQUIRE);         \
    SHIM_ATOMIC_AND_FETCH(var, (type)~1, __ATOMIC_RELAXED);         \
    SHIM_ATOMIC_AND_FETCH(var, (type)~2, __ATOMIC_ACQUIRE);         \
    SHIM_ATOMIC_XOR_FETCH(var, (type)~4, __ATOMIC_RELAXED);         \
    SHIM_ATOMIC_XOR_FETCH(var, (type)~8, __ATOMIC_ACQUIRE);         \
    SHIM_ATOMIC_OR_FETCH(var,  (type)16, __ATOMIC_RELAXED);         \
    SHIM_ATOMIC_OR_FETCH(var,  (type)32, __ATOMIC_ACQUIRE);         \
    SHIM_ATOMIC_NAND_FETCH(var, (type)64,  __ATOMIC_RELAXED);       \
    SHIM_ATOMIC_NAND_FETCH(var, (type)128, __ATOMIC_ACQUIRE);       \
    SHIM_ATOMIC_CLEAR(var, __ATOMIC_RELAXED);                       \
                                                                    \
    SHIM_ATOMIC_STORE(var, &tmp, __ATOMIC_RELAXED);                 \
    SHIM_ATOMIC_FETCH_ADD(var,  (type)1,  __ATOMIC_RELAXED);        \
    SHIM_ATOMIC_FETCH_SUB(var,  (type)3,  __ATOMIC_RELAXED);        \
    SHIM_ATOMIC_FETCH_AND(var,  (type)~1, __ATOMIC_RELAXED);        \
    SHIM_ATOMIC_FETCH_XOR(var,  (type)~4, __ATOMIC_RELAXED);        \
    SHIM_ATOMIC_FETCH_OR(var,   (type)16, __ATOMIC_RELAXED);        \
    SHIM_ATOMIC_FETCH_NAND(var, (type)64, __ATOMIC_RELAXED);        \
    SHIM_ATOMIC_CLEAR(var, __ATOMIC_RELAXED);                       \
                                                                    \
    (void)tmp;                                                      \
    check2--;                                                       \
    if (UNLIKELY(check2 != check1)) {                               \
        stress_osal_print(                                          \
            "rtos_stress: fail: [atomic] store/load mismatch"       \
            " check2=0x%lx check1=0x%lx\n",                        \
            (unsigned long)check2, (unsigned long)check1);          \
        rc = -1;                                                    \
    }                                                               \
} while (0)

/* ------------------------------------------------------------------ */
/* 各宽度压测函数（无对应能力时整体不编译，不产生任何符号引用）         */
/* ------------------------------------------------------------------ */

#if defined(STRESS_ATOMIC_HAVE_8BIT)
static int stress_atomic_uint8(atomic_shared_data_t *data)
{
    static volatile int idx = 0;
    int rc = 0;
    int i  = __atomic_fetch_add(&idx, 1, __ATOMIC_RELAXED)
             & (ATOMIC_ARRAY_SIZE - 1);
    DO_ATOMIC_OPS(uint8_t, &data->val8[i], rc);
    return rc;
}
#endif

#if defined(STRESS_ATOMIC_HAVE_16BIT)
static int stress_atomic_uint16(atomic_shared_data_t *data)
{
    static volatile int idx = 0;
    int rc = 0;
    int i  = __atomic_fetch_add(&idx, 1, __ATOMIC_RELAXED)
             & (ATOMIC_ARRAY_SIZE - 1);
    DO_ATOMIC_OPS(uint16_t, &data->val16[i], rc);
    return rc;
}
#endif

#if defined(STRESS_ATOMIC_HAVE_32BIT)
static int stress_atomic_uint32(atomic_shared_data_t *data)
{
    static volatile int idx = 0;
    int rc = 0;
    int i  = __atomic_fetch_add(&idx, 1, __ATOMIC_RELAXED)
             & (ATOMIC_ARRAY_SIZE - 1);
    DO_ATOMIC_OPS(uint32_t, &data->val32[i], rc);
    return rc;
}
#endif

#if defined(STRESS_ATOMIC_HAVE_64BIT)
static int stress_atomic_uint64(atomic_shared_data_t *data)
{
    static volatile int idx = 0;
    int rc = 0;
    int i  = __atomic_fetch_add(&idx, 1, __ATOMIC_RELAXED)
             & (ATOMIC_ARRAY_SIZE - 1);
    DO_ATOMIC_OPS(uint64_t, &data->val64[i], rc);
    return rc;
}
#endif

/* ------------------------------------------------------------------ */
/* ops/iter 计数宏：编译期计算实际执行的操作数                          */
/* ------------------------------------------------------------------ */

#if defined(STRESS_ATOMIC_HAVE_8BIT)
    #define _OPS_8  1
#else
    #define _OPS_8  0
#endif
#if defined(STRESS_ATOMIC_HAVE_16BIT)
    #define _OPS_16 1
#else
    #define _OPS_16 0
#endif
#if defined(STRESS_ATOMIC_HAVE_32BIT)
    #define _OPS_32 1
#else
    #define _OPS_32 0
#endif
#if defined(STRESS_ATOMIC_HAVE_64BIT)
    #define _OPS_64 1
#else
    #define _OPS_64 0
#endif

#define ATOMIC_OPS_PER_ITER  (_OPS_8 + _OPS_16 + _OPS_32 + _OPS_64)

/* ------------------------------------------------------------------ */
/* 所有可用宽度集中执行                                                 */
/* ------------------------------------------------------------------ */

static void stress_do_all_atomics(atomic_shared_data_t *data)
{
#if defined(STRESS_ATOMIC_HAVE_8BIT)
    stress_atomic_uint8(data);
#endif
#if defined(STRESS_ATOMIC_HAVE_16BIT)
    stress_atomic_uint16(data);
#endif
#if defined(STRESS_ATOMIC_HAVE_32BIT)
    stress_atomic_uint32(data);
#endif
#if defined(STRESS_ATOMIC_HAVE_64BIT)
    stress_atomic_uint64(data);
#endif
}

/* ------------------------------------------------------------------ */
/* worker 线程                                                         */
/* ------------------------------------------------------------------ */

static void stress_atomic_worker(void *parameter)
{
    atomic_thread_ctx_t *ctx = (atomic_thread_ctx_t *)parameter;

    while (stress_continue(ctx->args)) {
        stress_do_all_atomics(ctx->data);
        stress_osal_thread_yield();
    }

    stress_osal_sem_release(ctx->sem_done);
}

/* ------------------------------------------------------------------ */
/* 主体                                                                */
/* ------------------------------------------------------------------ */

void stress_atomic(stress_args_t *args)
{
    atomic_shared_data_t *shared_data = NULL;
    atomic_thread_ctx_t  *thread_ctxs = NULL;
    int                   n_threads   = s_atomic_threads;
    int                   i;

#if !defined(STRESS_ATOMIC_HAVE_32BIT)
    stress_osal_print("rtos_stress: warn: [atomic-%d] no always-lock-free"
                      " 32-bit atomics, skipping\n", args->instance);
    return;
#endif

    stress_osal_print(
        "rtos_stress: info: [atomic-%d] lock-free:"
        " 8bit=%s 16bit=%s 32bit=%s 64bit=%s ops/iter=%d\n",
        args->instance,
#if defined(STRESS_ATOMIC_HAVE_8BIT)
        "yes",
#else
        "no",
#endif
#if defined(STRESS_ATOMIC_HAVE_16BIT)
        "yes",
#else
        "no",
#endif
#if defined(STRESS_ATOMIC_HAVE_32BIT)
        "yes",
#else
        "no",
#endif
#if defined(STRESS_ATOMIC_HAVE_64BIT)
        "yes",
#else
        "no",
#endif
        ATOMIC_OPS_PER_ITER
    );

    shared_data = (atomic_shared_data_t *)stress_osal_malloc(
                      sizeof(atomic_shared_data_t));
    if (!shared_data) {
        stress_osal_print("rtos_stress: error: [atomic-%d] OOM\n",
                          args->instance);
        return;
    }
    stress_osal_memset(shared_data, 0, sizeof(atomic_shared_data_t));

    if (n_threads > 0) {
        thread_ctxs = (atomic_thread_ctx_t *)stress_osal_malloc(
                          n_threads * sizeof(atomic_thread_ctx_t));
        if (!thread_ctxs) {
            stress_osal_print("rtos_stress: error: [atomic-%d] OOM"
                              " thread_ctxs\n", args->instance);
            stress_osal_free(shared_data);
            return;
        }
        stress_osal_memset(thread_ctxs, 0,
                           n_threads * sizeof(atomic_thread_ctx_t));

        for (i = 0; i < n_threads; i++) {
            char         sem_name[16];
            stress_tid_t tid;

            stress_osal_snprintf(sem_name, sizeof(sem_name),
                                 "at_%d_%d", args->instance, i);

            thread_ctxs[i].args      = args;
            thread_ctxs[i].data      = shared_data;
            thread_ctxs[i].thread_id = i;
            thread_ctxs[i].sem_done  = stress_osal_sem_create(sem_name, 0);

            if (!thread_ctxs[i].sem_done) {
                stress_osal_print("rtos_stress: warn: [atomic-%d]"
                                  " sem create failed for worker %d\n",
                                  args->instance, i);
                continue;
            }

            tid = stress_osal_thread_spawn("at_wrk",
                                           stress_atomic_worker,
                                           &thread_ctxs[i],
                                           8192,
                                           20);
            if (!tid) {
                stress_osal_print("rtos_stress: warn: [atomic-%d]"
                                  " spawn worker %d failed\n",
                                  args->instance, i);
                stress_osal_sem_delete(thread_ctxs[i].sem_done);
                thread_ctxs[i].sem_done = NULL;
            }
        }
    }

    while (stress_continue(args)) {
        stress_do_all_atomics(shared_data);
        args->bogo.current_ops += ATOMIC_OPS_PER_ITER;
    }

    if (n_threads > 0 && thread_ctxs) {
        stress_tick_t timeout = (stress_tick_t)(
            (uint64_t)3000 * stress_osal_tick_hz() / 1000ULL);

        for (i = 0; i < n_threads; i++) {
            if (!thread_ctxs[i].sem_done) continue;
            if (stress_osal_sem_take(thread_ctxs[i].sem_done,
                                     timeout) != 0) {
                stress_osal_print("rtos_stress: warn: [atomic-%d]"
                                  " worker %d exit timeout\n",
                                  args->instance, i);
            }
            stress_osal_sem_delete(thread_ctxs[i].sem_done);
        }
        stress_osal_free(thread_ctxs);
    }

    stress_osal_free(shared_data);
}
