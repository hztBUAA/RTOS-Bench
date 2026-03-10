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
#define EXIT_NO_RESOURCE EXIT_FAILURE
#endif

#define THREAD_STACK_SIZE (8192)

typedef struct {
    stress_args_t    *args;
    int               fds[2];
    size_t            chunk_size;
    stress_bool_t     verify;
    stress_sem_t      sem_reader_done;
    stress_sem_t      sem_writer_done;
    volatile uint64_t total_bytes;
    volatile int      stop_flag;
} pipe_context_t;

static int32_t s_pipe_data_size = DEFAULT_PIPE_DATA_SIZE;

static int stress_pipe_opt_data_size(const char *opt_name, const char *opt_arg)
{
    char *endptr;
    unsigned long long val = strtoull(opt_arg, &endptr, 10);
    if      (*endptr == 'k' || *endptr == 'K') val *= 1024ULL;
    else if (*endptr == 'm' || *endptr == 'M') val *= 1024ULL * 1024ULL;
    if (val < MIN_PIPE_DATA_SIZE) val = MIN_PIPE_DATA_SIZE;
    if (val > MAX_PIPE_DATA_SIZE) val = MAX_PIPE_DATA_SIZE;
    s_pipe_data_size = (int32_t)val;
    return 0;
}

const stress_opt_t stress_pipe_opts[] = {
    { "pipe-data-size", stress_pipe_opt_data_size },
    { NULL, NULL }
};

/* ---------------------------------------------------------------
 * Reader 线程（阻塞式 read，由关闭写端触发 EOF 退出）
 * --------------------------------------------------------------- */
static void stress_pipe_reader(void *parameter)
{
    pipe_context_t *ctx = (pipe_context_t *)parameter;
    char *buf = (char *)stress_osal_malloc(ctx->chunk_size);

    if (!buf) {
        stress_osal_print("rtos_stress: error: [pipe] OOM in reader\n");
        goto done;
    }

    while (!ctx->stop_flag) {
        int fd = ctx->fds[0];
        if (fd < 0) break;

        ssize_t n = read(fd, buf, ctx->chunk_size);
        if (n > 0) {
            if (ctx->verify) {
                for (size_t i = 0; i < (size_t)n; i++) {
                    volatile char v = buf[i]; (void)v;
                }
            }
        } else if (n == 0) {
            break;
        } else {
            if (errno == EINTR) continue;
            break;
        }
    }

    stress_osal_free(buf);
done:
    stress_osal_sem_release(ctx->sem_reader_done);
}

/* ---------------------------------------------------------------
 * Writer 线程（阻塞式 write，由关闭读端触发 EPIPE 退出）
 * --------------------------------------------------------------- */
static void stress_pipe_writer(void *parameter)
{
    pipe_context_t *ctx = (pipe_context_t *)parameter;
    char *buf = (char *)stress_osal_malloc(ctx->chunk_size);
    uint32_t counter = 0;

    if (!buf) {
        stress_osal_print("rtos_stress: error: [pipe] OOM in writer\n");
        goto done;
    }

    stress_osal_memset(buf, 0xA5, ctx->chunk_size);

    while (!ctx->stop_flag) {
        int fd = ctx->fds[1];
        if (fd < 0) break;

        if (ctx->verify && ctx->chunk_size >= sizeof(uint32_t))
            *(uint32_t *)buf = counter++;

        ssize_t n = write(fd, buf, ctx->chunk_size);
        if (n > 0) {
            ctx->total_bytes += (uint64_t)n;
            ctx->args->bogo.current_ops++;
        } else if (n < 0) {
            if (errno == EINTR) continue;
            /* EPIPE: 读端关闭; EBADF: fd 无效 → 退出 */
            break;
        }
    }

    stress_osal_free(buf);
done:
    stress_osal_sem_release(ctx->sem_writer_done);
}

/* ---------------------------------------------------------------
 * 主 stressor 函数
 * --------------------------------------------------------------- */
