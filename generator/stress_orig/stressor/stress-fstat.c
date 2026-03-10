/* applications/stress-ng/stress-fstat.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <config.h>

#define NUM_WORKER_THREADS  (1)
#define FILE_DATA_SIZE      (64)

#define UNLIKELY(x)         __builtin_expect(!!(x), 0)

static int32_t s_fstat_files = DEFAULT_FSTAT_FILES;

static int stress_fstat_opt_files(const char *opt_name, const char *opt_arg)
{
    int val = atoi(opt_arg);
    if (val < MIN_FSTAT_FILES) val = MIN_FSTAT_FILES;
    if (val > MAX_FSTAT_FILES) val = MAX_FSTAT_FILES;

    s_fstat_files = val;
    stress_osal_print("rtos_stress: debug: fstat-files set to %d\n", s_fstat_files);
    return 0;
}

const stress_opt_t stress_fstat_opts[] = {
    { "fstat-files", stress_fstat_opt_files },
    { NULL, NULL }
};

typedef struct {
    char path[128];
} file_entry_t;

typedef struct {
    stress_args_t *args;
    file_entry_t *files;
    int num_files;
    stress_sem_t sem_done;
} fstat_context_t;

static uint32_t stress_mwc32(void) { return (uint32_t)stress_osal_rand(); }

static int verify_fstat(const char *path)
{
    struct stat s1, s2;
    int fd;
    int ret = 0;

    stress_osal_memset(&s1, 0xAA, sizeof(s1));
    stress_osal_memset(&s2, 0x55, sizeof(s2));

    if (stat(path, &s1) < 0) return 0;

    fd = stress_osal_open(path, O_RDONLY, 0);
    if (fd < 0) return 0;

    if (fstat(fd, &s2) < 0) {
        stress_osal_close(fd);
        return 0;
    }
    stress_osal_close(fd);

    if (s1.st_size != s2.st_size) {
        stress_osal_print("rtos_stress: fail: [fstat] size mismatch %ld vs %ld on %s\n",
                   (long)s1.st_size, (long)s2.st_size, path);
        ret = -1;
    }

    if (s1.st_mode != s2.st_mode) {
        stress_osal_print("rtos_stress: fail: [fstat] mode mismatch 0x%x vs 0x%x on %s\n",
                   (unsigned int)s1.st_mode, (unsigned int)s2.st_mode, path);
        ret = -1;
    }

    return ret;
}

static void stress_fstat_worker(void *parameter)
{
    fstat_context_t *ctx = (fstat_context_t *)parameter;

    while (stress_continue(ctx->args))
    {
        int idx = stress_mwc32() % ctx->num_files;

        if (verify_fstat(ctx->files[idx].path) < 0) {
        }

        ctx->args->bogo.current_ops++;
        stress_osal_thread_yield();
    }

    stress_osal_sem_release(ctx->sem_done);
}

void stress_fstat(stress_args_t *args)
{
    char dir_path[64];
    file_entry_t *file_list = NULL;
    fstat_context_t ctx;
    int i;
    int files_created = 0;
    int num_files = s_fstat_files;
    int workers_spawned = 0;

    stress_osal_snprintf(dir_path, sizeof(dir_path), "st_fstat_%d", (int)args->instance);

    if (stress_osal_mkdir(dir_path, 0777) < 0 && errno != EEXIST) {
        stress_osal_print("rtos_stress: error: [fstat] mkdir %s failed\n", dir_path);
        return;
    }

    file_list = (file_entry_t *)stress_osal_malloc(num_files * sizeof(file_entry_t));
    if (!file_list) {
        stress_osal_print("rtos_stress: error: [fstat] OOM\n");
        goto cleanup_dir;
    }

    if (!g_stress_silent_mode) {
        stress_osal_print("rtos_stress: info: [fstat-%d] populating %d files in %s\n",
                   args->instance, num_files, dir_path);
    }

    for (i = 0; i < num_files; i++) {
        stress_osal_snprintf(file_list[i].path, sizeof(file_list[i].path), "%s/f%d.dat", dir_path, i);

        int fd = stress_osal_open(file_list[i].path, O_CREAT | O_RDWR | O_TRUNC, 0666);
        if (fd >= 0) {
            int len = (i % 16) * 4;
            char buf[64];
            stress_osal_memset(buf, i, sizeof(buf));
            stress_osal_write(fd, buf, len);
            stress_osal_close(fd);
            files_created++;
        }
    }

    if (files_created == 0) {
        stress_osal_print("rtos_stress: error: [fstat] failed to create any files\n");
        goto cleanup_files;
    }

    ctx.args = args;
    ctx.files = file_list;
    ctx.num_files = files_created;
    ctx.sem_done = stress_osal_sem_create("fstat_done", 0);

    if (!ctx.sem_done) goto cleanup_files;

    for (i = 0; i < NUM_WORKER_THREADS; i++) {
        stress_tid_t tid = stress_osal_thread_spawn("ng_fstat",
                                           stress_fstat_worker,
                                           &ctx,
                                           16384,
                                           20);
        if (tid) {
            workers_spawned++;
        } else {
            stress_osal_print("rtos_stress: warn: [fstat] failed to spawn worker %d\n", i);
        }
    }

    if (workers_spawned == 0) {
        stress_osal_print("rtos_stress: error: [fstat] failed to spawn any workers\n");
        stress_osal_sem_delete(ctx.sem_done);
        goto cleanup_files;
    }

    while (stress_continue(args)) {
        stress_osal_sleep_ms(100);
    }

    for (i = 0; i < workers_spawned; i++) {
        stress_osal_sem_take(ctx.sem_done, STRESS_WAIT_FOREVER);
    }
    stress_osal_sem_delete(ctx.sem_done);

cleanup_files:
    for (i = 0; i < files_created; i++) {
        stress_osal_unlink(file_list[i].path);
    }
    stress_osal_free(file_list);

cleanup_dir:
    stress_osal_rmdir(dir_path);
}
