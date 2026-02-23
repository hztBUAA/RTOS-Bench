/* applications/stress-ng/stress-context.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <config.h>

#define STACK_SIZE          (16384)
#define THREAD_PRIORITY     (20)

#define UNLIKELY(x)         __builtin_expect(!!(x), 0)
#define LIKELY(x)           __builtin_expect(!!(x), 1)

typedef struct context_info {
    stress_tid_t tid;
    stress_sem_t sem_wait;
    stress_sem_t sem_signal;
    stress_sem_t sem_done;

    uint32_t    check0;
    uint32_t    check1;

    stress_args_t *args;
    int         index;
} context_info_t;

static int32_t s_context_threads = DEFAULT_CONTEXT_THREADS;

static int stress_context_opt_threads(const char *opt_name, const char *opt_arg)
{
    int val = atoi(opt_arg);
    if (val < 1) val = 1;
    if (val > 128) val = 128;
    s_context_threads = val;
    return 0;
}

const stress_opt_t stress_context_opts[] = {
    { "context-threads", stress_context_opt_threads },
    { NULL, NULL }
};

static uint32_t stress_mwc32(void) { return (uint32_t)stress_osal_rand(); }

static void stress_context_worker(void *parameter)
{
    context_info_t *info = (context_info_t *)parameter;
    const uint32_t c0 = info->check0;
    const uint32_t c1 = info->check1;

    while (stress_continue(info->args))
    {
        if (stress_osal_sem_take(info->sem_wait, STRESS_WAIT_FOREVER) != 0) {
            break;
        }

        if (!stress_continue(info->args)) {
            stress_osal_sem_release(info->sem_signal);
            break;
        }

        if (UNLIKELY(info->check0 != c0 || info->check1 != c1)) {
            stress_osal_print("rtos_stress: fail: [context] memory corruption detected on thread %d\n", info->index);
            break;
        }

        info->args->bogo.current_ops++;

        stress_osal_sem_release(info->sem_signal);
    }

    if (info->sem_done) {
        stress_osal_sem_release(info->sem_done);
    }

    while (1) {
        stress_osal_sleep_ms(1000);
    }
}

void stress_context(stress_args_t *args)
{
    context_info_t *contexts = NULL;
    stress_sem_t barrier_sem = NULL;
    int n_threads = s_context_threads;
    int i;
    int cleanup_safe = 1;

    barrier_sem = stress_osal_sem_create("ctx_bar", 0);
    if (!barrier_sem) {
        stress_osal_print("rtos_stress: error: [context] OOM creating barrier semaphore\n");
        return;
    }

    contexts = (context_info_t *)stress_osal_malloc(n_threads * sizeof(context_info_t));
    if (!contexts) {
        stress_osal_print("rtos_stress: error: [context] OOM allocating contexts\n");
        stress_osal_sem_delete(barrier_sem);
        return;
    }
    stress_osal_memset(contexts, 0, n_threads * sizeof(context_info_t));

    if (!g_stress_silent_mode) {
        stress_osal_print("rtos_stress: info: [context-%d] starting %d threads\n", args->instance, n_threads);
    }

    for (i = 0; i < n_threads; i++) {
        char name[16];
        stress_osal_snprintf(name, sizeof(name), "ctx_s%d", i);

        contexts[i].sem_wait = stress_osal_sem_create(name, 0);
        if (!contexts[i].sem_wait) {
            goto cleanup_startup;
        }

        contexts[i].check0 = stress_mwc32();
        contexts[i].check1 = stress_mwc32();
        contexts[i].args = args;
        contexts[i].index = i;
        contexts[i].sem_done = barrier_sem;
    }

    for (i = 0; i < n_threads; i++) {
        contexts[i].sem_signal = contexts[(i + 1) % n_threads].sem_wait;
    }

    int created_count = 0;
    for (i = 0; i < n_threads; i++) {
        char name[16];
        stress_osal_snprintf(name, sizeof(name), "ng_ctx%d", i);

        contexts[i].tid = stress_osal_thread_spawn(name,
                                           stress_context_worker,
                                           &contexts[i],
                                           STACK_SIZE,
                                           THREAD_PRIORITY);
        if (contexts[i].tid) {
            created_count++;
        } else {
            stress_osal_print("rtos_stress: error: [context] failed to spawn thread %d\n", i);
        }
    }

    if (created_count < n_threads) {
        stress_osal_print("rtos_stress: error: [context] Partial thread creation (%d/%d). Aborting.\n",
                          created_count, n_threads);
        cleanup_safe = 0;
    } else {
        stress_osal_sem_release(contexts[0].sem_wait);

        while (stress_continue(args)) {
            stress_osal_sleep_ms(100);
        }
    }

    if (cleanup_safe) {
        int finished_count = 0;
        for (i = 0; i < created_count; i++) {
            if (stress_osal_sem_take(barrier_sem, 2000) != 0) {
                stress_osal_print("rtos_stress: warning: [context] timeout waiting for thread signal %d/%d.\n", i + 1, created_count);
            } else {
                finished_count++;
            }
        }

        for (i = 0; i < created_count; i++) {
            if (contexts[i].tid) {
                stress_osal_thread_delete(contexts[i].tid);
                contexts[i].tid = NULL;
            }
        }
    }

    stress_osal_sleep_ms(50);

    if (cleanup_safe) {
        for (i = 0; i < n_threads; i++) {
            if (contexts[i].sem_wait) {
                stress_osal_sem_delete(contexts[i].sem_wait);
            }
        }
        stress_osal_free(contexts);
        stress_osal_sem_delete(barrier_sem);
    }

    return;

cleanup_startup:
    for (int k = 0; k < n_threads; k++) {
        if (contexts[k].sem_wait) stress_osal_sem_delete(contexts[k].sem_wait);
    }
    stress_osal_free(contexts);
    stress_osal_sem_delete(barrier_sem);
}
