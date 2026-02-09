# rt-bench 构建与部署指南

本文档面向新人，详细说明如何在 RT-Thread 和 SylixOS 平台上构建和运行 rt-bench。

## 目录
- [环境准备](#环境准备)
- [RT-Thread 平台](#rt-thread-平台)
- [SylixOS 平台](#sylixos-平台)
- [跨平台架构说明](#跨平台架构说明)
- [常见问题](#常见问题)

---

## 环境准备

### 主机环境
- macOS（已验证）或 Linux
- 需要安装的工具：
  - QEMU（用于模拟器运行）
  - Python 3.x（用于 SCons 构建系统）

### 安装 QEMU（macOS）
```bash
brew install qemu
```

验证安装：
```bash
which qemu-system-aarch64  # RT-Thread 用
which qemu-system-x86_64   # SylixOS 用
```

---

## RT-Thread 平台

### 工具链
| 项目 | 说明 |
|------|------|
| 编译器 | xpack-aarch64-none-elf-gcc-14.2.1-1.1 |
| 架构 | AArch64 (ARM64) |
| 构建系统 | SCons (Python) |
| 目标 BSP | qemu-virt64-aarch64 |

### 工具链准备

工具链已预置在项目中，首次使用需解压：

```bash
cd extern/toolchains
tar xzf xpack-aarch64-none-elf-gcc-14.2.1-1.1-darwin-x64.tar.gz
```

### SCons 准备

SCons 已安装在项目的 Python 虚拟环境中：

```bash
# 验证 scons
/Users/dp/dp/RTOS/rt-bench/extern/.venv/bin/scons --version
```

如需手动安装：
```bash
cd extern
python3 -m venv .venv
source .venv/bin/activate
pip install scons
```

### 构建流程

#### 方式一：使用脚本（推荐）

```bash
# 编译并运行
./run-rtthread.sh

# 仅编译
./run-rtthread.sh -b

# 仅运行（使用已有镜像）
./run-rtthread.sh -r

# 自动测试
./run-rtthread.sh -t
```

#### 方式二：手动构建

```bash
# 1. 设置工具链路径
export RTT_EXEC_PATH="/Users/dp/dp/RTOS/rt-bench/extern/toolchains/xpack-aarch64-none-elf-gcc-14.2.1-1.1/bin"
export PATH="$RTT_EXEC_PATH:$PATH"

# 2. 激活 Python 环境
source /Users/dp/dp/RTOS/rt-bench/extern/.venv/bin/activate

# 3. 进入 BSP 目录编译
cd extern/rt-thread/bsp/qemu-virt64-aarch64
scons -j8

# 4. 验证产物
ls -la rtthread.bin rtthread.elf
```

### 运行 RT-Thread

```bash
# 使用 QEMU 启动
qemu-system-aarch64 -M virt,gic-version=2 -cpu cortex-a53 -m 128M -smp 4 \
    -kernel rtthread.bin -nographic
```

启动后进入 msh shell，可执行 rtbench 命令：

```bash
# 在 msh 中运行 rtbench
msh /> rtbench -b busywait -p 0.5 -t 1 -q
msh /> rtbench -b epnp -p 1 -t 1 -q
msh /> rtbench -b ekf -p 2 -t 1 -q
```

按 `Ctrl+A X` 退出 QEMU。

### 构建产物说明

| 文件 | 说明 |
|------|------|
| `rtthread.bin` | 二进制镜像，QEMU 直接加载 |
| `rtthread.elf` | ELF 格式，用于调试 |
| `rtthread.map` | 链接映射表 |

### 构建时间参考

在 M1 Mac 上从零构建约需 30-60 秒。

---

## SylixOS 平台

### 当前状态

| 项目 | 状态 |
|------|------|
| 运行环境 | ✅ QEMU x86_64 |
| 系统镜像 | ✅ 已转换 (qcow2) |
| rt-bench 编译 | ⚠️ 需要手动配置 |

### 镜像准备

SylixOS 镜像已从 VMware 格式转换为 QEMU 格式：

```
extern/yihui/SylixOS IDE 6.5.0_professional/VMware/SylixOSx86/
├── sylixos_boot.qcow2  # 启动分区
├── sylixos_main.qcow2  # 主系统分区
├── x86_boot.vmdk       # 原始 VMware 镜像
└── x86_main.vmdk
```

如需重新转换：
```bash
cd "extern/yihui/SylixOS IDE 6.5.0_professional/VMware/SylixOSx86"
qemu-img convert -f vmdk -O qcow2 x86_boot.vmdk sylixos_boot.qcow2
qemu-img convert -f vmdk -O qcow2 x86_main.vmdk sylixos_main.qcow2
```

### 运行 SylixOS

```bash
# 使用脚本启动
./run-sylixos.sh

# 无图形模式
./run-sylixos.sh -n

# 启用共享目录
./run-sylixos.sh -s
```

启动后会进入 SylixOS shell：
```
[root@sylixos:/root]#
```

### rt-bench 编译方式

**重要发现（2026-01-24 完整验证）**：
- SylixOS 默认镜像**没有 GCC 编译器**
- SylixOS **不支持 9p 文件系统**（`mount -t 9p` 返回 "No driver"）
- SSH 服务默认未启动
- QEMU user mode networking 与 SylixOS 镜像不兼容（网段不匹配）
- **Linux GCC 编译的二进制文件不兼容 SylixOS**（即使静态编译也不行）
- **必须使用 SylixOS 专用工具链 (RealEvo IDE)** 编译

#### ❌ Docker + Linux 交叉编译（不可用）

> **警告**：使用 Linux GCC 编译的二进制文件在 SylixOS 上会报错 `Invalid format!`，这是因为 SylixOS 加载器期望特定的 ELF 格式，与标准 Linux ELF 不兼容。

验证结果：
```
[ld]Load file "/media/hdd1/rtbench_sylixos_static" error Invalid format!
```

#### ✅ 方式一：使用 RealEvo IDE (Windows) 编译 ⭐ 推荐

SylixOS 官方提供的开发工具：
- **RealEvo IDE**：Windows 集成开发环境，包含 x86/ARM 交叉编译器
- 位于 `extern/yihui/SylixOS IDE 6.5.0_professional/RealEvo/`

**编译步骤**：
1. 在 Windows 上安装 RealEvo IDE
2. 创建 SylixOS 应用项目
3. 导入 rt-bench 源码（`generator/` 目录）
4. 配置平台为 SylixOS x86
5. 编译生成 `.so` 或可执行文件

**文档参考**：
| 文档 | 说明 |
|------|------|
| RealEvo-IDE使用手册.pdf | IDE 完整使用说明 |
| RealEvo-IDE快速入门.pdf | 快速上手指南 |
| SylixOS应用开发手册.pdf | 应用开发 API |

#### ✅ 方式二：使用 VMware Fusion 运行和传输 ⭐ 推荐

由于 QEMU 网络与 SylixOS 不兼容，**推荐使用 VMware Fusion**：

1. **打开 VMware 镜像**
   ```
   extern/yihui/SylixOS IDE 6.5.0_professional/VMware/SylixOSx86/SylixOS x86.vmx
   ```

2. **配置共享文件夹**
   - VMware 菜单 → 虚拟机 → 设置 → 共享
   - 添加共享文件夹，选择 rt-bench 项目目录
   - 启用"启用 Guest 使用共享文件夹"

3. **在 SylixOS 中挂载共享文件夹**
   ```bash
   # 在 SylixOS shell 中
   mount -t vmhgfs .host:/rt-bench /mnt
   ls /mnt
   ```

4. **运行 rtbench**
   ```bash
   cd /mnt
   ./rtbench_sylixos -b busywait -p 0.5 -t 1
   ```

#### 方式三：使用 HTTP 传输（需要正确网络配置）

如果 VMware 网络正常工作：

**宿主机端**：
```bash
cd /path/to/rt-bench
python3 -m http.server 8000
```

**SylixOS 端**：
```bash
# 在 VMware 中，宿主机通常是网关 IP
curl http://网关IP:8000/rtbench_sylixos -o /tmp/rtbench
chmod 755 /tmp/rtbench
/tmp/rtbench -b busywait -p 0.5 -t 1
```

### SylixOS 系统信息

| 项目 | 值 |
|------|-----|
| 内核版本 | 3.2.9 |
| 架构 | x86 (32-bit) |
| 可用工具 | curl, tar, lua, sqlite3 |
| 不可用 | gcc, make, 9p, SSH |
| 推荐虚拟化 | VMware Fusion |

### SylixOS 编译工具链说明

SylixOS 官方提供的开发工具：
- **RealEvo IDE**：Windows 集成开发环境，包含 x86/ARM 交叉编译器
- **文档**：位于 `extern/yihui/SylixOS IDE 6.5.0_professional/doc/`

| 文档 | 说明 |
|------|------|
| RealEvo-IDE使用手册.pdf | IDE 完整使用说明 |
| RealEvo-IDE快速入门.pdf | 快速上手指南 |
| SylixOS应用开发手册.pdf | 应用开发 API |

---

## 跨平台架构说明

### 平台抽象层

rt-bench 通过平台抽象层实现跨平台支持：

```
generator/platform/
├── linux/          # Linux 平台实现
├── rt-thread/      # RT-Thread 平台实现
└── sylixos/        # SylixOS 平台实现
```

每个平台目录包含：
| 文件 | 功能 |
|------|------|
| timer.c | 定时器实现 |
| sync.c | 同步原语（信号量） |
| scheduler.c | 调度器（优先级/亲和性） |
| timestamp.c | 时间戳获取 |
| signal.c | 信号处理 |

### POSIX 契约

| 平台 | POSIX 支持 | 说明 |
|------|-----------|------|
| Linux | 完整 | 原生支持 |
| SylixOS | 完整 | 原生 POSIX 兼容 |
| RT-Thread | 部分 | 通过 POSIX 适配层 |

### 对开发者的意义

**作为 workload 开发者**：
- 只需关心上层业务逻辑
- 使用统一的 `rtosbench_register_workload()` 接口
- 平台差异由抽象层自动处理
- 不同架构（ARM/x86）的适配由编译工具链处理

**作为平台移植者**：
- 实现 `generator/platform/<new-platform>/` 下的 5 个文件
- 确保 POSIX API 可用（或提供垫片）
- 在 Makefile 中添加平台分支

### 构建系统对比

| 平台 | 构建系统 | 入口文件 |
|------|---------|---------|
| Linux | Make | main.c |
| SylixOS | Make | main.c (与 Linux 相同) |
| RT-Thread | SCons | rtthread_entry.c |

---

## 常见问题

### RT-Thread

**Q: 编译报错找不到工具链**
```
A: 检查 RTT_EXEC_PATH 环境变量是否正确设置
   export RTT_EXEC_PATH="/path/to/toolchains/xpack-aarch64-none-elf-gcc-14.2.1-1.1/bin"
```

**Q: scons 命令找不到**
```
A: 激活 Python 虚拟环境
   source extern/.venv/bin/activate
```

**Q: QEMU 启动后无输出**
```
A: 检查 rtthread.bin 是否存在，尝试重新编译
   ./run-rtthread.sh -b
```

### SylixOS

**Q: 镜像文件找不到**
```
A: 需要先转换 VMware 镜像为 qcow2 格式
   参见"镜像准备"章节
```

**Q: SSH 连接被拒绝**
```
A: SylixOS 可能未启动完成，等待几秒后重试
   ssh -p 2222 root@localhost
```

**Q: 如何将文件传入 SylixOS**
```bash
# 方式一：SCP
scp -P 2222 rtbench root@localhost:/root/

# 方式二：使用 9p 共享目录
./run-sylixos.sh -s
# 在 SylixOS 内：mount -t 9p -o trans=virtio rtbench /mnt
```

---

## 验证日志（2026-01-24）

### RT-Thread 构建验证

```
环境：macOS Darwin 23.6.0, M1 Mac
工具链：xpack-aarch64-none-elf-gcc-14.2.1-1.1
SCons：v4.10.1

构建命令：./run-rtthread.sh -b
结果：成功
产物：
  - rtthread.bin (3.8MB)
  - rtthread.elf (58.7MB)
```

### RT-Thread 运行验证

```
启动日志：
  [I/libcpu.aarch64.cpu] Using MPID 0x0 as cpu 0
  [I/libcpu.aarch64.cpu] Using MPID 0x1 as cpu 1
  [I/libcpu.aarch64.cpu] Using MPID 0x2 as cpu 2
  [I/libcpu.aarch64.cpu] Using MPID 0x3 as cpu 3
  RT-Thread 5.0.2
  lwIP-2.0.3 initialized!
  msh />

状态：4核心启动成功，msh shell 可用
```

### SylixOS 运行验证

```
启动日志：
  SylixOS kernel version: 3.2.9 Code name: Cen-Ci swords
  CPU: QEMU Virtual CPU version 2.5+
  RAM SIZE: 0x10000000 Bytes (256MB)
  Block device /dev/blk/hdd-0 part 0 mount to /media/hdd0 use vfat file system.
  Block device /dev/blk/hdd-1 part 0 mount to /media/hdd1 use vfat file system.
  Block device /dev/blk/hdd-1 part 1 mount to /media/hdd2 use tpsfs file system.
  Linux compatibility layer init finished.
  [root@sylixos:/root]#

状态：系统启动成功，shell 可用
```

### SylixOS 二进制兼容性验证

```
测试1：动态链接 Linux ELF
  结果：崩溃 (lib_strlen 异常)
  原因：SylixOS 加载器不兼容 Linux 动态链接器

测试2：静态链接 Linux ELF
  结果：失败 "[ld]Load file error Invalid format!"
  原因：SylixOS 期望特定 ELF 格式

结论：必须使用 SylixOS 工具链 (RealEvo IDE) 编译
```

### SylixOS RealEvo IDE 编译验证（2026-02-10）✅

**环境配置**：
```
IDE: RealEvo-IDE 6.1.0 Ultimate (Windows)
工具链: aarch64-sylixos-elf-gcc
目标架构: AArch64
Base工程: SylixOS 基础库 (libsylixos, libvpmpdm)
```

**项目结构**：
```
yihui-workspace/rtos-bench/
├── RTOS-Bench/              # Git submodule (本仓库)
├── src/
│   └── cusum_workload.c     # Workload 注册包装器
├── rtos-bench.mk            # 源文件配置
├── .reproject               # IDE 配置 (关键!)
└── Makefile                 # 顶层构建入口
```

**关键配置 - `.reproject`**：
```xml
<BuildSetting CoustomCfgMakefile="false" NotScanSourceFile="true"/>
```
> ⚠️ `NotScanSourceFile="true"` 是关键！防止 IDE 自动扫描所有源文件，只使用 rtos-bench.mk 中手动指定的文件。

**关键配置 - `rtos-bench.mk`**：
```makefile
LOCAL_SRCS := \
RTOS-Bench/generator/posixlite_entry.c \
RTOS-Bench/generator/periodic_benchmark.c \
RTOS-Bench/generator/workload_registry.c \
RTOS-Bench/generator/workload_stub.c \
RTOS-Bench/generator/workload_busywait.c \
RTOS-Bench/generator/logging.c \
RTOS-Bench/generator/memory_watcher.c \
RTOS-Bench/generator/platform/sylixos/timer.c \
RTOS-Bench/generator/platform/sylixos/sync.c \
RTOS-Bench/generator/platform/sylixos/scheduler.c \
RTOS-Bench/generator/platform/sylixos/timestamp.c \
RTOS-Bench/generator/platform/sylixos/signal.c \
RTOS-Bench/workloads/CUSUM/cusum_bench.c \
src/cusum_workload.c

LOCAL_INC_PATH := \
-I"./RTOS-Bench/generator" \
-I"./RTOS-Bench/workloads/CUSUM"

LOCAL_DSYMBOL := \
-DSYLIXOS_PLATFORM \
-D_GNU_SOURCE

LOCAL_DEPEND_LIB := -lm
```

**Workload 注册包装器 - `src/cusum_workload.c`**：
```c
#include "workload_registry.h"

extern int cusum_bench_run(void);

static int cusum_init(int parameters_num, void **parameters) {
    (void)parameters_num; (void)parameters;
    return 0;
}

static void cusum_exec(int parameters_num, void **parameters) {
    (void)parameters_num; (void)parameters;
    cusum_bench_run();
}

static void cusum_teardown(int parameters_num, void **parameters) {
    (void)parameters_num; (void)parameters;
}

static const struct rtosbench_workload cusum_workload = {
    .name = "cusum",
    .description = "CUSUM change-point detection benchmark",
    .category = "signal",
    .init = cusum_init,
    .exec = cusum_exec,
    .teardown = cusum_teardown,
};

RTOSBENCH_REGISTER_WORKLOAD(cusum_workload);
```

**编译结果**：
```
编译命令: make (通过 IDE 或命令行)
编译文件: 14 个 .c 文件 (仅 SylixOS 平台)
产物: Debug/rtos-bench (352KB)
状态: ✅ 成功 (0 errors, 5 warnings)

警告说明:
- _GNU_SOURCE 重复定义 (无影响)
- on_exit/CPU_COUNT 隐式声明 (SylixOS POSIX 扩展差异)
```

**验证要点**：
1. 必须先编译 `base` 工程生成 `libvpmpdm.a`
2. 设置 `NotScanSourceFile="true"` 避免 IDE 扫描错误平台文件
3. 只编译 `platform/sylixos/` 目录下的平台实现
4. 使用 `posixlite_entry.c` 作为程序入口（不是 main.c）

### SylixOS 磁盘扩展限制

```
测试的方式：
  - QEMU 额外 IDE 磁盘 → 不被检测（只支持 2 个 IDE 设备）
  - AHCI 控制器 → 驱动错误
  - virtio-blk → 驱动不支持
  - USB 存储 → 驱动不支持

结论：SylixOS x86 VMware 镜像只支持原有的 2 个 IDE 磁盘
解决方案：直接修改 sylixos_main.qcow2 的 FAT32 分区添加文件
```

---

## 附录：目录结构

```
rt-bench/
├── generator/              # 核心框架
│   ├── platform/          # 平台抽象层
│   │   ├── linux/
│   │   ├── rt-thread/
│   │   └── sylixos/
│   ├── main.c             # Linux/SylixOS 入口
│   ├── rtthread_entry.c   # RT-Thread 入口
│   └── Makefile           # 构建变量定义
├── workloads/             # 打包的负载
├── extern/
│   ├── rt-thread/         # RT-Thread 源码
│   │   └── bsp/qemu-virt64-aarch64/  # 目标 BSP
│   ├── toolchains/        # 交叉编译工具链
│   ├── yihui/             # SylixOS IDE 和镜像
│   └── .venv/             # Python 虚拟环境 (scons)
├── run-rtthread.sh        # RT-Thread 一键脚本
├── run-sylixos.sh         # SylixOS 启动脚本
└── docs/
    └── BUILD_GUIDE.md     # 本文档
```
