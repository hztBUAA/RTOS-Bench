/**
 * @file result_export.c
 * @brief RTOS-Bench result export implementation
 */

#include "result_export.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef RT_THREAD_PLATFORM
#include <rtthread.h>
#define RESULT_PRINTF rt_kprintf
#define RESULT_MALLOC rt_malloc
#define RESULT_FREE   rt_free
#else
#define RESULT_PRINTF printf
#define RESULT_MALLOC malloc
#define RESULT_FREE   free
#endif

/* Global result storage */
static struct rtbench_result g_result;
static int g_initialized = 0;
static char g_auto_export_dir[RTBENCH_MAX_PATH_LEN] = {0};
static time_t g_start_time = 0;

/* ============================================================================
 * Initialization and Cleanup
 * ============================================================================ */

int rtbench_result_init(void)
{
    memset(&g_result, 0, sizeof(g_result));
    strncpy(g_result.framework_version, RTBENCH_RESULT_VERSION,
            sizeof(g_result.framework_version) - 1);

    /* Initialize N/A values */
    g_result.realtime.context_switch_avg_us = -1;
    g_result.realtime.interrupt_min_us = -1;
    g_result.realtime.interrupt_max_us = -1;
    g_result.realtime.interrupt_avg_us = -1;
    g_result.realtime.syscall_min_us = -1;
    g_result.realtime.syscall_max_us = -1;
    g_result.realtime.syscall_avg_us = -1;

    g_initialized = 1;
    return 0;
}

void rtbench_result_cleanup(void)
{
    g_initialized = 0;
}

struct rtbench_result *rtbench_result_get(void)
{
    if (!g_initialized) {
        rtbench_result_init();
    }
    return &g_result;
}

/* ============================================================================
 * Environment and Timing
 * ============================================================================ */

void rtbench_result_set_env(const char *os_name, const char *os_version,
                            const char *board, const char *cpu_type,
                            uint32_t cpu_freq_mhz, uint32_t cpu_core_num)
{
    struct rtbench_env_info *env = &g_result.env;

    if (os_name) strncpy(env->os_name, os_name, sizeof(env->os_name) - 1);
    if (os_version) strncpy(env->os_version, os_version, sizeof(env->os_version) - 1);
    if (board) strncpy(env->board, board, sizeof(env->board) - 1);
    if (cpu_type) strncpy(env->cpu_type, cpu_type, sizeof(env->cpu_type) - 1);
    env->cpu_freq_mhz = cpu_freq_mhz;
    env->cpu_core_num = cpu_core_num;
}

void rtbench_result_start(void)
{
    g_start_time = time(NULL);

    /* Format ISO 8601 timestamp */
    struct tm *tm_info = gmtime(&g_start_time);
    if (tm_info) {
        strftime(g_result.test_timestamp, sizeof(g_result.test_timestamp),
                 "%Y-%m-%dT%H:%M:%SZ", tm_info);
    }
}

void rtbench_result_end(void)
{
    time_t end_time = time(NULL);
    g_result.total_duration_sec = difftime(end_time, g_start_time);

    /* Auto-export if configured */
    if (g_auto_export_dir[0] != '\0') {
        char filepath[RTBENCH_MAX_PATH_LEN];
        char filename[128];

        rtbench_result_gen_filename(filename, sizeof(filename),
                                     "rtbench_result_", ".json");
        snprintf(filepath, sizeof(filepath), "%s/%s",
                 g_auto_export_dir, filename);

        int ret = rtbench_result_export_json(filepath);
        if (ret == 0) {
            RESULT_PRINTF("[result-export] Auto-saved to: %s\n", filepath);
        } else {
            RESULT_PRINTF("[result-export] Auto-save failed: %d\n", ret);
        }
    }
}

void rtbench_result_set_auto_export(const char *dirpath)
{
    if (dirpath) {
        strncpy(g_auto_export_dir, dirpath, sizeof(g_auto_export_dir) - 1);
    } else {
        g_auto_export_dir[0] = '\0';
    }
}

