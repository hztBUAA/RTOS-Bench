/* applications/stress-ng/stress-copy-file.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <config.h>

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

#ifndef EXIT_NO_RESOURCE
#define EXIT_NO_RESOURCE 2
#endif

#define CHUNK_SIZE              (4096)
#define FILENAME_BASE           "c"

#define UNLIKELY(x)             __builtin_expect(!!(x), 0)
#define LIKELY(x)               __builtin_expect(!!(x), 1)
#define STRESS_MINIMUM(a,b)     ((a) < (b) ? (a) : (b))

static uint64_t s_copy_file_bytes = DEFAULT_COPY_FILE_BYTES;

static int stress_copy_file_opt_bytes(const char *opt_name, const char *opt_arg)
{
    char *endptr;
    unsigned long long val = strtoull(opt_arg, &endptr, 10);

    if (*endptr == 'k' || *endptr == 'K') val *= 1024;
    else if (*endptr == 'm' || *endptr == 'M') val *= (1024 * 1024);
    else if (*endptr == 'g' || *endptr == 'G') val *= (1024 * 1024 * 1024);

    if (val < MIN_COPY_FILE_BYTES) val = MIN_COPY_FILE_BYTES;
    if (val > MAX_COPY_FILE_BYTES) val = MAX_COPY_FILE_BYTES;

    s_copy_file_bytes = (uint64_t)val;
    stress_osal_print("rtos_stress: debug: copy-file-bytes set to %llu bytes\n", (unsigned long long)s_copy_file_bytes);
    return 0;
}

const stress_opt_t stress_copy_file_opts[] = {
    { "copy-file-bytes", stress_copy_file_opt_bytes },
    { NULL, NULL }
};

static uint32_t stress_mwc32(void) { return (uint32_t)stress_osal_rand(); }
static uint8_t  stress_mwc8(void)  { return (uint8_t)stress_osal_rand(); }

static ssize_t stress_copy_file_range_emu(
    int fd_in, off_t *off_in,
    int fd_out, off_t *off_out,
    size_t len)
{
    char *buf;
    ssize_t total_copied = 0;
    ssize_t n_read, n_written;

    buf = (char *)stress_osal_malloc(CHUNK_SIZE);
    if (!buf) return -1;

    if (lseek(fd_in, *off_in, SEEK_SET) < 0) {
        stress_osal_free(buf);
        return -1;
    }
    if (lseek(fd_out, *off_out, SEEK_SET) < 0) {
        stress_osal_free(buf);
        return -1;
    }

    size_t bytes_left = len;
    while (bytes_left > 0) {
        size_t to_read = STRESS_MINIMUM(bytes_left, CHUNK_SIZE);

        n_read = stress_osal_read(fd_in, buf, to_read);
        if (n_read < 0) {
            total_copied = -1;
            break;
        }
        if (n_read == 0) break;

        n_written = stress_osal_write(fd_out, buf, n_read);
        if (n_written < 0) {
            total_copied = -1;
            break;
        }

        total_copied += n_written;
        *off_in += n_written;
        *off_out += n_written;
        bytes_left -= n_written;

        if (n_written < n_read) break;
    }

    stress_osal_free(buf);
    return total_copied;
}

static int stress_copy_file_fill(
    int fd, off_t offset, size_t size)
{
    char *buf;
    ssize_t written;
    size_t left = size;
    int rc = 0;

    buf = (char *)stress_osal_malloc(CHUNK_SIZE);
    if (!buf) return -1;

    stress_osal_memset(buf, stress_mwc8(), CHUNK_SIZE);

    if (lseek(fd, offset, SEEK_SET) < 0) {
        stress_osal_free(buf);
        return -1;
    }

    while (left > 0) {
        size_t to_write = STRESS_MINIMUM(left, CHUNK_SIZE);
        written = stress_osal_write(fd, buf, to_write);
        if (written < 0) {
            rc = -1;
            break;
        }
        left -= written;
    }

    stress_osal_free(buf);
    return rc;
}

static int stress_copy_file_verify(
    int fd_in, off_t off_in,
    int fd_out, off_t off_out,
    size_t len)
{
    char *buf_in, *buf_out;
    int rc = 0;
    size_t left = len;

    buf_in = (char *)stress_osal_malloc(CHUNK_SIZE);
    buf_out = (char *)stress_osal_malloc(CHUNK_SIZE);

    if (!buf_in || !buf_out) {
        if (buf_in) stress_osal_free(buf_in);
        if (buf_out) stress_osal_free(buf_out);
        return -1;
    }

    if (lseek(fd_in, off_in, SEEK_SET) < 0 || lseek(fd_out, off_out, SEEK_SET) < 0) {
        rc = -1;
        goto exit_free;
    }

    while (left > 0) {
        size_t check_len = STRESS_MINIMUM(left, CHUNK_SIZE);
        ssize_t n_in = stress_osal_read(fd_in, buf_in, check_len);
        ssize_t n_out = stress_osal_read(fd_out, buf_out, check_len);

        if (n_in != n_out || n_in < 0) {
            rc = -1;
            break;
        }

        if (memcmp(buf_in, buf_out, n_in) != 0) {
            rc = -1;
            break;
        }
        left -= n_in;
    }

exit_free:
    stress_osal_free(buf_in);
    stress_osal_free(buf_out);
    return rc;
}

void stress_copy_file(stress_args_t *args)
{
    int fd_in = -1, fd_out = -1;
    char filename_in[PATH_MAX];
    char filename_out[PATH_MAX];
    uint64_t file_bytes = s_copy_file_bytes;

    stress_osal_snprintf(filename_in, sizeof(filename_in), "%s_%d.orig", FILENAME_BASE, (int)args->instance);
    stress_osal_snprintf(filename_out, sizeof(filename_out), "%s_%d.copy", FILENAME_BASE, (int)args->instance);

    stress_osal_print("rtos_stress: info: [copy-file-%d] using %d MB files\n",
               args->instance, (int)(file_bytes / 1024 / 1024));

    fd_in = stress_osal_open(filename_in, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd_in < 0) {
        stress_osal_print("rtos_stress: fail: [copy-file] open %s failed\n", filename_in);
        return;
    }

    fd_out = stress_osal_open(filename_out, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd_out < 0) {
        stress_osal_print("rtos_stress: fail: [copy-file] open %s failed\n", filename_out);
        stress_osal_close(fd_in);
        stress_osal_unlink(filename_in);
        return;
    }

    while (stress_continue(args))
    {
        off_t off_in, off_out;
        off_t off_in_orig, off_out_orig;
        ssize_t copy_ret;

        if (file_bytes > CHUNK_SIZE) {
            off_in_orig = (off_t)(stress_mwc32() % (file_bytes - CHUNK_SIZE));
            off_out_orig = (off_t)(stress_mwc32() % (file_bytes - CHUNK_SIZE));
        } else {
            off_in_orig = 0;
            off_out_orig = 0;
        }

        off_in = off_in_orig;
        off_out = off_out_orig;

        if (stress_copy_file_fill(fd_in, off_in, CHUNK_SIZE) < 0) {
             stress_osal_print("rtos_stress: fail: [copy-file] fill failed\n");
             break;
        }

        if (!stress_continue(args)) break;

        copy_ret = stress_copy_file_range_emu(fd_in, &off_in, fd_out, &off_out, CHUNK_SIZE);

        if (copy_ret < 0) {
            if (errno == ENOSPC) {
                stress_osal_print("rtos_stress: warn: [copy-file] disk full, truncating files...\n");
                ftruncate(fd_in, 0);
                ftruncate(fd_out, 0);
                continue;
            }
            stress_osal_print("rtos_stress: fail: [copy-file] copy failed (errno=%d)\n", errno);
            break;
        }

        if (stress_copy_file_verify(fd_in, off_in_orig, fd_out, off_out_orig, copy_ret) < 0) {
             stress_osal_print("rtos_stress: fail: [copy-file] data corruption detected!\n");
             break;
        }

        if ((args->bogo.current_ops % 16) == 0) {
            stress_osal_fsync(fd_out);
        }

        args->bogo.current_ops++;
        stress_osal_sleep_ms(1);
    }

    stress_osal_close(fd_in);
    stress_osal_close(fd_out);
    stress_osal_unlink(filename_in);
    stress_osal_unlink(filename_out);
}
