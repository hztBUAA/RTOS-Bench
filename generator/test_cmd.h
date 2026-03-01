/**
 * @file test_cmd.h
 * @brief Shell command support test for RTOS-Bench
 * @details Tests whether the RTOS supports common shell commands
 *          (date, ps, mkdir, cd, pwd, echo, cp, mv, ls, cat, rm).
 *
 * Usage: rtbench test-cmd [-q]
 */

#ifndef TEST_CMD_H
#define TEST_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

#define TEST_CMD_MAX_COMMANDS  16

/**
 * @brief Result for a single command test
 */
struct test_cmd_result {
    const char *command;   /**< Full command string */
    const char *name;      /**< Command name (first word) */
    int supported;         /**< 1 = success, 0 = failure */
};

/**
 * @brief Aggregate result for the test-cmd module
 */
struct test_cmd_module_result {
    int valid;             /**< 0 = not run, 1 = valid */
    int cmd_count;         /**< Total number of commands tested */
    int pass_count;        /**< Number of commands that succeeded */
    struct test_cmd_result results[TEST_CMD_MAX_COMMANDS];
};

/**
 * @brief Run shell command support test with default command list
 * @return 0 on success, negative on error
 */
int test_cmd_run(void);

/**
 * @brief Get pointer to last test-cmd result
 * @return Pointer to static result structure (valid until next test_cmd_run)
 */
const struct test_cmd_module_result *test_cmd_get_result(void);

#ifdef __cplusplus
}
#endif

#endif /* TEST_CMD_H */
