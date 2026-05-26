/* applications/stress-ng/stress-unlink.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stress-config.h>

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

#define FILENAME_TEMPLATE       STRESS_FILE_BASE_DIR "u%d_%04d"

#define UNLIKELY(x)             __builtin_expect(!!(x), 0)
#define LIKELY(x)               __builtin_expect(!!(x), 1)

#define FSYNC_STRIDE            (1 << 5)

static int32_t s_unlink_files = DEFAULT_UNLINK_FILES;

static int stress_unlink_opt_files(const char *opt_name, const char *opt_arg)
{
    (void)opt_name;
    int val = atoi(opt_arg);
    if (val < MIN_UNLINK_FILES) val = MIN_UNLINK_FILES;
    if (val > MAX_UNLINK_FILES) val = MAX_UNLINK_FILES;
    s_unlink_files = val;
    stress_osal_print("rtos_stress: debug: unlink-files set to %d\n", s_unlink_files);
    return 0;
}

const stress_opt_t stress_unlink_opts[] = {
    { "unlink-files", stress_unlink_opt_files },
    { NULL, NULL }
};

static uint32_t stress_mwc32(void) { return (uint32_t)stress_osal_rand(); }

static void stress_unlink_shuffle(int *idx, int size)
{
    for (int i = 0; i < size; i++) {
        int j   = (int)(stress_mwc32() % (uint32_t)size);
        int tmp = idx[i];
        idx[i]  = idx[j];
        idx[j]  = tmp;
    }
}

static void safe_unlink_quiet(const char *path)
{
    stress_osal_unlink(path);
}

void stress_unlink(stress_args_t *args)
{
    int       num_files      = s_unlink_files;
    int       actual_limit   = num_files;
    char    **filenames      = NULL;
    int      *fds            = NULL;
    int      *perm_idx       = NULL;
    char     *tmp_path       = NULL;
    int       i;
    int       fsync_supported    = 0;
    int       fd_limit_total     = 0;
    int       fd_limit_logged    = 0;
    int       open_err_count     = 0;
    int       unlink_err_count   = 0;
    uint32_t  consecutive_zero   = 0;

    #define MAX_ERR_LOGS      5
    #define MAX_FD_LIMIT_LOGS 3

    tmp_path = (char *)stress_osal_malloc(PATH_MAX);
    if (!tmp_path) {
        stress_osal_print("rtos_stress: error: [unlink-%d] OOM for path buf\n",
                          args->instance);
        return;
    }

    {
        stress_osal_snprintf(tmp_path, PATH_MAX,
                             "%s__unlink_pre_%d.tmp",
                             STRESS_FILE_BASE_DIR, (int)args->instance);
        int fd_test = -1;
        int retries = 3;
        while (retries-- > 0) {
            fd_test = stress_osal_open(tmp_path, O_CREAT | O_RDWR, 0666);
            if (fd_test >= 0) break;
            stress_osal_sleep_ms(100);
        }
        if (fd_test >= 0) {
            stress_osal_close(fd_test);
            stress_osal_unlink(tmp_path);
        } else {
            stress_osal_print("rtos_stress: warn: [unlink-%d] base dir \"%s\""
                              " not writable (errno=%d), skipping\n",
                              args->instance, STRESS_FILE_BASE_DIR, errno);
            stress_osal_free(tmp_path);
            return;
        }
    }

    {
        stress_osal_snprintf(tmp_path, PATH_MAX,
                             "%s__unlink_fsync_%d.tmp",
                             STRESS_FILE_BASE_DIR, (int)args->instance);
        int fd_test = stress_osal_open(tmp_path, O_CREAT | O_RDWR, 0666);
        if (fd_test >= 0) {
            errno = 0;
            if (stress_osal_fsync(fd_test) == 0) {
                fsync_supported = 1;
            }
            stress_osal_close(fd_test);
            stress_osal_unlink(tmp_path);
        }
    }

    filenames = (char **)stress_osal_malloc(num_files * sizeof(char *));
    fds       = (int   *)stress_osal_malloc(num_files * sizeof(int));
    perm_idx  = (int   *)stress_osal_malloc(num_files * sizeof(int));

    if (!filenames || !fds || !perm_idx) {
        stress_osal_print("rtos_stress: error: [unlink-%d] OOM allocating arrays\n",
                          args->instance);
        goto cleanup;
    }

    stress_osal_memset(filenames, 0, num_files * sizeof(char *));
    for (i = 0; i < num_files; i++) {
        fds[i]      = -1;
        perm_idx[i] = i;
    }

    for (i = 0; i < num_files; i++) {
        filenames[i] = (char *)stress_osal_malloc(PATH_MAX);
        if (!filenames[i]) {
            stress_osal_print("rtos_stress: error: [unlink-%d] OOM buf %d\n",
                              args->instance, i);
            goto cleanup;
        }
        stress_osal_snprintf(filenames[i], PATH_MAX,
                             FILENAME_TEMPLATE,
                             (int)args->instance, i);
    }

    stress_osal_print("rtos_stress: info: [unlink-%d] starting with %d files\n",
                      args->instance, num_files);

    while (stress_continue(args)) {
        int files_opened  = 0;
        int fd_limit_this = 0;

        for (i = 0; i < actual_limit; i++) {
            if (UNLIKELY(!stress_continue(args))) break;

            int flags = O_CREAT | O_RDWR;
            if (stress_mwc32() & 1) flags |= O_TRUNC;

            fds[i] = stress_osal_open(filenames[i], flags, 0666);

            if (fds[i] < 0) {
                if (errno == EMFILE || errno == ENFILE) {
                    fd_limit_total++;
                    fd_limit_this = 1;
                    actual_limit  = (i >= 2) ? i : 2;
                    if (fd_limit_logged < MAX_FD_LIMIT_LOGS) {
                        fd_limit_logged++;
                        stress_osal_print("rtos_stress: info: [unlink-%d]"
                                          " fd limit at slot %d, adapted to"
                                          " %d files [%d/%d]\n",
                                          args->instance, i, actual_limit,
                                          fd_limit_logged, MAX_FD_LIMIT_LOGS);
                    }
                    break;
                }
                if (open_err_count < MAX_ERR_LOGS) {
                    open_err_count++;
                    stress_osal_print("rtos_stress: warn: [unlink-%d]"
                                      " open '%s' errno=%d [%d/%d]\n",
                                      args->instance, filenames[i], errno,
                                      open_err_count, MAX_ERR_LOGS);
                }
                continue;
            }

            files_opened++;

            if (fsync_supported && (i & (FSYNC_STRIDE - 1)) == 0) {
                char tiny = (char)(i & 0xFF);
                stress_osal_write(fds[i], &tiny, 1);
                stress_osal_fsync(fds[i]);
            }

            args->bogo.current_ops++;
        }

        if (files_opened == 0) {
            consecutive_zero++;
            if (consecutive_zero >= 50) {
                stress_osal_print("rtos_stress: fail: [unlink-%d]"
                                  " 50 consecutive rounds with 0 files"
                                  " opened, aborting\n",
                                  args->instance);
                break;
            }
            stress_osal_sleep_ms(10);
            continue;
        }
        consecutive_zero = 0;

        if (!fd_limit_this && actual_limit < num_files) {
            actual_limit += 2;
            if (actual_limit > num_files) actual_limit = num_files;
        }

        for (i = 0; i < num_files; i++) {
            if (fds[i] >= 0) {
                stress_osal_close(fds[i]);
                fds[i] = -1;
            }
        }

        stress_unlink_shuffle(perm_idx, actual_limit);
        for (i = 0; i < actual_limit; i++) {
            if (UNLIKELY(!stress_continue(args))) break;

            int idx = perm_idx[i];
            if (idx >= num_files) continue;

            if (stress_osal_unlink(filenames[idx]) == 0) {
                args->bogo.current_ops++;
            } else if (errno != ENOENT) {
                if (unlink_err_count < MAX_ERR_LOGS) {
                    unlink_err_count++;
                    stress_osal_print("rtos_stress: warn: [unlink-%d]"
                                      " unlink '%s' errno=%d [%d/%d]\n",
                                      args->instance, filenames[idx], errno,
                                      unlink_err_count, MAX_ERR_LOGS);
                }
            }
        }

        for (i = 0; i < num_files; i++) {
            if (fds[i] >= 0) {
                stress_osal_close(fds[i]);
                fds[i] = -1;
            }
        }

        for (i = 0; i < actual_limit && i < num_files; i++) {
            safe_unlink_quiet(filenames[i]);
        }

        stress_osal_sleep_ms(1);
    }

    stress_osal_print("rtos_stress: SUMMARY: [unlink-%d]"
                      " fd limit hit %d times, final limit=%d/%d\n",
                      args->instance, fd_limit_total, actual_limit, num_files);

cleanup:
    if (fds) {
        for (i = 0; i < num_files; i++) {
            if (fds[i] >= 0) stress_osal_close(fds[i]);
        }
        stress_osal_free(fds);
    }

    if (filenames) {
        for (i = 0; i < num_files; i++) {
            if (filenames[i]) {
                safe_unlink_quiet(filenames[i]);
                stress_osal_free(filenames[i]);
            }
        }
        stress_osal_free(filenames);
    }

    if (perm_idx) stress_osal_free(perm_idx);
    if (tmp_path) stress_osal_free(tmp_path);
}
