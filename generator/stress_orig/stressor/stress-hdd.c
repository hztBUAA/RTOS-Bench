/* applications/stress-ng/stress-hdd.c */
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

#define HDD_WRITE_SIZE      (64 * 1024)
#define HDD_FILENAME_FMT    "%d.tmp"

#define UNLIKELY(x)         __builtin_expect(!!(x), 0)
#define LIKELY(x)           __builtin_expect(!!(x), 1)

static uint64_t s_hdd_bytes = DEFAULT_HDD_BYTES;

static int stress_hdd_opt_bytes(const char *opt_name, const char *opt_arg)
{
    char *endptr;
    unsigned long long val = strtoull(opt_arg, &endptr, 10);

    if (*endptr == 'k' || *endptr == 'K') val *= 1024;
    else if (*endptr == 'm' || *endptr == 'M') val *= (1024 * 1024);
    else if (*endptr == 'g' || *endptr == 'G') val *= (1024 * 1024 * 1024);

    if (val < MIN_HDD_BYTES) val = MIN_HDD_BYTES;
    if (val > MAX_HDD_BYTES) val = MAX_HDD_BYTES;

    s_hdd_bytes = (uint64_t)val;
    stress_osal_print("rtos_stress: debug: hdd-bytes set to %llu bytes\n", (unsigned long long)s_hdd_bytes);
    return 0;
}

const stress_opt_t stress_hdd_opts[] = {
    { "hdd-bytes", stress_hdd_opt_bytes },
    { NULL, NULL }
};

static inline uint8_t data_value(uint64_t offset, uint64_t index, uint32_t instance)
{
    return (uint8_t)(((offset + index) >> 9) + offset + index + instance);
}

static void hdd_fill_buf(uint8_t *buf, size_t size, uint64_t offset, uint32_t instance)
{
    for (size_t i = 0; i < size; i++) {
        buf[i] = data_value(offset, i, instance);
    }
}

static int hdd_verify_buf(uint8_t *buf, size_t size, uint64_t offset, uint32_t instance)
{
    for (size_t i = 0; i < size; i++) {
        uint8_t expected = data_value(offset, i, instance);
        if (buf[i] != expected) {
            return -1;
        }
    }
    return 0;
}

void stress_hdd(stress_args_t *args)
{
    int fd = -1;
    uint8_t *buf = NULL;
    char filename[64];
    uint64_t hdd_bytes = s_hdd_bytes;
    uint64_t total_written = 0;
    uint64_t total_read = 0;
    stress_tick_t tick_start;
    int rc = EXIT_SUCCESS;

    if (hdd_bytes < HDD_WRITE_SIZE) hdd_bytes = HDD_WRITE_SIZE;

    buf = stress_osal_malloc(HDD_WRITE_SIZE);
    if (!buf) {
        stress_osal_print("rtos_stress: error: [hdd] OOM allocating %d bytes\n", HDD_WRITE_SIZE);
        return;
    }

    stress_osal_snprintf(filename, sizeof(filename), HDD_FILENAME_FMT, (int)args->instance);

    stress_osal_print("rtos_stress: info: [hdd-%d] testing file '%s', size %d MB\n",
               args->instance, filename, (int)(hdd_bytes / 1024 / 1024));

    while (stress_continue(args))
    {
        fd = stress_osal_open(filename, O_CREAT | O_RDWR | O_TRUNC, 0666);
        if (fd < 0) {
            stress_osal_print("rtos_stress: fail: [hdd] open '%s' failed (errno=%d)\n", filename, errno);
            rc = EXIT_FAILURE;
            break;
        }

        total_written = 0;
        tick_start = stress_osal_tick_get();

        while (total_written < hdd_bytes) {
            if (!stress_continue(args)) break;

            size_t chunk = HDD_WRITE_SIZE;
            if (total_written + chunk > hdd_bytes) {
                chunk = hdd_bytes - total_written;
            }

            hdd_fill_buf(buf, chunk, total_written, args->instance);

            int ret = stress_osal_write(fd, buf, chunk);
            if (ret <= 0) {
                if (errno == ENOSPC) {
                    stress_osal_print("rtos_stress: warn: [hdd] device full at %d bytes\n", (int)total_written);
                    break;
                }
                stress_osal_print("rtos_stress: fail: [hdd] write failed (errno=%d)\n", errno);
                rc = EXIT_FAILURE;
                goto cleanup;
            }
            total_written += ret;
            args->bogo.current_ops++;

            if ((total_written % (1024 * 1024)) == 0) stress_osal_sleep_ms(1);
        }

        stress_osal_fsync(fd);

        if (!stress_continue(args)) goto cleanup;

        if (lseek(fd, 0, SEEK_SET) != 0) {
            stress_osal_print("rtos_stress: fail: [hdd] lseek failed\n");
            rc = EXIT_FAILURE;
            goto cleanup;
        }

        total_read = 0;
        while (total_read < total_written) {
            if (!stress_continue(args)) break;

            size_t chunk = HDD_WRITE_SIZE;
            if (total_read + chunk > total_written) {
                chunk = total_written - total_read;
            }

            ssize_t ret = stress_osal_read(fd, buf, chunk);
            if (ret <= 0) {
                stress_osal_print("rtos_stress: fail: [hdd] read failed (errno=%d)\n", errno);
                rc = EXIT_FAILURE;
                goto cleanup;
            }

            if (hdd_verify_buf(buf, ret, total_read, args->instance) != 0) {
                stress_osal_print("rtos_stress: fail: [hdd] data corruption detected at offset %d\n", (int)total_read);
                rc = EXIT_FAILURE;
                goto cleanup;
            }

            total_read += ret;
            args->bogo.current_ops++;

            if ((total_read % (1024 * 1024)) == 0) stress_osal_sleep_ms(1);
        }

        stress_tick_t duration = stress_osal_tick_get() - tick_start;
        (void)duration;

cleanup:
        stress_osal_close(fd);
        fd = -1;
        stress_osal_unlink(filename);

        if (rc != EXIT_SUCCESS) break;

        stress_osal_sleep_ms(10);
    }

    if (fd >= 0) stress_osal_close(fd);
    stress_osal_unlink(filename);
    stress_osal_free(buf);
}
