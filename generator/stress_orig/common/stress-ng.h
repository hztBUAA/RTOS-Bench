/* applications/stress-ng/stress-ng.h */
#ifndef __STRESS_NG_H__
#define __STRESS_NG_H__

#include "stress_osal.h"
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#define stress_osal_print(fmt, ...) \
    do { \
        if (!g_stress_silent_mode) { \
            (stress_osal_print)(fmt, ##__VA_ARGS__); \
        } \
    } while (0)

#define stress_table_print(fmt, ...) \
    (stress_osal_print)(fmt, ##__VA_ARGS__)

/* 引入文件系统支持 (用于 Jobfile) */
int stress_ng_main(int argc, char **argv);
int stress_ng_main_stop(void);

typedef struct {
    const char *filename;
    const char *content;
} stress_vfile_t;

typedef struct {
    uint64_t current_ops;
    uint64_t max_ops;
    double   metric_val[2];
    char     metric_name[2][32];
} stress_bogo_t;


typedef struct stress_args {
    const char *name;
    const char *method_name;
    uint32_t instance;
    uint32_t num_instances;
    stress_bogo_t bogo;
    stress_tick_t time_end;
    stress_tick_t time_start;
    void *user_data;
    stress_sem_t complete_sem;

    stress_bool_t stop_request;
} stress_args_t;

typedef int (*stress_opt_func_t)(const char *opt_name, const char *opt_arg);

typedef struct {
    const char *opt_name;
    stress_opt_func_t opt_func;
} stress_opt_t;

typedef void (*stress_func_t)(stress_args_t *args);

typedef struct {
    const char *name;
    stress_func_t entry;
    uint32_t stack_size;
    uint8_t priority;
    const stress_opt_t *opts;
} stressor_info_t;

/* ==================================================================
 * 临时结果存储结构 (用于运行后统一打印)
 * ================================================================== */
typedef struct {
    char name[32];
    stress_bogo_t bogo;
    int retval;
    double duration;
    int stage;           /* 1-5, stage index within job */
    char job_type[16];   /* "cpu"/"memory"/"file" */
} stress_job_result_t;

/* =========================================================================
 * 全局变量与函数声明
 * ========================================================================= */
extern volatile stress_bool_t g_stress_global_stop;
extern volatile stress_bool_t g_stress_silent_mode;


stress_bool_t stress_continue(stress_args_t *args);
stress_tick_t stress_parse_time(const char *str);
double stress_time_now(void);

/* Extended jobfile execution with result output */
int stress_jobfile_exec_ex(const char *filepath, const char *job_type,
                           int stressors_per_stage,
                           stress_job_result_t *results_out, int results_max);

/* Extended job command dispatcher with result output */
int handle_job_command_ex(const char *job_name,
                          stress_job_result_t *results_out, int results_max);


#endif

