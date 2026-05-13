/* applications/stress-ng/common/stress-ng.c */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stress-config.h"
#include "stress_osal.h"
#include "stress-ng.h"

/* ==================================================================
 * 外部声明
 * ================================================================== */
extern void stress_vecmath(stress_args_t *args);
extern void stress_cpu(stress_args_t *args);
extern void stress_matrix(stress_args_t *args);
extern void stress_bitops(stress_args_t *args);
extern void stress_prime(stress_args_t *args);
extern void stress_str(stress_args_t *args);
extern void stress_qsort(stress_args_t *args);
extern void stress_vm(stress_args_t *args);
extern void stress_bsearch(stress_args_t *args);
extern void stress_malloc(stress_args_t *args);
extern void stress_memcpy(stress_args_t *args);
extern void stress_memthrash(stress_args_t *args);
extern void stress_hdd(stress_args_t *args);
extern void stress_open(stress_args_t *args);
extern void stress_copy_file(stress_args_t *args);
extern void stress_unlink(stress_args_t *args);
extern void stress_dentry(stress_args_t *args);
extern void stress_fp(stress_args_t *args);
extern void stress_trig(stress_args_t *args);
extern void stress_atomic(stress_args_t *args);
extern void stress_context(stress_args_t *args);
extern void stress_stack(stress_args_t *args);
extern void stress_stream(stress_args_t *args);
extern void stress_ptr_chase(stress_args_t *args);
extern void stress_rename(stress_args_t *args);
extern void stress_fstat(stress_args_t *args);
extern void stress_pipe(stress_args_t *args);

extern const stress_opt_t stress_cpu_opts[];
extern const stress_opt_t stress_atomic_opts[];
extern const stress_opt_t stress_bitops_opts[];
extern const stress_opt_t stress_bsearch_opts[];
extern const stress_opt_t stress_context_opts[];
extern const stress_opt_t stress_copy_file_opts[];
extern const stress_opt_t stress_dentry_opts[];
extern const stress_opt_t stress_fp_opts[];
extern const stress_opt_t stress_fstat_opts[];
extern const stress_opt_t stress_hdd_opts[];
extern const stress_opt_t stress_malloc_opts[];
extern const stress_opt_t stress_matrix_opts[];
extern const stress_opt_t stress_memcpy_opts[];
extern const stress_opt_t stress_memthrash_opts[];
extern const stress_opt_t stress_open_opts[];
extern const stress_opt_t stress_pipe_opts[];
extern const stress_opt_t stress_prime_opts[];
extern const stress_opt_t stress_ptr_chase_opts[];
extern const stress_opt_t stress_qsort_opts[];
extern const stress_opt_t stress_rename_opts[];
extern const stress_opt_t stress_stack_opts[];
extern const stress_opt_t stress_str_opts[];
extern const stress_opt_t stress_stream_opts[];
extern const stress_opt_t stress_trig_opts[];
extern const stress_opt_t stress_unlink_opts[];
extern const stress_opt_t stress_vecmath_opts[];
extern const stress_opt_t stress_vm_opts[];

