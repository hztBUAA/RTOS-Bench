/*
 * This entry is a clang framework smoke entry for acceptance test TC-TOOL-002.
 * It intentionally excludes RTOS/BSP-dependent benchmark modules.
 *
 * The file models a minimal RTOS-Bench framework closure that can be compiled
 * as a freestanding ARM target object.  It does not depend on libc output,
 * pthread, shell/msh, serial, filesystem, BSP code, workloads, or RTOS APIs.
 */

static const char *const rtbench_clang_modules[] = {
    "test-all",
    "test-realtime",
    "test-schedule",
    "test-stress",
    "test-cmd",
    "typical-workload",
};

static const char rtbench_clang_version[] =
    "RTOS-Bench clang framework smoke TC-TOOL-002";

static const char rtbench_clang_help[] =
    "commands: help, version, modules, result";

static const char rtbench_clang_result_skeleton[] =
    "{"
    "\"meta\":{\"case\":\"TC-TOOL-002\",\"scope\":\"framework-smoke\"},"
    "\"env\":{\"target\":\"arm-none-eabi\",\"mode\":\"freestanding-object\"},"
    "\"modules\":["
    "\"test-all\","
    "\"test-realtime\","
    "\"test-schedule\","
    "\"test-stress\","
    "\"test-cmd\","
    "\"typical-workload\""
    "]"
    "}";

static int rtbench_clang_streq(const char *left, const char *right)
{
    if (left == 0 || right == 0) {
        return 0;
    }

    while (*left != '\0' && *right != '\0') {
        if (*left != *right) {
            return 0;
        }
        ++left;
        ++right;
    }

    return *left == *right;
}

static int rtbench_clang_count_bytes(const char *text)
{
    int count = 0;

    if (text == 0) {
        return 0;
    }

    while (text[count] != '\0') {
        ++count;
    }

    return count;
}

int rtbench_clang_list_modules(void)
{
    int count = 0;
    int index = 0;
    const int module_count =
        (int)(sizeof(rtbench_clang_modules) / sizeof(rtbench_clang_modules[0]));

    for (index = 0; index < module_count; ++index) {
        if (rtbench_clang_modules[index][0] != '\0') {
            ++count;
        }
    }

    return count;
}

int rtbench_clang_export_result_skeleton(void)
{
    return rtbench_clang_count_bytes(rtbench_clang_result_skeleton);
}

int rtbench_clang_dispatch(const char *cmd)
{
    if (rtbench_clang_streq(cmd, "help")) {
        return rtbench_clang_count_bytes(rtbench_clang_help);
    }

    if (rtbench_clang_streq(cmd, "version")) {
        return rtbench_clang_count_bytes(rtbench_clang_version);
    }

    if (rtbench_clang_streq(cmd, "modules") ||
        rtbench_clang_streq(cmd, "list")) {
        return rtbench_clang_list_modules();
    }

    if (rtbench_clang_streq(cmd, "result") ||
        rtbench_clang_streq(cmd, "export")) {
        return rtbench_clang_export_result_skeleton();
    }

    return -1;
}

int main(int argc, char **argv)
{
    if (argc > 1) {
        return rtbench_clang_dispatch(argv[1]) < 0 ? 2 : 0;
    }

    return rtbench_clang_list_modules() > 0 ? 0 : 1;
}
