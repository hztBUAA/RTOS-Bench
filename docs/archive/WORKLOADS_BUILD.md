# Workloads 构建系统说明

本文档说明 RTOS-Bench 中 workloads 的构建方式，帮助新平台移植者理解如何集成负载。

## 概述

RTOS-Bench 的 9 个典型负载位于 `workloads/` 目录：

| 负载 | 类别 | 说明 | 依赖 |
|------|------|------|------|
| MQTT | 网络通信 | 发布/订阅消息协议测试 | mongoose |
| MODBUS | 工业通信 | TCP 请求/响应往返测试 | nanomodbus |
| PID | 控制算法 | 闭环控制计算性能 | C++ |
| EKF | 状态估计 | 扩展卡尔曼滤波（飞控数据） | C++, Eigen-like |
| FAST | 机器视觉 | 角点检测算法 | stb_image |
| EPNP | 机器视觉 | PnP 位姿求解 | C++, OpenGV |
| ICP | 机器视觉 | 点云配准算法 | C++ |
| CUSUM | 异常检测 | 均值漂移检测 | libm |
| EWMA | 异常检测 | 指数加权移动平均 | libm |

## 构建系统架构

RTOS-Bench 支持多种构建系统，**不同平台使用不同的构建方式**：

```
workloads/
├── CUSUM/
│   ├── cusum_bench.c       # 源码
│   ├── SConscript          # RT-Thread 构建（SCons）
│   └── CMakeLists.txt      # 独立构建（CMake）
├── EKF/
│   ├── ekf_bench.cpp
│   ├── SConscript
│   └── (子目录: EKF/, geo/, include/ 等)
├── ...
├── SConscript              # RT-Thread 顶层构建
└── rtbench_workloads.cpp   # Workload 注册表
```

## 各平台构建方式

### 1. RT-Thread（SCons）

RT-Thread 使用 SCons 构建系统，通过 `SConscript` 自动发现和编译 workloads。

**顶层 SConscript (`workloads/SConscript`)**：
```python
# 自动发现所有子目录中的 SConscript
for item in os.listdir(cwd):
    path = os.path.join(cwd, item)
    sconscript = os.path.join(path, 'SConscript')
    if os.path.isdir(path) and os.path.isfile(sconscript):
        objs = objs + SConscript(os.path.join(item, 'SConscript'))
```

**特点**：
- 自动扫描子目录，新增 workload 无需修改顶层配置
- 每个 workload 目录需要一个 `SConscript` 文件
- 依赖通过 `depend` 参数声明（如 `RT_USING_CPLUSPLUS`）

### 2. Linux/SylixOS/其他 POSIX（Makefile）

`generator/Makefile` 使用 `find` 命令自动扫描所有源文件：

```makefile
WORKLOADS_DIR := $(abspath $(GENERATOR_DIR)/../workloads)
ifeq ($(RTOS_WORKLOADS),1)
HSW_WORKLOADS_SRC := $(shell find $(WORKLOADS_DIR) -name '*.c' -o -name '*.cpp')
override CFLAGS += -I$(WORKLOADS_DIR)
endif
```

**特点**：
- 使用 `find` 自动扫描 `workloads/` 下所有 `.c` 和 `.cpp` 文件
- 新增 workload 只需放入目录，无需修改任何配置
- 通过 `RTOS_WORKLOADS=0` 可禁用打包负载

### 3. 独立编译（CMake）

部分 workload 提供 `CMakeLists.txt`，用于：
- 第三方平台（如 VxWorks）独立移植验证
- 单独构建某个 workload 进行调试

**示例 (`workloads/CUSUM/CMakeLists.txt`)**：
```cmake
cmake_minimum_required(VERSION 3.10)
project(CUSUM C)

add_library(cusum STATIC cusum_bench.c)
target_link_libraries(cusum PRIVATE m)
target_include_directories(cusum PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
```

**注意**：CMakeLists.txt 不是主构建方式，仅作为独立编译参考。

## Workload 注册机制

所有 workload 通过 `rtbench_workloads.cpp` 统一注册：

```cpp
// 声明外部函数
extern "C" {
int cusum_bench_run(void);
int ewma_bench_run(void);
// ...
}

// 定义 workload 结构
const struct rtosbench_workload rtosbench_cusum_workload = {
    .name = "cusum",
    .description = "CUSUM mean-shift detector (step/drift)",
    .category = "detection",
    .init = cusum_init,
    .exec = cusum_exec,
    .teardown = cusum_teardown,
};

// 注册所有 workload
static void register_all_workloads(void)
{
    rtosbench_register_workload(&rtosbench_cusum_workload);
    rtosbench_register_workload(&rtosbench_ewma_workload);
    // ...
}
```

## 新增 Workload 指南

### 步骤 1：创建目录和源码

```bash
mkdir workloads/NEW_WORKLOAD
# 添加源文件 new_workload.c 或 new_workload.cpp
```

### 步骤 2：添加 SConscript（RT-Thread）

```python
from building import *

cwd = GetCurrentDir()
src = Glob('*.c')  # 或 '*.cpp'
path = [cwd]

group = DefineGroup('NEW_WORKLOAD',
                    src,
                    depend = [''],  # 或 ['RT_USING_CPLUSPLUS']
                    CPPPATH = path,
                    LIBS = ['m'])   # 如需数学库

Return('group')
```

### 步骤 3：（可选）添加 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(NEW_WORKLOAD C)

add_library(new_workload STATIC new_workload.c)
target_link_libraries(new_workload PRIVATE m)
target_include_directories(new_workload PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
```

### 步骤 4：在 rtbench_workloads.cpp 中注册

```cpp
// 添加外部声明
extern "C" {
int new_workload_bench_run(void);
}

// 添加 workload 定义和注册
```

## 常见问题

### Q: 为什么有的 workload 没有 CMakeLists.txt？

A: CMakeLists.txt 是可选的，主构建系统（Makefile/SCons）会自动扫描源文件。CMakeLists.txt 仅用于独立编译场景。

### Q: 新增 workload 后需要修改哪些文件？

A:
1. 必须：创建 `workloads/NAME/` 目录和源码
2. 必须：添加 `SConscript`（RT-Thread 支持）
3. 必须：在 `rtbench_workloads.cpp` 中注册
4. 可选：添加 `CMakeLists.txt`（独立编译）

### Q: Makefile 如何知道要编译哪些文件？

A: `generator/Makefile` 使用 `find` 命令自动扫描 `workloads/` 目录下所有 `.c` 和 `.cpp` 文件，无需手动维护文件列表。

---

## 附录：构建系统对比

| 平台 | 构建系统 | 配置文件 | 自动发现 |
|------|---------|---------|---------|
| RT-Thread | SCons | SConscript | 扫描子目录 |
| Linux | Make | generator/Makefile | `find *.c *.cpp` |
| SylixOS | Make | generator/Makefile | `find *.c *.cpp` |
| VxWorks | CMake | CMakeLists.txt | 需手动配置 |
| 其他 | 自定义 | - | 参考 CMake |