static const stressor_info_t stress_registry[] = {
    /* --- 计算密集型 --- */
    {"vecmath",    stress_vecmath,    16384,  20,   stress_vecmath_opts},
    {"cpu",        stress_cpu,        65536,  20,   stress_cpu_opts},      /* 64KB: ackermann(3,2) needs ~25KB stack */
    {"bitops",     stress_bitops,     16384,  20,   stress_bitops_opts},
    {"prime",      stress_prime,      16384,  19,   stress_prime_opts},
    {"fp",         stress_fp,         16384,  20,   stress_fp_opts},
    {"trig",       stress_trig,       16384,  20,   stress_trig_opts},
    {"atomic",     stress_atomic,     16384,  20,   stress_atomic_opts},
    {"context",    stress_context,    16384,  20,   stress_context_opts},
    {"ptr-chase",  stress_ptr_chase,  16384,  20,   stress_ptr_chase_opts},

    /* --- 递归/重逻辑型 (32KB) --- */
    {"matrix",     stress_matrix,     32768,  21,   stress_matrix_opts},
    {"qsort",      stress_qsort,      32768,  20,   stress_qsort_opts},
    {"bsearch",    stress_bsearch,    32768,  20,   stress_bsearch_opts},
    {"str",        stress_str,        32768,  20,   stress_str_opts},

    /* --- 内存操作型 (64KB ) --- */
    {"vm",         stress_vm,         65536,  20,   stress_vm_opts},
    {"malloc",     stress_malloc,     65536,  20,   stress_malloc_opts},
    {"memcpy",     stress_memcpy,     65536,  20,   stress_memcpy_opts},
    {"memthrash",  stress_memthrash,  65536,  20,   stress_memthrash_opts},
    {"stream",     stress_stream,     65536,  20,   stress_stream_opts},

    /* --- 文件系统型 (16KB) --- */
    {"hdd",        stress_hdd,        16384,  25,   stress_hdd_opts},
    {"open",       stress_open,       16384,  25,   stress_open_opts},
    {"copy-file",  stress_copy_file,  16384,  25,   stress_copy_file_opts},
    {"unlink",     stress_unlink,     16384,  25,   stress_unlink_opts},
    {"fstat",      stress_fstat,      16384,  25,   stress_fstat_opts},
    {"dentry",     stress_dentry,     16384,  20,   stress_dentry_opts},
    {"rename",     stress_rename,     16384,  20,   stress_rename_opts},
    {"pipe",       stress_pipe,       16384,  20,   stress_pipe_opts},

    {"stack",      stress_stack,      65536,  20,   stress_stack_opts},

    {NULL, NULL, 0, 0, NULL}
};


volatile stress_bool_t g_stress_global_stop = STRESS_FALSE;
volatile stress_bool_t g_stress_silent_mode = STRESS_FALSE;

/* Accumulated bogo_ops from the last stress_run_one_job() call */
static stress_bogo_t g_last_bogo;

/* ==================================================================
 * 内置 Jobfile 数据
 * ================================================================== */

extern const char JOB_DATA_CPU[];
extern const char JOB_DATA_MEMORY[];
extern const char JOB_DATA_FILE[];
extern const char JOB_DATA_CPU_QUICK[];
extern const char JOB_DATA_MEMORY_QUICK[];
extern const char JOB_DATA_FILE_QUICK[];

static const stress_vfile_t BUILTIN_JOBS[] = {
    { "stored_jobfile_cpu.txt",          JOB_DATA_CPU },
    { "stored_jobfile_memory.txt",       JOB_DATA_MEMORY },
    { "stored_jobfile_file.txt",         JOB_DATA_FILE },
    { "stored_jobfile_cpu_quick.txt",    JOB_DATA_CPU_QUICK },
    { "stored_jobfile_memory_quick.txt", JOB_DATA_MEMORY_QUICK },
    { "stored_jobfile_file_quick.txt",   JOB_DATA_FILE_QUICK },
    { NULL, NULL }
};

/* ==================================================================
 * 核心功能函数
 * ================================================================== */

static void stress_thread_trampoline(void *parameter)
{
    stress_args_t *args = (stress_args_t *)parameter;
    stress_func_t func = (stress_func_t)args->user_data;

    if (func) {
        if (!g_stress_silent_mode) {
             stress_osal_print("rtos_stress: info: [%s-%d] started (pid %p)\n",
                        args->name, args->instance, stress_osal_thread_self());
        }

        func(args);

        if (!g_stress_silent_mode) {
             stress_osal_print("rtos_stress: info: [%s-%d] completed, 0x%08x%08x ops\n",
                        args->name, args->instance, (uint32_t)(args->bogo.current_ops >> 32), (uint32_t)(args->bogo.current_ops));
        }
    }

    if (args->complete_sem) {
        stress_osal_sem_release(args->complete_sem);
    }

    /* 注意：此处不释放内存，防止Double Free */
}

static int stress_handle_stressor_opt(const stressor_info_t *info, const char *opt, const char *arg)
{
    if (info == NULL || info->opts == NULL) return -1;
    const stress_opt_t *opt_ptr = info->opts;
    while (opt_ptr->opt_name != NULL) {
        if (strncmp(opt, "--", 2) == 0) {
            if (strcmp(opt + 2, opt_ptr->opt_name) == 0) {
                if (opt_ptr->opt_func) return opt_ptr->opt_func(opt_ptr->opt_name, arg);
                return 0;
            }
        }
        opt_ptr++;
    }
    return -1;
}

/*
 * 辅助函数：安全等待所有 Worker
 * 统一管理所有线程的截止时间，避免串行等待导致的延时累加
 */
