#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __RT_THREAD__


#ifdef __ONEOS__
#include <shell.h>
#define EXEC_CMD(cmd) sh_exec(cmd)
const char *test_seq[] = {
    "date",
    "ps",
    "mkdir test_dir",
    "cd .",
    "pwd",
    "echo 'HELLO' > ./test_dir/original.txt",
    "cp ./test_dir/original.txt ./test_dir/backup.txt",
    "mv ./test_dir/backup.txt ./test_dir/final_target.txt",
    "ls",
    "cat ./test_dir/original.txt",
    "rm ./test_dir",
};
#endif

#ifdef __RT_THREAD__
#include <msh.h>
#define EXEC_CMD(cmd) msh_exec(cmd, strlen(cmd))
const char *test_seq[] = {
    "date",
    "ps",
    "mkdir test_dir",
    "cd .",
    "pwd",
    "echo 'HELLO' ./test_dir/original.txt",
    "cp ./test_dir/original.txt ./test_dir/backup.txt",
    "mv ./test_dir/backup.txt ./test_dir/final_target.txt",
    "ls",
    "cat ./test_dir/original.txt",
    "rm -r ./test_dir",
};
#endif


#define TEST_CMD_COUNT (sizeof(test_seq) / sizeof(test_seq[0]))

int success_flag[50];

int cmd_examine(const char *s) {
	// 拷贝原命令
	char *buffer = (char *)malloc(strlen(s) + 1);
	if (buffer == NULL) {
		return 0;
	}
    strcpy(buffer, s);
    // 执行命令
    int ret = EXEC_CMD(buffer);
    if (ret < 0) {
        return 0;	//执行失败
    } else {
        return 1;	//执行成功
    }
}

void cmd_test(void) {
	// 测试
    for (int i = 0; i < TEST_CMD_COUNT; i++) {
        success_flag[i] = cmd_examine(test_seq[i]);
    }
    printf("=======================================================================\n");
    // 信息打印1
    for (int i = 0; i < TEST_CMD_COUNT; i++) {
        if (success_flag[i] == 1) {
            printf(">> Result: Command '%s' finished.\n", test_seq[i]);
        } else {
            printf(">> Result: Command '%s' failed.\n", test_seq[i]);
        }
    }
    printf("=======================================================================\n");
    // 信息打印2
    printf("Supported commands: ");
    for (int i = 0; i < TEST_CMD_COUNT; i++) {
        if (success_flag[i] == 1) {
        	const char *str = test_seq[i];
        	const char *space_ptr = strchr(str, ' ');
            if (space_ptr != NULL) {
				int length = space_ptr - str;
				printf("%.*s ", length, str);
			} else {
				printf("%s ", str);
			}
        }
    }
    printf("\n");
    printf("Unsupported commands: ");
    for (int i = 0; i < TEST_CMD_COUNT; i++) {
        if (success_flag[i] == 0) {
        	const char *str = test_seq[i];
        	const char *space_ptr = strchr(str, ' ');
            if (space_ptr != NULL) {
				int length = space_ptr - str;
				printf("%.*s ", length, str);
			} else {
				printf("%s ", str);
			}
        }
    }
    printf("\n");
    // 信息打印结束
}

#ifdef __ONEOS__
SH_CMD_EXPORT(cmdtest, cmd_test, "cmd test");
#endif

#ifdef __RT_THREAD__
MSH_CMD_EXPORT(cmd_test, cmd test);
#endif