void rtbench_result_gen_filename(char *buf, size_t bufsize,
                                  const char *prefix, const char *ext)
{
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    if (tm_info) {
        snprintf(buf, bufsize, "%s%04d%02d%02d_%02d%02d%02d%s",
                 prefix ? prefix : "",
                 tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
                 tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
                 ext ? ext : "");
    } else {
        snprintf(buf, bufsize, "%s%lu%s",
                 prefix ? prefix : "", (unsigned long)now, ext ? ext : "");
    }
}

/* ============================================================================
 * Module Result Setters
 * ============================================================================ */

void rtbench_result_set_realtime(const struct rtbench_realtime_result *result)
{
    if (result) {
        memcpy(&g_result.realtime, result, sizeof(g_result.realtime));
    }
}

void rtbench_result_set_schedule(const struct rtbench_schedule_result *result)
{
    if (result) {
        memcpy(&g_result.schedule, result, sizeof(g_result.schedule));
    }
}

void rtbench_result_set_stress(const struct rtbench_stress_result *result)
{
    if (result) {
        memcpy(&g_result.stress, result, sizeof(g_result.stress));
    }
}

void rtbench_result_set_workload(const struct rtbench_workload_module_result *result)
{
    if (result) {
        memcpy(&g_result.workload, result, sizeof(g_result.workload));
    }
}

/* ============================================================================
 * Helper Functions for Building Results
 * ============================================================================ */

void rtbench_realtime_add_service_cost(struct rtbench_realtime_result *r,
                                        const char *op, double immediate,
                                        double suspend, double low_prio,
                                        double high_prio)
{
    if (!r || r->service_cost_count >= RTBENCH_MAX_SERVICE_OPS) return;

    struct rtbench_service_cost *sc = &r->service_cost[r->service_cost_count++];
    if (op) strncpy(sc->operation, op, sizeof(sc->operation) - 1);
    sc->immediate_us = immediate;
    sc->suspend_us = suspend;
    sc->low_prio_us = low_prio;
    sc->high_prio_us = high_prio;
}

void rtbench_realtime_add_mem_bw(struct rtbench_realtime_result *r,
                                  const char *type, double c1, double c2,
                                  double c4, double c8)
{
    if (!r || r->mem_bw_count >= RTBENCH_MAX_MEM_BW_TYPES) return;

    struct rtbench_mem_bw_entry *e = &r->mem_bw[r->mem_bw_count++];
    if (type) strncpy(e->type, type, sizeof(e->type) - 1);
    e->c1 = c1; e->c2 = c2; e->c4 = c4; e->c8 = c8;
}

void rtbench_schedule_add_wcet(struct rtbench_schedule_result *r,
                                const char *workload, double wcet_ms)
{
    if (!r || r->wcet_count >= RTBENCH_MAX_WORKLOADS) return;

    struct rtbench_wcet_entry *e = &r->wcet[r->wcet_count++];
    if (workload) strncpy(e->workload, workload, sizeof(e->workload) - 1);
    e->wcet_ms = wcet_ms;
}

void rtbench_stress_add_stressor(struct rtbench_stress_result *r,
                                  const char *name, const char *type,
                                  int stage,
                                  uint64_t bogo_ops, double duration,
                                  double metric_val, const char *metric_unit)
{
    if (!r || r->stressor_count >= RTBENCH_MAX_STRESSORS) return;

    struct rtbench_stressor_result *s = &r->stressors[r->stressor_count++];
    if (name) strncpy(s->name, name, sizeof(s->name) - 1);
    if (type) strncpy(s->type, type, sizeof(s->type) - 1);
    s->stage = stage;
    s->bogo_ops = bogo_ops;
    s->duration_sec = duration;
    s->metric_value = metric_val;
    if (metric_unit) strncpy(s->metric_unit, metric_unit, sizeof(s->metric_unit) - 1);
}

