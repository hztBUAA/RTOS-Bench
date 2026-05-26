/**
 * @file result_export.h
 * @brief RTOS-Bench result export and persistence
 * @details Provides structured result collection and JSON export for all test modules.
 *          Results are saved to the target machine's filesystem for later analysis.
 *
 * Usage:
 *   1. Initialize: rtbench_result_init()
 *   2. Run tests (results auto-collected via callbacks)
 *   3. Export: rtbench_result_export_json("/path/to/result.json")
 *   4. Cleanup: rtbench_result_cleanup()
 *
 * Or use auto mode: rtbench_result_set_auto_export("/results/")
 */

#ifndef RESULT_EXPORT_H
#define RESULT_EXPORT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Configuration
 * ============================================================================ */

#define RTBENCH_RESULT_VERSION       "1.0.0"
#define RTBENCH_MAX_WORKLOADS        32
#define RTBENCH_MAX_STRESSORS        160
#define RTBENCH_MAX_CMD_COMMANDS     16
#define RTBENCH_MAX_GRADIENTS        16
#define RTBENCH_MAX_SERVICE_OPS      16
#define RTBENCH_MAX_MEM_BW_TYPES     8
#define RTBENCH_MAX_NAME_LEN         64
#define RTBENCH_MAX_PATH_LEN         256

/* ============================================================================
 * Data Structures - Environment Info
 * ============================================================================ */

/**
 * @brief Environment/platform information
 */
struct rtbench_env_info {
    char os_name[RTBENCH_MAX_NAME_LEN];
    char os_version[RTBENCH_MAX_NAME_LEN];
    char board[RTBENCH_MAX_NAME_LEN];
    char cpu_type[RTBENCH_MAX_NAME_LEN];
    uint32_t cpu_freq_mhz;
    uint32_t cpu_core_num;
};

/* ============================================================================
 * Data Structures - Realtime Test Results
 * ============================================================================ */

/**
 * @brief Single-core service cost entry
 */
struct rtbench_service_cost {
    char operation[RTBENCH_MAX_NAME_LEN];
    double immediate_us;      /* -1 means N/A */
    double suspend_us;
    double low_prio_us;
    double high_prio_us;
};

/**
 * @brief Memory bandwidth entry for one type
 */
struct rtbench_mem_bw_entry {
    char type[16];            /* rd, wr, cp, frd, fwr, fcp, memset, memcpy */
    double c1, c2, c4, c8;    /* concurrency 1/2/4/8 */
};

/**
 * @brief Realtime test results
 */
struct rtbench_realtime_result {
    int valid;                /* 0 = not run, 1 = valid */
    double duration_sec;

    /* Single-core */
    double context_switch_avg_us;
    double interrupt_min_us;
    double interrupt_max_us;
    double interrupt_avg_us;
    double syscall_min_us;
    double syscall_max_us;
    double syscall_avg_us;

    struct rtbench_service_cost service_cost[RTBENCH_MAX_SERVICE_OPS];
    int service_cost_count;

    /* Multi-core */
    int multicore_valid;
    struct rtbench_mem_bw_entry mem_bw[RTBENCH_MAX_MEM_BW_TYPES];
    int mem_bw_count;

    double ipc_bw_c1, ipc_bw_c2, ipc_bw_c4, ipc_bw_c8;  /* GB/s */
    double task_lat_c1, task_lat_c2, task_lat_c4, task_lat_c8;  /* us */
    double core_comm_intra;   /* GB/s */
    double core_comm_inter;   /* GB/s */
};

/* ============================================================================
 * Data Structures - Schedule Test Results
 * ============================================================================ */

/**
 * @brief WCET measurement for one workload
 */
struct rtbench_wcet_entry {
    char workload[RTBENCH_MAX_NAME_LEN];
    double wcet_ms;
};

/**
 * @brief Task stats within a gradient
 */
