/* applications/stress-ng/stress-unlink.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <config.h>

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

#define FILENAME_TEMPLATE       "u%d_%04d"

#define UNLIKELY(x)             __builtin_expect(!!(x), 0)
#define LIKELY(x)               __builtin_expect(!!(x), 1)

static int32_t s_unlink_files = DEFAULT_UNLINK_FILES;

static int stress_unlink_opt_files(const char *opt_name, const char *opt_arg)
{
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
        int j = stress_mwc32() % size;
        int tmp = idx[i];
        idx[i] = idx[j];
        idx[j] = tmp;
    }
}

void stress_unlink(stress_args_t *args)
{
    int num_files = s_unlink_files;
    char **filenames = NULL;
    int *fds = NULL;
    int *perm_idx = NULL;

    filenames = (char **)stress_osal_malloc(num_files * sizeof(char *));
    fds = (int *)stress_osal_malloc(num_files * sizeof(int));
    perm_idx = (int *)stress_osal_malloc(num_files * sizeof(int));

    if (!filenames || !fds || !perm_idx) {
        stress_osal_print("rtos_stress: error: [unlink] OOM allocating arrays\n");
        goto cleanup;
    }

    stress_osal_memset(filenames, 0, num_files * sizeof(char *));
    for (int i = 0; i < num_files; i++) {
        fds[i] = -1;
        perm_idx[i] = i;
        filenames[i] = (char *)stress_osal_malloc(PATH_MAX);
        if (!filenames[i]) {
            stress_osal_print("rtos_stress: error: [unlink] OOM allocating filename buffers\n");
            goto cleanup;
        }
        stress_osal_snprintf(filenames[i], PATH_MAX, FILENAME_TEMPLATE, (int)args->instance, i);
    }

    stress_osal_print("rtos_stress: info: [unlink-%d] starting with %d files\n", args->instance, num_files);

    while (stress_continue(args))
    {
        int i;
        int files_opened = 0;

        for (i = 0; i < num_files; i++) {
            if (UNLIKELY(!stress_continue(args))) break;

            int flags = O_CREAT | O_RDWR;
            if (stress_mwc32() & 1) flags |= O_TRUNC;
            if (stress_mwc32() & 1) flags |= O_EXCL;

            fds[i] = stress_osal_open(filenames[i], flags, 0666);

            if (fds[i] < 0) {
                if (errno == EEXIST) {
                    fds[i] = stress_osal_open(filenames[i], O_RDWR, 0666);
                } else if (errno == EMFILE || errno == ENFILE) {
                    break;
                }
            }

            if (fds[i] >= 0) {
                files_opened++;

                if ((i & 0x1F) == 0) {
                    stress_osal_fsync(fds[i]);
                }
            }
            args->bogo.current_ops++;
        }

        if (files_opened == 0 && i > 0) {
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
            stress_osal_unlink(filenames[idx]);
            args->bogo.current_ops++;
        }

        for (i = 0; i < num_files; i++) {
            if (fds[i] >= 0) {
                stress_osal_close(fds[i]);
                fds[i] = -1;
            }
        }

        for (i = 0; i < num_files; i++) {
            stress_osal_unlink(filenames[i]);
        }

        stress_osal_sleep_ms(1);
    }

cleanup:
    if (fds) {
        for (int i = 0; i < num_files; i++) {
            if (fds[i] >= 0) stress_osal_close(fds[i]);
        }
        stress_osal_free(fds);
    }
    if (filenames) {
        for (int i = 0; i < num_files; i++) {
            if (filenames[i]) {
                stress_osal_unlink(filenames[i]);
                stress_osal_free(filenames[i]);
            }
        }
        stress_osal_free(filenames);
    }
    if (perm_idx) stress_osal_free(perm_idx);
}
