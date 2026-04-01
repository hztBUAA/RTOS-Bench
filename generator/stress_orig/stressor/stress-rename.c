/* applications/stress-ng/stress-rename.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <stress-config.h>

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

/*
 * 文件当前以哪个路径命名的枚举，替代原来语义不清的 int 0/1。
 */
typedef enum {
    FILE_IS_A = 0,   /* 文件当前命名为 path_a */
    FILE_IS_B = 1    /* 文件当前命名为 path_b */
} rename_state_t;

/*
 * 连续 rename 失败超过此阈值，认为文件系统已异常，主动退出循环。
 */
#define RENAME_MAX_CONSECUTIVE_FAIL  16

static int32_t s_rename_file_size = DEFAULT_RENAME_FILE_SIZE;

static int stress_rename_opt_size(const char *opt_name, const char *opt_arg)
{
    char *endptr;
    unsigned long long val = strtoull(opt_arg, &endptr, 10);

    if (*endptr == 'k' || *endptr == 'K') val *= 1024ULL;
    else if (*endptr == 'm' || *endptr == 'M') val *= (1024ULL * 1024ULL);

    if (val > MAX_RENAME_FILE_SIZE) val = MAX_RENAME_FILE_SIZE;
    if (val < 1) val = 1;

    s_rename_file_size = (int32_t)val;
    stress_osal_print("rtos_stress: debug: rename-file-size set to %d bytes\n",
                      s_rename_file_size);
    return 0;
}

const stress_opt_t stress_rename_opts[] = {
    { "rename-file-size", stress_rename_opt_size },
    { NULL, NULL }
};

/*
 * 修复问题四辅助函数：
 * 对 unlink 返回值进行区分——ENOENT 为预期行为（安静忽略），
 * 其他错误记录 warn。
 */
static void safe_unlink(const char *path, int instance, const char *tag)
{
    if (stress_osal_unlink(path) != 0 && errno != ENOENT) {
        stress_osal_print("rtos_stress: warn: [rename-%d] %s unlink '%s'"
                          " failed (errno=%d)\n",
                          instance, tag, path, errno);
    }
}

void stress_rename(stress_args_t *args)
{
    char         dir_path[64];
    char         path_a[PATH_MAX];
    char         path_b[PATH_MAX];
    rename_state_t state = FILE_IS_A;
    int          fd;
    int          consecutive_fail = 0;

    /*
     * 注意：dir_path 是相对路径，创建在进程 CWD。
     * 若从系统盘目录运行，将打在系统盘 TPSFS。
     * 建议 cd 到数据分区后再启动。
     */
    stress_osal_snprintf(dir_path, sizeof(dir_path),
                         "R%d", (int)args->instance);
    stress_osal_snprintf(path_a,   sizeof(path_a),
                         "%s/a.dat", dir_path);
    stress_osal_snprintf(path_b,   sizeof(path_b),
                         "%s/b.dat", dir_path);

    /* ---- 创建工作目录 ---- */
    if (stress_osal_mkdir(dir_path, 0777) < 0 && errno != EEXIST) {
        stress_osal_print("rtos_stress: fail: [rename-%d] mkdir '%s'"
                          " failed (errno=%d)\n",
                          args->instance, dir_path, errno);
        return;
    }

    /*
     * 修复问题五：清理历史遗留文件，并记录非预期的失败。
     * ENOENT 是正常情况（上次正常退出已清理），静默忽略。
     */
    safe_unlink(path_a, args->instance, "pre-clean");
    safe_unlink(path_b, args->instance, "pre-clean");

    /* ---- 创建初始文件 ---- */
    fd = stress_osal_open(path_a, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd < 0) {
        stress_osal_print("rtos_stress: fail: [rename-%d] create '%s'"
                          " failed (errno=%d)\n",
                          args->instance, path_a, errno);
        stress_osal_rmdir(dir_path);
        return;
    }

    /*
     * 修复问题一：将 s_rename_file_size 字节内容写入文件。
     * 原代码创建 0 字节空文件，--rename-file-size 选项完全无效。
     * 写入实际内容使 rename 测试更接近真实场景（文件系统需维护数据块引用）。
     */
    if (s_rename_file_size > 0) {
        /* 以 4 KB 为单位分块写入，避免大文件一次性分配栈上 buf */
        char     write_buf[4096];
        int32_t  remaining = s_rename_file_size;
        int      write_ok  = 1;

        stress_osal_memset(write_buf, 0xAB, sizeof(write_buf));

        while (remaining > 0) {
            int32_t  chunk = remaining < (int32_t)sizeof(write_buf)
                             ? remaining
                             : (int32_t)sizeof(write_buf);
            ssize_t  ret   = stress_osal_write(fd, write_buf, (size_t)chunk);
            if (ret <= 0) {
                stress_osal_print("rtos_stress: fail: [rename-%d] write"
                                  " initial file failed (errno=%d)\n",
                                  args->instance, errno);
                write_ok = 0;
                break;
            }
            remaining -= (int32_t)ret;
        }

        stress_osal_close(fd);
        fd = -1;

        if (!write_ok) {
            safe_unlink(path_a, args->instance, "init-fail");
            stress_osal_rmdir(dir_path);
            return;
        }
    } else {
        stress_osal_close(fd);
        fd = -1;
    }

    stress_osal_print("rtos_stress: info: [rename-%d] using dir '%s',"
                      " file size %d bytes\n",
                      args->instance, dir_path, s_rename_file_size);

    /* ---- 主循环 ---- */
    while (stress_continue(args)) {
        const char *src = (state == FILE_IS_A) ? path_a : path_b;
        const char *dst = (state == FILE_IS_A) ? path_b : path_a;

        int ret = stress_osal_rename(src, dst);

        if (ret == 0) {
            state = (state == FILE_IS_A) ? FILE_IS_B : FILE_IS_A;
            args->bogo.current_ops++;
            consecutive_fail = 0;
        } else {
            stress_osal_print("rtos_stress: warn: [rename-%d]"
                              " rename('%s' -> '%s') failed (errno=%d)\n",
                              args->instance, src, dst, errno);

            consecutive_fail++;
            if (consecutive_fail >= RENAME_MAX_CONSECUTIVE_FAIL) {
                stress_osal_print("rtos_stress: fail: [rename-%d]"
                                  " %d consecutive rename failures,"
                                  " aborting\n",
                                  args->instance,
                                  RENAME_MAX_CONSECUTIVE_FAIL);
                break;
            }
        }

        stress_osal_sleep_ms(1);
    }

    /* ---- 清理 ---- */
    /*
     * 修复问题四：
     * 循环结束时，文件只可能以 path_a 或 path_b 之一存在。
     * 另一个 unlink 会返回 ENOENT，safe_unlink 会静默忽略。
     */
    safe_unlink(path_a, args->instance, "cleanup");
    safe_unlink(path_b, args->instance, "cleanup");

    if (stress_osal_rmdir(dir_path) != 0) {
        stress_osal_print("rtos_stress: warn: [rename-%d] rmdir '%s'"
                          " failed (errno=%d)\n",
                          args->instance, dir_path, errno);
    }
}
