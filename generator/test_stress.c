/**
 * @file test_stress.c
 * @brief Stress test implementation for RTOS-Bench
 * @details Wraps rtos_stress job execution for graduated stress testing
 */

#include "test_stress.h"
#include "logging.h"

#include <stdio.h>
#include <stdlib.h>
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

/* Helper function to parse argument with equals sign support */
const char *test_stress_parse_opt_arg(const char *arg, const char *prefix)
{
    size_t prefix_len = strlen(prefix);
    if (strncmp(arg, prefix, prefix_len) == 0) {
        return arg + prefix_len;
    }
    return NULL;
}

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
  STRESS_PRINTF("  all-quick   - Run all quick jobs(27 total runs)\n");
  STRESS_PRINTF("\n");
  STRESS_PRINTF("Examples:\n");
  STRESS_PRINTF("  rtbench test-stress --job cpu");
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
    return test_stress_run_single(name, duration_sec, 1, 0, NULL, NULL);
}

int test_stress_run_single(const char *stressor_name, int duration_sec,
                           int num_workers, uint64_t max_ops,
                           const char *method_name, const char *extra_opts)
{
    char duration_str[16];
    char workers_str[16];
    char ops_str[32];
    char method_arg[64];
    char *argv[32];
    int argc = 0;

    /* Validate and set defaults */
    if (!stressor_name || duration_sec <= 0) {
        stressor_name = "cpu";
        duration_sec = 10;
    }
    if (num_workers <= 0) num_workers = 1;

    STRESS_PRINTF("\n=============================================================\n");
    STRESS_PRINTF("[test-stress]   Running stressor: %s\n", stressor_name);
    
    /* Build argv array */
    argv[argc++] = "rtos_stress";
    argv[argc++] = (char *)stressor_name;
    
    /* Parameter combination logic */
    if (max_ops > 0 && duration_sec> 0) {
        /* Both: time-bounded with ops limit */
        snprintf(duration_str, sizeof(duration_str), "%ds", duration_sec);
        snprintf(ops_str, sizeof(ops_str), "%lld", (long long)max_ops);
        argv[argc++] = "-t";
        argv[argc++] = duration_str;
        argv[argc++] = "--ops";
        argv[argc++] = ops_str;
        STRESS_PRINTF("                Duration: %d seconds\n", duration_sec);
        STRESS_PRINTF("                Max ops: %lld\n", (long long)max_ops);
    } else if (max_ops > 0) {
        /* Ops only: no time limit */
        snprintf(ops_str, sizeof(ops_str), "%lld", (long long)max_ops);
        argv[argc++] = "--ops";
        argv[argc++] = ops_str;
        STRESS_PRINTF("                Max ops: %lld\n", (long long)max_ops);
    } else {
        /* Time only (or default): time-bounded */
        snprintf(duration_str, sizeof(duration_str), "%ds", duration_sec);
        argv[argc++] = "-t";
        argv[argc++] = duration_str;
        STRESS_PRINTF("                Duration: %d seconds\n", duration_sec);
    }
    
    STRESS_PRINTF("                Workers: %d\n", num_workers);
    if (method_name) {
        STRESS_PRINTF("                Method: %s\n", method_name);
    }
    if (extra_opts) {
        STRESS_PRINTF("                Extra opts: %s\n", extra_opts);
    }
    STRESS_PRINTF("=============================================================\n");

    /* Add workers parameter */
    argv[argc++] = "-c";
    snprintf(workers_str, sizeof(workers_str), "%d", num_workers);
    argv[argc++] = workers_str;

    /* Add method parameter */
    if (method_name) {
        snprintf(method_arg, sizeof(method_arg), "--method=%s", method_name);
        argv[argc++] = method_arg;
    }

    /* Parse and add extra options */
    if (extra_opts) {
        char opts_copy[512];
        strncpy(opts_copy, extra_opts, sizeof(opts_copy) -1);
        opts_copy[sizeof(opts_copy) -1] = '\0';
        
        char *token = strtok(opts_copy, " ");
        while (token != NULL && argc < 30) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
    }

    int ret = stress_ng_main(argc, argv);
    s_last_bogo_ops = stress_ng_get_last_bogo_ops();

    STRESS_PRINTF("\n[test-stress] Stressor %s completed with code: %d\n", stressor_name, ret);
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
  STRESS_PRINTF("Single stressor mode:\n");
  STRESS_PRINTF("  rtbench test-stress -s <stressor> [OPTIONS]\n");

  STRESS_PRINTF("\n");
  STRESS_PRINTF("Common Options:\n");
  STRESS_PRINTF("  -t <seconds>     Duration in seconds\n");
  STRESS_PRINTF("  --ops <max>      Maximum operations limit\n");
  STRESS_PRINTF("  -c <workers>     Number of worker threads (default: 1)\n");
  STRESS_PRINTF("  --method <name>  Specific algorithm/method to use(default: all)\n");

  STRESS_PRINTF("Available stressors and their specific options:\n");
  STRESS_PRINTF("\n");

  STRESS_PRINTF("CPU Stressors:\n");
  STRESS_PRINTF("  cpu       --cpu-load <0-100>     CPU load percentage\n");
  STRESS_PRINTF("            Methods: sqrt, bitops, matrixprod, ackermann, fibonacci, prime\n");
  STRESS_PRINTF("  matrix    --matrix-size <N>      Matrix dimension (default: 64)\n");
  STRESS_PRINTF("            Methods: prod, add, sub, trans, mean, identity\n");
  STRESS_PRINTF("  qsort     --qsort-size <size>    Array size to sort (supports K/M suffix)\n");
  STRESS_PRINTF("  atomic    --atomic-threads <N>   Number of atomic operation threads (0-64)\n");
  STRESS_PRINTF("  bitops    --bitops-loops <N>     Number of bit operation loops\n");
  STRESS_PRINTF("  bsearch   --bsearch-size <N>     Array size for binary search(supports K/M/G suffix)\n");
  STRESS_PRINTF("            Methods: bsearch-libc, bsearch-nonlibc, ternary\n");
  STRESS_PRINTF("  context   --context-threads <N>  Number of context switch threads (1-128)\n");
  STRESS_PRINTF("  fp        --fp-loops <N>         Number of FP operation loops\n");
  STRESS_PRINTF("  prime     --prime-start <N>      Starting value for prime search\n");
  STRESS_PRINTF("            Methods: inc, sieve, factorial\n");
  STRESS_PRINTF("  stack     --stack-size <size>    Stack size (supports K/M suffix)\n");
  STRESS_PRINTF("  str       --str-size <size>      String size (supports K/M suffix)\n");
  STRESS_PRINTF("  trig      --trig-loops <N>       Number of trigonometric loops\n");
  STRESS_PRINTF("  vecmath   --vecmath-loops <N>    Number of vector math loops\n");
  STRESS_PRINTF("\n");

  STRESS_PRINTF("Memory Stressors:\n");
  STRESS_PRINTF("  memcpy    --memcpy-loops <N>     Number of memcpy loops\n");
  STRESS_PRINTF("            --memcpy-size <N>      Size of each memcpy operation (supports K/M/G suffix)\n");
  STRESS_PRINTF("  stream    --stream-elem <N>      Number of stream elements (supports K/M suffix)\n");
  STRESS_PRINTF("  vm        --vm-bytes <size>      Memory size (supports K/M/G suffix)\n");
  STRESS_PRINTF("            Methods: write64, read64, rand-set, toggle, walk-1, galpat-1, gray, rowhammer, modulo-x\n");
  STRESS_PRINTF("  malloc    --malloc-bytes <size>  Max bytes per allocation (supports K/M/G suffix)\n");
  STRESS_PRINTF("            --malloc-max <slots>   Max allocation slots\n");
  STRESS_PRINTF("  memthrash --memthrash-size <sz>  Memory size (supports K/M/G suffix)\n");
  STRESS_PRINTF("  ptr-chase --ptr-chase-pages <N>  Number of pages to chase (supports K/M suffix)\n");
  STRESS_PRINTF("\n");
  
  STRESS_PRINTF("File I/O Stressors:\n");
  STRESS_PRINTF("  hdd       --hdd-bytes <size>     Total bytes to write (supports K/M/G suffix)\n");
  STRESS_PRINTF("  open      --open-max <N>         Maximum number of open files\n");
  STRESS_PRINTF("  copy-file --copy-file-bytes <sz> File size in bytes (supports K/M/G suffix)\n");
  STRESS_PRINTF("  unlink    --unlink-files <N>     Number of files to unlink\n");
  STRESS_PRINTF("  fstat     --fstat-files <N>      Number of files to fstat\n");
  STRESS_PRINTF("  dentry    --dentries <N>         Number of directory entries\n");
  STRESS_PRINTF("  rename    --rename-file-size <sz> File size for rename test (supports K/M suffix)\n");
  STRESS_PRINTF("  pipe      --pipe-data-size <sz>  Pipe chunk size (supports K/M suffix)\n");
  STRESS_PRINTF("\n");
  
  STRESS_PRINTF("Examples:\n");
  STRESS_PRINTF("  rtbench test-stress -s cpu -t 30 -c 2 --cpu-load 50 --method ackermann\n");
  STRESS_PRINTF("  rtbench test-stress -s cpu -t 30 -c 2 --cpu-load=50 --method=ackermann\n");
  STRESS_PRINTF("  rtbench test-stress -s bsearch --ops 1000 --bsearch-size 10000 --method ternary\n");
  STRESS_PRINTF("  rtbench test-stress -s context -t 10s --context-threads 8\n");

}
