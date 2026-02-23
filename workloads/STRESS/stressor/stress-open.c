/* applications/stress-ng/stress-open.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <config.h>

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

#ifndef EXIT_NO_RESOURCE
#define EXIT_NO_RESOURCE 2
#endif

#define FILENAME_TEMPLATE   "o%d_%d.tmp"

static const int open_flags[] = {
    0,
#if defined(O_APPEND)
    O_APPEND,
#endif
#if defined(O_TRUNC)
    O_TRUNC,
#endif
#if defined(O_EXCL)
    O_EXCL,
#endif
#if defined(O_NONBLOCK)
    O_NONBLOCK,
#endif
};

typedef int (*stress_open_func_t)(stress_args_t *args, char *filename, size_t file_idx);

static int32_t s_open_max = DEFAULT_OPEN_MAX;

static int stress_open_opt_max(const char *opt_name, const char *opt_arg)
{
    int val = atoi(opt_arg);
    if (val < 1) val = 1;

    s_open_max = val;
    stress_osal_print("rtos_stress: debug: open-max set to %d\n", s_open_max);
    return 0;
}

const stress_opt_t stress_open_opts[] = {
    { "open-max", stress_open_opt_max },
    { NULL, NULL }
};

static uint32_t stress_mwc32(void) { return (uint32_t)stress_osal_rand(); }

static int do_open_creat(stress_args_t *args, char *filename, size_t file_idx)
{
    int flags = O_CREAT | O_RDWR;

    if (sizeof(open_flags) > 0) {
        flags |= open_flags[stress_mwc32() % (sizeof(open_flags) / sizeof(open_flags[0]))];
    }

    stress_osal_snprintf(filename, PATH_MAX, FILENAME_TEMPLATE, (int)args->instance, (int)file_idx);

    return stress_osal_open(filename, flags, 0666);
}

static int do_open_dev_null(stress_args_t *args, char *filename, size_t file_idx)
{
    (void)args; (void)filename; (void)file_idx;
    return stress_osal_open("/dev/null", O_WRONLY, 0);
}

static int do_open_dev_zero(stress_args_t *args, char *filename, size_t file_idx)
{
    (void)args; (void)filename; (void)file_idx;
    return stress_osal_open("/dev/zero", O_RDONLY, 0);
}

static const stress_open_func_t open_funcs[] = {
    do_open_creat,
    do_open_creat,
    do_open_creat,
    do_open_dev_null,
    do_open_dev_zero
};

static void stress_open_clean_files(int *fds, char *filenames, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        if (fds[i] >= 0) {
            stress_osal_close(fds[i]);
            fds[i] = -1;
        }
        char *fname = filenames + (i * PATH_MAX);

        if (fname[0] != '\0') {
            if (stress_osal_strncmp(fname, "/dev/", 5) != 0) {
                stress_osal_unlink(fname);
            }
            fname[0] = '\0';
        }
    }
}

void stress_open(stress_args_t *args)
{
    int *fds = NULL;
    char *filenames = NULL;
    size_t open_max = (size_t)s_open_max;
    size_t fds_alloc_len = 0;

    if (open_max < 1) open_max = 1;
    if (open_max > MAX_OPEN_MAX) open_max = MAX_OPEN_MAX;

    fds_alloc_len = open_max * sizeof(int);
    fds = (int *)stress_osal_malloc(fds_alloc_len);
    if (!fds) {
        stress_osal_print("rtos_stress: error: [open] OOM allocating fds array\n");
        return;
    }

    size_t names_alloc_len = open_max * PATH_MAX;
    filenames = (char *)stress_osal_malloc(names_alloc_len);
    if (!filenames) {
        stress_osal_free(fds);
        stress_osal_print("rtos_stress: error: [open] OOM allocating filenames array\n");
        return;
    }

    stress_osal_memset(fds, -1, fds_alloc_len);
    stress_osal_memset(filenames, 0, names_alloc_len);

    stress_osal_print("rtos_stress: info: [open-%d] attempting to open up to %d files\n", args->instance, (int)open_max);

    while (stress_continue(args))
    {
        for (size_t i = 0; i < open_max; i++) {
            if (!stress_continue(args)) break;

            int func_idx = stress_mwc32() % (sizeof(open_funcs) / sizeof(open_funcs[0]));

            char *current_filename = filenames + (i * PATH_MAX);

            current_filename[0] = '\0';

            fds[i] = open_funcs[func_idx](args, current_filename, i);

            if (fds[i] >= 0) {
            } else {
                if (errno == EMFILE || errno == ENFILE) {
                    break;
                }
            }
            args->bogo.current_ops++;
        }

        stress_open_clean_files(fds, filenames, open_max);

        stress_osal_sleep_ms(1);
    }

    stress_open_clean_files(fds, filenames, open_max);
    stress_osal_free(filenames);
    stress_osal_free(fds);
}
