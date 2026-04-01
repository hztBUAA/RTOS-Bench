/* applications/stress-ng/stress-open.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stress-config.h>

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

static uint32_t stress_mwc32(void)
{
    return (uint32_t)stress_osal_rand();
}

static int do_open_creat(stress_args_t *args, char *filename, size_t file_idx)
{
    int flags = O_CREAT | O_RDWR;

    if (sizeof(open_flags) / sizeof(open_flags[0]) > 0) {
        flags |= open_flags[stress_mwc32() %
                            (sizeof(open_flags) / sizeof(open_flags[0]))];
    }

    stress_osal_snprintf(filename, PATH_MAX, FILENAME_TEMPLATE,
                         (int)args->instance, (int)file_idx);

    int fd = stress_osal_open(filename, flags, 0666);
    if (fd < 0) {
        filename[0] = '\0';
    }
    return fd;
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
    do_open_dev_zero,
};

#define NUM_OPEN_FUNCS  (sizeof(open_funcs) / sizeof(open_funcs[0]))

static void stress_open_clean_files(int *fds, char *filenames, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        if (fds[i] >= 0) {
            stress_osal_close(fds[i]);
            fds[i] = -1;
        }

        char *fname = filenames + (i * PATH_MAX);
        if (fname[0] != '\0') {
            /* 不删除 /dev/ 节点 */
            if (stress_osal_strncmp(fname, "/dev/", 5) != 0) {
                if (stress_osal_unlink(fname) != 0 && errno != ENOENT) {
                    /* 仅在非"文件不存在"的情况下记录警告 */
                    stress_osal_print("rtos_stress: warn: [open] unlink '%s'"
                                      " failed (errno=%d)\n",
                                      fname, errno);
                }
            }
            fname[0] = '\0';
        }
    }
}

void stress_open(stress_args_t *args)
{
    int   *fds       = NULL;
    char  *filenames = NULL;
    size_t open_max  = (size_t)s_open_max;

    if (open_max < 1)        open_max = 1;
    if (open_max > MAX_OPEN_MAX) open_max = MAX_OPEN_MAX;

    fds = (int *)stress_osal_malloc(open_max * sizeof(int));
    if (!fds) {
        stress_osal_print("rtos_stress: error: [open-%d] OOM allocating"
                          " fds array (%zu bytes)\n",
                          args->instance, open_max * sizeof(int));
        return;
    }

    filenames = (char *)stress_osal_malloc(open_max * PATH_MAX);
    if (!filenames) {
        stress_osal_free(fds);
        stress_osal_print("rtos_stress: error: [open-%d] OOM allocating"
                          " filenames array (%zu bytes)\n",
                          args->instance, open_max * PATH_MAX);
        return;
    }

    stress_osal_memset(fds,       -1, open_max * sizeof(int));
    stress_osal_memset(filenames,  0, open_max * PATH_MAX);

    stress_osal_print("rtos_stress: info: [open-%d] opening up to %zu files"
                      " per round\n",
                      args->instance, open_max);

    while (stress_continue(args)) {

        for (size_t i = 0; i < open_max; i++) {
            if (!stress_continue(args)) break;

            int    func_idx        = (int)(stress_mwc32() % NUM_OPEN_FUNCS);
            char  *current_filename = filenames + (i * PATH_MAX);

            current_filename[0] = '\0';

            fds[i] = open_funcs[func_idx](args, current_filename, i);

            if (fds[i] >= 0) {
                args->bogo.current_ops++;
            } else {
                if (errno == EMFILE || errno == ENFILE) {
                    stress_osal_print("rtos_stress: info: [open-%d] fd limit"
                                      " reached at slot %zu (errno=%d),"
                                      " flushing\n",
                                      args->instance, i, errno);
                    break;
                }
                stress_osal_print("rtos_stress: warn: [open-%d] open failed"
                                  " at slot %zu (errno=%d)\n",
                                  args->instance, i, errno);
            }
        }

        stress_open_clean_files(fds, filenames, open_max);

        stress_osal_sleep_ms(1);
    }

    stress_open_clean_files(fds, filenames, open_max);
    stress_osal_free(filenames);
    stress_osal_free(fds);
}
