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

#define FSTAT_THREAD_NAME            "fst_wrk"
#define NUM_WORKER_THREADS           (1)
#define FILE_DATA_SIZE               (64)
#define FSTAT_MAX_CORRUPT_FAIL       (8)
#define FSTAT_WORKER_EXIT_TIMEOUT_MS (5000U)

#define UNLIKELY(x)  __builtin_expect(!!(x), 0)

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
    stress_args_t   *args;
    file_entry_t    *files;
    int              num_files;
    stress_sem_t     sem_done;
    volatile int     abort;
    volatile int     abandoned;
} fstat_context_t;

static uint32_t stress_mwc32(void) { return (uint32_t)stress_osal_rand(); }

static void safe_unlink(const char *path, int instance, const char *tag)
{
    if (stress_osal_unlink(path) != 0 && errno != ENOENT) {
        stress_osal_print("rtos_stress: warn: [fstat-%d] %s unlink '%s' failed (errno=%d)\n",
                          instance, tag, path, errno);
    }
}

typedef enum {
    VERIFY_SKIP    =  1,
    VERIFY_OK      =  0,
    VERIFY_CORRUPT = -1
} verify_result_t;

static verify_result_t verify_fstat(const char *path, volatile int *p_abort)
{
    struct stat     s1, s2;
    int             fd;
    verify_result_t ret = VERIFY_OK;

    if (*p_abort) return VERIFY_SKIP;

    stress_osal_memset(&s1, 0xAA, sizeof(s1));
    stress_osal_memset(&s2, 0x55, sizeof(s2));

    if (stress_osal_stat(path, &s1) < 0) return VERIFY_SKIP;
    if (*p_abort) return VERIFY_SKIP;

    fd = stress_osal_open(path, O_RDONLY, 0);
    if (fd < 0) return VERIFY_SKIP;

    if (stress_osal_fstat(fd, &s2) < 0) {
        stress_osal_close(fd);
        return VERIFY_SKIP;
    }
    stress_osal_close(fd);

    if (s1.st_size != s2.st_size) {
        struct stat s1r, s2r;
        int         fd2;
        int         size_match = 0;

        stress_osal_sleep_ms(1);

        if (stress_osal_stat(path, &s1r) == 0) {
            fd2 = stress_osal_open(path, O_RDONLY, 0);
            if (fd2 >= 0) {
                if (stress_osal_fstat(fd2, &s2r) == 0) {
                    if (s1r.st_size == s2r.st_size) {
                        size_match = 1;
                    }
                }
                stress_osal_close(fd2);
            }
        }

        if (!size_match) {
            stress_osal_print("rtos_stress: fail: [fstat] st_size mismatch"
                              " %lld vs %lld on '%s'\n",
                              (long long)s1.st_size, (long long)s2.st_size, path);
            ret = VERIFY_CORRUPT;
        }
    }

    {
        mode_t type_s1 = s1.st_mode & S_IFMT;
        mode_t type_s2 = s2.st_mode & S_IFMT;

        if (type_s1 != type_s2) {
            stress_osal_print("rtos_stress: fail: [fstat] st_mode type mismatch"
                              " 0x%x vs 0x%x on '%s'\n",
                              (unsigned int)s1.st_mode, (unsigned int)s2.st_mode, path);
            ret = VERIFY_CORRUPT;
        } else if (s1.st_mode != s2.st_mode) {
            stress_osal_print("rtos_stress: warn: [fstat] st_mode perm mismatch"
                              " 0x%x vs 0x%x on '%s'\n",
                              (unsigned int)s1.st_mode, (unsigned int)s2.st_mode, path);
        }
    }

    return ret;
}

