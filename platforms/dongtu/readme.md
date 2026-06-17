# 东土/Intewell 平台 RTOS-Bench 项目部署文档

## 1. 下载并安装 IDE

东土/Intewell 的 x86 平台和 ARM 平台使用不同的 IDE 环境。部署前必须确认目标平台，避免使用错误的 IDE 创建或导入工程。

- x86 工程：使用 x86 对应的 InteWell IDE。
- ARM / 香橙派 / vm_3588 工程：使用 ARM 对应的 InteWell IDE。

不要用 x86 IDE 打开 ARM 工程，也不要用 ARM IDE 创建 x86 工程。否则后续会出现工具链、BSP、编译宏和链接库不匹配的问题。

---

## 2. 创建或导入项目

### 2.1 x86 项目创建方式

打开 x86 对应的 InteWell IDE，按以下步骤创建项目：

```text
左上角 文件
  -> 新建
  -> InteWell RTOS 应用项目
  -> 项目体系选择 x86
  -> 编程语言选择 C++
  -> 完成
```

项目创建完成后，IDE 会在工作空间中生成一个 x86 RTOS 应用工程，例如：

```text
rtosbench_x86/
  src/
```
---

### 2.2 香橙派 / vm_3588 项目导入方式

打开 ARM 对应的 InteWell IDE，按以下步骤导入已经下载好的 3588 工程文件夹：

```text
左上角 文件
  -> 导入
  -> 常规
  -> 现有项目到工作空间中
  -> 选择下载的 3588 文件夹
```

选择后，IDE 中会显示两个项目：

```text
os
vm_3588
```

勾选：

```text
将项目复制到工作空间中
```

然后点击：

```text
完成
```

---

## 3. 部署 Makefile 接入文件

### 3.1 复制 config_os 示例文件

将 RTOS-Bench 仓库中的示例配置文件复制到目标 InteWell 工程根目录，并重命名为：

```text
config_os.mk
```

源文件路径为：

```text
RTOS-Bench/platforms/config_os_example.mk
```

复制后的目标位置示例：

x86 工程：

```text
A:/IntelWell-IDE/eclipse/workspace/rtosbench_x86/config_os.mk
```

香橙派 / vm_3588 工程：

```text
A:/IntelWell-IDE/eclipse/workspace/vm_3588/config_os.mk
```

---

### 3.2 配置 config_os.mk

`config_os.mk` 只放项目相关配置，例如 RTOS-Bench 仓库路径和当前目标架构。不要把临时修复参数、强制 include 参数或源码补丁写在这里。

示例内容如下：

```makefile
################################################################################
# Example config_os.mk for Dongtu/Intewell projects.
#
# Copy this file to the Intewell project root and rename it to config_os.mk.
################################################################################

RTOS_BENCH_ROOT := A:/IntelWell-IDE/eclipse/workspace/rtos_bench/RTOS-Bench

# x86/i386 project:
ARCH := _X86_

# Orange Pi / vm_3588 project:
# ARCH := __ARM64__

include $(RTOS_BENCH_ROOT)/platforms/dongtu/intewell.mk
```

配置说明：

- `RTOS_BENCH_ROOT` 必须改成当前电脑上 RTOS-Bench 仓库的真实路径。
- x86 工程使用：

```makefile
ARCH := _X86_
```

- 香橙派 / vm_3588 工程使用：

```makefile
ARCH := __ARM64__
```

- `include` 行用于接入东土平台统一 Makefile：

```makefile
include $(RTOS_BENCH_ROOT)/platforms/dongtu/intewell.mk
```

---

## 4. 调整项目入口文件 userAppInit

### 4.1 x86 工程

x86 工程创建时，如果 `src/` 目录下默认生成的是：

```text
src/userAppInit.cpp
```

需要将其重命名为：

```text
src/userAppInit.c
```

然后将 RTOS-Bench 提供的东土入口文件内容复制进去。

源文件路径：

```text
RTOS-Bench/platforms/userAppInit.c
```

---

### 4.2 香橙派 / vm_3588 工程

香橙派 / vm_3588 工程同样需要检查 `src/userAppInit.*` 文件。

如果工程中已有旧的 `userAppInit.c` 或 `userAppInit.cpp`，建议统一替换为 RTOS-Bench 提供的：

```text
RTOS-Bench/platforms/userAppInit.c
```

替换后，确认工程入口文件为：

```text
src/userAppInit.c
```

---

## 5. 构建项目

在 InteWell IDE 中右键目标项目，选择：

```text
构建项目
```

或：

```text
重新构建项目
```

构建过程中，IDE 会自动进入项目的 `Debug/make` 目录执行 Makefile。

构建成功后，会在项目路径下生成镜像文件，通常位于：

```text
项目根目录/Debug/make/
```
该目录下通常会生成：

```text
*.elf
*.bin
```

实际部署运行时一般使用 `.bin` 镜像文件。

---

## 6. 运行项目

## 6.1 x86 平台运行方式

### 6.1.1 获取编译产物

构建完成后，在 x86 工程目录中找到生成的 `.bin` 文件，例如：

```text
A:/IntelWell-IDE/eclipse/workspace/rtosbench_x86/Debug/make/rtosbench_x86.bin
```

如果文件名不同，以 `Debug/make` 目录中实际生成的 `.bin` 文件为准。

---

### 6.1.2 连接东土 x86 运行环境

在 Windows PowerShell 中执行：

```powershell
ssh -L 5556:192.168.31.240:5556 rtbench@10.134.151.45 -p 1026
```

