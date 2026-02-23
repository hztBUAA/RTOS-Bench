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
#include <config.h>

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

static int32_t s_rename_file_size = DEFAULT_RENAME_FILE_SIZE;

static int stress_rename_opt_size(const char *opt_name, const char *opt_arg)
{
    char *endptr;
    unsigned long long val = strtoull(opt_arg, &endptr, 10);

    if (*endptr == 'k' || *endptr == 'K') val *= 1024;
    else if (*endptr == 'm' || *endptr == 'M') val *= (1024 * 1024);

    if (val > MAX_RENAME_FILE_SIZE) val = MAX_RENAME_FILE_SIZE;

    s_rename_file_size = (int32_t)val;
    stress_osal_print("rtos_stress: debug: rename-file-size set to %d bytes\n", s_rename_file_size);
    return 0;
}

const stress_opt_t stress_rename_opts[] = {
    { "rename-file-size", stress_rename_opt_size },
    { NULL, NULL }
};

void stress_rename(stress_args_t *args)
{
    char dir_path[64];
    char path_a[PATH_MAX];
    char path_b[PATH_MAX];
    int current_state = 0;
    int fd;

    stress_osal_snprintf(dir_path, sizeof(dir_path), "R%d", (int)args->instance);

    if (stress_osal_mkdir(dir_path, 0777) < 0 && errno != EEXIST) {
        return;
    }

    stress_osal_snprintf(path_a, sizeof(path_a), "%s/a.dat", dir_path);
    stress_osal_snprintf(path_b, sizeof(path_b), "%s/b.dat", dir_path);

    stress_osal_unlink(path_a);
    stress_osal_unlink(path_b);

    fd = stress_osal_open(path_a, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd < 0) {
        stress_osal_rmdir(dir_path);
        return;
    }
    stress_osal_close(fd);

    while (stress_continue(args))
    {
        int ret;
        if (current_state == 0) {
            ret = stress_osal_rename(path_a, path_b);
        } else {
            ret = stress_osal_rename(path_b, path_a);
        }

        if (ret == 0) {
            current_state = !current_state;
            args->bogo.current_ops++;
        }

        stress_osal_sleep_ms(1);
    }

    stress_osal_unlink(path_a);
    stress_osal_unlink(path_b);
    stress_osal_rmdir(dir_path);
}
