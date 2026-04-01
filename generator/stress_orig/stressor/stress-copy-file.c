/* applications/stress-ng/stress-copy-file.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <stress-config.h>

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
#define STRESS_MINIMUM(a, b)    ((a) < (b) ? (a) : (b))

static uint64_t s_copy_file_bytes = DEFAULT_COPY_FILE_BYTES;

static int stress_copy_file_opt_bytes(const char *opt_name, const char *opt_arg)
{
    char *endptr;
    unsigned long long val = strtoull(opt_arg, &endptr, 10);

    if      (*endptr == 'k' || *endptr == 'K') val *= 1024ULL;
    else if (*endptr == 'm' || *endptr == 'M') val *= (1024ULL * 1024ULL);
    else if (*endptr == 'g' || *endptr == 'G') val *= (1024ULL * 1024ULL * 1024ULL);

    if (val < MIN_COPY_FILE_BYTES) val = MIN_COPY_FILE_BYTES;
    if (val > MAX_COPY_FILE_BYTES) val = MAX_COPY_FILE_BYTES;

    s_copy_file_bytes = (uint64_t)val;
    stress_osal_print("rtos_stress: debug: copy-file-bytes set to %llu bytes\n",
                      (unsigned long long)s_copy_file_bytes);
    return 0;
}

const stress_opt_t stress_copy_file_opts[] = {
    { "copy-file-bytes", stress_copy_file_opt_bytes },
    { NULL, NULL }
};

static uint32_t stress_mwc32(void) { return (uint32_t)stress_osal_rand(); }
static uint8_t  stress_mwc8(void)  { return (uint8_t)stress_osal_rand(); }

/* ------------------------------------------------------------------ */
/* 辅助函数                                                            */
/* ------------------------------------------------------------------ */

static void safe_unlink(const char *path, int instance, const char *tag)
{
    if (stress_osal_unlink(path) != 0 && errno != ENOENT) {
        stress_osal_print("rtos_stress: warn: [copy-file-%d] %s unlink '%s'"
                          " failed (errno=%d)\n",
                          instance, tag, path, errno);
    }
}

static int stress_copy_file_fill(int fd, off_t offset, size_t size)
{
    char   *buf;
    size_t  left = size;
    int     rc   = 0;

    buf = (char *)stress_osal_malloc(CHUNK_SIZE);
    if (!buf) return -1;

    if (lseek(fd, offset, SEEK_SET) < 0) {
        stress_osal_free(buf);
        return -1;
    }

    while (left > 0) {
        size_t  to_write = STRESS_MINIMUM(left, (size_t)CHUNK_SIZE);

        stress_osal_memset(buf, stress_mwc8(), to_write);

        ssize_t written = stress_osal_write(fd, buf, to_write);
        if (written <= 0) {
            rc = -1;
            break;
        }
        left -= (size_t)written;
    }

    stress_osal_free(buf);
    return rc;
}

static ssize_t stress_copy_file_range_emu(
    int fd_in,  off_t *off_in,
    int fd_out, off_t *off_out,
    size_t len)
{
    char   *buf;
    ssize_t total_copied = 0;

    buf = (char *)stress_osal_malloc(CHUNK_SIZE);
    if (!buf) return -1;

    if (lseek(fd_in,  *off_in,  SEEK_SET) < 0 ||
        lseek(fd_out, *off_out, SEEK_SET) < 0) {
        stress_osal_free(buf);
        return -1;
    }

    size_t bytes_left = len;
    while (bytes_left > 0) {
        size_t  to_read   = STRESS_MINIMUM(bytes_left, (size_t)CHUNK_SIZE);

        ssize_t n_read    = stress_osal_read(fd_in, buf, to_read);
        if (n_read < 0)  { total_copied = -1; break; }
        if (n_read == 0) { break; }           /* 源文件 EOF，正常终止 */

        ssize_t n_written = stress_osal_write(fd_out, buf, (size_t)n_read);
        if (n_written < 0) { total_copied = -1; break; }

        total_copied += n_written;

        *off_in  += (off_t)n_read;
        *off_out += (off_t)n_written;
        bytes_left -= (size_t)n_written;

        if (n_written < n_read) break;
    }

    stress_osal_free(buf);
    return total_copied;
}

static int stress_copy_file_verify(
    int fd_in,  off_t off_in,
    int fd_out, off_t off_out,
    size_t len)
{
    char *buf_in  = NULL;
    char *buf_out = NULL;
    int   rc      = 0;
    size_t left   = len;

    if (len == 0) return 0;

    buf_in  = (char *)stress_osal_malloc(CHUNK_SIZE);
    buf_out = (char *)stress_osal_malloc(CHUNK_SIZE);

    if (!buf_in || !buf_out) {
        stress_osal_free(buf_in);
        stress_osal_free(buf_out);
        return -1;
    }

    if (lseek(fd_in,  off_in,  SEEK_SET) < 0 ||
        lseek(fd_out, off_out, SEEK_SET) < 0) {
        rc = -1;
        goto exit_free;
    }

    while (left > 0) {
        size_t  check_len = STRESS_MINIMUM(left, (size_t)CHUNK_SIZE);
        ssize_t n_in      = stress_osal_read(fd_in,  buf_in,  check_len);
        ssize_t n_out     = stress_osal_read(fd_out, buf_out, check_len);

        if (n_in < 0 || n_out < 0) {
            rc = -1;
            break;
        }


        if (n_in == 0 || n_out == 0 || n_in != n_out) {
            rc = -1;
            break;
        }

        if (memcmp(buf_in, buf_out, (size_t)n_in) != 0) {
            rc = -1;
            break;
        }

        left -= (size_t)n_in;
    }

exit_free:
    stress_osal_free(buf_in);
    stress_osal_free(buf_out);
    return rc;
}