void rtbench_workload_add_result(struct rtbench_workload_module_result *r,
                                  const char *name, const char *category,
                                  int success, int rounds,
                                  double exec_time_ms, double avg_time_ms)
{
    if (!r || r->workload_count >= RTBENCH_MAX_WORKLOADS) return;

    struct rtbench_workload_result *w = &r->workloads[r->workload_count++];
    if (name) strncpy(w->name, name, sizeof(w->name) - 1);
    if (category) strncpy(w->category, category, sizeof(w->category) - 1);
    w->success = success;
    w->rounds = rounds;
    w->exec_time_ms = exec_time_ms;
    w->avg_time_ms = avg_time_ms;
}

/* ============================================================================
 * JSON Serialization (Lightweight, no external deps)
 * ============================================================================ */

/* Helper macros for JSON output */
#define JSON_APPEND(...) do { \
    int n = snprintf(p, remain, __VA_ARGS__); \
    if (n < 0 || (size_t)n >= remain) goto truncated; \
    p += n; remain -= n; \
} while(0)

#define JSON_DOUBLE(name, val) do { \
    if ((val) >= 0) { \
        JSON_APPEND("\"%s\": %.6f", name, val); \
    } else { \
        JSON_APPEND("\"%s\": null", name); \
    } \
} while(0)

static const char *schedule_failure_reason_to_string(int reason)
{
    switch (reason) {
    case 0:
        return "none";
    case 1:
        return "timeout";
    case 2:
        return "setup_failure";
    default:
        return "unknown";
    }
}

