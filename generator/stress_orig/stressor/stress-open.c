/* applications/stress-ng/stress-open.c */
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

#define OPEN_FILE_TEMPLATE      "o%d_%04d"

static int32_t s_open_max = DEFAULT_OPEN_MAX;

static int stress_open_opt_max(const char *opt_name, const char *opt_arg)
{
    (void)opt_name;
    int val = atoi(opt_arg);
    if (val < MIN_OPEN_MAX) val = MIN_OPEN_MAX;
    if (val > MAX_OPEN_MAX) val = MAX_OPEN_MAX;
    s_open_max = val;
    stress_osal_print("rtos_stress: debug: open-max set to %d\n", s_open_max);
    return 0;
}

const stress_opt_t stress_open_opts[] = {
    { "open-max", stress_open_opt_max },
    { NULL, NULL }
};

/* ------------------------------------------------------------------ */
/* 打开方式探测                                                        */
/* ------------------------------------------------------------------ */

typedef int (*open_func_t)(int instance, int idx);

static int open_method_creat(int instance, int idx)
{
    char path[PATH_MAX];
    stress_osal_snprintf(path, PATH_MAX, OPEN_FILE_TEMPLATE, instance, idx);
    return stress_osal_open(path, O_CREAT | O_RDWR, 0666);
}

static int open_method_creat_trunc(int instance, int idx)
{
    char path[PATH_MAX];
    stress_osal_snprintf(path, PATH_MAX, OPEN_FILE_TEMPLATE, instance, idx);
    return stress_osal_open(path, O_CREAT | O_RDWR | O_TRUNC, 0666);
}

static int open_method_creat_append(int instance, int idx)
{
    char path[PATH_MAX];
    stress_osal_snprintf(path, PATH_MAX, OPEN_FILE_TEMPLATE, instance, idx);
    return stress_osal_open(path, O_CREAT | O_RDWR | O_APPEND, 0666);
}

static int open_method_dev_null(int instance, int idx)
{
    (void)instance; (void)idx;
    return stress_osal_open("/dev/null", O_RDWR, 0);
}

static int open_method_dev_zero(int instance, int idx)
{
    (void)instance; (void)idx;
    return stress_osal_open("/dev/zero", O_RDONLY, 0);
}

#define MAX_OPEN_METHODS 5
static open_func_t s_open_methods[MAX_OPEN_METHODS];
static int         s_num_open_methods = 0;

static int s_open_probe_done = 0;

static void stress_open_probe_funcs(void)
{
    if (s_open_probe_done) return;
    s_open_probe_done = 1;

    s_num_open_methods = 0;

    /* 这 3 个方法在所有 RTOS 上都可用（只要 base dir 可写） */
    s_open_methods[s_num_open_methods++] = open_method_creat;
    s_open_methods[s_num_open_methods++] = open_method_creat_trunc;
    s_open_methods[s_num_open_methods++] = open_method_creat_append;

    /* 可选：/dev/null */
    {
        int fd = stress_osal_open("/dev/null", O_RDWR, 0);
        if (fd >= 0) {
            stress_osal_close(fd);
            s_open_methods[s_num_open_methods++] = open_method_dev_null;
        } else {
            stress_osal_print("rtos_stress: info: [open] /dev/null not available,"
                              " skipping\n");
        }
    }

    /* 可选：/dev/zero */
    {
        int fd = stress_osal_open("/dev/zero", O_RDONLY, 0);
        if (fd >= 0) {
            stress_osal_close(fd);
            s_open_methods[s_num_open_methods++] = open_method_dev_zero;
        } else {
            stress_osal_print("rtos_stress: info: [open] /dev/zero not available,"
                              " skipping\n");
        }
    }

    stress_osal_print("rtos_stress: info: [open] %d open methods active\n",
                      s_num_open_methods);
}

/* ------------------------------------------------------------------ */
/* 辅助                                                                */
/* ------------------------------------------------------------------ */

static uint32_t stress_mwc32(void) { return (uint32_t)stress_osal_rand(); }

static void safe_unlink_quiet(const char *path)
{
    if (stress_osal_unlink(path) != 0 && errno != ENOENT) {
        /* 静默 */
    }
}

/* ------------------------------------------------------------------ */
/* 主 stressor 函数                                                    */
/* ------------------------------------------------------------------ */

