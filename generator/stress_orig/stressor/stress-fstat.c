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
#include <stress-config.h>

/*
 * NUM_WORKER_THREADS：当前固定为 1，与 sem_done 逻辑对应。
 * 若需多线程，需修改 sem 计数和等待逻辑。
 */
#define NUM_WORKER_THREADS      (1)

#define DIR_TEMPLATE            STRESS_FILE_BASE_DIR "st_fstat_%d"

#define FILE_DATA_SIZE          (64)

/*
 * worker 连续遇到数据损坏超过此阈值后主动退出。
 */
#define FSTAT_MAX_CORRUPT_FAIL  (8)

#define UNLIKELY(x)             __builtin_expect(!!(x), 0)

static int32_t s_fstat_files = DEFAULT_FSTAT_FILES;

static int stress_fstat_opt_files(const char *opt_name, const char *opt_arg)
{
    int val = atoi(opt_arg);
    if (val < MIN_FSTAT_FILES) val = MIN_FSTAT_FILES;
    if (val > MAX_FSTAT_FILES) val = MAX_FSTAT_FILES;

    s_fstat_files = val;
    stress_osal_print("rtos_stress: debug: fstat-files set to %d\n",
                      s_fstat_files);
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
    stress_args_t  *args;
    file_entry_t   *files;        /* 仅含成功创建的文件（修复问题二） */
    int             num_files;    /* 成功创建的文件数 */
    stress_sem_t    sem_done;
} fstat_context_t;

static uint32_t stress_mwc32(void) { return (uint32_t)stress_osal_rand(); }

static void safe_unlink(const char *path, int instance, const char *tag)
{
    if (stress_osal_unlink(path) != 0 && errno != ENOENT) {
        stress_osal_print("rtos_stress: warn: [fstat-%d] %s unlink '%s'"
                          " failed (errno=%d)\n",
                          instance, tag, path, errno);
    }
}

typedef enum {
    VERIFY_SKIP    =  1,
    VERIFY_OK      =  0,
    VERIFY_CORRUPT = -1
} verify_result_t;

static verify_result_t verify_fstat(const char *path)
{
    struct stat s1, s2;
    int fd;
    verify_result_t ret = VERIFY_OK;

    stress_osal_memset(&s1, 0xAA, sizeof(s1));
    stress_osal_memset(&s2, 0x55, sizeof(s2));

    if (stress_osal_stat(path, &s1) < 0) {
        return VERIFY_SKIP;
    }

    fd = stress_osal_open(path, O_RDONLY, 0);
    if (fd < 0) {
        return VERIFY_SKIP;
    }

    if (stress_osal_fstat(fd, &s2) < 0) {
        stress_osal_close(fd);
        return VERIFY_SKIP;
    }
    stress_osal_close(fd);

    if (s1.st_size != s2.st_size) {
        stress_osal_print("rtos_stress: fail: [fstat] st_size mismatch"
                          " %ld vs %ld on '%s'\n",
                          (long)s1.st_size, (long)s2.st_size, path);
        ret = VERIFY_CORRUPT;
    }

    if (s1.st_mode != s2.st_mode) {
        stress_osal_print("rtos_stress: fail: [fstat] st_mode mismatch"
                          " 0x%x vs 0x%x on '%s'\n",
                          (unsigned int)s1.st_mode,
                          (unsigned int)s2.st_mode,
                          path);
        ret = VERIFY_CORRUPT;
    }

    return ret;
}

static void stress_fstat_worker(void *parameter)
{
    fstat_context_t *ctx      = (fstat_context_t *)parameter;
    int              fail_cnt = 0;

    while (stress_continue(ctx->args)) {
        int             idx    = (int)(stress_mwc32() % (uint32_t)ctx->num_files);
        verify_result_t result = verify_fstat(ctx->files[idx].path);

        switch (result) {
        case VERIFY_OK:
            ctx->args->bogo.current_ops++;
            fail_cnt = 0;
            break;

        case VERIFY_SKIP:
            break;

        case VERIFY_CORRUPT:
            fail_cnt++;
            stress_osal_print("rtos_stress: warn: [fstat-%d] corrupt"
                              " count=%d/%d\n",
                              ctx->args->instance,
                              fail_cnt,
                              FSTAT_MAX_CORRUPT_FAIL);
            if (fail_cnt >= FSTAT_MAX_CORRUPT_FAIL) {
                stress_osal_print("rtos_stress: fail: [fstat-%d]"
                                  " too many corrupt results, aborting"
                                  " worker\n",
                                  ctx->args->instance);
                goto worker_exit;
            }
            break;

        default:
            break;
        }

        stress_osal_thread_yield();
    }

worker_exit:
    stress_osal_sem_release(ctx->sem_done);
}

