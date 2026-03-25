#include "stress-ng.h"
#include "stress_osal.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <config.h>

#define STRESS_STR_INNER_LOOPS  (16)

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

    if (*endptr == 'k' || *endptr == 'K') val *= 1024ULL;
    else if (*endptr == 'm' || *endptr == 'M') val *= (1024ULL * 1024ULL);

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
    size_t cmp_len;
    char *strdst;
    size_t strdstlen;
} stress_str_args_t;

static void stress_rndstr(char *str, size_t len)
{
    size_t i;
    if (len < 2) {
        if (len == 1) str[0] = '\0';
        return;
    }
    for (i = 0; i < len - 1; i++) {
        str[i] = (char)(' ' + (stress_osal_rand() % 95));
    }
    str[len - 1] = '\0';
}

static void stress_ensure_different(char *str1, char *str2, size_t len2, size_t cmp_len)
{
    int attempts;
    size_t pos;

    for (attempts = 0; attempts < 8; attempts++) {
        if (stress_osal_strcmp(str1, str2) != 0 &&
            stress_strcasecmp_internal(str1, str2) != 0 &&
            stress_osal_strncmp(str1, str2, cmp_len) != 0 &&
            stress_strncasecmp_internal(str1, str2, cmp_len) != 0) {
            return;
        }
        stress_rndstr(str2, len2);
    }

    for (pos = 0; pos < cmp_len && pos < len2 - 1; pos++) {
        unsigned char c1_lower = (unsigned char)stress_osal_tolower((unsigned char)str1[pos]);
        unsigned char candidate;

        candidate = (c1_lower != 'x') ? 'x' : 'y';
        str2[pos] = (char)candidate;

        if ((unsigned char)stress_osal_tolower((unsigned char)str1[pos]) !=
            (unsigned char)stress_osal_tolower(candidate)) {
            break;
        }
    }
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
    size_t step;
    size_t i;
    size_t inner;

    //STRCHK("strcmp-self", 0 == stress_osal_strcmp(str1, str1));
    STRCHK("strcmp-diff", 0 != stress_osal_strcmp(str1, str2));

    if (info->len1 <= 1) {
        info->args->bogo.current_ops++;
        return;
    }

    step = (info->len1 - 2) / STRESS_STR_INNER_LOOPS;
    if (step < 1) step = 1;

    for (inner = 0; inner < STRESS_STR_INNER_LOOPS; inner++) {
        if (!stress_continue(info->args)) break;
        i = 1 + inner * step;
        if (i >= info->len1 - 1) break;
        STRCHK("strcmp-offset", 0 != stress_osal_strcmp(str1 + i, str1));
        info->args->bogo.current_ops++;
    }
}

static void stress_str_strncmp(stress_str_args_t *info)
{
    const char *str1 = info->str1;
    const char *str2 = info->str2;
    size_t cmp_len = info->cmp_len;
    size_t inner;

    STRCHK("strncmp-self", 0 == stress_osal_strncmp(str1, str1, info->len1));
    STRCHK("strncmp-diff", 0 != stress_osal_strncmp(str1, str2, cmp_len));

    for (inner = 0; inner < STRESS_STR_INNER_LOOPS; inner++) {
        if (!stress_continue(info->args)) break;
        STRCHK("strncmp-loop", 0 == stress_osal_strncmp(str1, str1, cmp_len));
        info->args->bogo.current_ops++;
    }
}

static void stress_str_strcasecmp(stress_str_args_t *info)
{
    const char *str1 = info->str1;
    const char *str2 = info->str2;
    size_t inner;

    STRCHK("strcasecmp-self", 0 == stress_strcasecmp_internal(str1, str1));
    STRCHK("strcasecmp-diff", 0 != stress_strcasecmp_internal(str1, str2));

    for (inner = 0; inner < STRESS_STR_INNER_LOOPS; inner++) {
        if (!stress_continue(info->args)) break;
        STRCHK("strcasecmp-loop", 0 == stress_strcasecmp_internal(str1, str1));
        info->args->bogo.current_ops++;
    }
}

static void stress_str_strncasecmp(stress_str_args_t *info)
{
    const char *str1 = info->str1;
    const char *str2 = info->str2;
    size_t cmp_len = info->cmp_len;
    size_t inner;

    STRCHK("strncasecmp-self", 0 == stress_strncasecmp_internal(str1, str1, info->len1));
    STRCHK("strncasecmp-diff", 0 != stress_strncasecmp_internal(str1, str2, cmp_len));

    for (inner = 0; inner < STRESS_STR_INNER_LOOPS; inner++) {
        if (!stress_continue(info->args)) break;
        STRCHK("strncasecmp-loop", 0 == stress_strncasecmp_internal(str1, str1, cmp_len));
        info->args->bogo.current_ops++;
    }
}

static void stress_str_strlen(stress_str_args_t *info)
{
    const char *str1 = info->str1;
    size_t step;
    size_t inner;
    size_t i;

    STRCHK("strlen-base", (info->len1 - 1) == stress_osal_strlen(str1));

    if (info->len1 <= 1) {
        info->args->bogo.current_ops++;
        return;
    }

    step = (info->len1 - 1) / STRESS_STR_INNER_LOOPS;
    if (step < 1) step = 1;

    for (inner = 0; inner < STRESS_STR_INNER_LOOPS; inner++) {
        if (!stress_continue(info->args)) break;
        i = inner * step;
        if (i >= info->len1 - 1) break;
        STRCHK("strlen-offset", (info->len1 - 1 - i) == stress_osal_strlen(str1 + i));
        info->args->bogo.current_ops++;
    }
}