struct rtbench_task_stat {
    char name[RTBENCH_MAX_NAME_LEN];
    double utilization;
    double period_ms;
    uint64_t jobs;
    uint64_t misses;
    double max_response_ms;
};

/**
 * @brief Gradient result
 */
struct rtbench_gradient_result {
    int utilization_percent;
    double actual_utilization;
    uint64_t total_jobs;
    uint64_t deadline_misses;
    double miss_rate;
    struct rtbench_task_stat task_stats[RTBENCH_MAX_WORKLOADS];
    int task_count;
};

/**
 * @brief Schedule test results
 */
struct rtbench_schedule_result {
    int valid;
    double duration_sec;

    /* Config */
    int cycles;
    int util_start, util_end, util_step;

    /* WCET measurements */
    struct rtbench_wcet_entry wcet[RTBENCH_MAX_WORKLOADS];
    int wcet_count;

    /* Gradients */
    struct rtbench_gradient_result gradients[RTBENCH_MAX_GRADIENTS];
    int gradient_count;

    /* Summary */
    double average_miss_rate;
    double final_score;
};

/* ============================================================================
 * Data Structures - Stress Test Results
 * ============================================================================ */

/**
 * @brief Stressor result entry
 */
struct rtbench_stressor_result {
    char name[RTBENCH_MAX_NAME_LEN];
    char type[16];            /* cpu, memory, file */
    int stage;                /* 1-5, stage index within job */
    uint64_t bogo_ops;
    double duration_sec;
    double metric_value;      /* optional: throughput etc */
    char metric_unit[16];     /* optional: MB/sec etc */
};

/**
 * @brief Stress test results
 */
struct rtbench_stress_result {
    int valid;
    double duration_sec;
    struct rtbench_stressor_result stressors[RTBENCH_MAX_STRESSORS];
    int stressor_count;
};

/* ============================================================================
 * Data Structures - Command Support Test Results
 * ============================================================================ */

/**
 * @brief Single command test result
 */
struct rtbench_cmd_result {
    char command[RTBENCH_MAX_NAME_LEN];  /**< Full command string */
    char name[32];                        /**< Command name (first word) */
    int supported;                        /**< 1 = success, 0 = failure */
};

/**
 * @brief Command support test results
 */
struct rtbench_cmd_module_result {
    int valid;
    int cmd_count;
    int pass_count;
    struct rtbench_cmd_result results[RTBENCH_MAX_CMD_COMMANDS];
};

/* ============================================================================
 * Data Structures - Typical Workload Results
 * ============================================================================ */

/**
 * @brief Workload result entry
 */
struct rtbench_workload_result {
    char name[RTBENCH_MAX_NAME_LEN];
    char category[RTBENCH_MAX_NAME_LEN];
    int success;
    int rounds;
    double exec_time_ms;
    double avg_time_ms;
};

/**
 * @brief Typical workload test results
 */
struct rtbench_workload_module_result {
    int valid;
    double duration_sec;
    struct rtbench_workload_result workloads[RTBENCH_MAX_WORKLOADS];
    int workload_count;
};

/* ============================================================================
 * Data Structures - Complete Result
 * ============================================================================ */

/**
 * @brief Complete RTOS-Bench result structure
 */
struct rtbench_result {
    /* Meta */
    char framework_version[16];
    char test_timestamp[32];  /* ISO 8601 */
    double total_duration_sec;

    /* Environment */
    struct rtbench_env_info env;

    /* Module results */
    struct rtbench_realtime_result realtime;
    struct rtbench_schedule_result schedule;
    struct rtbench_stress_result stress;
    struct rtbench_cmd_module_result cmd;
    struct rtbench_workload_module_result workload;
};

/* ============================================================================
 * API Functions
 * ============================================================================ */

/**
 * @brief Initialize result collection
 * @return 0 on success
 */
int rtbench_result_init(void);

/**
 * @brief Cleanup result collection
 */
void rtbench_result_cleanup(void);