static int stress_wait_for_workers(stress_sem_t job_sem, int spawned_count, stress_tick_t timeout_ticks)
{
    stress_tick_t start_wait = stress_osal_tick_get();
    /* 安全余量：给线程 5秒 的时间进行清理和退出 */
    stress_tick_t safety_margin = 5000;

    /*
     * 总最大等待时间 = 任务预定运行时间 + 安全余量。
     * 即使是无限运行的任务，也必须有一个轮询周期
     */
    stress_tick_t deadline = (timeout_ticks > 0) ? (start_wait + timeout_ticks + safety_margin) : STRESS_WAIT_FOREVER;

    int finished_count = 0;

    while (finished_count < spawned_count) {
        stress_tick_t now = stress_osal_tick_get();
        stress_tick_t remain = 0;

        if (deadline != STRESS_WAIT_FOREVER) {
            if (now >= deadline) {
                stress_table_print("\nrtos_stress: error: Job timed out! Waiting for %d/%d workers.\n",
                                  spawned_count - finished_count, spawned_count);
                break;
            }
            remain = deadline - now;
        } else {
            remain = 1000; /* 无限模式下，每秒检查一次 */
        }

        /*
         * 使用较短的超时 (1秒或剩余时间) 进行 sem_take
         * 保持主线程响应，并定期检查停止标志
         */
        stress_tick_t wait_slice = (remain > 1000) ? 1000 : remain;

        if (stress_osal_sem_take(job_sem, wait_slice) == 0) {
            finished_count++;
        }

        /* 检查全局停止信号 */
        if (g_stress_global_stop && deadline == STRESS_WAIT_FOREVER) {
             deadline = stress_osal_tick_get() + safety_margin;
        }
    }

    return finished_count;
}

