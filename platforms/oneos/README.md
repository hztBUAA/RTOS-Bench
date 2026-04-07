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

### 准备一个tftp工具

可以安装 Tftpd64，以便向开发板传输文件

---

## 2. 测试套件移植

### 使用飞腾派工程

#### A.1: 导入工程

1. 将厂家提供的phytium_pi工程复制到工作区下
2. 在 IDE 中点击左上角“电视”图标，导入工程phytium_pi
3. 点击左上角“+”号，创建一个新的工程，命名为phytium_pi_out，类型为Out Project，依赖phytium_pi工程

（**注**：目前最新版工程为projects V1.4，分为 phytium_pi 和 phytium_pi_out 两个工程，前者编译出系统内核，后者编译出依赖内核的模块，与翼辉系统的base和app类似。在使用时，也要先导入phytium_pi，再创建out。）

#### A.2: 添加 RTOS-Bench 源码

**方式 1：Git Submodule（推荐）**
```bash
# 切换到phytium_pi_out目录下
git init  # 如果还不是 git 仓库
git submodule add https://github.com/hztBUAA/RTOS-Bench.git
cd RTOS-Bench
git checkout feat/oneos
```

**方式 2：直接克隆**
```bash
# 切换到phytium_pi_out目录下
git clone https://github.com/hztBUAA/RTOS-Bench.git
cd RTOS-Bench
git checkout feat/oneos
```

（注：hello.c可直接删除）

#### A.3: 移动和修改文件

1. 将 phytium_pi_out/RTOS-bench/platforms/oneos/rtbench_cmd_stub.c 复制到 phytium_pi/application 下
2. 在 phytium_pi_out/CMakeLists.txt 结尾新增内容：
```
# 添加子目录
add_subdirectory(src) 

# 新增
add_subdirectory(RTOS-Bench) 
```
3. 最终的目录结构如下：

```
phytium_pi/                         # 内核工程
├── application/
│   ├── ...
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

可参照中移提供的 OneOS使用说明-v1.4 进行编译、部署和运行。

### 3.1 编译
点击IDE中“锤子”图标编译工程

### 3.2 部署
请阅读中移**使用说明V1.4 “5.1 动态加载使用用法”**
+ 如果能通过telnet连接到单板，则只需导入out模块并执行（并且应该可以跳过ip设置）
+ 打开电脑上的tftp工具，执行 tftp_client <你的主机ip>  get phytium_pi_out.out  /user/phytium_pi_out.out，将你的主机作为服务端，上传out模块。
+ **注意"phytium_pi_out.out"模块名与"/user/phytium_pi_out.out"路径不要修改**
+ 使用 list_lmodule 查看已导入模块。由于模块可重复导入，可先使用 unld 清除已有的模块。
+ 执行ld /user/phytium_pi_out.out，注意，由于导入的模块名被设置为此命令中的路径，请不要对"/user/phytium_pi_out.out"这一路径进行修改，以免rtbench_cmd_stub.c找不到该模块。

### 3.3 运行
执行 rtbench 命令，开始运行
> 常见错误：
> Error: Module '/user/phytium_pi_out.out' not loaded.
> 请确保执行ld /user/phytium_pi_out.out这条语句来导入模块

---

## 4. 贡献指南

**我的改动**

> 1. CMakeLists.txt
**参考翼辉的编译逻辑，编写了CMake编译文件**
> 2. generator/oneos_entry.c
在oneos程序入口处加入了实时性能入口（与翼辉逻辑相同），加入了方便内核调用的包装层，（函数cmd_rtbench_stub）
> 3. generator/periodic_benchmark.c, 
进行了oneos的条件编译设置
> 4. platforms/oneos/mqtt_stub.c
编写了桩函数
> 5. platforms/oneos/rtbench_cmd_stub.c
在内核中预留了"rtbench"命令，以便加载模块后能够带参数执行
> 6. platforms/oneos/README.md
编写了说明文档 

**待完成的任务**

- 框架层代码编译
> 报错信息请见 BUILDLOG.md
- 压力测试的系统入口补充
- 对压力测试、可调度性测试及负载测试的命令的支持（在oneos_entry.c中）
> 目前负载测试基础命令已支持，但尚未实现"rtbench -b pid -p 0.1 -t 10"中的 -p/-t 等附加选项
- mqtt模块的调试

---

## 5. 常见问题

+ **修改代码后编译未更新**：请尝试清理编译，再重新编译。

---

## 联系与贡献

- GitHub: https://github.com/hztBUAA/RTOS-Bench
- Issues: 欢迎提交问题和建议
- PR: 欢迎贡献代码
