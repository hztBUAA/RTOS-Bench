/* applications/stress-ng/stress-str.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <config.h>

static int stress_strcasecmp_internal(const char *s1, const char *s2)
{
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;
    int result;

    if (p1 == p2) return 0;

    while ((result = stress_osal_tolower(*p1) - stress_osal_tolower(*p2++)) == 0) {
        if (*p1++ == '\0') break;
    }
    return result;
}

static int stress_strncasecmp_internal(const char *s1, const char *s2, size_t n)
{
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;
    int result;

    if (p1 == p2 || n == 0) return 0;

    while ((result = stress_osal_tolower(*p1) - stress_osal_tolower(*p2++)) == 0) {
        if (*p1++ == '\0' || --n == 0) break;
    }
    return result;
}

static int32_t s_str_size = DEFAULT_STR_SIZE;

static int stress_str_opt_size(const char *opt_name, const char *opt_arg)
{
    char *endptr;
    unsigned long long val = strtoull(opt_arg, &endptr, 10);

    if (*endptr == 'k' || *endptr == 'K') val *= 1024;
    else if (*endptr == 'm' || *endptr == 'M') val *= (1024 * 1024);

    if (val < MIN_STR_SIZE) val = MIN_STR_SIZE;
    if (val > MAX_STR_SIZE) val = MAX_STR_SIZE;

    s_str_size = (int32_t)val;
    stress_osal_print("rtos_stress: debug: str-size set to %d bytes\n", s_str_size);
    return 0;
}

const stress_opt_t stress_str_opts[] = {
    { "str-size", stress_str_opt_size },
    { NULL, NULL }
};

typedef struct stress_str_args {
    stress_args_t *args;
    char *str1;
    size_t len1;
    char *str2;
    size_t len2;
    char *strdst;
    size_t strdstlen;
} stress_str_args_t;

static void stress_rndstr(char *str, size_t len)
{
    size_t i;
    for (i = 0; i < len - 1; i++) {
        str[i] = ' ' + (stress_osal_rand() % 95);
    }
    str[len - 1] = '\0';
}

#define STRCHK(name, test) \
    do { \
        if (!(test)) { \
            stress_osal_print("rtos_stress: %s check failed at line %d\n", name, __LINE__); \
            return; \
        } \
    } while (0)

typedef void (*stress_str_func)(stress_str_args_t *info);

static void stress_str_strcmp(stress_str_args_t *info)
{
    const char *str1 = info->str1;
    const char *str2 = info->str2;
    size_t i;

    for (i = 1; i < info->len1; i++) {
        if (!stress_continue(info->args)) break;
        STRCHK("strcmp", 0 == stress_osal_strcmp(str1, str1));
        STRCHK("strcmp", 0 != stress_osal_strcmp(str1, str2));
        STRCHK("strcmp", 0 != stress_osal_strcmp(str1 + i, str1));
        info->args->bogo.current_ops++;
    }
}

static void stress_str_strncmp(stress_str_args_t *info)
{
    const char *str1 = info->str1;
    const char *str2 = info->str2;
    size_t i;
    for (i = 1; i < info->len1; i++) {
        if (!stress_continue(info->args)) break;
        STRCHK("strncmp", 0 == stress_osal_strncmp(str1, str1, info->len1));
        STRCHK("strncmp", 0 != stress_osal_strncmp(str1, str2, info->len2));
        info->args->bogo.current_ops++;
    }
}

static void stress_str_strcasecmp(stress_str_args_t *info)
{
    const char *str1 = info->str1;
    const char *str2 = info->str2;
    size_t i;
    for (i = 1; i < info->len1; i++) {
        if (!stress_continue(info->args)) break;
        STRCHK("strcasecmp", 0 == stress_strcasecmp_internal(str1, str1));
        STRCHK("strcasecmp", 0 != stress_strcasecmp_internal(str1, str2));
        info->args->bogo.current_ops++;
    }
}

static void stress_str_strncasecmp(stress_str_args_t *info)
{
    const char *str1 = info->str1;
    const char *str2 = info->str2;
    size_t i;
    for (i = 1; i < info->len1; i++) {
        if (!stress_continue(info->args)) break;
        STRCHK("strncasecmp", 0 == stress_strncasecmp_internal(str1, str1, info->len1));
        STRCHK("strncasecmp", 0 != stress_strncasecmp_internal(str1, str2, info->len2));
        info->args->bogo.current_ops++;
    }
}

static void stress_str_strlen(stress_str_args_t *info)
{
    const char *str1 = info->str1;
    size_t i;
    for (i = 0; i < info->len1 - 1; i++) {
        if (!stress_continue(info->args)) break;
        STRCHK("strlen", (info->len1 - 1 - i) == stress_osal_strlen(str1 + i));
        info->args->bogo.current_ops++;
    }
}

static void stress_str_strcpy(stress_str_args_t *info)
{
    const char *str1 = info->str1;
    char *strdst = info->strdst;
    size_t i;
    for (i = 0; i < info->len1 - 1; i++) {
        if (!stress_continue(info->args)) break;
        STRCHK("strcpy", strdst == stress_osal_strcpy(strdst, str1));
        info->args->bogo.current_ops++;
    }
}

static void stress_str_strcat(stress_str_args_t *info)
{
    const char *str1 = info->str1;
    char *strdst = info->strdst;
    size_t i;

    if (info->strdstlen < (info->len1 * 2)) {
        stress_osal_print("rtos_stress: error: strdstlen too small for strcat test\n");
        return;
    }

    for (i = 0; i < info->len1 - 1; i++) {
        if (!stress_continue(info->args)) break;
        strdst[0] = '\0';
        STRCHK("strcat", strdst == stress_osal_strcat(strdst, str1));
        STRCHK("strcat", strdst == stress_osal_strcat(strdst, str1));
        info->args->bogo.current_ops++;
    }
}

static void stress_str_strchr(stress_str_args_t *info)
{
    const char *str1 = info->str1;
    size_t i;
    for (i = 0; i < info->len1 - 1; i++) {
        if (!stress_continue(info->args)) break;
        STRCHK("strchr", NULL != stress_osal_strchr(str1, str1[i]));
        STRCHK("strchr", NULL == stress_osal_strchr(str1, 1));
        info->args->bogo.current_ops++;
    }
}

typedef struct {
    const char *name;
    stress_str_func func;
} stress_str_method_info_t;

static const stress_str_method_info_t str_methods[] = {
    { "strcmp",      stress_str_strcmp },
    { "strncmp",     stress_str_strncmp },
    { "strcasecmp",  stress_str_strcasecmp },
    { "strncasecmp", stress_str_strncasecmp },
    { "strlen",      stress_str_strlen },
    { "strcpy",      stress_str_strcpy },
    { "strcat",      stress_str_strcat },
    { "strchr",      stress_str_strchr },
    { NULL,          NULL }
};

void stress_str(stress_args_t *args)
{
    size_t len1 = (size_t)s_str_size;
    size_t len2 = len1 / 2;
    if (len2 < 16) len2 = len1;
    size_t lendst = (len1 * 2) + 32;

    stress_str_args_t info;
    info.args = args;
    info.len1 = len1;
    info.len2 = len2;
    info.strdstlen = lendst;
    info.str1 = NULL;
    info.str2 = NULL;
    info.strdst = NULL;

    info.str1 = (char *)stress_osal_malloc(len1);
    info.str2 = (char *)stress_osal_malloc(len2);
    info.strdst = (char *)stress_osal_malloc(lendst);

    if (!info.str1 || !info.str2 || !info.strdst) {
        stress_osal_print("rtos_stress: error: [str] OOM allocating strings\n");
        goto cleanup;
    }

    stress_osal_print("rtos_stress: info: [str-%d] string size %d bytes\n", args->instance, (int)len1);

    stress_rndstr(info.str1, len1);
    stress_rndstr(info.str2, len2);

    stress_str_func specific_func = NULL;
    stress_bool_t run_all = STRESS_FALSE;

    if (args->method_name == NULL || stress_osal_strcmp(args->method_name, "all") == 0) {
        run_all = STRESS_TRUE;
        stress_osal_print("rtos_stress: info: [str-%d] using 'all' methods\n", args->instance);
    } else {
        for (int i = 0; str_methods[i].name != NULL; i++) {
            if (stress_osal_strcmp(args->method_name, str_methods[i].name) == 0) {
                specific_func = str_methods[i].func;
                break;
            }
        }
        if (!specific_func) {
            stress_osal_print("rtos_stress: error: unknown method '%s', using 'all'\n", args->method_name);
            run_all = STRESS_TRUE;
        } else {
            stress_osal_print("rtos_stress: info: [str-%d] using method '%s'\n", args->instance, args->method_name);
        }
    }

    while (stress_continue(args))
    {
        if (args->bogo.current_ops % 100 == 0) {
            stress_rndstr(info.str2, len2);
        }

        if (run_all) {
            for (int i = 0; str_methods[i].name != NULL; i++) {
                if (!stress_continue(args)) break;
                str_methods[i].func(&info);
            }
        } else {
            specific_func(&info);
        }

        if (args->bogo.current_ops % 50 == 0) {
            stress_osal_sleep_ms(1);
        }
    }

cleanup:
    if (info.str1) stress_osal_free(info.str1);
    if (info.str2) stress_osal_free(info.str2);
    if (info.strdst) stress_osal_free(info.strdst);
}
