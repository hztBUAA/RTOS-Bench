# Shell 命令支持测试 (test-cmd) 使用指南

本文档说明 RTOS-Bench 中的 Shell 命令支持检测功能 (`test-cmd`)，用于验证 RTOS 是否支持常见的 Shell 命令。

## 概述

`test-cmd` 是 RTOS-Bench 的独立子命令，通过逐一执行预定义的 Shell 命令序列来检测 RTOS 的命令行支持情况。测试涵盖文件管理、进程查看、目录操作等基础 Shell 功能。

**测试命令集**:
- `date` - 日期时间查看
- `ps` - 进程/线程列表
- `mkdir` - 创建目录
- `cd` - 切换目录
- `pwd` - 显示当前目录
- `echo` - 输出文本
- `cp` - 文件拷贝
- `mv` - 文件移动/重命名
- `ls` - 列出目录内容
- `cat` - 查看文件内容
- `rm` - 删除文件/目录

## 命令用法

### 基本用法

```bash
# RT-Thread msh 中
msh /> rtbench test-cmd

# OneOS 中
sh /> rtbench test-cmd
```

### 可选参数

```bash
rtbench test-cmd [OPTIONS]

OPTIONS:
  -q    安静模式，减少输出
```

## 输出格式

```
[test-cmd] Starting shell command support test
local time: Mon Jan  1 08:00:04 2018
thread   cpu bind pri  status      sp     stack size ...
...
/
./test_cmd_dir/backup.txt => ./test_cmd_dir/final_target.txt
Directory /:
...
=======================================================================
>> Result: Command 'date' finished.
>> Result: Command 'ps' finished.
>> Result: Command 'mkdir test_cmd_dir' finished.
>> Result: Command 'cd .' finished.
>> Result: Command 'pwd' finished.
>> Result: Command 'echo 'HELLO' ./test_cmd_dir/original.txt' finished.
>> Result: Command 'cp ...' finished.
>> Result: Command 'mv ...' finished.
>> Result: Command 'ls' finished.
>> Result: Command 'cat ...' finished.
>> Result: Command 'rm -r ./test_cmd_dir' finished.
=======================================================================
Supported commands: date ps mkdir cd pwd echo cp mv ls cat rm
Unsupported commands:
[test-cmd] Result: 11/11 commands supported
```

## 平台差异

不同 RTOS 的 Shell 命令语法存在差异，`test-cmd` 已针对每个平台预设了对应的命令序列：

| 差异点 | RT-Thread (msh) | OneOS (sh) | Linux / SylixOS / Dongtu |
|--------|-----------------|------------|--------------------------|
| 命令执行接口 | `msh_exec()` | `sh_exec()` | `system()` |
| echo 重定向 | 不支持 `>` | 支持 `>` | 支持 `>` |
| rm 删除目录 | `rm -r` | `rm` | `rm -rf` |
| mkdir 参数 | 无 `-p` | 无 `-p` | 支持 `-p` |

## 架构设计

### 命令执行抽象

```
┌────────────────────────────────────────────┐
│           rtbench test-cmd                  │
├────────────────────────────────────────────┤
│          test_cmd.c / test_cmd.h            │
│  ┌──────────────────────────────────────┐  │
│  │  #if RT_THREAD_PLATFORM              │  │
│  │    CMD_EXEC = msh_exec(cmd, len)     │  │
│  │  #elif ONEOS_PLATFORM                │  │
│  │    CMD_EXEC = sh_exec(cmd)           │  │
│  │  #elif LINUX / SYLIXOS / DONGTU      │  │
│  │    CMD_EXEC = system(cmd)            │  │
│  │  #else                               │  │
│  │    CMD_EXEC = -1 (不支持)             │  │
│  └──────────────────────────────────────┘  │
│                                            │
│  test_seq[] → 平台特定的命令序列            │
│  cmd_examine() → 执行并判断成功/失败        │
│  test_cmd_run() → 遍历执行并汇总结果       │
└────────────────────────────────────────────┘
```

### 数据结构

```c
/* 单命令结果 */
struct test_cmd_result {
    const char *command;   /* 完整命令字符串 */
    const char *name;      /* 命令名（首个单词） */
    int supported;         /* 1=成功, 0=失败 */
};

/* 模块结果 */
struct test_cmd_module_result {
    int valid;             /* 0=未运行, 1=有效 */
    int cmd_count;         /* 测试命令总数 */
    int pass_count;        /* 通过数 */
    struct test_cmd_result results[16];
};
```

### JSON 导出

在 `test-all` 模式下，结果自动纳入 JSON 导出：

```json
{
  "modules": {
    "test-cmd": {
      "status": "passed",
      "cmd_count": 11,
      "pass_count": 11,
      "commands": [
        { "name": "date", "command": "date", "supported": true },
        { "name": "ps", "command": "ps", "supported": true },
        { "name": "mkdir", "command": "mkdir test_cmd_dir", "supported": true },
        ...
      ]
    }
  }
}
```

## 关键源文件

| 文件 | 说明 |
|------|------|
| `generator/test_cmd.h` | 接口定义和数据结构 |
| `generator/test_cmd.c` | 实现（含多平台条件编译） |
| `generator/rtthread_entry.c` | RT-Thread 命令入口 |
| `generator/oneos_entry.c` | OneOS 命令入口 |
| `generator/sylixos_entry.c` | SylixOS 命令入口 |
| `generator/dongtu_entry.c` | Dongtu 命令入口 |

## 平台支持

| 平台 | 状态 | 命令执行方式 |
|------|------|-------------|
| RT-Thread | ✅ 已验证 (QEMU aarch64, 11/11) | `msh_exec()` |
| OneOS | ✅ 接入 | `sh_exec()` |
| SylixOS | ✅ 接入 | `system()` |
| Dongtu | ✅ 接入 | `system()` |
| Ruihua | ⚡ 编译支持 | `system()` (无独立入口) |
| Linux | ⚡ 编译支持 | `system()` |

## 故障排查

### 问题：所有命令都显示 failed

**可能原因**:
1. RTOS 未注册对应的 Shell 命令
2. 平台宏未正确定义（编译时走入了 `else` 分支）

**解决方案**:
- 确认 `RT_THREAD_PLATFORM` / `ONEOS_PLATFORM` 等宏已在构建系统中定义
- 检查 RTOS 的 finsh/msh 组件是否启用

### 问题：文件操作命令 (cp/mv/cat/rm) 失败

**可能原因**:
1. 前置的 `mkdir` 或 `echo` 命令未成功，后续命令找不到文件
2. 文件系统未挂载

**解决方案**:
- 确认 RTOS 已启用 DFS（设备文件系统）
- 确认存储设备（SD 卡/Flash/RomFS）已挂载到 `/`

### 问题：echo 命令行为异常

**说明**: RT-Thread msh 的 `echo` 不支持重定向 (`>`)，因此 RT-Thread 平台的测试序列使用不带重定向的 echo（只验证命令本身可用）。其他平台使用 `echo 'HELLO' > file` 以同时验证文件写入。

## 参考资料

1. 工业操作系统通用基准检测指标体系指导书 v1.5
2. RT-Thread finsh/msh 组件文档
3. RTOS-Bench 架构概览: [ARCHITECTURE_OVERVIEW.md](ARCHITECTURE_OVERVIEW.md)