static int stress_run_one_job(int argc, char **argv, stress_bool_t silent, stress_bogo_t *output_result)
{
    stress_bool_t old_silent = g_stress_silent_mode;
    g_stress_silent_mode = silent;

    int num_instances = 1;
    stress_tick_t timeout_ticks = 0;
    uint64_t max_ops = 0;
    const char *module_name = NULL;
    const char *target_method = NULL;
    const stressor_info_t *target_info = NULL;
    int i, j;
    stress_sem_t job_sem = NULL;
    stress_tid_t *tid_list = NULL;
    int has_orphans = 0;

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') continue;
        for (j = 0; stress_registry[j].name != NULL; j++) {
            if (strcmp(argv[i], stress_registry[j].name) == 0) {
                module_name = argv[i];
                target_info = &stress_registry[j];
                break;
            }
        }
        if (module_name) break;
    }

    if (!module_name) {
        g_stress_silent_mode = old_silent;
        return -1;
    }

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], module_name) == 0) continue;

        if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            num_instances = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            timeout_ticks = stress_parse_time(argv[++i]);
        }
        else if (strcmp(argv[i], "--ops") == 0 && i + 1 < argc) {
            max_ops = atol(argv[++i]);
        }
        else if (strcmp(argv[i], "--method") == 0 && i + 1 < argc) {
            target_method = argv[++i];
        }
        else if (strncmp(argv[i], "--", 2) == 0) {
            if (i + 1 < argc) {
                if (stress_handle_stressor_opt(target_info, argv[i], argv[i+1]) == 0) {
                    i++;
                    continue;
                }
            }
        }
    }

    g_stress_global_stop = STRESS_FALSE;
    stress_tick_t now = stress_osal_tick_get();
    stress_tick_t end = (timeout_ticks == 0) ? 0 : (now + timeout_ticks);

    if (!silent) {
        stress_osal_print("rtos_stress: info: spawning %d instances of '%s' (duration: %d ticks)...\n",
                   num_instances, module_name, (int)timeout_ticks);
    }

    job_sem = stress_osal_sem_create("st_job", 0);
    if (!job_sem) {
        g_stress_silent_mode = old_silent;
        return -1;
    }

    stress_args_t **args_list = stress_osal_malloc(sizeof(stress_args_t*) * num_instances);
    tid_list = stress_osal_malloc(sizeof(stress_tid_t) * num_instances);
    if (tid_list) memset(tid_list, 0, sizeof(stress_tid_t) * num_instances);

    int spawned_count = 0;

    for (i = 0; i < num_instances; i++)
    {
        stress_args_t *args = stress_osal_malloc(sizeof(stress_args_t));
        if (!args) break;
        memset(args, 0, sizeof(stress_args_t));

        args->name = target_info->name;
        if (target_method) args->method_name = stress_osal_strdup(target_method);
        
        args->instance = i;
        args->num_instances = num_instances;
        args->time_start = now;
        args->time_end = end;
        args->bogo.max_ops = max_ops;
        args->user_data = (void *)target_info->entry;
        args->complete_sem = job_sem;
        
        args_list[i] = args;

        char thread_name[STRESS_OSAL_NAME_MAX];
        stress_osal_snprintf(thread_name, STRESS_OSAL_NAME_MAX, "%s-%d", module_name, i);

        stress_tid_t tid = stress_osal_thread_spawn(thread_name,
                                           stress_thread_trampoline,
                                           args,
                                           target_info->stack_size,
                                           target_info->priority);

        if (tid) {
            if (tid_list) tid_list[spawned_count] = tid;
            spawned_count++;
        }
    }

    int finished = stress_wait_for_workers(job_sem, spawned_count, timeout_ticks);

    if (finished < spawned_count) {
        stress_osal_print("rtos_stress: warn: Cleaning up %d zombie threads...\n", spawned_count - finished);
        for (i = 0; i < spawned_count; i++) {
            if (tid_list && tid_list[i]) {
                 stress_osal_thread_delete(tid_list[i]);
            }
        }
        has_orphans = 1;

        stress_osal_sleep_ms(100);
    }
   
    stress_osal_sleep_ms(50);

    if (output_result && spawned_count > 0) {
        memset(output_result, 0, sizeof(stress_bogo_t));
        for (i = 0; i < spawned_count; i++) {
            if (args_list[i]) {
                output_result->current_ops += args_list[i]->bogo.current_ops;
                for (int k = 0; k < 2; k++) {
                    output_result->metric_val[k] += args_list[i]->bogo.metric_val[k];
                    if (i == 0) {
                        strncpy(output_result->metric_name[k],
                                args_list[i]->bogo.metric_name[k],
                                sizeof(output_result->metric_name[k]) - 1);
                    }
                }
            }
        }
    }

    /* Always capture last run's bogo_ops for external retrieval */
    if (spawned_count > 0) {
        memset(&g_last_bogo, 0, sizeof(g_last_bogo));
        for (i = 0; i < spawned_count; i++) {
            if (args_list[i]) {
                g_last_bogo.current_ops += args_list[i]->bogo.current_ops;
                for (int k = 0; k < 2; k++) {
                    g_last_bogo.metric_val[k] += args_list[i]->bogo.metric_val[k];
                    if (i == 0) {
                        strncpy(g_last_bogo.metric_name[k],
                                args_list[i]->bogo.metric_name[k],
                                sizeof(g_last_bogo.metric_name[k]) - 1);
                    }
                }
            }
        }
    }
    
    if (!has_orphans) {
        for (i = 0; i < spawned_count; i++) {
            if (args_list[i]) {
                if (args_list[i]->method_name) {
                    stress_osal_free((void *)args_list[i]->method_name);
                }
                stress_osal_free(args_list[i]);
            }
        }
        stress_osal_sem_delete(job_sem);
    } else {
        stress_osal_print("rtos_stress: warn: Memory (args/sem) leaked intentionally to prevent orphan crash.\n");
        /* 注意：args_list 指针数组本身可以释放，因为线程不访问它 */
    }

    if (tid_list) stress_osal_free(tid_list);
    stress_osal_free(args_list); /* 这个是安全的 */

    g_stress_silent_mode = old_silent;
    return 0;
}

#define MAX_ARGS 32
static int stress_tokenize(char *line, char **argv)
{
    int argc = 0;
    char *token = strtok(line, " \t\r\n");
    argv[argc++] = "rtos_stress";
    while (token != NULL && argc < MAX_ARGS) {
        argv[argc++] = token;
        token = strtok(NULL, " \t\r\n");
    }
    return argc;
}

static char *stress_read_mem_line(char *buffer, int size, const char **cursor)
{
    if (*cursor == NULL || **cursor == '\0') return NULL;
    int i = 0;
    while (i < size - 1 && **cursor != '\0') {
        char c = *(*cursor)++;
        buffer[i++] = c;
        if (c == '\n') break;
    }
    buffer[i] = '\0';
    return buffer;
}

