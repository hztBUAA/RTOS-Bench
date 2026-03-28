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

1. 从中移对接群中下载 IDE 和模板工程
2. 安装到本地 (建议路径不含中文和空格)
3. 启动 IDE，创建或打开一个工作空间 (Workspace)

---

## 2. 测试套件移植

### 方法 A：使用QEMU工程

#### A.1: 导入工程

下载链接中提供了两种模板工程，分别是qemu工程和飞腾派工程，选择**qemu工程**

1. 将模板工程复制到工作区下
2. 在 IDE 中点击左上角“电视”图标，导入工程
3. 选择相应的工程导入

#### A.2: 目录结构说明

```
你的工作空间/
├── application/                    # 用户程序目录
│   ├── les/
│   ├── multicore/
│   ├── realtime/
│   ├── verify/                     # les、multicore、realtime、verify都是实时性能测试组件，请直接删除
│   ├── CMakeLists.txt              # 系统采用cmake进行构建
│   └── main.c/
└── ...
```

#### A.3: 添加 RTOS-Bench 源码

1. 删除实时性能组件（可能没有verify，如果有，也删除掉）
2. 克隆 git 仓库，切换到 OneOS 分支。

**方式 1：Git Submodule（推荐）**
```bash
# 切换到application目录下
git init  # 如果还不是 git 仓库
git submodule add https://github.com/hztBUAA/RTOS-Bench.git
git checkout feat/oneos
```

**方式 2：直接克隆**
```bash
# 切换到application目录下
git clone https://github.com/hztBUAA/RTOS-Bench.git
git checkout feat/oneos
```

#### A.4: 替换文件

1. 打开**application路径下的CMakeLists.txt**（注：不是工作区根目录下，也不是RTOS-Bench下）
2. 替换为platforms/oneos/CMakeLists(application).txt中的内容：
```
# 添加应用程序组的源文件
file(GLOB APP_SOURCES "./*.c")

# 创建应用程序静态库
add_library(application STATIC 
    ${APP_SOURCES}
    )

# 设置包含目录
target_include_directories(application
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

remove_c_compile_target(application "-mgeneral-regs-only")

oneos_add_subdirectories()
```

#### A.5: 进行编译设置

1. 进行特殊设置原因：QEMU 似乎没有提供 socket 接口的实现，所以暂时关闭了典型负载中的 modbus
2. 打开**RTOS-Bench路径下的CMakeLists.txt**（注：不是工作区根目录下，也不是RTOS-Bench下）
3. 替换为platforms/oneos/CMakeLists(application).txt中的内容：
```
# QEMU 运行
option(QEMU_ENABLE "Run on QEMU" OFF)         # 改为 ON
```

### 方法 B：使用飞腾派工程

> 注：需要开发板已接入网络，或就在手边。

#### B.1: 导入工程

下载链接中提供了两种模板工程，分别是qemu工程和飞腾派工程，选择**飞腾派工程**

#### B.2-B.4 

与方法A操作类似。

#### B.5 进行编译设置

可正常进行modbus测试，不用进行额外的编译设置。

---

## 3. 编译、部署和运行

可参照中移提供的 OneOS使用说明-v1.3 进行编译、部署和运行。

---

## 4. 贡献指南

**我的改动**

> 1. CMakeLists.txt
**参考翼辉的编译逻辑，编写了CMake编译文件**
> 2. generator/oneos_entry.c
在oneos程序入口处加入了实时性能入口（与翼辉逻辑相同）
> 3. generator/platform_abstraction.h 注释掉了oneos条件编译下clock_t、clockid_t、timer_t等已存在的类型，使其能够通过编译，**但有待完善**
> 4. generator/periodic_benchmark.c
进行了适配系统的条件编译设置
> 5. generator/platform/oneos/timestamp.c
参考翼辉进行了框架层部分函数的设置
> 6. platforms/oneos/CMakeLists(application).txt
编写了覆盖到application文件夹下的CMakeLists.txt的内容
> 7. platforms/oneos/modbus_stub.c、platforms/oneos/mqtt_stub.c
编写了桩函数
> 8. platforms/oneos/README.md
编写了说明文档 

**待完成的任务**

- 框架层代码完善
> 目前框架层的部分内容尚未进行编译，需要完善其实现，包括：
> posix_sched_adapter.c（不知道这个文件是否要编译进来）
> scheduler.c
> signal.c
> sync.c
> timer.c
> timestamp.c（已进行编译，但需要确认正确性）
> platform_abstraction.h（已包含头文件，但部分平台宏需要进一步确认）
- 压力测试的系统入口补充
- 对压力测试、可调度性测试及负载测试的命令的支持（在oneos_entry.c中）
> 目前负载测试基础命令已支持，但尚未实现"rtbench -b pid -p 0.1 -t 10"中的 -p/-t 等附加选项
- mqtt模块的调试
- 文件系统和网络的调试（可参照 OneOS使用说明-v1.3 章节6.3进行）

---

## 5. 常见问题

+ **修改代码后编译未更新**：请尝试清理编译，再重新编译。

---

## 联系与贡献

- GitHub: https://github.com/hztBUAA/RTOS-Bench
- Issues: 欢迎提交问题和建议
- PR: 欢迎贡献代码
