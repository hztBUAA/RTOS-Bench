/* applications/stress-ng/core-helper.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>

/*
 * 判断是否继续运行 (Keep Stressing)
 * 检查：全局停止位、超时、操作次数上限
 */
stress_bool_t stress_continue(stress_args_t *args)
{
    /* 1. 强制停止 */
    if (g_stress_global_stop) {
        return STRESS_FALSE;
    }

    /* 2. 检查时间限制 (-t) */
    if (args->time_end != 0) {
        stress_tick_t now = stress_osal_tick_get();
        if (now >= args->time_end) {
            return STRESS_FALSE;
        }
    }

    /* 3. 检查操作次数限制 (--ops) */
    if (args->bogo.max_ops > 0) {
        if (args->bogo.current_ops >= args->bogo.max_ops) {
            return STRESS_FALSE;
        }
    }

    return STRESS_TRUE;
}

/*
 * 解析时间字符串
 * 输入: "500ms", "0.5s", "10s", "1m", "1h"
 * 输出: OSAL Ticks
 */
stress_tick_t stress_parse_time(const char *str)
{
    char *endptr;
    double val = strtod(str, &endptr);

    if (val <= 0.0) return 0;

    double ticks_per_sec = (double)stress_osal_tick_hz();
    double total_ticks = 0.0;

    if (endptr && *endptr != '\0') {
        if (stress_osal_strncmp(endptr, "ms", 2) == 0) {
            /* 毫秒: value / 1000.0 * Hz */
            total_ticks = (val / 1000.0) * ticks_per_sec;
        }
        else if (*endptr == 's') {
            /* 秒: value * Hz */
            total_ticks = val * ticks_per_sec;
        }
        else if (*endptr == 'm') {
            /* 分: value * 60 * Hz */
            total_ticks = val * 60.0 * ticks_per_sec;
        }
        else if (*endptr == 'h') {
            /* 时: value * 3600 * Hz */
            total_ticks = val * 3600.0 * ticks_per_sec;
        }
        else {
            /* 未知单位，默认按秒处理 */
            total_ticks = val * ticks_per_sec;
        }
    } else {
        /* 无单位，默认按秒处理 */
        total_ticks = val * ticks_per_sec;
    }

    return (stress_tick_t)total_ticks;
}

double stress_time_now(void)
{
    return stress_osal_time_now();
}