static void stress_fstat_worker(void *parameter)
{
    fstat_context_t *ctx      = (fstat_context_t *)parameter;
    int              fail_cnt = 0;

    while (stress_continue(ctx->args) && !ctx->abort) {
        int             idx    = (int)(stress_mwc32() % (uint32_t)ctx->num_files);
        verify_result_t result = verify_fstat(ctx->files[idx].path, &ctx->abort);

        switch (result) {
        case VERIFY_OK:
            ctx->args->bogo.current_ops++;
            fail_cnt = 0;
            break;
        case VERIFY_SKIP:
            break;
        case VERIFY_CORRUPT:
            fail_cnt++;
            stress_osal_print("rtos_stress: warn: [fstat-%d] corrupt count=%d/%d\n",
                              ctx->args->instance, fail_cnt, FSTAT_MAX_CORRUPT_FAIL);
            if (fail_cnt >= FSTAT_MAX_CORRUPT_FAIL) {
                stress_osal_print("rtos_stress: fail: [fstat-%d] too many corrupt"
                                  " results, aborting worker\n",
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
    stress_osal_mb();

    if (!ctx->abandoned) {
        stress_osal_sem_release(ctx->sem_done);
    } else {
        stress_osal_print("rtos_stress: info: [fstat-%d] worker freeing abandoned ctx\n",
                          ctx->args->instance);
        stress_osal_free(ctx);
    }
}

void stress_fstat(stress_args_t *args)
{
    char             dir_path[64];
    file_entry_t    *file_list     = NULL;
    file_entry_t    *valid_list    = NULL;
    fstat_context_t *ctx           = NULL;
    int              i;
    int              files_attempted = 0;
    int              files_created   = 0;
    int              num_files       = s_fstat_files;
    int              workers_spawned = 0;

    stress_osal_snprintf(dir_path, sizeof(dir_path),
                         "st_fstat_%d", (int)args->instance);

    if (stress_osal_rmdir(dir_path) != 0 && errno != ENOENT) {
        char stale_path[128];
        for (i = 0; i < MAX_FSTAT_FILES; i++) {
            stress_osal_snprintf(stale_path, sizeof(stale_path),
                                 "%s/f%d.dat", dir_path, i);
            stress_osal_unlink(stale_path);
        }
        stress_osal_rmdir(dir_path);
    }

    if (stress_osal_mkdir(dir_path, 0777) < 0) {
        if (errno == EROFS) {
            stress_osal_print("rtos_stress: info: [fstat-%d] filesystem is"
                              " read-only, skipping\n",
                              args->instance);
            return;
        } else if (errno != EEXIST) {
            stress_osal_print("rtos_stress: error: [fstat-%d] mkdir '%s'"
                              " failed (errno=%d)\n",
                              args->instance, dir_path, errno);
            return;
        }
    }

    file_list  = (file_entry_t *)stress_osal_malloc(num_files * sizeof(file_entry_t));
    valid_list = (file_entry_t *)stress_osal_malloc(num_files * sizeof(file_entry_t));
    ctx        = (fstat_context_t *)stress_osal_malloc(sizeof(fstat_context_t));

    if (!file_list || !valid_list || !ctx) {
        stress_osal_print("rtos_stress: error: [fstat-%d] OOM\n", args->instance);
        goto cleanup_files;
    }

    stress_osal_memset(ctx, 0, sizeof(fstat_context_t));

    if (!g_stress_silent_mode) {
        stress_osal_print("rtos_stress: info: [fstat-%d] populating %d files in '%s'\n",
                          args->instance, num_files, dir_path);
    }

    for (i = 0; i < num_files; i++) {
        int     fd;
        char    buf[FILE_DATA_SIZE];
        ssize_t wret;

        stress_osal_snprintf(file_list[i].path, sizeof(file_list[i].path),
                             "%s/f%d.dat", dir_path, i);
        files_attempted++;

        fd = stress_osal_open(file_list[i].path, O_CREAT | O_RDWR | O_TRUNC, 0666);
        if (fd < 0) {
            if (errno == EROFS) {
                stress_osal_print("rtos_stress: info: [fstat-%d] read-only"
                                  " filesystem, skipping\n",
                                  args->instance);
                files_attempted--;
                goto cleanup_files;
            }
            stress_osal_print("rtos_stress: warn: [fstat-%d] create '%s'"
                              " failed (errno=%d)\n",
                              args->instance, file_list[i].path, errno);
            continue;
        }

        stress_osal_memset(buf, (unsigned char)i, FILE_DATA_SIZE);
        wret = stress_osal_write(fd, buf, FILE_DATA_SIZE);

        if (stress_osal_fsync(fd) != 0) {
            stress_osal_print("rtos_stress: warn: [fstat-%d] fsync '%s'"
                              " failed (errno=%d)\n",
                              args->instance, file_list[i].path, errno);
        }
        stress_osal_close(fd);

        if (wret != (ssize_t)FILE_DATA_SIZE) {
            stress_osal_print("rtos_stress: warn: [fstat-%d] write '%s'"
                              " returned %lld (errno=%d)\n",
                              args->instance, file_list[i].path,
                              (long long)wret, errno);
            continue;
        }

        stress_osal_snprintf(valid_list[files_created].path,
                             sizeof(valid_list[files_created].path),
                             "%s/f%d.dat", dir_path, i);
        files_created++;
    }

    if (files_created == 0) {
        stress_osal_print("rtos_stress: error: [fstat-%d] no files created\n",
                          args->instance);
        goto cleanup_files;
    }

    stress_osal_print("rtos_stress: info: [fstat-%d] created %d/%d files\n",
                      args->instance, files_created, files_attempted);

    ctx->args      = args;
    ctx->files     = valid_list;
    ctx->num_files = files_created;
    ctx->abort     = 0;
    ctx->abandoned = 0;
    ctx->sem_done  = stress_osal_sem_create("fst_sem", 0);

    if (!ctx->sem_done) {
        stress_osal_print("rtos_stress: error: [fstat-%d] sem create failed\n",
                          args->instance);
        goto cleanup_files;
    }

    for (i = 0; i < NUM_WORKER_THREADS; i++) {
        stress_tid_t tid = stress_osal_thread_spawn(FSTAT_THREAD_NAME,
                                                    stress_fstat_worker,
                                                    ctx,
                                                    16384,
                                                    20);
        if (tid) {
            workers_spawned++;
        } else {
            stress_osal_print("rtos_stress: warn: [fstat-%d] spawn worker %d failed\n",
                              args->instance, i);
        }
    }

    if (workers_spawned == 0) {
        stress_osal_print("rtos_stress: error: [fstat-%d] no workers\n",
                          args->instance);
        stress_osal_sem_delete(ctx->sem_done);
        goto cleanup_files;
    }

    while (stress_continue(args)) {
        stress_osal_sleep_ms(100);
    }

    stress_osal_mb();
    ctx->abort = 1;
    stress_osal_mb();

    {
        stress_tick_t timeout_ticks = (stress_tick_t)(
            (uint64_t)FSTAT_WORKER_EXIT_TIMEOUT_MS
            * stress_osal_tick_hz() / 1000ULL);
        int timed_out = 0;

        for (i = 0; i < workers_spawned; i++) {
            int ret = stress_osal_sem_take(ctx->sem_done, timeout_ticks);
            if (ret != 0) {
                stress_osal_print("rtos_stress: warn: [fstat-%d] worker %d"
                                  " did not exit within %u ms\n",
                                  args->instance, i,
                                  FSTAT_WORKER_EXIT_TIMEOUT_MS);
                timed_out = 1;
            }
        }

        if (!timed_out) {
            stress_osal_sem_delete(ctx->sem_done);
            stress_osal_free(ctx);
            ctx = NULL;
        } else {
            stress_osal_sem_delete(ctx->sem_done);
            stress_osal_mb();
            ctx->abandoned = 1;
            stress_osal_mb();
            ctx = NULL;
        }
    }

cleanup_files:
    for (i = 0; i < files_attempted; i++) {
        safe_unlink(file_list[i].path, args->instance, "cleanup");
    }

    if (valid_list) stress_osal_free(valid_list);
    if (file_list)  stress_osal_free(file_list);
    if (ctx)        stress_osal_free(ctx);

    if (stress_osal_rmdir(dir_path) != 0 && errno != ENOENT) {
        stress_osal_print("rtos_stress: warn: [fstat-%d] rmdir '%s' failed (errno=%d)\n",
                          args->instance, dir_path, errno);
    }
}
