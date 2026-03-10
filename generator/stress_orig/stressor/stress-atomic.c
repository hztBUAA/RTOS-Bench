/* applications/stress-ng/stress-atomic.c */
#include "stress_osal.h"
#include "stress-ng.h"
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <config.h>

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 8
    #define STRESS_ATOMIC_64BIT
#endif

#define ATOMIC_ARRAY_SIZE       (64 * 4)

#define UNLIKELY(x)         __builtin_expect(!!(x), 0)
#define LIKELY(x)           __builtin_expect(!!(x), 1)

#define DO_NOTHING()    do { } while (0)

#define SHIM_ATOMIC_ADD_FETCH(ptr, val, memorder)   __atomic_add_fetch(ptr, val, memorder)
#define SHIM_ATOMIC_SUB_FETCH(ptr, val, memorder)   __atomic_sub_fetch(ptr, val, memorder)
#define SHIM_ATOMIC_AND_FETCH(ptr, val, memorder)   __atomic_and_fetch(ptr, val, memorder)
#define SHIM_ATOMIC_XOR_FETCH(ptr, val, memorder)   __atomic_xor_fetch(ptr, val, memorder)
#define SHIM_ATOMIC_OR_FETCH(ptr, val, memorder)    __atomic_or_fetch(ptr, val, memorder)
#define SHIM_ATOMIC_NAND_FETCH(ptr, val, memorder)  __atomic_nand_fetch(ptr, val, memorder)

#define SHIM_ATOMIC_FETCH_ADD(ptr, val, memorder)   __atomic_fetch_add(ptr, val, memorder)
#define SHIM_ATOMIC_FETCH_SUB(ptr, val, memorder)   __atomic_fetch_sub(ptr, val, memorder)
#define SHIM_ATOMIC_FETCH_AND(ptr, val, memorder)   __atomic_fetch_and(ptr, val, memorder)
#define SHIM_ATOMIC_FETCH_XOR(ptr, val, memorder)   __atomic_fetch_xor(ptr, val, memorder)
#define SHIM_ATOMIC_FETCH_OR(ptr, val, memorder)    __atomic_fetch_or(ptr, val, memorder)
#define SHIM_ATOMIC_FETCH_NAND(ptr, val, memorder)  __atomic_fetch_nand(ptr, val, memorder)

#define SHIM_ATOMIC_LOAD(ptr, val, memorder)        __atomic_load(ptr, val, memorder)
#define SHIM_ATOMIC_STORE(ptr, val, memorder)       __atomic_store(ptr, val, memorder)
#define SHIM_ATOMIC_CLEAR(ptr, memorder)            __atomic_clear(ptr, memorder)

typedef struct {
    uint8_t  val8[ATOMIC_ARRAY_SIZE];
    uint16_t val16[ATOMIC_ARRAY_SIZE];
    uint32_t val32[ATOMIC_ARRAY_SIZE];
#if defined(STRESS_ATOMIC_64BIT)
    uint64_t val64[ATOMIC_ARRAY_SIZE];
#endif
} atomic_shared_data_t;

typedef struct {
    stress_args_t *args;
    atomic_shared_data_t *data;
    int thread_id;
    stress_sem_t sem_done;
} atomic_thread_ctx_t;

static int32_t s_atomic_threads = DEFAULT_ATOMIC_THREADS;

