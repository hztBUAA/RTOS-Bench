/**
 * @file test_cmd.c
 * @brief Shell command support test implementation
 * @details Ported from cmd-test/cmdtest.c prototype.
 *          Tests common shell commands via platform-specific exec API.
 */

#include "test_cmd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * Platform-specific command execution
 * ============================================================================ */

#if defined(RT_THREAD_PLATFORM)

#include <rtthread.h>
#include <msh.h>
#define CMD_EXEC(cmd, len) msh_exec(cmd, len)
#define CMD_PRINTF rt_kprintf

static const char *test_seq[] = {
    "date",
    "ps",
    "mkdir test_cmd_dir",
    "cd .",
    "pwd",
    "echo 'HELLO' ./test_cmd_dir/original.txt",
    "cp ./test_cmd_dir/original.txt ./test_cmd_dir/backup.txt",
    "mv ./test_cmd_dir/backup.txt ./test_cmd_dir/final_target.txt",
    "ls",
    "cat ./test_cmd_dir/original.txt",
    "rm -r ./test_cmd_dir",
};

#elif defined(ONEOS_PLATFORM)

#include <shell.h>
#define CMD_EXEC(cmd, len) sh_exec(cmd)
#define CMD_PRINTF printf

static const char *test_seq[] = {
    "date",
    "ps",
    "mkdir test_cmd_dir",
    "cd .",
    "pwd",
    "echo 'HELLO' > ./test_cmd_dir/original.txt",
    "cp ./test_cmd_dir/original.txt ./test_cmd_dir/backup.txt",
    "mv ./test_cmd_dir/backup.txt ./test_cmd_dir/final_target.txt",
    "ls",
    "cat ./test_cmd_dir/original.txt",
    "rm ./test_cmd_dir",
};

#elif defined(LINUX_PLATFORM) || defined(__linux__)

#define CMD_EXEC(cmd, len) system(cmd)
#define CMD_PRINTF printf

static const char *test_seq[] = {
    "date",
    "ps",
    "mkdir -p test_cmd_dir",
    "cd .",
    "pwd",
    "echo 'HELLO' > ./test_cmd_dir/original.txt",
    "cp ./test_cmd_dir/original.txt ./test_cmd_dir/backup.txt",
    "mv ./test_cmd_dir/backup.txt ./test_cmd_dir/final_target.txt",
    "ls",
    "cat ./test_cmd_dir/original.txt",
    "rm -rf ./test_cmd_dir",
};

#else

/* Unsupported platform stub */
#define CMD_EXEC(cmd, len) (-1)
#define CMD_PRINTF printf

static const char *test_seq[] = {
    "date",
    "ps",
    "mkdir test_cmd_dir",
    "cd .",
    "pwd",
    "echo 'HELLO'",
    "cp",
    "mv",
    "ls",
    "cat",
    "rm test_cmd_dir",
};

#endif

#define TEST_CMD_COUNT (sizeof(test_seq) / sizeof(test_seq[0]))

/* Static result storage */
static struct test_cmd_module_result s_cmd_result;

/* Static name buffers - avoid dynamic allocation for command names */
static char s_name_bufs[TEST_CMD_MAX_COMMANDS][32];

/**
 * @brief Execute a single command and return success/failure
 */
static int cmd_examine(const char *s)
{
    /* Copy to mutable buffer (msh_exec may modify its argument) */
    size_t len = strlen(s);
    char *buffer = (char *)malloc(len + 1);
    if (buffer == NULL) {
        return 0;
    }
    strcpy(buffer, s);

    int ret = CMD_EXEC(buffer, len);
    free(buffer);

    return (ret < 0) ? 0 : 1;
}

/**
 * @brief Extract command name (first word) into a static buffer
 */
static const char *extract_cmd_name(const char *cmd, int index)
{
    if (index < 0 || index >= TEST_CMD_MAX_COMMANDS) {
        return cmd;
    }

    const char *space = strchr(cmd, ' ');
    if (space != NULL) {
        size_t name_len = (size_t)(space - cmd);
        if (name_len >= sizeof(s_name_bufs[0])) {
            name_len = sizeof(s_name_bufs[0]) - 1;
        }
        memcpy(s_name_bufs[index], cmd, name_len);
        s_name_bufs[index][name_len] = '\0';
    } else {
        strncpy(s_name_bufs[index], cmd, sizeof(s_name_bufs[0]) - 1);
        s_name_bufs[index][sizeof(s_name_bufs[0]) - 1] = '\0';
    }
    return s_name_bufs[index];
}

int test_cmd_run(void)
{
    int count = (int)TEST_CMD_COUNT;
    if (count > TEST_CMD_MAX_COMMANDS) {
        count = TEST_CMD_MAX_COMMANDS;
    }

    memset(&s_cmd_result, 0, sizeof(s_cmd_result));
    s_cmd_result.cmd_count = count;
    s_cmd_result.pass_count = 0;

    /* Run all commands */
    for (int i = 0; i < count; i++) {
        int ok = cmd_examine(test_seq[i]);
        s_cmd_result.results[i].command = test_seq[i];
        s_cmd_result.results[i].name = extract_cmd_name(test_seq[i], i);
        s_cmd_result.results[i].supported = ok;
        if (ok) {
            s_cmd_result.pass_count++;
        }
    }

    s_cmd_result.valid = 1;

    /* Print individual results */
    CMD_PRINTF("=======================================================================\n");
    for (int i = 0; i < count; i++) {
        CMD_PRINTF(">> Result: Command '%s' %s.\n",
                   s_cmd_result.results[i].command,
                   s_cmd_result.results[i].supported ? "finished" : "failed");
    }
    CMD_PRINTF("=======================================================================\n");

    /* Print summary */
    CMD_PRINTF("Supported commands: ");
    for (int i = 0; i < count; i++) {
        if (s_cmd_result.results[i].supported) {
            CMD_PRINTF("%s ", s_cmd_result.results[i].name);
        }
    }
    CMD_PRINTF("\n");

    CMD_PRINTF("Unsupported commands: ");
    for (int i = 0; i < count; i++) {
        if (!s_cmd_result.results[i].supported) {
            CMD_PRINTF("%s ", s_cmd_result.results[i].name);
        }
    }
    CMD_PRINTF("\n");

    CMD_PRINTF("[test-cmd] Result: %d/%d commands supported\n",
               s_cmd_result.pass_count, s_cmd_result.cmd_count);

    return 0;
}

const struct test_cmd_module_result *test_cmd_get_result(void)
{
    return &s_cmd_result;
}