static void stress_print_progress(int total, int current, const char *running_name)
{
    int bar_width = 30;
    float percent = (float)current / (float)total;
    int filled = (int)(percent * bar_width);
    int i;

    char bar[64];
    char *ptr = bar;

    *ptr++ = '[';
    for (i = 0; i < bar_width; i++) {
        if (i < filled) *ptr++ = '=';
        else if (i == filled) *ptr++ = '>';
        else *ptr++ = ' ';
    }
    *ptr++ = ']';
    *ptr = '\0';

    stress_table_print("\rRunning Job: %s %d/%d (%d%%) -> %s      \n",
                       bar, current, total, (int)(percent * 100), running_name ? running_name : "Done");
}

int stress_jobfile_exec_ex(const char *filepath, const char *job_type,
                           int stressors_per_stage,
                           stress_job_result_t *results_out, int results_max)
{
    FILE *fp = NULL;
    const char *mem_ptr_start = NULL;
    const char *mem_ptr = NULL;

    char line_buf[256];
    char *argv[MAX_ARGS];
    int argc;
    int total_tasks = 0;
    int caller_owns_buf = (results_out != NULL);

    for (int i = 0; BUILTIN_JOBS[i].filename != NULL; i++) {
        if (stress_osal_strcmp(filepath, BUILTIN_JOBS[i].filename) == 0) {
            mem_ptr_start = BUILTIN_JOBS[i].content;
            mem_ptr = mem_ptr_start;
            break;
        }
    }
    if (!mem_ptr_start) {
        fp = fopen(filepath, "r");
        if (!fp) {
            stress_table_print("rtos_stress: error: failed to open jobfile '%s'\n", filepath);
            return -1;
        }
    }

    stress_table_print("Initializing Job: %s...\n", filepath);

    while (1) {
        char *ret;
        if (mem_ptr_start) {
            ret = stress_read_mem_line(line_buf, sizeof(line_buf), &mem_ptr);
        } else {
            ret = fgets(line_buf, sizeof(line_buf), fp);
        }
        if (ret == NULL) break;

        char *comment = stress_osal_strchr(line_buf, '#');
        if (comment) *comment = '\0';
        if (stress_osal_strlen(line_buf) < 2) continue;

        char dummy_buf[256];
        stress_osal_strcpy(dummy_buf, line_buf);
        argc = stress_tokenize(dummy_buf, argv);
        if (argc > 1) {
             total_tasks++;
        }
    }

    if (total_tasks == 0) {
        stress_table_print("No valid stressors found in jobfile.\n");
        if (fp) fclose(fp);
        return 0;
    }

    /* If caller provided buffer, use it; otherwise malloc internally */
    stress_job_result_t *results;
    if (caller_owns_buf) {
        results = results_out;
        if (total_tasks > results_max) total_tasks = results_max;
    } else {
        results = stress_osal_malloc(sizeof(stress_job_result_t) * total_tasks);
        if (!results) {
            stress_table_print("Error: OOM for job results.\n");
            if (fp) fclose(fp);
            return -1;
        }
    }
    stress_osal_memset(results, 0, sizeof(stress_job_result_t) * total_tasks);

    if (mem_ptr_start) {
        mem_ptr = mem_ptr_start;
    } else {
        fseek(fp, 0, SEEK_SET);
    }

    double total_time_start = stress_osal_time_now();
    int executed_count = 0;

    while (executed_count < total_tasks) {
        char *ret;
        if (mem_ptr_start) {
            ret = stress_read_mem_line(line_buf, sizeof(line_buf), &mem_ptr);
        } else {
            ret = fgets(line_buf, sizeof(line_buf), fp);
        }
        if (ret == NULL) break;

        char *comment = stress_osal_strchr(line_buf, '#');
        if (comment) *comment = '\0';
        if (stress_osal_strlen(line_buf) < 2) continue;

        argc = stress_tokenize(line_buf, argv);

        const char *stressor_name = NULL;
        if (argc > 1) {
             if (stress_osal_strcmp(argv[1], "run") == 0 && argc > 2) stressor_name = argv[2];
             else stressor_name = argv[1];
        }

        if (stressor_name) {
            stress_print_progress(total_tasks, executed_count, stressor_name);
            stress_osal_strcpy(results[executed_count].name, stressor_name);

            /* Compute stage (1-based) from position */
            if (stressors_per_stage > 0) {
                results[executed_count].stage = (executed_count / stressors_per_stage) + 1;
            }
            if (job_type && job_type[0] != '\0') {
                strncpy(results[executed_count].job_type, job_type,
                        sizeof(results[executed_count].job_type) - 1);
            }

            double t_start = stress_osal_time_now();
            results[executed_count].retval = stress_run_one_job(argc, argv, STRESS_TRUE, &results[executed_count].bogo);
            double t_end = stress_osal_time_now();
            results[executed_count].duration = t_end - t_start;

            executed_count++;
        }
    }

    stress_print_progress(total_tasks, total_tasks, "Finished");
    stress_table_print("\n");

    if (fp) fclose(fp);

    stress_table_print("\n========================================================================\n");
    stress_table_print(" Job: %s [%s]\n", filepath, mem_ptr_start ? "Built-in" : "External");
    stress_table_print("========================================================================\n");
    stress_table_print(" %-12s| %-6s| %-13s| %-9s| %s\n", "Stressor", "Stage", "Bogo Ops", "Time(s)", "Metric");
    stress_table_print("------------------------------------------------------------------------\n");

    for (int i = 0; i < executed_count; i++) {
        if (results[i].retval == 0) {
            char metric_buf[64] = "";
            int printed_count = 0;

            for (int k = 0; k < 2; k++) {
                double val_safe;
                char *name_ptr = results[i].bogo.metric_name[k];
                memcpy(&val_safe, &results[i].bogo.metric_val[k], sizeof(double));

                if (val_safe > 0.00001 && name_ptr[0] != '\0') {
                    char tmp[32];
                    stress_osal_snprintf(tmp, sizeof(tmp), "%s%.2f %s",
                                (printed_count > 0) ? ", " : "",
                                val_safe,
                                name_ptr);
                    strncat(metric_buf, tmp, sizeof(metric_buf) - stress_osal_strlen(metric_buf) - 1);
                    printed_count++;
                }
            }
            if (printed_count == 0) stress_osal_strcpy(metric_buf, "N/A");

            uint64_t ops_safe;
            memcpy(&ops_safe, &results[i].bogo.current_ops, sizeof(uint64_t));

            stress_table_print(" %-12s| %-6d| %-13llu| %-9.2f| %s\n",
                                results[i].name,
                                results[i].stage,
                                (unsigned long long)ops_safe,
                                results[i].duration,
                                metric_buf);
        } else {
            stress_table_print(" %-12s| %-6d| %-13s| %-9.2f| %s\n",
                                results[i].name, results[i].stage, "FAILED", results[i].duration, "N/A");
        }
    }

    double total_time = stress_osal_time_now() - total_time_start;
    stress_table_print("------------------------------------------------------------------------\n");
    stress_table_print("Total Run Time: %dm %ds\n", (int)total_time/60, (int)total_time%60);

    if (!caller_owns_buf) {
        stress_osal_free(results);
    }

    return executed_count;
}

