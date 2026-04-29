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
#ifdef RT_USING_DFS
#include <dfs_fs.h>
#endif
#define CMD_EXEC(cmd, len) msh_exec(cmd, len)
#define CMD_PRINTF rt_kprintf

/* Commands that require a writable filesystem */
static int cmd_needs_filesystem(const char *cmd)
{
    /* Extract first word */
    const char *fs_cmds[] = {"mkdir", "cp", "mv", "cat", "rm", "echo", NULL};
    for (int i = 0; fs_cmds[i] != NULL; i++) {
        size_t len = strlen(fs_cmds[i]);
        if (strncmp(cmd, fs_cmds[i], len) == 0 &&
            (cmd[len] == ' ' || cmd[len] == '\0')) {
            return 1;
        }
    }
    return 0;
}

static int test_cmd_has_filesystem(void)
{
#ifdef RT_USING_DFS
    struct dfs_filesystem *fs = dfs_filesystem_lookup("/");
    return (fs != NULL && fs->ops != NULL) ? 1 : 0;
#else
    return 0;
#endif
}

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

#elif defined(DONGTU_PLATFORM)

#include <cmd.h>

#define CMD_PRINTF printf

extern const struct shell *rtbench_dongtu_current_shell(void);

typedef int (*dongtu_shell_cmd_fn)(const struct shell *shell,
                                   size_t argc, char **argv);

struct dongtu_shell_cmd_entry {
    const char *name;
    dongtu_shell_cmd_fn fn;
};

static const struct dongtu_shell_cmd_entry dongtu_shell_cmds[] = {
    {"date", shell_main_date},
    {"task", shell_main_task},
    {"pwd", shell_main_pwd},
    {"ls", shell_main_ls},
    {"version", shell_main_version},
};

#define DONGTU_CMD_MAX_ARGS 8

static int dongtu_tokenize_command(char *cmd, char **argv, int max_args)
{
    int argc = 0;
    char *p = cmd;

    while (*p != '\0' && argc < max_args) {
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        if (*p == '\0') {
            break;
        }

        argv[argc++] = p;

        while (*p != '\0' && *p != ' ' && *p != '\t') {
            p++;
        }
        if (*p == '\0') {
            break;
        }
        *p++ = '\0';
    }

    return argc;
}

static int dongtu_cmd_exec(char *cmd, size_t len)
{
    const struct shell *shell = rtbench_dongtu_current_shell();
    char *argv[DONGTU_CMD_MAX_ARGS];
    int argc;

    (void)len;

    if (shell == NULL || cmd == NULL) {
        return -1;
    }

    argc = dongtu_tokenize_command(cmd, argv, DONGTU_CMD_MAX_ARGS);
    if (argc <= 0) {
        return -1;
    }

    for (size_t i = 0;
         i < sizeof(dongtu_shell_cmds) / sizeof(dongtu_shell_cmds[0]); i++) {
        if (strcmp(argv[0], dongtu_shell_cmds[i].name) == 0) {
            dongtu_shell_cmds[i].fn(shell, (size_t)argc, argv);
            return 0;
        }
    }

    return -1;
}

#define CMD_EXEC(cmd, len) dongtu_cmd_exec(cmd, len)

static const char *test_seq[] = {
    "date",
    "task",
    "pwd",
    "ls",
    "version",
};

#elif defined(LINUX_PLATFORM) || defined(__linux__) || \
      defined(SYLIXOS_PLATFORM) || defined(RUIHUA_PLATFORM)

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

#if defined(RT_THREAD_PLATFORM)
    int has_fs = test_cmd_has_filesystem();
    if (!has_fs) {
        CMD_PRINTF("[test-cmd] No writable filesystem detected, file commands will be skipped\n");
    }
#endif

    /* Run all commands */
    for (int i = 0; i < count; i++) {
#if defined(RT_THREAD_PLATFORM)
        if (!has_fs && cmd_needs_filesystem(test_seq[i])) {
            s_cmd_result.results[i].command = test_seq[i];
            s_cmd_result.results[i].name = extract_cmd_name(test_seq[i], i);
            s_cmd_result.results[i].supported = 0;
            CMD_PRINTF("[test-cmd] Skipped (no FS): %s\n", test_seq[i]);
            continue;
        }
#endif
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