void stress_open(stress_args_t *args)
{
    int    max_fds  = s_open_max;
    int   *fds      = NULL;
    char **filenames = NULL;
    int    i;
    int    open_err_count = 0;
    #define MAX_ERR_LOGS 5

    /* 探测可用打开方式（全局只执行一次） */
    stress_open_probe_funcs();

    if (s_num_open_methods == 0) {
        stress_osal_print("rtos_stress: error: [open-%d] no open methods"
                          " available\n", args->instance);
        return;
    }

    /* 分配资源 */
    fds = (int *)stress_osal_malloc(max_fds * sizeof(int));
    filenames = (char **)stress_osal_malloc(max_fds * sizeof(char *));
    if (!fds || !filenames) {
        stress_osal_print("rtos_stress: error: [open-%d] OOM\n",
                          args->instance);
        if (fds) stress_osal_free(fds);
        if (filenames) stress_osal_free(filenames);
        return;
    }

    stress_osal_memset(filenames, 0, max_fds * sizeof(char *));
    for (i = 0; i < max_fds; i++) {
        fds[i] = -1;
        filenames[i] = (char *)stress_osal_malloc(PATH_MAX);
        if (!filenames[i]) {
            stress_osal_print("rtos_stress: error: [open-%d] OOM buf %d\n",
                              args->instance, i);
            goto cleanup;
        }
        stress_osal_snprintf(filenames[i], PATH_MAX,
                             OPEN_FILE_TEMPLATE,
                             (int)args->instance, i);
    }

    {
        char test_path[PATH_MAX];
        stress_osal_snprintf(test_path, PATH_MAX,
                             "__open_pre_%d.tmp",
                             (int)args->instance);

        int fd_test = -1;
        int retries = 3;

        while (retries-- > 0) {
            errno = 0;
            fd_test = stress_osal_open(test_path, O_CREAT | O_RDWR, 0666);
            if (fd_test >= 0) break;
            stress_osal_sleep_ms(100);
        }

        if (fd_test >= 0) {
            stress_osal_close(fd_test);
            stress_osal_unlink(test_path);
        } else {
            stress_osal_print("rtos_stress: warn: [open-%d] "
                              " not writable after retries (errno=%d),"
                              " skipping\n",
                              args->instance, errno);
            goto cleanup;
        }
    }
    /* =================================================================== */

    stress_osal_print("rtos_stress: info: [open-%d] opening up to %d files"
                      " per round\n",
                      args->instance, max_fds);

    /* ===== 主循环 ===== */
    while (stress_continue(args)) {
        int files_opened = 0;

        for (i = 0; i < max_fds; i++) {
            if (!stress_continue(args)) break;

            /* 随机选择一种打开方式 */
            int method_idx = (int)(stress_mwc32() % (uint32_t)s_num_open_methods);
            fds[i] = s_open_methods[method_idx](args->instance, i);

            if (fds[i] < 0) {
                if (errno == EMFILE || errno == ENFILE) {
                    /* fd 耗尽：正常压力行为，直接 break 去关闭 */
                    break;
                }
                if (open_err_count < MAX_ERR_LOGS) {
                    open_err_count++;
                    stress_osal_print("rtos_stress: warn: [open-%d]"
                                      " open failed (errno=%d) [%d/%d]\n",
                                      args->instance, errno,
                                      open_err_count, MAX_ERR_LOGS);
                }
                continue;
            }

            files_opened++;
            args->bogo.current_ops++;
        }

        /* 关闭本轮打开的所有 fd */
        for (i = 0; i < max_fds; i++) {
            if (fds[i] >= 0) {
                stress_osal_close(fds[i]);
                fds[i] = -1;
            }
        }

        /* 清理文件 */
        for (i = 0; i < max_fds; i++) {
            safe_unlink_quiet(filenames[i]);
        }

        if (files_opened == 0 && stress_continue(args)) {
            stress_osal_sleep_ms(10);
        }

        stress_osal_sleep_ms(1);
    }

cleanup:
    /* 确保所有 fd 关闭 */
    if (fds) {
        for (i = 0; i < max_fds; i++) {
            if (fds[i] >= 0) stress_osal_close(fds[i]);
        }
        stress_osal_free(fds);
    }

    /* 清理所有文件 */
    if (filenames) {
        for (i = 0; i < max_fds; i++) {
            if (filenames[i]) {
                safe_unlink_quiet(filenames[i]);
                stress_osal_free(filenames[i]);
            }
        }
        stress_osal_free(filenames);
    }
}

