## 命令行基准测试工具说明

### 测试目的
检测test_seq中的基准命令行command是否受到系统支持

### 测试内容
date ps mkdir cd pwd echo cp mv ls cat rm 

### 测试逻辑
主要测试逻辑具有跨平台通用性
```c
// 辅助函数
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

// 主测试函数
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
```

### 样例输出
```
=======================================================================
>> Result: Command 'date' finished.
>> Result: Command 'ps' failed.
>> Result: Command 'mkdir test_dir' finished.
>> Result: Command 'cd .' finished.
>> Result: Command 'pwd' finished.
>> Result: Command 'echo 'HELLO' > ./test_dir/original.txt' finished.
>> Result: Command 'cp ./test_dir/original.txt ./test_dir/backup.txt' finished.
>> Result: Command 'mv ./test_dir/backup.txt ./test_dir/final_target.txt' finished.
>> Result: Command 'ls' finished.
>> Result: Command 'cat ./test_dir/original.txt' finished.
>> Result: Command 'rm ./test_dir' finished.
=======================================================================
Supported commands: date mkdir cd pwd echo cp mv ls cat rm 
Unsupported commands: ps
```

### 抽象层说明

```c
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
```

1. 每个OS都定义一个相同的宏EXEC_CMD，并调用OS中执行命令的函数，如rt-thread是msh_exec()
2. 每个OS都定义了一个测试序列，即const char *test_seq[]，是因为虽然在不同OS上选取了相同的命令，但它们的格式不尽相同。例如rt-thread是 echo 'string' \<position\>，而oneos是 echo 'string' > \<position\>；又比如rm删除文件夹时加不加-r因OS而异。
3. 目前所使用的条件编译，需要和框架层相连接，使其尽量能跟随框架层设置进行编译