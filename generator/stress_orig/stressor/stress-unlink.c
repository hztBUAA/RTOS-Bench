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

#define FILENAME_TEMPLATE       "u%d_%04d"

#define UNLIKELY(x)             __builtin_expect(!!(x), 0)
#define LIKELY(x)               __builtin_expect(!!(x), 1)

#define FSYNC_STRIDE            (1 << 5)

static int32_t s_unlink_files = DEFAULT_UNLINK_FILES;

static int stress_unlink_opt_files(const char *opt_name, const char *opt_arg)
{
    int val = atoi(opt_arg);
    if (val < MIN_UNLINK_FILES) val = MIN_UNLINK_FILES;
    if (val > MAX_UNLINK_FILES) val = MAX_UNLINK_FILES;

    s_unlink_files = val;
    stress_osal_print("rtos_stress: debug: unlink-files set to %d\n",
                      s_unlink_files);
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

static void safe_unlink(const char *path, int instance, const char *tag)
{
    if (stress_osal_unlink(path) != 0 && errno != ENOENT) {
        stress_osal_print("rtos_stress: warn: [unlink-%d] %s unlink '%s'"
                          " failed (errno=%d)\n",
                          instance, tag, path, errno);
    }
}

void stress_unlink(stress_args_t *args)
{
    int   num_files = s_unlink_files;
    char **filenames = NULL;
    int   *fds       = NULL;
    int   *perm_idx  = NULL;
    int    i;

    filenames = (char **)stress_osal_malloc(num_files * sizeof(char *));
    fds       = (int  *)stress_osal_malloc(num_files * sizeof(int));
    perm_idx  = (int  *)stress_osal_malloc(num_files * sizeof(int));

    if (!filenames || !fds || !perm_idx) {
        stress_osal_print("rtos_stress: error: [unlink-%d] OOM allocating"
                          " arrays\n", args->instance);
        goto cleanup;
    }

    stress_osal_memset(filenames, 0, num_files * sizeof(char *));
    for (i = 0; i < num_files; i++) {
        fds[i]      = -1;
        perm_idx[i] = i;

        filenames[i] = (char *)stress_osal_malloc(PATH_MAX);
        if (!filenames[i]) {
            stress_osal_print("rtos_stress: error: [unlink-%d] OOM"
                              " allocating filename buffer %d\n",
                              args->instance, i);
            goto cleanup;
        }
        stress_osal_snprintf(filenames[i], PATH_MAX,
                             FILENAME_TEMPLATE,
                             (int)args->instance, i);
    }

    stress_osal_print("rtos_stress: info: [unlink-%d] starting with"
                      " %d files\n",
                      args->instance, num_files);

    while (stress_continue(args)) {
        int files_opened = 0;

        for (i = 0; i < num_files; i++) {
            if (UNLIKELY(!stress_continue(args))) break;

            int flags = O_CREAT | O_RDWR;
            if (stress_mwc32() & 1) flags |= O_TRUNC;

            fds[i] = stress_osal_open(filenames[i], flags, 0666);

            if (fds[i] < 0) {
                if (errno == EMFILE || errno == ENFILE) {
                    stress_osal_print("rtos_stress: info: [unlink-%d]"
                                      " fd limit at slot %d (errno=%d),"
                                      " flushing\n",
                                      args->instance, i, errno);
                    break;
                }
                /* 其他失败：记录 warn，不自增 ops */
                stress_osal_print("rtos_stress: warn: [unlink-%d]"
                                  " open '%s' failed (errno=%d)\n",
                                  args->instance, filenames[i], errno);
                continue;
            }

            files_opened++;

            if ((i & (FSYNC_STRIDE - 1)) == 0) {
                if (stress_osal_fsync(fds[i]) != 0) {
                    stress_osal_print("rtos_stress: warn: [unlink-%d]"
                                      " fsync fd[%d] failed (errno=%d)\n",
                                      args->instance, i, errno);
                }
            }
            args->bogo.current_ops++;
        }

        if (files_opened == 0) {
            stress_osal_sleep_ms(10);
        }

        stress_unlink_shuffle(perm_idx, num_files);
        for (i = 0; i < num_files; i += 8) {
            int idx = perm_idx[i];
            if (fds[idx] >= 0) {
                stress_osal_close(fds[idx]);
                fds[idx] = -1;
            }
        }

        stress_unlink_shuffle(perm_idx, num_files);
        for (i = 0; i < num_files; i++) {
            if (UNLIKELY(!stress_continue(args))) break;

            int idx = perm_idx[i];

            if (stress_osal_unlink(filenames[idx]) == 0) {
                args->bogo.current_ops++;
            } else if (errno != ENOENT) {
                stress_osal_print("rtos_stress: warn: [unlink-%d]"
                                  " unlink '%s' failed (errno=%d)\n",
                                  args->instance, filenames[idx], errno);
            }
        }

        for (i = 0; i < num_files; i++) {
            if (fds[i] >= 0) {
                stress_osal_close(fds[i]);
                fds[i] = -1;
            }
        }

        for (i = 0; i < num_files; i++) {
            safe_unlink(filenames[i], args->instance, "loop-cleanup");
        }

        stress_osal_sleep_ms(1);
    }

cleanup:
    if (fds) {
        for (i = 0; i < num_files; i++) {
            if (fds[i] >= 0) {
                stress_osal_close(fds[i]);
            }
        }
        stress_osal_free(fds);
    }

    if (filenames) {
        for (i = 0; i < num_files; i++) {
            if (filenames[i]) {
                safe_unlink(filenames[i], args->instance, "final-cleanup");
                stress_osal_free(filenames[i]);
            }
        }
        stress_osal_free(filenames);
    }

    if (perm_idx) {
        stress_osal_free(perm_idx);
    }
}