/**
 * @brief Get pointer to global result structure
 * @return Pointer to result (valid until cleanup)
 */
struct rtbench_result *rtbench_result_get(void);

/**
 * @brief Set environment info
 */
void rtbench_result_set_env(const char *os_name, const char *os_version,
                            const char *board, const char *cpu_type,
                            uint32_t cpu_freq_mhz, uint32_t cpu_core_num);

/**
 * @brief Mark test start time
 */
void rtbench_result_start(void);

/**
 * @brief Mark test end time and calculate duration
 */
void rtbench_result_end(void);

/**
 * @brief Export result to JSON file
 * @param filepath Output file path
 * @return 0 on success, negative on error
 */
int rtbench_result_export_json(const char *filepath);

/**
 * @brief Export result to JUnit-compatible XML file
 * @param filepath Output file path
 * @return 0 on success, negative on error
 *
 * The Flow upload API currently accepts XML files only. This exporter keeps the
 * structured RTOS-Bench JSON payload inside the XML system-out section so the
 * uploaded artifact preserves the full benchmark result.
 */
int rtbench_result_export_xml(const char *filepath);

/**
 * @brief Export result to JSON string
 * @param buf Output buffer
 * @param bufsize Buffer size
 * @return Number of bytes written, or negative on error
 */
int rtbench_result_to_json(char *buf, size_t bufsize);

/**
 * @brief Set auto-export directory
 * @param dirpath Directory path (results saved as rtbench_result_<timestamp>.json)
 *
 * When set, results are automatically exported after rtbench_result_end()
 */
void rtbench_result_set_auto_export(const char *dirpath);

/**
 * @brief Generate filename with timestamp
 * @param buf Output buffer
 * @param bufsize Buffer size
 * @param prefix Filename prefix
 * @param ext File extension (e.g., ".json")
 */
void rtbench_result_gen_filename(char *buf, size_t bufsize,
                                  const char *prefix, const char *ext);

/* ============================================================================
 * Module Result Setters (called by test modules)
 * ============================================================================ */

/**
 * @brief Store realtime test results
 */
void rtbench_result_set_realtime(const struct rtbench_realtime_result *result);

/**
 * @brief Store schedule test results
 */
void rtbench_result_set_schedule(const struct rtbench_schedule_result *result);

/**
 * @brief Store stress test results
 */
void rtbench_result_set_stress(const struct rtbench_stress_result *result);

/**
 * @brief Store workload test results
 */
void rtbench_result_set_workload(const struct rtbench_workload_module_result *result);

/* ============================================================================
 * Helper Functions for Building Results
 * ============================================================================ */

/**
 * @brief Add a service cost entry to realtime result
 */
void rtbench_realtime_add_service_cost(struct rtbench_realtime_result *r,
                                        const char *op, double immediate,
                                        double suspend, double low_prio,
                                        double high_prio);

/**
 * @brief Add a memory bandwidth entry to realtime result
 */
void rtbench_realtime_add_mem_bw(struct rtbench_realtime_result *r,
                                  const char *type, double c1, double c2,
                                  double c4, double c8);

/**
 * @brief Add a WCET entry to schedule result
 */
void rtbench_schedule_add_wcet(struct rtbench_schedule_result *r,
                                const char *workload, double wcet_ms);

/**
 * @brief Add a stressor result to stress result
 */
void rtbench_stress_add_stressor(struct rtbench_stress_result *r,
                                  const char *name, const char *type,
                                  int stage,
                                  uint64_t bogo_ops, double duration,
                                  double metric_val, const char *metric_unit);

/**
 * @brief Add a workload result to workload module result
 */
void rtbench_workload_add_result(struct rtbench_workload_module_result *r,
                                  const char *name, const char *category,
                                  int success, int rounds,
                                  double exec_time_ms, double avg_time_ms);

#ifdef __cplusplus
}
#endif

#endif /* RESULT_EXPORT_H */