/* ------------------------------------------------------------------ */
/* 主体                                                                */
/* ------------------------------------------------------------------ */

void stress_copy_file(stress_args_t *args)
{
    int      fd_in  = -1;
    int      fd_out = -1;
    char     filename_in [PATH_MAX];
    char     filename_out[PATH_MAX];
    uint64_t file_bytes = s_copy_file_bytes;

    stress_osal_snprintf(filename_in,  sizeof(filename_in),
                         "%s_%d.orig", FILENAME_BASE, (int)args->instance);
    stress_osal_snprintf(filename_out, sizeof(filename_out),
                         "%s_%d.copy", FILENAME_BASE, (int)args->instance);

    stress_osal_print("rtos_stress: info: [copy-file-%d] file size %llu bytes\n",
                      args->instance, (unsigned long long)file_bytes);

    fd_in = stress_osal_open(filename_in, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd_in < 0) {
        stress_osal_print("rtos_stress: fail: [copy-file-%d] open '%s'"
                          " failed (errno=%d)\n",
                          args->instance, filename_in, errno);
        return;
    }

    fd_out = stress_osal_open(filename_out, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd_out < 0) {
        stress_osal_print("rtos_stress: fail: [copy-file-%d] open '%s'"
                          " failed (errno=%d)\n",
                          args->instance, filename_out, errno);
        stress_osal_close(fd_in);
        safe_unlink(filename_in, args->instance, "init-fail");
        return;
    }

    stress_osal_print("rtos_stress: info: [copy-file-%d] pre-allocating"
                      " %llu bytes...\n",
                      args->instance, (unsigned long long)file_bytes);

    if (stress_copy_file_fill(fd_in,  0, (size_t)file_bytes) < 0 ||
        stress_copy_file_fill(fd_out, 0, (size_t)file_bytes) < 0) {
        stress_osal_print("rtos_stress: fail: [copy-file-%d] pre-alloc"
                          " failed (errno=%d)\n",
                          args->instance, errno);
        goto cleanup;
    }

    stress_osal_print("rtos_stress: info: [copy-file-%d] pre-alloc done,"
                      " starting copy loop\n",
                      args->instance);

    while (stress_continue(args)) {
        off_t   off_in_orig, off_out_orig;
        off_t   off_in,      off_out;
        ssize_t copy_ret;

        if (file_bytes > (uint64_t)CHUNK_SIZE) {
            off_in_orig  = (off_t)(stress_mwc32() %
                                   (uint32_t)(file_bytes - CHUNK_SIZE));
            off_out_orig = (off_t)(stress_mwc32() %
                                   (uint32_t)(file_bytes - CHUNK_SIZE));
        } else {
            off_in_orig  = 0;
            off_out_orig = 0;
        }

        off_in  = off_in_orig;
        off_out = off_out_orig;

        if (stress_copy_file_fill(fd_in, off_in_orig, CHUNK_SIZE) < 0) {
            stress_osal_print("rtos_stress: fail: [copy-file-%d] fill"
                              " failed (errno=%d)\n",
                              args->instance, errno);
            break;
        }

        if (!stress_continue(args)) break;

        copy_ret = stress_copy_file_range_emu(fd_in,  &off_in,
                                              fd_out, &off_out,
                                              CHUNK_SIZE);
        if (copy_ret < 0) {
            if (errno == ENOSPC) {
                stress_osal_print("rtos_stress: warn: [copy-file-%d]"
                                  " disk full, truncating files\n",
                                  args->instance);
                if (stress_osal_ftruncate(fd_in,  0) != 0 ||
                    stress_osal_ftruncate(fd_out, 0) != 0) {
                    stress_osal_print("rtos_stress: fail: [copy-file-%d]"
                                      " ftruncate failed (errno=%d)\n",
                                      args->instance, errno);
                    break;
                }
                if (stress_copy_file_fill(fd_in,  0, (size_t)file_bytes) < 0 ||
                    stress_copy_file_fill(fd_out, 0, (size_t)file_bytes) < 0) {
                    stress_osal_print("rtos_stress: fail: [copy-file-%d]"
                                      " re-alloc after ENOSPC failed\n",
                                      args->instance);
                    break;
                }
                continue;
            }
            stress_osal_print("rtos_stress: fail: [copy-file-%d] copy"
                              " failed (errno=%d)\n",
                              args->instance, errno);
            break;
        }

        if (copy_ret == 0) {
            stress_osal_print("rtos_stress: warn: [copy-file-%d] copy"
                              " returned 0 bytes\n",
                              args->instance);
            continue;
        }

        if (stress_copy_file_verify(fd_in,  off_in_orig,
                                    fd_out, off_out_orig,
                                    (size_t)copy_ret) < 0) {
            stress_osal_print("rtos_stress: fail: [copy-file-%d]"
                              " data corruption detected!"
                              " in_off=%llu out_off=%llu len=%lld\n",
                              args->instance,
                              (unsigned long long)off_in_orig,
                              (unsigned long long)off_out_orig,
                              (long long)copy_ret);
            break;
        }

        if ((args->bogo.current_ops % 16) == 0) {
            if (stress_osal_fsync(fd_out) != 0) {
                stress_osal_print("rtos_stress: warn: [copy-file-%d]"
                                  " fsync failed (errno=%d)\n",
                                  args->instance, errno);
            }
        }

        args->bogo.current_ops++;
        stress_osal_sleep_ms(1);
    }

cleanup:
    if (fd_in >= 0)  stress_osal_close(fd_in);
    if (fd_out >= 0) stress_osal_close(fd_out);

    safe_unlink(filename_in,  args->instance, "cleanup");
    safe_unlink(filename_out, args->instance, "cleanup");
}