static int stress_atomic_opt_threads(const char *opt_name, const char *opt_arg)
{
    int val = atoi(opt_arg);
    if (val < MIN_ATOMIC_THREADS) val = MIN_ATOMIC_THREADS;
    if (val > MAX_ATOMIC_THREADS) val = MAX_ATOMIC_THREADS;

    s_atomic_threads = val - 1;
    stress_osal_print("rtos_stress: debug: atomic-threads set to %d\n", s_atomic_threads);
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

#define DO_ATOMIC_OPS(args, type, var, rc)                      \
do {                                                            \
    type tmp = (type)stress_mwc64();                            \
    type unshared, check1 = tmp, check2 = ~tmp;                 \
    SHIM_ATOMIC_STORE(&unshared, &check1, __ATOMIC_RELAXED);    \
    SHIM_ATOMIC_ADD_FETCH(&unshared, (type)2, __ATOMIC_RELAXED);\
    SHIM_ATOMIC_SUB_FETCH(&unshared, (type)1, __ATOMIC_RELAXED);\
    SHIM_ATOMIC_LOAD(&unshared, &check2, __ATOMIC_RELAXED);     \
                                                                \
    SHIM_ATOMIC_STORE(var, &tmp, __ATOMIC_RELAXED);             \
    SHIM_ATOMIC_LOAD(var, &tmp, __ATOMIC_RELAXED);              \
    SHIM_ATOMIC_LOAD(var, &tmp, __ATOMIC_ACQUIRE);              \
    SHIM_ATOMIC_ADD_FETCH(var, (type)1, __ATOMIC_RELAXED);      \
    SHIM_ATOMIC_ADD_FETCH(var, (type)2, __ATOMIC_ACQUIRE);      \
    SHIM_ATOMIC_SUB_FETCH(var, (type)3, __ATOMIC_RELAXED);      \
    SHIM_ATOMIC_SUB_FETCH(var, (type)4, __ATOMIC_ACQUIRE);      \
    SHIM_ATOMIC_AND_FETCH(var, (type)~1, __ATOMIC_RELAXED);     \
    SHIM_ATOMIC_AND_FETCH(var, (type)~2, __ATOMIC_ACQUIRE);     \
    SHIM_ATOMIC_XOR_FETCH(var, (type)~4, __ATOMIC_RELAXED);     \
    SHIM_ATOMIC_XOR_FETCH(var, (type)~8, __ATOMIC_ACQUIRE);     \
    SHIM_ATOMIC_OR_FETCH(var, (type)16, __ATOMIC_RELAXED);      \
    SHIM_ATOMIC_OR_FETCH(var, (type)32, __ATOMIC_ACQUIRE);      \
    SHIM_ATOMIC_NAND_FETCH(var, (type)64, __ATOMIC_RELAXED);    \
    SHIM_ATOMIC_NAND_FETCH(var, (type)128, __ATOMIC_ACQUIRE);   \
    SHIM_ATOMIC_CLEAR(var, __ATOMIC_RELAXED);                   \
                                                                \
    SHIM_ATOMIC_STORE(var, &tmp, __ATOMIC_RELAXED);             \
    SHIM_ATOMIC_FETCH_ADD(var, (type)1, __ATOMIC_RELAXED);      \
    SHIM_ATOMIC_FETCH_SUB(var, (type)3, __ATOMIC_RELAXED);      \
    SHIM_ATOMIC_FETCH_AND(var, (type)~1, __ATOMIC_RELAXED);     \
    SHIM_ATOMIC_FETCH_XOR(var, (type)~4, __ATOMIC_RELAXED);     \
    SHIM_ATOMIC_FETCH_OR(var, (type)16, __ATOMIC_RELAXED);      \
    SHIM_ATOMIC_FETCH_NAND(var, (type)64, __ATOMIC_RELAXED);    \
    SHIM_ATOMIC_CLEAR(var, __ATOMIC_RELAXED);                   \
                                                                \
    (void)tmp;                                                  \
    check2--;                                                   \
    if (UNLIKELY(check2 != check1)) {                           \
        stress_osal_print("rtos_stress: fail: atomic store/load mismatch check2=%lx check1=%lx\n", \
            (unsigned long)check2, (unsigned long)check1);      \
        rc = -1;                                                \
    }                                                           \
} while (0)

static int stress_atomic_uint8(atomic_shared_data_t *data)
{
    static volatile int idx = 0;
    int rc = 0;
    int i = __atomic_fetch_add(&idx, 1, __ATOMIC_RELAXED) & (ATOMIC_ARRAY_SIZE - 1);
    DO_ATOMIC_OPS(NULL, uint8_t, &data->val8[i], rc);
    return rc;
}

static int stress_atomic_uint16(atomic_shared_data_t *data)
{
    static volatile int idx = 0;
    int rc = 0;
    int i = __atomic_fetch_add(&idx, 1, __ATOMIC_RELAXED) & (ATOMIC_ARRAY_SIZE - 1);
    DO_ATOMIC_OPS(NULL, uint16_t, &data->val16[i], rc);
    return rc;
}

static int stress_atomic_uint32(atomic_shared_data_t *data)
{
    static volatile int idx = 0;
    int rc = 0;
    int i = __atomic_fetch_add(&idx, 1, __ATOMIC_RELAXED) & (ATOMIC_ARRAY_SIZE - 1);
    DO_ATOMIC_OPS(NULL, uint32_t, &data->val32[i], rc);
    return rc;
}

#if defined(STRESS_ATOMIC_64BIT)
static int stress_atomic_uint64(atomic_shared_data_t *data)
{
    static volatile int idx = 0;
    int rc = 0;
    int i = __atomic_fetch_add(&idx, 1, __ATOMIC_RELAXED) & (ATOMIC_ARRAY_SIZE - 1);
    DO_ATOMIC_OPS(NULL, uint64_t, &data->val64[i], rc);
    return rc;
}
#endif

static void stress_atomic_worker(void *parameter)
{
    atomic_thread_ctx_t *ctx = (atomic_thread_ctx_t *)parameter;

    while (stress_continue(ctx->args))
    {
        stress_atomic_uint8(ctx->data);
        stress_atomic_uint16(ctx->data);
        stress_atomic_uint32(ctx->data);
#if defined(STRESS_ATOMIC_64BIT)
        stress_atomic_uint64(ctx->data);
#endif
        stress_osal_thread_yield();
    }

    stress_osal_sem_release(ctx->sem_done);
}

void stress_atomic(stress_args_t *args)
{
    atomic_shared_data_t *shared_data = NULL;
    atomic_thread_ctx_t *thread_ctxs = NULL;
    int n_threads = s_atomic_threads;
    int i;

    shared_data = (atomic_shared_data_t *)stress_osal_malloc(sizeof(atomic_shared_data_t));
    if (!shared_data) {
        stress_osal_print("rtos_stress: error: [atomic] OOM allocating shared data\n");
        return;
    }
    stress_osal_memset(shared_data, 0, sizeof(atomic_shared_data_t));

    if (n_threads > 0) {
        thread_ctxs = (atomic_thread_ctx_t *)stress_osal_malloc(n_threads * sizeof(atomic_thread_ctx_t));
        if (!thread_ctxs) {
            stress_osal_free(shared_data);
            return;
        }

        if (!g_stress_silent_mode) {
             stress_osal_print("rtos_stress: info: [atomic-%d] starting %d threads to stress atomics\n",
                        args->instance, n_threads + 1);
        }

        for (i = 0; i < n_threads; i++) {
            thread_ctxs[i].args = args;
            thread_ctxs[i].data = shared_data;
            thread_ctxs[i].thread_id = i;
            thread_ctxs[i].sem_done = stress_osal_sem_create("atom_done", 0);

            stress_tid_t tid = stress_osal_thread_spawn("ng_atomic",
                                               stress_atomic_worker,
                                               &thread_ctxs[i],
                                               8192,
                                               20);
            (void)tid;
        }
    }

    while (stress_continue(args)) {
        stress_atomic_uint8(shared_data);
        stress_atomic_uint16(shared_data);
        stress_atomic_uint32(shared_data);
#if defined(STRESS_ATOMIC_64BIT)
        stress_atomic_uint64(shared_data);
        args->bogo.current_ops += 4;
#else
        args->bogo.current_ops += 3;
#endif
    }

    if (n_threads > 0 && thread_ctxs) {
        for (i = 0; i < n_threads; i++) {
            if (stress_osal_sem_take(thread_ctxs[i].sem_done, 3000) != 0) {
                stress_osal_print("rtos_stress: warn: atomic worker %d timeout\n", i);
            }
            stress_osal_sem_delete(thread_ctxs[i].sem_done);
        }
        stress_osal_free(thread_ctxs);
    }

    stress_osal_free(shared_data);
}
