/**
 * @file test_stress.c
 * @brief Stress test implementation for RTOS-Bench
 * @details Wraps rtos_stress job execution for graduated stress testing
 */

#include "test_stress.h"
#include "logging.h"

#include <stdio.h>
#include <string.h>

#ifdef RT_THREAD_PLATFORM
#include <rtthread.h>
#define STRESS_PRINTF rt_kprintf
#else
#define STRESS_PRINTF printf
#endif

/* Forward declarations - implemented in stress_orig/ */
extern int stress_ng_main(int argc, char **argv);
extern int stress_ng_main_stop(void);
extern uint64_t stress_ng_get_last_bogo_ops(void);

/*
 * Binary-compatible redeclaration of stress_job_result_t from stress-ng.h.
 * We avoid including stress-ng.h directly because it pulls in <math.h>,
 * which clashes with logging.h's logf macro.
 */
typedef struct {
    uint64_t current_ops;
    uint64_t max_ops;
    double   metric_val[2];
    char     metric_name[2][32];
} stress_bogo_local_t;

typedef struct {
    char name[32];
    stress_bogo_local_t bogo;
    int retval;
    double duration;
    int stage;
    char job_type[16];
} stress_job_result_local_t;

extern int handle_job_command_ex(const char *job_name,
                                 stress_job_result_local_t *results_out,
                                 int results_max);

/* Static storage for job results */
static struct test_stress_job_result s_job_results[TEST_STRESS_MAX_RESULTS];
static int s_job_result_count = 0;

/* Legacy bogo_ops */
static uint64_t s_last_bogo_ops = 0;

int test_stress_run_job(const char *job_name)
{
    if (!job_name) job_name = "all";

    STRESS_PRINTF("\n");
    STRESS_PRINTF("=============================================================\n");
    STRESS_PRINTF("[test-stress] Running job: %s\n", job_name);
    STRESS_PRINTF("=============================================================\n");

    /* Static buffer for raw engine results (too large for stack) */
    static stress_job_result_local_t raw[TEST_STRESS_MAX_RESULTS];
    memset(raw, 0, sizeof(raw));

    int count = handle_job_command_ex(job_name, raw, TEST_STRESS_MAX_RESULTS);
    if (count < 0) {
        STRESS_PRINTF("[test-stress] Job '%s' failed\n", job_name);
        s_job_result_count = 0;
        return -1;
    }

    /* Convert raw results to test_stress_job_result */
    s_job_result_count = count;
    for (int i = 0; i < count; i++) {
        struct test_stress_job_result *dst = &s_job_results[i];
        memset(dst, 0, sizeof(*dst));

        strncpy(dst->name, raw[i].name, sizeof(dst->name) - 1);
        strncpy(dst->type, raw[i].job_type, sizeof(dst->type) - 1);
        dst->stage = raw[i].stage;
        dst->bogo_ops = raw[i].bogo.current_ops;
        dst->duration_sec = raw[i].duration;
        dst->success = (raw[i].retval == 0) ? 1 : 0;

        /* Copy first metric if available */
        if (raw[i].bogo.metric_val[0] > 0.00001 &&
            raw[i].bogo.metric_name[0][0] != '\0') {
            dst->metric_value = raw[i].bogo.metric_val[0];
            strncpy(dst->metric_unit, raw[i].bogo.metric_name[0],
                    sizeof(dst->metric_unit) - 1);
        }
    }

    STRESS_PRINTF("\n");
    STRESS_PRINTF("[test-stress] Job '%s' completed: %d stressor runs\n", job_name, count);
    STRESS_PRINTF("=============================================================\n");

    return 0;
}

const struct test_stress_job_result *test_stress_get_job_results(int *count_out)
{
    if (count_out) *count_out = s_job_result_count;
    return s_job_results;
}