void stress_fstat(stress_args_t *args)
{
    char          dir_path[64];
    file_entry_t *file_list     = NULL;
    file_entry_t *valid_list    = NULL;
    fstat_context_t ctx;
    int i;
    int files_attempted = 0;
    int files_created   = 0;
    int num_files       = s_fstat_files;
    int workers_spawned = 0;

    stress_osal_snprintf(dir_path, sizeof(dir_path),
                         DIR_TEMPLATE, (int)args->instance);

    if (stress_osal_mkdir(dir_path, 0777) < 0 && errno != EEXIST) {
        stress_osal_print("rtos_stress: error: [fstat-%d] mkdir '%s'"
                          " failed (errno=%d)\n",
                          args->instance, dir_path, errno);
        return;
    }

    file_list = (file_entry_t *)stress_osal_malloc(
                    num_files * sizeof(file_entry_t));
    valid_list = (file_entry_t *)stress_osal_malloc(
                    num_files * sizeof(file_entry_t));

    if (!file_list || !valid_list) {
        stress_osal_print("rtos_stress: error: [fstat-%d] OOM\n",
                          args->instance);
        goto cleanup_dir;
    }

    if (!g_stress_silent_mode) {
        stress_osal_print("rtos_stress: info: [fstat-%d] populating"
                          " %d files in '%s'\n",
                          args->instance, num_files, dir_path);
    }

    for (i = 0; i < num_files; i++) {
        stress_osal_snprintf(file_list[i].path,
                             sizeof(file_list[i].path),
                             "%s/f%d.dat", dir_path, i);
        files_attempted++;

        int fd = stress_osal_open(file_list[i].path,
                                  O_CREAT | O_RDWR | O_TRUNC, 0666);
        if (fd < 0) {
            stress_osal_print("rtos_stress: warn: [fstat-%d] create"
                              " '%s' failed (errno=%d)\n",
                              args->instance, file_list[i].path, errno);
            continue;
        }

        char    buf[FILE_DATA_SIZE];
        ssize_t wret;

        stress_osal_memset(buf, (unsigned char)i, FILE_DATA_SIZE);
        wret = stress_osal_write(fd, buf, FILE_DATA_SIZE);
        if (wret != FILE_DATA_SIZE) {
            stress_osal_print("rtos_stress: warn: [fstat-%d] write '%s'"
                              " returned %ld (errno=%d)\n",
                              args->instance, file_list[i].path,
                              (long)wret, errno);
            stress_osal_close(fd);
            continue;
        }
        stress_osal_close(fd);

        stress_osal_snprintf(valid_list[files_created].path,
                             sizeof(valid_list[files_created].path),
                             "%s/f%d.dat", dir_path, i);
        files_created++;
    }

    if (files_created == 0) {
        stress_osal_print("rtos_stress: error: [fstat-%d] failed to"
                          " create any files\n",
                          args->instance);
        goto cleanup_files;
    }

    stress_osal_print("rtos_stress: info: [fstat-%d] created %d/%d files\n",
                      args->instance, files_created, files_attempted);

    ctx.args      = args;
    ctx.files     = valid_list;
    ctx.num_files = files_created;
    ctx.sem_done  = stress_osal_sem_create("fstat_done", 0);

    if (!ctx.sem_done) {
        stress_osal_print("rtos_stress: error: [fstat-%d] sem create"
                          " failed\n", args->instance);
        goto cleanup_files;
    }

    for (i = 0; i < NUM_WORKER_THREADS; i++) {
        stress_tid_t tid = stress_osal_thread_spawn("ng_fstat",
                                                    stress_fstat_worker,
                                                    &ctx,
                                                    16384,
                                                    20);
        if (tid) {
            workers_spawned++;
        } else {
            stress_osal_print("rtos_stress: warn: [fstat-%d] failed to"
                              " spawn worker %d\n",
                              args->instance, i);
        }
    }

    if (workers_spawned == 0) {
        stress_osal_print("rtos_stress: error: [fstat-%d] failed to"
                          " spawn any workers\n",
                          args->instance);
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
    for (i = 0; i < files_attempted; i++) {
        safe_unlink(file_list[i].path, args->instance, "cleanup");
    }

    stress_osal_free(valid_list);
    stress_osal_free(file_list);

cleanup_dir:
    if (stress_osal_rmdir(dir_path) != 0 && errno != ENOENT) {
        stress_osal_print("rtos_stress: warn: [fstat-%d] rmdir '%s'"
                          " failed (errno=%d)\n",
                          args->instance, dir_path, errno);
    }
}
