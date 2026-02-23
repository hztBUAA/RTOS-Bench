/* applications/stress-ng/stress-pipe.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <config.h>

#ifndef EXIT_NO_RESOURCE
#define EXIT_NO_RESOURCE    EXIT_FAILURE
#endif
#define THREAD_STACK_SIZE       (8192)

typedef struct {
    stress_args_t *args;
    int fds[2];
    size_t chunk_size;
    stress_bool_t verify;
    stress_sem_t sem_reader_done;
    stress_sem_t sem_writer_done;
    volatile uint64_t total_bytes;
} pipe_context_t;

static inline int is_eagain(int err)
{
    if (err < 0) err = -err;
    return (err == EAGAIN || err == EWOULDBLOCK);
}

static int32_t s_pipe_data_size = DEFAULT_PIPE_DATA_SIZE;

static int stress_pipe_opt_data_size(const char *opt_name, const char *opt_arg)
{
    char *endptr;
    unsigned long long val = strtoull(opt_arg, &endptr, 10);
    if (*endptr == 'k' || *endptr == 'K') val *= 1024;
    else if (*endptr == 'm' || *endptr == 'M') val *= (1024 * 1024);

    if (val < MIN_PIPE_DATA_SIZE) val = MIN_PIPE_DATA_SIZE;
    if (val > MAX_PIPE_DATA_SIZE) val = MAX_PIPE_DATA_SIZE;

    s_pipe_data_size = (int32_t)val;
    return 0;
}

const stress_opt_t stress_pipe_opts[] = {
    { "pipe-data-size", stress_pipe_opt_data_size },
    { NULL, NULL }
};

static void stress_pipe_reader(void *parameter)
{
    pipe_context_t *ctx = (pipe_context_t *)parameter;
    int fd = ctx->fds[0];
    char *buf = (char *)stress_osal_malloc(ctx->chunk_size);

    if (!buf) {
        stress_osal_print("rtos_stress: error: [pipe] OOM in reader\n");
        goto done;
    }

    while (stress_continue(ctx->args))
    {
        ssize_t n = read(fd, buf, ctx->chunk_size);

        if (n > 0) {
            if (ctx->verify) {
                for (size_t i = 0; i < (size_t)n; i++) {
                    if (!stress_continue(ctx->args)) break;
                    volatile char val = buf[i];
                    (void)val;
                }
            }
        } else if (n == 0) {
            break;
        } else {
            if (is_eagain(errno)) {
                stress_osal_sleep_ms(1);
            } else {
                stress_osal_print("rtos_stress: fail: [pipe] read error %d\n", errno);
                break;
            }
        }
    }

    if (buf) stress_osal_free(buf);

done:
    stress_osal_sem_release(ctx->sem_reader_done);
}

static void stress_pipe_writer(void *parameter)
{
    pipe_context_t *ctx = (pipe_context_t *)parameter;
    int fd = ctx->fds[1];
    char *buf = (char *)stress_osal_malloc(ctx->chunk_size);
    uint32_t val_counter = 0;

    if (!buf) {
        stress_osal_print("rtos_stress: error: [pipe] OOM in writer\n");
        goto done;
    }

    stress_osal_memset(buf, 0xA5, ctx->chunk_size);

    while (stress_continue(ctx->args))
    {
        if (ctx->verify && ctx->chunk_size >= sizeof(uint32_t)) {
            *(uint32_t *)buf = val_counter++;
        }

        ssize_t n = write(fd, buf, ctx->chunk_size);

        if (n > 0) {
            ctx->total_bytes += (uint64_t)n;
            ctx->args->bogo.current_ops++;
        } else if (n < 0) {
            if (is_eagain(errno)) {
                stress_osal_sleep_ms(1);
            } else if (errno == EPIPE) {
                break;
            } else {
                stress_osal_print("rtos_stress: fail: [pipe] write error %d\n", errno);
                break;
            }
        }
    }

    if (buf) stress_osal_free(buf);

done:
    stress_osal_sem_release(ctx->sem_writer_done);
}

void stress_pipe(stress_args_t *args)
{
    pipe_context_t ctx;
    stress_tid_t t_reader = NULL;
    stress_tid_t t_writer = NULL;
    double t_start, t_end, dt;
    int reader_done = 0;
    int writer_done = 0;

    stress_osal_memset(&ctx, 0, sizeof(ctx));
    ctx.args = args;
    ctx.chunk_size = (size_t)s_pipe_data_size;
    ctx.verify = STRESS_FALSE;
    ctx.total_bytes = 0;

    if (pipe(ctx.fds) < 0) {
        stress_osal_print("rtos_stress: error: [pipe] pipe creation failed\n");
        return;
    }

    int flags0 = fcntl(ctx.fds[0], F_GETFL, 0);
    fcntl(ctx.fds[0], F_SETFL, flags0 | O_NONBLOCK);

    int flags1 = fcntl(ctx.fds[1], F_GETFL, 0);
    fcntl(ctx.fds[1], F_SETFL, flags1 | O_NONBLOCK);

    stress_osal_print("rtos_stress: info: [pipe-%d] using NON-BLOCKING IO (chunk: %d)\n",
               args->instance, (int)ctx.chunk_size);

    ctx.sem_reader_done = stress_osal_sem_create("pipe_rd", 0);
    ctx.sem_writer_done = stress_osal_sem_create("pipe_wr", 0);

    t_reader = stress_osal_thread_spawn("ng_pipe_r", stress_pipe_reader, &ctx, THREAD_STACK_SIZE, 20);
    t_writer = stress_osal_thread_spawn("ng_pipe_w", stress_pipe_writer, &ctx, THREAD_STACK_SIZE, 20);

    t_start = stress_osal_time_now();

    while (stress_continue(args)) {
        if (!reader_done && stress_osal_sem_take(ctx.sem_reader_done, 0) == 0) reader_done = 1;
        if (!writer_done && stress_osal_sem_take(ctx.sem_writer_done, 0) == 0) writer_done = 1;

        if (reader_done && writer_done) break;

        stress_osal_sleep_ms(10);
    }

    if (!reader_done) {
        if (stress_osal_sem_take(ctx.sem_reader_done, 2000) != 0) {
            stress_osal_print("rtos_stress: warn: [pipe] reader timed out\n");
            if (t_reader) stress_osal_thread_delete(t_reader);
        }
    }

    if (!writer_done) {
        if (stress_osal_sem_take(ctx.sem_writer_done, 2000) != 0) {
            stress_osal_print("rtos_stress: warn: [pipe] writer timed out\n");
            if (t_writer) stress_osal_thread_delete(t_writer);
        }
    }

    close(ctx.fds[0]);
    close(ctx.fds[1]);

    t_end = stress_osal_time_now();
    dt = t_end - t_start;

    if (dt > 0.0) {
        double rate_kb = (double)ctx.total_bytes / 1024.0 / dt;
        args->bogo.metric_val[0] = rate_kb;
        stress_osal_strcpy(args->bogo.metric_name[0], "KB/sec");
        stress_osal_print("rtos_stress: info: [pipe-%d] throughput: %.2f KB/sec\n", args->instance, rate_kb);
    }

    if (ctx.sem_reader_done) stress_osal_sem_delete(ctx.sem_reader_done);
    if (ctx.sem_writer_done) stress_osal_sem_delete(ctx.sem_writer_done);
}
