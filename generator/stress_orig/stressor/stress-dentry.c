/* applications/stress-ng/stress-dentry.c */
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

#ifndef EXIT_NO_RESOURCE
#define EXIT_NO_RESOURCE 2
#endif

static inline int __file_exists(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (fp != NULL) {
        fclose(fp);
        return 0;   /* 文件存在，对应 access() 返回 0 */
    }
    return -1;      /* 文件不存在 */
}
#define FILE_ACCESS(path, mode) __file_exists(path)

#define FILENAME_TEMPLATE   "d%d_%x"

#define UNLIKELY(x)         __builtin_expect(!!(x), 0)
#define LIKELY(x)           __builtin_expect(!!(x), 1)

static int32_t s_dentries = DEFAULT_DENTRIES;

static int stress_dentry_opt_dentries(const char *opt_name, const char *opt_arg)
{
    int val = atoi(opt_arg);

    if (val < MIN_DENTRIES) val = MIN_DENTRIES;
    if (val > MAX_DENTRIES) val = MAX_DENTRIES;

    s_dentries = val;
    stress_osal_print("rtos_stress: debug: dentries set to %d\n", s_dentries);
    return 0;
}

const stress_opt_t stress_dentry_opts[] = {
    { "dentries", stress_dentry_opt_dentries },
    { NULL, NULL }
};

static uint32_t stress_mwc32(void) { return (uint32_t)stress_osal_rand(); }

static void stress_shuffle_indices(int *array, int n)
{
    if (n > 1) {
        for (int i = 0; i < n - 1; i++) {
            int j = i + stress_mwc32() / (0xFFFFFFFF / (n - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

static inline uint64_t gray_code(uint64_t n)
{
    return (n >> 1) ^ n;
}

static int stress_dentry_unlink(
    stress_args_t *args,
    int num_dentries,
    char *dir_path,
    int *indices,
    int verify)
{
    char path[PATH_MAX];
    int rc = 0;

    stress_shuffle_indices(indices, num_dentries);

    for (int i = 0; i < num_dentries; i++) {
        if (!stress_continue(args)) break;

        int idx = indices[i];
        uint64_t gc = gray_code(idx);

        stress_osal_snprintf(path, PATH_MAX, "%s/" FILENAME_TEMPLATE, dir_path, (int)args->instance, (unsigned int)gc);

        if (verify) {
            int fd = stress_osal_open(path, O_RDONLY, 0);
            if (fd >= 0) {
                uint64_t val = 0;
                if (stress_osal_read(fd, &val, sizeof(val)) == sizeof(val)) {
                    if (val != gc) {
                        stress_osal_print("rtos_stress: fail: [dentry] content mismatch %x vs %x\n", (unsigned int)val, (unsigned int)gc);
                        rc = -1;
                    }
                }
                stress_osal_close(fd);
            }
        }

        if (stress_osal_unlink(path) < 0) {
            if (errno != ENOENT) {
            }
        }
        args->bogo.current_ops++;
    }
    return rc;
}

void stress_dentry(stress_args_t *args)
{
    char dir_path[PATH_MAX];
    char path[PATH_MAX];
    int num_dentries = s_dentries;
    int *indices = NULL;
    int verify = 1;

    indices = (int *)stress_osal_malloc(num_dentries * sizeof(int));
    if (!indices) {
        stress_osal_print("rtos_stress: error: [dentry] OOM allocating indices\n");
        return;
    }
    for(int i=0; i<num_dentries; i++) indices[i] = i;

    stress_osal_snprintf(dir_path, sizeof(dir_path), "stress_dentry_%d_dir", (int)args->instance);

    if (stress_osal_mkdir(dir_path, 0777) < 0 && errno != EEXIST) {
        stress_osal_print("rtos_stress: error: [dentry] failed to create dir %s (errno=%d)\n", dir_path, errno);
        stress_osal_free(indices);
        return;
    }

    stress_osal_print("rtos_stress: info: [dentry-%d] using %d dentries in %s\n",
               args->instance, num_dentries, dir_path);

    while (stress_continue(args))
    {
        int created_count = 0;

        for (int i = 0; i < num_dentries; i++) {
            if (UNLIKELY(!stress_continue(args))) break;

            uint64_t gc = gray_code(i);
            stress_osal_snprintf(path, PATH_MAX, "%s/" FILENAME_TEMPLATE, dir_path, (int)args->instance, (unsigned int)gc);

            int fd = stress_osal_open(path, O_CREAT | O_RDWR, 0666);
            if (fd < 0) {
                if (errno == ENOSPC) {
                    break;
                }
            } else {
                if (verify) {
                    stress_osal_write(fd, &gc, sizeof(gc));
                }
                stress_osal_close(fd);
                created_count++;
            }
            args->bogo.current_ops++;
        }

        for (int i = 0; i < (num_dentries / 4); i++) {
            if (UNLIKELY(!stress_continue(args))) break;
            uint64_t bogus_gc = gray_code(num_dentries + i + 100);
            stress_osal_snprintf(path, PATH_MAX, "%s/" FILENAME_TEMPLATE, dir_path, (int)args->instance, (unsigned int)bogus_gc);
            if (FILE_ACCESS(path, F_OK) == 0) {
            }
            args->bogo.current_ops++;
        }

        if (stress_dentry_unlink(args, created_count, dir_path, indices, verify) < 0) {
            break;
        }

        stress_osal_sleep_ms(1);
    }

    for (int i = 0; i < num_dentries; i++) {
        uint64_t gc = gray_code(i);
        stress_osal_snprintf(path, PATH_MAX, "%s/" FILENAME_TEMPLATE, dir_path, (int)args->instance, (unsigned int)gc);
        stress_osal_unlink(path);
    }

    stress_osal_rmdir(dir_path);
    stress_osal_free(indices);
}
