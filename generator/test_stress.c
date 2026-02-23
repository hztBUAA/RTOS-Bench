/**
 * @file test_stress.c
 * @brief Stress test implementation for RTOS-Bench
 * @details Wraps rtos_stress (stress-ng port) for power consumption testing
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

/* Forward declaration - implemented in stress_orig/ */
extern int stress_ng_main(int argc, char **argv);
extern int stress_ng_main_stop(void);

/* Stressor name table */
static const char *stressor_names[] = {
    "cpu",
    "matrix",
    "vm",
    "malloc",
    "memcpy",
    "prime",
    "trig",
    "fp",
    NULL
};

static const char *stress_type_to_name(stress_type_t type)
{
    if (type >= 0 && type < STRESS_TYPE_ALL) {
        return stressor_names[type];
    }
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
        /* Run all stressors sequentially */
        STRESS_PRINTF("\n");
        STRESS_PRINTF("=============================================================\n");
        STRESS_PRINTF("[test-stress] Running ALL stressors (%d seconds each)\n", duration);
        STRESS_PRINTF("=============================================================\n");

        for (int i = 0; stressor_names[i] != NULL; i++) {
            int r = test_stress_run_stressor(stressor_names[i], duration);
            if (r != 0) {
                ret = r;
            }
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

int test_stress_run(void)
{
    struct test_stress_config config = {
        .type = STRESS_TYPE_CPU,
        .duration_sec = 10,
        .num_workers = 1,
        .quiet = 0
    };

    return test_stress_run_config(&config);
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

void test_stress_stop(void)
{
    stress_ng_main_stop();
}