int rtbench_result_to_json(char *buf, size_t bufsize)
{
    char *p = buf;
    size_t remain = bufsize;
    int i, j;
    const char *schedule_status;

    struct rtbench_result *r = &g_result;

    /* Root object */
    JSON_APPEND("{\n");

    /* Meta section */
    JSON_APPEND("  \"meta\": {\n");
    JSON_APPEND("    \"framework_version\": \"%s\",\n", r->framework_version);
    JSON_APPEND("    \"test_timestamp\": \"%s\",\n", r->test_timestamp);
    JSON_APPEND("    \"total_duration_sec\": %.3f\n", r->total_duration_sec);
    JSON_APPEND("  },\n");

    /* Env section */
    JSON_APPEND("  \"env\": {\n");
    JSON_APPEND("    \"os_name\": \"%s\",\n", r->env.os_name);
    JSON_APPEND("    \"os_version\": \"%s\",\n", r->env.os_version);
    JSON_APPEND("    \"board\": \"%s\",\n", r->env.board);
    JSON_APPEND("    \"cpu_type\": \"%s\",\n", r->env.cpu_type);
    JSON_APPEND("    \"cpu_freq_mhz\": %u,\n", r->env.cpu_freq_mhz);
    JSON_APPEND("    \"cpu_core_num\": %u\n", r->env.cpu_core_num);
    JSON_APPEND("  },\n");

    /* Modules section */
    JSON_APPEND("  \"modules\": {\n");

    /* test-realtime */
    JSON_APPEND("    \"test-realtime\": {\n");
    JSON_APPEND("      \"status\": \"%s\",\n", r->realtime.valid ? "passed" : "skipped");
    JSON_APPEND("      \"duration_sec\": %.3f,\n", r->realtime.duration_sec);

    if (r->realtime.valid) {
        /* Single-core */
        JSON_APPEND("      \"single_core\": {\n");
        JSON_APPEND("        \"context_switch\": { \"avg_us\": %.3f },\n",
                    r->realtime.context_switch_avg_us);
        JSON_APPEND("        \"interrupt\": { \"min_us\": %.3f, \"max_us\": %.3f, \"avg_us\": %.3f },\n",
                    r->realtime.interrupt_min_us, r->realtime.interrupt_max_us,
                    r->realtime.interrupt_avg_us);
        JSON_APPEND("        \"syscall\": { \"min_us\": %.3f, \"max_us\": %.3f, \"avg_us\": %.3f },\n",
                    r->realtime.syscall_min_us, r->realtime.syscall_max_us,
                    r->realtime.syscall_avg_us);

        /* Service cost array */
        JSON_APPEND("        \"service_cost\": [\n");
        for (i = 0; i < r->realtime.service_cost_count; i++) {
            struct rtbench_service_cost *sc = &r->realtime.service_cost[i];
            JSON_APPEND("          { \"operation\": \"%s\", ", sc->operation);
            if (sc->immediate_us >= 0) JSON_APPEND("\"immediate_us\": %.3f, ", sc->immediate_us);
            else JSON_APPEND("\"immediate_us\": null, ");
            if (sc->suspend_us >= 0) JSON_APPEND("\"suspend_us\": %.3f, ", sc->suspend_us);
            else JSON_APPEND("\"suspend_us\": null, ");
            if (sc->low_prio_us >= 0) JSON_APPEND("\"low_prio_us\": %.3f, ", sc->low_prio_us);
            else JSON_APPEND("\"low_prio_us\": null, ");
            if (sc->high_prio_us >= 0) JSON_APPEND("\"high_prio_us\": %.3f ", sc->high_prio_us);
            else JSON_APPEND("\"high_prio_us\": null ");
            JSON_APPEND("}%s\n", (i < r->realtime.service_cost_count - 1) ? "," : "");
        }
        JSON_APPEND("        ]\n");
        JSON_APPEND("      }");

        /* Multi-core (if available) */
        if (r->realtime.multicore_valid) {
            JSON_APPEND(",\n      \"multi_core\": {\n");

            /* Memory bandwidth */
            JSON_APPEND("        \"memory_bandwidth\": [\n");
            for (i = 0; i < r->realtime.mem_bw_count; i++) {
                struct rtbench_mem_bw_entry *e = &r->realtime.mem_bw[i];
                JSON_APPEND("          { \"type\": \"%s\", \"unit\": \"GB/s\", "
                            "\"c1\": %.3f, \"c2\": %.3f, \"c4\": %.3f, \"c8\": %.3f }%s\n",
                            e->type, e->c1, e->c2, e->c4, e->c8,
                            (i < r->realtime.mem_bw_count - 1) ? "," : "");
            }
            JSON_APPEND("        ],\n");

            /* IPC bandwidth */
            JSON_APPEND("        \"ipc_bandwidth\": { \"c1\": %.3f, \"c2\": %.3f, "
                        "\"c4\": %.3f, \"c8\": %.3f, \"unit\": \"GB/s\" },\n",
                        r->realtime.ipc_bw_c1, r->realtime.ipc_bw_c2,
                        r->realtime.ipc_bw_c4, r->realtime.ipc_bw_c8);

            /* Task latency */
            JSON_APPEND("        \"task_latency\": { \"c1\": %.3f, \"c2\": %.3f, "
                        "\"c4\": %.3f, \"c8\": %.3f, \"unit\": \"us\" },\n",
                        r->realtime.task_lat_c1, r->realtime.task_lat_c2,
                        r->realtime.task_lat_c4, r->realtime.task_lat_c8);

            /* Core communication */
            JSON_APPEND("        \"core_comm\": { \"intra_core\": %.3f, "
                        "\"inter_core\": %.3f, \"unit\": \"GB/s\" }\n",
                        r->realtime.core_comm_intra, r->realtime.core_comm_inter);
            JSON_APPEND("      }");
        }
        JSON_APPEND("\n");
    }
    JSON_APPEND("    },\n");

    /* test-schedule */
    if (!r->schedule.valid) {
        schedule_status = "skipped";
    } else if (r->schedule.completed_with_degradation) {
        schedule_status = "passed_with_degradation";
    } else {
        schedule_status = "passed";
    }

    JSON_APPEND("    \"test-schedule\": {\n");
    JSON_APPEND("      \"status\": \"%s\",\n", schedule_status);
    JSON_APPEND("      \"duration_sec\": %.3f", r->schedule.duration_sec);

    if (r->schedule.valid) {
        JSON_APPEND(",\n      \"config\": { \"cycles\": %d, \"util_start\": %d, "
                    "\"util_end\": %d, \"util_step\": %d },\n",
                    r->schedule.cycles, r->schedule.util_start,
                    r->schedule.util_end, r->schedule.util_step);

        /* WCET measurements */
        JSON_APPEND("      \"wcet_measurements\": [\n");
        for (i = 0; i < r->schedule.wcet_count; i++) {
            JSON_APPEND("        { \"workload\": \"%s\", \"wcet_ms\": %.3f }%s\n",
                        r->schedule.wcet[i].workload, r->schedule.wcet[i].wcet_ms,
                        (i < r->schedule.wcet_count - 1) ? "," : "");
        }
        JSON_APPEND("      ],\n");

        /* Gradients */
        JSON_APPEND("      \"gradients\": [\n");
        for (i = 0; i < r->schedule.gradient_count; i++) {
            struct rtbench_gradient_result *g = &r->schedule.gradients[i];
            JSON_APPEND("        {\n");
            JSON_APPEND("          \"utilization_percent\": %d,\n", g->utilization_percent);
            JSON_APPEND("          \"actual_utilization\": %.4f,\n", g->actual_utilization);
            JSON_APPEND("          \"total_jobs\": %llu,\n", (unsigned long long)g->total_jobs);
            JSON_APPEND("          \"deadline_misses\": %llu,\n", (unsigned long long)g->deadline_misses);
            JSON_APPEND("          \"miss_rate\": %.6f,\n", g->miss_rate);
            JSON_APPEND("          \"attempts\": %d,\n", g->attempts);
            if (g->degraded && g->failure_reason == 1) {
                JSON_APPEND("          \"status\": \"partial_timeout\",\n");
            } else {
                JSON_APPEND("          \"status\": \"%s\",\n", g->passed ? "passed" : "fallback_failed");
            }
            JSON_APPEND("          \"degraded\": %s,\n", g->degraded ? "true" : "false");
            JSON_APPEND("          \"failure_reason\": \"%s\",\n",
                        schedule_failure_reason_to_string(g->failure_reason));

            /* Task stats */
            JSON_APPEND("          \"task_stats\": [\n");
            for (j = 0; j < g->task_count; j++) {
                struct rtbench_task_stat *ts = &g->task_stats[j];
                JSON_APPEND("            { \"name\": \"%s\", \"utilization\": %.4f, "
                            "\"period_ms\": %.3f, \"jobs\": %llu, \"misses\": %llu, "
                            "\"max_response_ms\": %.3f }%s\n",
                            ts->name, ts->utilization, ts->period_ms,
                            (unsigned long long)ts->jobs, (unsigned long long)ts->misses,
                            ts->max_response_ms,
                            (j < g->task_count - 1) ? "," : "");
            }
            JSON_APPEND("          ]\n");
            JSON_APPEND("        }%s\n", (i < r->schedule.gradient_count - 1) ? "," : "");
        }
        JSON_APPEND("      ],\n");

        /* Summary */
        JSON_APPEND("      \"summary\": { \"average_miss_rate\": %.6f, \"final_score\": %.2f, "
                    "\"failed_gradients\": %d, \"completed_with_degradation\": %s, "
                    "\"total_retry_count\": %d }\n",
                    r->schedule.average_miss_rate, r->schedule.final_score,
                    r->schedule.failed_gradients,
                    r->schedule.completed_with_degradation ? "true" : "false",
                    r->schedule.total_retry_count);
    } else {
        JSON_APPEND("\n");
    }
    JSON_APPEND("    },\n");

    /* test-stress */
    JSON_APPEND("    \"test-stress\": {\n");
    JSON_APPEND("      \"status\": \"%s\",\n", r->stress.valid ? "passed" : "skipped");
    JSON_APPEND("      \"duration_sec\": %.3f", r->stress.duration_sec);

    if (r->stress.valid && r->stress.stressor_count > 0) {
        JSON_APPEND(",\n      \"stressors\": [\n");
        for (i = 0; i < r->stress.stressor_count; i++) {
            struct rtbench_stressor_result *s = &r->stress.stressors[i];
            JSON_APPEND("        { \"name\": \"%s\", \"type\": \"%s\", \"stage\": %d, "
                        "\"bogo_ops\": %llu, \"duration_sec\": %.3f",
                        s->name, s->type, s->stage,
                        (unsigned long long)s->bogo_ops, s->duration_sec);
            if (s->metric_unit[0] != '\0') {
                JSON_APPEND(", \"metric_value\": %.3f, \"metric_unit\": \"%s\"",
                            s->metric_value, s->metric_unit);
            }
            JSON_APPEND(" }%s\n", (i < r->stress.stressor_count - 1) ? "," : "");
        }
        JSON_APPEND("      ]\n");
    } else {
        JSON_APPEND("\n");
    }
    JSON_APPEND("    },\n");

    /* test-cmd */
    JSON_APPEND("    \"test-cmd\": {\n");
    JSON_APPEND("      \"status\": \"%s\"", r->cmd.valid ? "passed" : "skipped");

    if (r->cmd.valid) {
        JSON_APPEND(",\n      \"cmd_count\": %d,\n", r->cmd.cmd_count);
        JSON_APPEND("      \"pass_count\": %d,\n", r->cmd.pass_count);
        JSON_APPEND("      \"commands\": [\n");
        for (i = 0; i < r->cmd.cmd_count; i++) {
            struct rtbench_cmd_result *c = &r->cmd.results[i];
            JSON_APPEND("        { \"name\": \"%s\", \"command\": \"%s\", "
                        "\"supported\": %s }%s\n",
                        c->name, c->command,
                        c->supported ? "true" : "false",
                        (i < r->cmd.cmd_count - 1) ? "," : "");
        }
        JSON_APPEND("      ]\n");
    } else {
        JSON_APPEND("\n");
    }
    JSON_APPEND("    },\n");

    /* typical-workload */
    JSON_APPEND("    \"typical-workload\": {\n");
    JSON_APPEND("      \"status\": \"%s\",\n", r->workload.valid ? "passed" : "skipped");
    JSON_APPEND("      \"duration_sec\": %.3f", r->workload.duration_sec);

    if (r->workload.valid && r->workload.workload_count > 0) {
        JSON_APPEND(",\n      \"workloads\": [\n");
        for (i = 0; i < r->workload.workload_count; i++) {
            struct rtbench_workload_result *w = &r->workload.workloads[i];
            JSON_APPEND("        { \"name\": \"%s\", \"category\": \"%s\", "
                        "\"success\": %s, \"rounds\": %d, "
                        "\"exec_time_ms\": %.3f, \"avg_time_ms\": %.3f }%s\n",
                        w->name, w->category,
                        w->success ? "true" : "false", w->rounds,
                        w->exec_time_ms, w->avg_time_ms,
                        (i < r->workload.workload_count - 1) ? "," : "");
        }
        JSON_APPEND("      ]\n");
    } else {
        JSON_APPEND("\n");
    }
    JSON_APPEND("    }\n");

    /* Close modules and root */
    JSON_APPEND("  }\n");
    JSON_APPEND("}\n");

    return (int)(p - buf);

truncated:
    RESULT_PRINTF("[result-export] JSON buffer too small\n");
    return -1;
}

int rtbench_result_export_json(const char *filepath)
{
    /* Allocate a large buffer for JSON */
    size_t bufsize = 128 * 1024;  /* 128KB should be enough */
    char *buf = (char *)RESULT_MALLOC(bufsize);
    if (!buf) {
        RESULT_PRINTF("[result-export] Failed to allocate buffer\n");
        return -1;
    }

    int len = rtbench_result_to_json(buf, bufsize);
    if (len < 0) {
        RESULT_FREE(buf);
        return -1;
    }

    /* Write to file */
    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        RESULT_PRINTF("[result-export] Failed to open %s\n", filepath);
        RESULT_FREE(buf);
        return -2;
    }

    size_t written = fwrite(buf, 1, len, fp);
    fclose(fp);
    RESULT_FREE(buf);

    if (written != (size_t)len) {
        RESULT_PRINTF("[result-export] Write error: %zu/%d\n", written, len);
        return -3;
    }

    return 0;
}

#undef JSON_APPEND
#undef JSON_DOUBLE
