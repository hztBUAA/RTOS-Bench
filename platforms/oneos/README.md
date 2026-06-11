# RTOS-Bench for OneOS 使用指南

本文档提供在 OneOS 上运行 RTOS-Bench 的步骤。

---

## 目录

1. [环境准备](#1-环境准备)
2. [测试套件移植](#2-测试套件移植)
3. [编译、部署和运行](#3-编译、部署和运行)
4. [贡献指南](#4-贡献指南)

---

## 1. 环境准备

### 安装 OneOS-Studio-V2.0

1. 从中移对接群中下载 IDE （最新：V2.0）、模板工程及使用说明（最新：V1.4）
2. 安装到本地 (建议路径不含中文和空格)
3. 启动 IDE，创建或打开一个工作空间 (Workspace)

### 哪吒派 D1H

哪吒派 D1H 使用 OneOS multi-CMake/Linux 命令行工程，和飞腾派 OneOS
Studio 工程不同。全量 workload 编译、TFTP/Telnet 验收和校园网路由器接入
流程见 [NEZHA_D1H_SOP.md](NEZHA_D1H_SOP.md)。

如果多人协作或需要摆脱个人 Windows TFTP 目录，请使用共享主机
`rtbench:/tftp` 的语义化目录流程，见
[NEZHA_D1H_SHARED_TFTP_SOP.md](NEZHA_D1H_SHARED_TFTP_SOP.md)。

---

## 2. 测试套件移植

### 使用飞腾派工程

#### A.1: 导入工程

1. 将厂家提供的phytium_pi V1.5工程复制到工作区下
2. 在 IDE 中点击左上角“电视”图标，导入工程phytium_pi
3. 点击左上角“+”号，创建一个新的工程，命名为phytium_pi_out，类型为Out Project，依赖phytium_pi工程

（**注**：目前最新版工程为projects V1.5，分为 phytium_pi 和 phytium_pi_out 两个工程，前者编译出系统内核，后者编译出依赖内核的模块，与翼辉系统的base和app类似。在使用时，也要先导入phytium_pi，再创建out。）

#### A.2: 添加 RTOS-Bench 源码

```bash
# 切换到phytium_pi_out目录下
git clone https://github.com/hztBUAA/RTOS-Bench
```

#### A.3: 移动和修改文件

*注：步骤1、2的修改针对image工程，编译出的镜像为oneos.bin；若测试主机上已存在该文件，可不必重复从个人电脑上传到测试主机。*

1. 将 phytium_pi_out/RTOS-bench/platforms/oneos/rtbench_cmd_stub.c 复制到 phytium_pi/application 下

2. 修改phytium_pi/application/main.c的内容：
```
// 新增头文件
#include <telnetd.h>


int main(void *arg)
{

/* ...... */

// 新增内容：自动加载模块、启动telnet
    os_module_ld("/user/phytium_pi_out.out", OS_FALSE);
    telnet_server_init(23);
    telnet_server_start();


// 原有内容
    os_task_tsleep(1000000);

    return 0;
}
```

3. 在 phytium_pi_out/CMakeLists.txt 结尾新增内容：
```
# 添加子目录
add_subdirectory(src)

# 新增
add_subdirectory(RTOS-Bench)
```

#### A.4: 最终的目录结构

```
phytium_pi/                         # 内核工程
├── application/
│   ├── ...
│   ├── main.c                          # 修改
│   └── rtbench_cmd_stub.c              # 新增
phytium_pi_out/                     # out工程
├── include/                            # 默认创建
│   ├── hello.h
├── src/                                # 默认创建
│   └── hello.c
├── RTOS-Bench/                         # 基准测试框架
└── CMakeLists.txt
```

（注：可将hello.c整个删掉，以免一直打印重复信息）

---

## 3. 编译、部署和运行

### 3.0 哪吒派共享 TFTP 流程

OneOS 哪吒派 D1H 推荐使用共享主机 `rtbench:/tftp` 作为统一部署入口，避免依赖某一台 Windows 电脑的本地 TFTP 服务。多人协作时，开发者在自己的电脑或厂家 IDE 中编译 `.out`，再通过 `scp` 上传到 `rtbench:/tftp`，板子统一从 `192.168.31.110` 拉取。

详细步骤见 [NEZHA_D1H_SHARED_TFTP_SOP.md](NEZHA_D1H_SHARED_TFTP_SOP.md)。

2026-06-10 哪吒派 D1H 完整验收通过的 `.out` 二进制已在 Windows 本机归档，路径和 SHA256 见 [NEZHA_D1H_ACCEPTED_BINARIES_20260610.md](NEZHA_D1H_ACCEPTED_BINARIES_20260610.md)。

若只使用 Windows 电脑和哪吒派直连进行快速 debug，不经过 `rtbench` 共享主机，请阅读闭包文档 [NEZHA_D1H_WIN_DIRECT_DEBUG_CLOSURE.md](NEZHA_D1H_WIN_DIRECT_DEBUG_CLOSURE.md)。其中包含 Windows 本地 TFTP、OneOS 启动镜像与 `.out` 动态模块的区别、构建数据流、部署命令和验收证据。

2026-06-11 已完成 Win 直连 TFTP + `COM9` 串口复验，日志摘要为 `utils/remote-test/logs/oneos-nezha-d1h-win-direct-20260611_152709/oneos_nezha_serial_tftp_acceptance_20260611_152711.md`，返回码为 `0`。

### 3.1 编译
点击IDE中“锤子”图标编译工程

### 3.2 部署

注：如果能通过telnet连接到开发板，跳过步骤A。

#### A 启动飞腾派

+ 确保飞腾派串口连接正确
+ 在测试主机上，通过 sudo minicom -D /dev/ttyUSB0 串口连接到开发板
+ ~~若提示“设备已锁定”，ps aux | grep ttyUSB0，把minicom进程kill掉~~
+ 执行 tftp 0x80100000 oneos.bin（如果该文件在/tftp中不存在，可在~/tbh下寻找）
+ 执行 go 0x80100000 启动系统


#### B 上传out模块

请阅读中移**使用说明V1.5 “5.1 动态加载使用用法”**
+ 先通过telnet连接到开发板，注意Linux命令行会自动断开连接。最好使用软件进行连接，如MobaXterm，在软件中设置Terminal Settings > Expert Settings > implicit CR in every LF，以解决缺少\n到\r\n转换的输出格式问题
+ 将out模块上传到测试主机的/tftp文件夹，打开测试主机终端，执行 tftp_client 192.168.31.110 get phytium_pi_out.out  /user/phytium_pi_out.out，上传out模块。**注意"phytium_pi_out.out"模块名与"/user/phytium_pi_out.out"路径不要修改**
+ 使用 list_lmodule 查看已导入模块。由于模块可重复导入，先使用 unld 清除已有的模块。
+ 执行ld /user/phytium_pi_out.out
+ **注意，由于导入的模块名被设置为此命令中的路径，请不要对"/user/phytium_pi_out.out"这一路径进行修改**，以免内核工程中的rtbench_cmd_stub.c找不到该模块。

### 3.3 运行
执行 rtbench 命令，开始运行
> 常见错误：
> Error: Module '/user/phytium_pi_out.out' not loaded.
> 请确保执行ld /user/phytium_pi_out.out这条语句来导入模块

---

## 4. 常见问题

**修改代码后编译未更新**：
+ 请尝试清理编译，再重新编译。

**对于中移提供的V1.4到V1.5的内容更新的部分解释：**
```
# 开机后配置
# 网络配置
set_if e01 192.168.31.205 192.168.31.1 255.255.255.0
default_netif e01
# 启动telnet
telnetd start
# 挂载文件系统
mkdir /user
mount -t fatfs sdmmc0a2 /user
# 上传模块
tftp_client 192.168.31.110 get phytium_pi_out.out /user/phytium_pi_out.out
# 加载模块
ld /user/phytium_pi_out.out
```
+ 开机后网络配置：系统启动后默认无IP配置，已通过创建/user/interfaces.txt配置，启动即自动配置
+ 文件系统：V1.5模板工程会自动挂载
+ 启动telnet：在main.c中编写代码，自动启动
+ 加载模块：在main.c中编写代码，自动加载
+ 上传模块：若out模块有改动，请参照3.2进行上传、模块卸载与加载
+ **更多详细内容请查阅中移飞腾派V1.5工程文档**
---

## 联系与贡献

- GitHub: https://github.com/hztBUAA/RTOS-Bench
- Issues: 欢迎提交问题和建议
- PR: 欢迎贡献代码
