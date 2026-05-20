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
    (void)opt_name;
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
    int    num_files = s_unlink_files;
    char **filenames = NULL;
    int   *fds       = NULL;
    int   *perm_idx  = NULL;
    int    i;
    uint32_t fd_limit_count     = 0;
    uint32_t fd_limit_log_max   = 3;
    uint32_t open_fail_count    = 0;
    uint32_t open_fail_log_max  = 5;
    uint32_t unlink_fail_count  = 0;
    uint32_t unlink_fail_log_max = 5;
    int      adapted_files      = num_files;
    uint32_t consecutive_zero   = 0;

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

        /*
         * ============================================================
         * Phase A: 创建 + 打开文件
         * ============================================================
         * 用 adapted_files 而非 num_files，自适应 fd 上限。
         */
        for (i = 0; i < adapted_files; i++) {
            if (UNLIKELY(!stress_continue(args))) break;

            int flags = O_CREAT | O_RDWR;
            if (stress_mwc32() & 1) flags |= O_TRUNC;

            fds[i] = stress_osal_open(filenames[i], flags, 0666);

            if (fds[i] < 0) {
                if (errno == EMFILE || errno == ENFILE) {
                    fd_limit_count++;
                    /* 自适应：下次少开一些 */
                    adapted_files = (i > 2) ? i : 2;
                    if (fd_limit_count <= fd_limit_log_max) {
                        stress_osal_print("rtos_stress: info: [unlink-%d]"
                                          " fd limit at slot %d,"
                                          " adapted to %d files [%u/%u]\n",
                                          args->instance, i,
                                          adapted_files,
                                          fd_limit_count, fd_limit_log_max);
                    }
                    break;
                }
                open_fail_count++;
                if (open_fail_count <= open_fail_log_max) {
                    stress_osal_print("rtos_stress: warn: [unlink-%d]"
                                      " open '%s' errno=%d [%u/%u]\n",
                                      args->instance, filenames[i], errno,
                                      open_fail_count, open_fail_log_max);
                }
                continue;
            }

            files_opened++;

            /* 偶尔写点数据给文件增加负载 */
            if ((i & (FSYNC_STRIDE - 1)) == 0) {
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
            continue;  /* 跳过后续阶段 */
        }
        consecutive_zero = 0;

        /*
         * ============================================================
         * Phase B: 关闭全部 fd（必须在 unlink 之前！）
         *
         * OneOS / 很多 RTOS 文件系统不支持 unlink 已打开的文件
         * （返回 EBUSY / errno=16）。
         * 必须先 close 再 unlink。
         * ============================================================
         */
        for (i = 0; i < adapted_files; i++) {
            if (fds[i] >= 0) {
                stress_osal_close(fds[i]);
                fds[i] = -1;
            }
        }

        /*
         * ============================================================
         * Phase C: 随机顺序 unlink
         *
         * 所有 fd 已关闭，unlink 不会遇到 EBUSY。
         * ============================================================
         */
        stress_unlink_shuffle(perm_idx, adapted_files);
        for (i = 0; i < adapted_files; i++) {
            if (UNLIKELY(!stress_continue(args))) break;

            int idx = perm_idx[i];

            if (stress_osal_unlink(filenames[idx]) == 0) {
                args->bogo.current_ops++;
            } else if (errno != ENOENT) {
                unlink_fail_count++;
                if (unlink_fail_count <= unlink_fail_log_max) {
                    stress_osal_print("rtos_stress: warn: [unlink-%d]"
                                      " unlink '%s' errno=%d [%u/%u]\n",
                                      args->instance, filenames[idx], errno,
                                      unlink_fail_count, unlink_fail_log_max);
                }
            }
        }

        /*
         * ============================================================
         * Phase D: 安全清理（兜底）
         *
         * 处理 Phase C 因 stress_continue=false 提前退出遗留的文件。
         * ============================================================
         */
        for (i = 0; i < adapted_files; i++) {
            if (fds[i] >= 0) {
                stress_osal_close(fds[i]);
                fds[i] = -1;
            }
        }
        for (i = 0; i < adapted_files; i++) {
            stress_osal_unlink(filenames[i]);
        }

        stress_osal_sleep_ms(1);
    }

    stress_osal_print("rtos_stress: SUMMARY: [unlink-%d]"
                      " fd limit hit %u times,"
                      " final limit=%d/%d\n",
                      args->instance,
                      fd_limit_count,
                      adapted_files, num_files);

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