void stress_pipe(stress_args_t *args)
{
    stress_tid_t t_reader = NULL;
    stress_tid_t t_writer = NULL;
    double t_start, t_end, dt;

    pipe_context_t *ctx = (pipe_context_t *)stress_osal_malloc(sizeof(pipe_context_t));
    if (!ctx) {
        stress_osal_print("rtos_stress: error: [pipe] OOM for context\n");
        return;
    }
    stress_osal_memset(ctx, 0, sizeof(pipe_context_t));
    ctx->fds[0]     = -1;
    ctx->fds[1]     = -1;
    ctx->args       = args;
    ctx->chunk_size = (size_t)s_pipe_data_size;
    ctx->verify     = STRESS_FALSE;
    ctx->total_bytes = 0;
    ctx->stop_flag  = 0;

    static volatile uint32_t s_run_id = 0;
    uint32_t run_id = s_run_id++;
    char sem_rd[32], sem_wr[32];
    stress_osal_snprintf(sem_rd, sizeof(sem_rd), "prd_%u", run_id);
    stress_osal_snprintf(sem_wr, sizeof(sem_wr), "pwr_%u", run_id);

    if (pipe(ctx->fds) < 0) {
        stress_osal_print("rtos_stress: error: [pipe] pipe() failed errno=%d\n", errno);
        stress_osal_free(ctx);
        return;
    }

    stress_osal_print("rtos_stress: info: [pipe-%d] blocking mode chunk=%d run_id=%u\n",
                      args->instance, (int)ctx->chunk_size, run_id);

    ctx->sem_reader_done = stress_osal_sem_create(sem_rd, 0);
    ctx->sem_writer_done = stress_osal_sem_create(sem_wr, 0);
    if (!ctx->sem_reader_done || !ctx->sem_writer_done) {
        stress_osal_print("rtos_stress: error: [pipe] sem_create failed\n");
        close(ctx->fds[0]); close(ctx->fds[1]);
        if (ctx->sem_reader_done) stress_osal_sem_delete(ctx->sem_reader_done);
        if (ctx->sem_writer_done) stress_osal_sem_delete(ctx->sem_writer_done);
        stress_osal_free(ctx);
        return;
    }

    t_reader = stress_osal_thread_spawn("ng_pipe_r", stress_pipe_reader,
                                        ctx, THREAD_STACK_SIZE, 20);
    t_writer = stress_osal_thread_spawn("ng_pipe_w", stress_pipe_writer,
                                        ctx, THREAD_STACK_SIZE, 20);

    t_start = stress_osal_time_now();

    while (stress_continue(args)) {
        stress_osal_sleep_ms(50);
    }

    ctx->stop_flag = 1;

    stress_osal_sleep_ms(50);

    if (ctx->fds[1] >= 0) {
        int tmp = ctx->fds[1];
        ctx->fds[1] = -1;
        close(tmp);
    }

    if (stress_osal_sem_take(ctx->sem_reader_done, 3000) != 0) {
        stress_osal_print("rtos_stress: warn: [pipe] reader timed out\n");
        if (t_reader) stress_osal_thread_delete(t_reader);
        stress_osal_sleep_ms(100);
    }

    if (ctx->fds[0] >= 0) {
        int tmp = ctx->fds[0];
        ctx->fds[0] = -1;
        close(tmp);
    }

    if (stress_osal_sem_take(ctx->sem_writer_done, 3000) != 0) {
        stress_osal_print("rtos_stress: warn: [pipe] writer timed out\n");
        if (t_writer) stress_osal_thread_delete(t_writer);
        stress_osal_sleep_ms(100);
    }

    t_end = stress_osal_time_now();
    dt = t_end - t_start;
    if (dt > 0.0) {
        double rate_kb = (double)ctx->total_bytes / 1024.0 / dt;
        args->bogo.metric_val[0] = rate_kb;
        stress_osal_strcpy(args->bogo.metric_name[0], "KB/sec");
        stress_osal_print("rtos_stress: info: [pipe-%d] throughput: %.2f KB/sec\n",
                          args->instance, rate_kb);
    }

    stress_osal_sleep_ms(50);

    stress_osal_sem_delete(ctx->sem_reader_done);
    stress_osal_sem_delete(ctx->sem_writer_done);

    stress_osal_free(ctx);
}