void test_stress_list_jobs(void)
{
    STRESS_PRINTF("Available stress jobs:\n");
    STRESS_PRINTF("  cpu         - CPU compute stressors (13 stressors x 5 stages)\n");
    STRESS_PRINTF("  memory      - Memory stressors (6 stressors x 5 stages)\n");
    STRESS_PRINTF("  file        - File I/O stressors (8 stressors x 5 stages)\n");
    STRESS_PRINTF("  all         - Run all jobs sequentially (135 total runs)\n");
    STRESS_PRINTF("  cpu-quick   - CPU quick smoke test (13 stressors x 1 stage)\n");
    STRESS_PRINTF("  memory-quick- Memory quick smoke test (6 stressors x 1 stage)\n");
    STRESS_PRINTF("  file-quick  - File I/O quick smoke test (8 stressors x 1 stage)\n");
    STRESS_PRINTF("  all-quick   - Run all quick jobs (27 total runs)\n");
}

void test_stress_stop(void)
{
    stress_ng_main_stop();
}

/* ============================================================================
 * Legacy API (backward compatibility)
 * ============================================================================ */

static const char *stressor_names[] = {
    "cpu", "matrix", "vm", "malloc", "memcpy", "prime", "trig", "fp", NULL
};

static const char *stress_type_to_name(stress_type_t type)
{
    if (type >= 0 && type < STRESS_TYPE_ALL) return stressor_names[type];
    return "cpu";
}

int test_stress_run_stressor(const char *name, int duration_sec)
{
    char duration_str[16];
    char *argv[8];
    int argc = 0;

    if (!name || duration_sec <= 0) {
        name = "cpu";
        duration_sec = 10;
    }

    snprintf(duration_str, sizeof(duration_str), "%ds", duration_sec);

    STRESS_PRINTF("\n");
    STRESS_PRINTF("=============================================================\n");
    STRESS_PRINTF("[test-stress] Running stressor: %s for %d seconds\n", name, duration_sec);
    STRESS_PRINTF("=============================================================\n");

    argv[argc++] = "rtos_stress";
    argv[argc++] = (char *)name;
    argv[argc++] = "-t";
    argv[argc++] = duration_str;
    argv[argc++] = "-c";
    argv[argc++] = "1";

    int ret = stress_ng_main(argc, argv);
    s_last_bogo_ops = stress_ng_get_last_bogo_ops();

    STRESS_PRINTF("\n");
    STRESS_PRINTF("[test-stress] Stressor %s completed with code: %d\n", name, ret);
    STRESS_PRINTF("=============================================================\n");

    return ret;
}

int test_stress_run_config(const struct test_stress_config *config)
{
    int ret = 0;
    int duration = config->duration_sec > 0 ? config->duration_sec : 10;

    if (config->type == STRESS_TYPE_ALL) {
        STRESS_PRINTF("\n");
        STRESS_PRINTF("=============================================================\n");
        STRESS_PRINTF("[test-stress] Running ALL stressors (%d seconds each)\n", duration);
        STRESS_PRINTF("=============================================================\n");

        for (int i = 0; stressor_names[i] != NULL; i++) {
            int r = test_stress_run_stressor(stressor_names[i], duration);
            if (r != 0) ret = r;
        }

        STRESS_PRINTF("\n");
        STRESS_PRINTF("=============================================================\n");
        STRESS_PRINTF("[test-stress] All stressors completed\n");
        STRESS_PRINTF("=============================================================\n");
    } else {
        const char *name = stress_type_to_name(config->type);
        ret = test_stress_run_stressor(name, duration);
    }

    return ret;
}

uint64_t test_stress_get_last_bogo_ops(void)
{
    return s_last_bogo_ops;
}

void test_stress_list_stressors(void)
{
    STRESS_PRINTF("Available stressors:\n");
    STRESS_PRINTF("  cpu      - CPU compute intensive stress\n");
    STRESS_PRINTF("  matrix   - Matrix multiplication\n");
    STRESS_PRINTF("  vm       - Virtual memory stress\n");
    STRESS_PRINTF("  malloc   - Memory allocation/free\n");
    STRESS_PRINTF("  memcpy   - Memory copy operations\n");
    STRESS_PRINTF("  prime    - Prime number calculation\n");
    STRESS_PRINTF("  trig     - Trigonometric functions\n");
    STRESS_PRINTF("  fp       - Floating point operations\n");
    STRESS_PRINTF("  all      - Run all stressors sequentially\n");
}