static void stress_jobfile_exec(const char *filepath)
{
    stress_jobfile_exec_ex(filepath, "", 0, NULL, 0);
}

#define JOB_CPU_STRESSORS_PER_STAGE    13
#define JOB_MEMORY_STRESSORS_PER_STAGE  6
#define JOB_FILE_STRESSORS_PER_STAGE    8

int handle_job_command_ex(const char *job_name,
                          stress_job_result_t *results_out, int results_max)
{
    int total = 0;
    int ret;

    if (strcmp(job_name, "all") == 0) {
        ret = stress_jobfile_exec_ex("stored_jobfile_cpu.txt", "cpu",
                    JOB_CPU_STRESSORS_PER_STAGE, results_out, results_max);
        if (ret < 0) return -1;
        total += ret;

        ret = stress_jobfile_exec_ex("stored_jobfile_memory.txt", "memory",
                    JOB_MEMORY_STRESSORS_PER_STAGE,
                    results_out ? results_out + total : NULL,
                    results_max > total ? results_max - total : 0);
        if (ret < 0) return -1;
        total += ret;

        ret = stress_jobfile_exec_ex("stored_jobfile_file.txt", "file",
                    JOB_FILE_STRESSORS_PER_STAGE,
                    results_out ? results_out + total : NULL,
                    results_max > total ? results_max - total : 0);
        if (ret < 0) return -1;
        total += ret;
    } else if (strcmp(job_name, "all-quick") == 0) {
        ret = stress_jobfile_exec_ex("stored_jobfile_cpu_quick.txt", "cpu",
                    JOB_CPU_STRESSORS_PER_STAGE, results_out, results_max);
        if (ret < 0) return -1;
        total += ret;

        ret = stress_jobfile_exec_ex("stored_jobfile_memory_quick.txt", "memory",
                    JOB_MEMORY_STRESSORS_PER_STAGE,
                    results_out ? results_out + total : NULL,
                    results_max > total ? results_max - total : 0);
        if (ret < 0) return -1;
        total += ret;

        ret = stress_jobfile_exec_ex("stored_jobfile_file_quick.txt", "file",
                    JOB_FILE_STRESSORS_PER_STAGE,
                    results_out ? results_out + total : NULL,
                    results_max > total ? results_max - total : 0);
        if (ret < 0) return -1;
        total += ret;
    } else if (strcmp(job_name, "cpu") == 0) {
        ret = stress_jobfile_exec_ex("stored_jobfile_cpu.txt", "cpu",
                    JOB_CPU_STRESSORS_PER_STAGE, results_out, results_max);
        if (ret < 0) return -1;
        total = ret;
    } else if (strcmp(job_name, "cpu-quick") == 0) {
        ret = stress_jobfile_exec_ex("stored_jobfile_cpu_quick.txt", "cpu",
                    JOB_CPU_STRESSORS_PER_STAGE, results_out, results_max);
        if (ret < 0) return -1;
        total = ret;
    } else if (strcmp(job_name, "memory") == 0) {
        ret = stress_jobfile_exec_ex("stored_jobfile_memory.txt", "memory",
                    JOB_MEMORY_STRESSORS_PER_STAGE, results_out, results_max);
        if (ret < 0) return -1;
        total = ret;
    } else if (strcmp(job_name, "memory-quick") == 0) {
        ret = stress_jobfile_exec_ex("stored_jobfile_memory_quick.txt", "memory",
                    JOB_MEMORY_STRESSORS_PER_STAGE, results_out, results_max);
        if (ret < 0) return -1;
        total = ret;
    } else if (strcmp(job_name, "file") == 0) {
        ret = stress_jobfile_exec_ex("stored_jobfile_file.txt", "file",
                    JOB_FILE_STRESSORS_PER_STAGE, results_out, results_max);
        if (ret < 0) return -1;
        total = ret;
    } else if (strcmp(job_name, "file-quick") == 0) {
        ret = stress_jobfile_exec_ex("stored_jobfile_file_quick.txt", "file",
                    JOB_FILE_STRESSORS_PER_STAGE, results_out, results_max);
        if (ret < 0) return -1;
        total = ret;
    } else {
        stress_table_print("Unknown job: %s\n", job_name);
        return -1;
    }

    return total;
}