static void stress_str_strcpy(stress_str_args_t *info)
{
    const char *str1 = info->str1;
    char *strdst = info->strdst;
    size_t inner;

    for (inner = 0; inner < STRESS_STR_INNER_LOOPS; inner++) {
        if (!stress_continue(info->args)) break;
        STRCHK("strcpy", strdst == stress_osal_strcpy(strdst, str1));
        info->args->bogo.current_ops++;
    }
}

static void stress_str_strcat(stress_str_args_t *info)
{
    const char *str1 = info->str1;
    char *strdst = info->strdst;
    size_t inner;

    if (info->strdstlen < (info->len1 * 2)) {
        stress_osal_print("rtos_stress: error: strdstlen too small for strcat test\n");
        return;
    }

    for (inner = 0; inner < STRESS_STR_INNER_LOOPS; inner++) {
        if (!stress_continue(info->args)) break;
        strdst[0] = '\0';
        STRCHK("strcat-1", strdst == stress_osal_strcat(strdst, str1));
        STRCHK("strcat-2", strdst == stress_osal_strcat(strdst, str1));
        info->args->bogo.current_ops++;
    }
}

static void stress_str_strchr(stress_str_args_t *info)
{
    const char *str1 = info->str1;
    size_t step;
    size_t inner;
    size_t i;

    STRCHK("strchr-absent", NULL == stress_osal_strchr(str1, 1));

    if (info->len1 <= 1) {
        info->args->bogo.current_ops++;
        return;
    }

    step = (info->len1 - 1) / STRESS_STR_INNER_LOOPS;
    if (step < 1) step = 1;

    for (inner = 0; inner < STRESS_STR_INNER_LOOPS; inner++) {
        if (!stress_continue(info->args)) break;
        i = inner * step;
        if (i >= info->len1 - 1) break;
        STRCHK("strchr-present", NULL != stress_osal_strchr(str1, str1[i]));
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
    size_t len1;
    size_t len2;
    size_t cmp_len;
    size_t lendst;
    stress_str_args_t info;
    stress_str_func specific_func;
    stress_bool_t run_all;
    int i;

    len1   = (size_t)s_str_size;
    len2   = len1 / 2;
    if (len2 < 16) len2 = len1;
    cmp_len = (len2 > 1) ? (len2 / 2) : 1;
    lendst = (len1 * 2) + 32;

    specific_func = NULL;
    run_all       = STRESS_FALSE;

    info.args      = args;
    info.len1      = len1;
    info.len2      = len2;
    info.cmp_len   = cmp_len;
    info.strdstlen = lendst;
    info.str1      = NULL;
    info.str2      = NULL;
    info.strdst    = NULL;

    info.str1   = (char *)stress_osal_malloc(len1);
    info.str2   = (char *)stress_osal_malloc(len2);
    info.strdst = (char *)stress_osal_malloc(lendst);

    if (!info.str1 || !info.str2 || !info.strdst) {
        stress_osal_print("rtos_stress: error: [str] OOM allocating strings\n");
        goto cleanup;
    }

    stress_osal_print("rtos_stress: info: [str-%d] string size %d bytes\n",
                      args->instance, (int)len1);

    stress_rndstr(info.str1, len1);
    stress_rndstr(info.str2, len2);
    stress_ensure_different(info.str1, info.str2, len2, cmp_len);

    if (args->method_name == NULL ||
        stress_osal_strcmp(args->method_name, "all") == 0) {
        run_all = STRESS_TRUE;
        stress_osal_print("rtos_stress: info: [str-%d] using 'all' methods\n",
                          args->instance);
    } else {
        for (i = 0; str_methods[i].name != NULL; i++) {
            if (stress_osal_strcmp(args->method_name, str_methods[i].name) == 0) {
                specific_func = str_methods[i].func;
                break;
            }
        }
        if (!specific_func) {
            stress_osal_print("rtos_stress: error: unknown method '%s', using 'all'\n",
                              args->method_name);
            run_all = STRESS_TRUE;
        } else {
            stress_osal_print("rtos_stress: info: [str-%d] using method '%s'\n",
                              args->instance, args->method_name);
        }
    }

    while (stress_continue(args)) {
        if (args->bogo.current_ops > 0 && args->bogo.current_ops % 100 == 0) {
            stress_rndstr(info.str2, len2);
            stress_ensure_different(info.str1, info.str2, len2, cmp_len);
        }

        if (run_all) {
            for (i = 0; str_methods[i].name != NULL; i++) {
                if (!stress_continue(args)) break;
                str_methods[i].func(&info);
            }
        } else {
            specific_func(&info);
        }

        if (args->bogo.current_ops > 0 && args->bogo.current_ops % 50 == 0) {
            stress_osal_sleep_ms(1);
        }
    }

cleanup:
    if (info.str1)   stress_osal_free(info.str1);
    if (info.str2)   stress_osal_free(info.str2);
    if (info.strdst) stress_osal_free(info.strdst);
}