输入密码：

```text
rtbench
```

该命令会将远端东土 IDE / 管理页面的：

```text
192.168.31.240:5556
```

转发到本机：

```text
127.0.0.1:5556
```

然后在浏览器中打开：

```text
http://127.0.0.1:5556
```

---

### 6.1.3 上传并生效镜像

进入浏览器页面后，按以下步骤操作：

```text
实时虚拟机
  -> rtos_bench
  -> 进入右侧配置/编辑页面
  -> 镜像
  -> 上传 Debug/make 下生成的 .bin 镜像
  -> 生效
  -> 点击生效
```

这里上传的是本地 InteWell IDE 编译生成的 `.bin` 文件。网页中的“编译”或“配置”入口只是进入该虚拟机的管理页面，不代表需要在网页端重新编译 RTOS-Bench。

---

### 6.1.4 在初始化中运行
如果你在userAppInit.c中调用了主函数，则系统启动时会立刻执行该任务并输出日志:

或者dongtu_entry在系统中导出了符号rtbench，可以直接在终端中运行命令,参见6.1.5节
```c
#include <commonTypes.h>
#include <syscallIoctl.h>
#include "dongtu_entry.c"

T_UWORD logAddr = 0;
T_UWORD sysTimerVectorInt=0;
int userAppInit(void)
{
	VMK_Ioctl(LOG_ADDR_GET, &logAddr, &sysTimerVectorInt);
	debugEventInit((T_VOID *)logAddr, 0x8000*4, 0xffffffff);
	/* argv: argv[0] should be program name, argv[1] the subcommand */
	char *argv[] = { "rtbench", "test-stress", "--job", "file", NULL };
	int argc = 0;
	while (argv[argc] != NULL) {
		argc++;
	}
	rtbench_dongtu_entry(argc, argv);
    return 0;
}
```
在网页管理界面中进入：

```text
系统服务配置
  -> 日志配置
```

默认日志输出位置为东土工控机上的：

```text
/download/log.txt
```

在前面 PowerShell 的 SSH 终端中继续连接东土工控机：

```sh
ssh kd@192.168.31.240
```

输入密码：

```text
1
```

查看日志：

```sh
cat /download/log.txt
```

如果需要实时观察输出，使用：

```sh
tail -f /download/log.txt
```

---

### 6.1.5 在终端中运行

## 6.2 香橙派 / vm_3588 平台运行方式

此处留空

## 7. 常见问题检查

### 7.1 使用了错误 IDE

现象：

```text
编译器路径、BSP 路径、库路径或 ARCH 宏不匹配
```

检查：

- x86 工程必须使用 x86 IDE。
- 香橙派 / vm_3588 工程必须使用 ARM IDE。
- `config_os.mk` 中的 `ARCH` 必须与当前工程一致。

---

### 7.2 config_os.mk 路径错误

现象：

```text
找不到 RTOS-Bench 源码
找不到 platforms/dongtu/intewell.mk
```

检查：

```makefile
RTOS_BENCH_ROOT := A:/IntelWell-IDE/eclipse/workspace/rtos_bench/RTOS-Bench
```

必须指向 RTOS-Bench 仓库根目录，而不是 `platforms` 目录，也不是 InteWell 工程目录。

---

### 7.3 userAppInit 未参与链接

现象：

```text
undefined reference to `userAppInit'
```

检查：

- 项目中是否存在：

```text
src/userAppInit.c
```

- 是否仍然保留旧的：

```text
src/userAppInit.cpp
```

- `Debug/make` 中是否生成了：

```text
src/userAppInit.o
```

- 东土平台 Makefile 中是否把 `src/userAppInit.o` 加入最终对象列表或归档列表。

---

### 7.4 dongtu_entry.c 头文件未正确包含

现象：

```text
unknown type name 'sem_t'
TEST_SCHEDULE_QUICK_CYCLES undeclared
implicit declaration of function ...
```

处理原则：

- 不建议在 `config_os.mk` 中加入强制 `-include` 补丁。
- `config_os.mk` 只保留项目路径和架构配置。
- 需要的头文件应直接在对应 `.c` 文件中正常 `#include`。
- 旧的、未使用的测试线程代码应从 `dongtu_entry.c` 中清理，而不是通过 Makefile 做临时补丁。

---

### 7.5 intewell_stub 未定义

现象：

```text
undefined reference to `intewell_stub'
```

检查：

- `intewell.mk` 中是否已经包含东土平台所需的 hook 源文件。
- 不建议恢复大而杂的 `compat.c`。
- 如果仅需要该符号，应在东土平台 hook 文件中提供最小实现。

---

## 8. 推荐目录结构

RTOS-Bench 仓库中东土平台相关文件建议保持如下结构：

```text
RTOS-Bench/
  platforms/
    config_os_example.mk
    userAppInit.c
    dongtu/
      intewell.mk
      readme.md
      VALIDATION_*.md
```

如果仍采用平台内聚目录，也可以保持：

```text
RTOS-Bench/
  platforms/
    dongtu/
      config_os.example.mk
      userAppInit.c
      intewell.mk
      readme.md
      VALIDATION_*.md
```

不建议继续保留以下历史重复目录：

```text
platforms/dongtu/x86/
platforms/dongtu/香橙派/
```

x86 和香橙派 / vm_3588 的差异应该通过 `config_os.mk` 中的 `ARCH` 配置区分，而不是维护两套重复源码。
:::