static void handle_job_command(const char *job_name)
{
    handle_job_command_ex(job_name, NULL, 0);
}

int stress_ng_main(int argc, char **argv)
{
#ifdef STRESS_SIMPLE_MODE
    if (argc < 2) {
        stress_table_print("Usage: ./rtos_stress [ --job <cpu|memory|file|all> ]\n");
        stress_table_print("    (Default: runs 'all' if no job specified)\n");
    } else if (strcmp(argv[1], "--job") == 0) {
        if (argc >= 3) handle_job_command(argv[2]);
        else handle_job_command("all");
    } else {
        /* Fallback for simple run if someone tries direct arguments */
        stress_run_one_job(argc, argv, STRESS_FALSE, NULL);
    }
#else
    if (argc < 2) {
        stress_table_print("Usage: ./rtos_stress.elf [options] <stressor>\n");
        return 0;
    }
    if (strcmp(argv[1], "--job") == 0) {
        if (argc >= 3) handle_job_command(argv[2]);
        else stress_table_print("Error: --job requires argument\n");
    }
    else if (strcmp(argv[1], "--jobfile") == 0) {
        if (argc >= 3) stress_jobfile_exec(argv[2]);
        else stress_table_print("Error: --jobfile requires path\n");
    } else {
        stress_run_one_job(argc, argv, STRESS_FALSE, NULL);
    }
#endif
    return 0;
}

int stress_ng_main_stop(void)
{
    g_stress_global_stop = STRESS_TRUE;
    stress_osal_print("rtos_stress: info: stopping all stressors...\n");
    return 0;
}

uint64_t stress_ng_get_last_bogo_ops(void)
{
    return g_last_bogo.current_ops;
}

