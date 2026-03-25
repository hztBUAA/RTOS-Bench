# RTOS-Bench for SylixOS 完整使用指南

本文档提供从零开始在 SylixOS 上运行 RTOS-Bench 的完整步骤。

---

## 目录

1. [环境准备](#1-环境准备)
2. [创建工程](#2-创建工程)
3. [编译](#3-编译)
4. [部署运行](#4-部署运行)
5. [使用方法](#5-使用方法)
6. [常见问题](#6-常见问题)

---

## 1. 环境准备

### 1.1 安装 RealEvo-IDE

1. 从翼辉官网下载 RealEvo-IDE
2. 安装到本地 (建议路径不含中文和空格)
3. 启动 IDE，创建或打开一个工作空间 (Workspace)

### 1.2 准备 SylixOS Base 工程

Base 工程是 SylixOS 应用开发的基础，包含内核和运行时库。

1. 在 IDE 中：File → Import → SylixOS Base Project
2. 选择对应你目标平台的 Base 工程
3. 右键 Base 工程 → Build Project
4. 等待编译完成 (首次约 5-10 分钟)

### 1.3 目录结构说明

```
你的工作空间/
├── base/                    # SylixOS Base 工程 (必须)
│   ├── libsylixos/
│   └── libcextern/
└── rtos-bench/              # 本项目 (接下来创建)
```

---

## 2. 创建工程

### 方法 A：使用模板文件（推荐）

#### 步骤 1：创建空的 SylixOS Application 工程

1. File → New → SylixOS Application Project
2. Project name: `rtos-bench`
3. 选择目标平台 (如 loongarch64)
4. Finish

#### 步骤 2：复制模板文件

将 `platforms/sylixos/` 下的文件复制到工程目录：

```bash
# 假设你的工作空间在 ~/workspace
cd ~/workspace/rtos-bench

# 复制 Makefile 和配置文件
cp /path/to/RTOS-Bench/platforms/sylixos/Makefile .
cp /path/to/RTOS-Bench/platforms/sylixos/config.mk .
cp /path/to/RTOS-Bench/platforms/sylixos/rtos-bench.mk .

# 复制源文件
mkdir -p src
cp /path/to/RTOS-Bench/platforms/sylixos/src/* src/
```

#### 步骤 3：添加 RTOS-Bench 源码

**方式 1：Git Submodule（推荐）**
```bash
cd ~/workspace/rtos-bench
git init  # 如果还不是 git 仓库
git submodule add https://github.com/hztBUAA/RTOS-Bench.git
```

**方式 2：直接克隆**
```bash
cd ~/workspace/rtos-bench
git clone https://github.com/hztBUAA/RTOS-Bench.git
```

#### 步骤 4：刷新工程

在 IDE 中：右键工程 → Refresh

### 最终目录结构

```
rtos-bench/
├── RTOS-Bench/              # RTOS-Bench 源码 (git submodule)
│   ├── generator/
│   │   └── stress_orig/osal/os_sylixos.c  # SylixOS OSAL 实现
│   ├── workloads/
│   └── platforms/
├── src/
│   └── cusum_workload.c     # 示例自定义工作负载
├── Makefile
├── config.mk
└── rtos-bench.mk
```

---

## 3. 编译

### 3.1 在 RealEvo-IDE 中编译

1. 右键 `rtos-bench` 工程
2. 选择 **Clean Project** (首次编译或修改 Makefile 后必须)
3. 右键工程 → **Build Project**
4. 查看 Console 窗口，确认 `Build Finished. 0 errors`

### 3.2 编译输出

成功后生成：
- `Debug/rtos-bench` - 可执行文件
- `Debug/strip/rtos-bench` - 裁剪后的可执行文件（更小）

### 3.3 常见编译错误

| 错误 | 原因 | 解决方法 |
|------|------|----------|
| `WORKSPACE_base not found` | Base 工程未正确配置 | 确保 Base 工程已导入并编译 |
| `cannot find -lxxx` | 缺少依赖库 | 确保 Base 工程编译成功 |
| `fatal error: xxx.h` | 头文件路径错误 | 检查 rtos-bench.mk 中的 LOCAL_INC_PATH |

---

## 4. 部署运行

### 4.1 上传到目标板

**方式 1：FTP**
```bash
ftp <目标板IP>
# 登录后
cd /apps
put Debug/rtos-bench
bye
```

**方式 2：SSH/SCP**
```bash
scp Debug/rtos-bench root@<目标板IP>:/apps/
```

**方式 3：SD卡/U盘**
- 将 `Debug/rtos-bench` 复制到存储介质
- 在 SylixOS 中挂载并复制到 `/apps/`

### 4.2 设置执行权限

在 SylixOS shell 中：
```bash
chmod +x /apps/rtos-bench
```

---

## 5. 使用方法

### 5.1 基本命令

```bash
cd /apps

# 显示帮助
./rtos-bench -h

# 列出所有工作负载
./rtos-bench -L

# 运行特定工作负载
./rtos-bench -b <workload> -p <period_sec> -t <duration_sec>
```

### 5.2 工作负载测试

```bash
# PID 控制器测试 - 周期 100ms, 运行 10 秒
./rtos-bench -b pid -p 0.1 -t 10

# EKF 滤波器测试 - 周期 50ms, 运行 5 秒
./rtos-bench -b ekf -p 0.05 -t 5

# CUSUM 算法测试
./rtos-bench -b cusum -p 0.2 -t 5

# 忙等待测试 (用于基准)
./rtos-bench -b busywait -p 0.5 -t 3
```

### 5.3 系统测试

```bash
# 实时性能测试（-v: 前置条件的验证, -m: 进行含“多核并发开销”的完整测试）
./rtos-bench test-realtime -v -m

# 可调度性测试
./rtos-bench test-schedule --cycles 100

# Shell 命令支持测试
./rtos-bench test-cmd

# 压力测试 (CPU)
./rtos-bench test-stress -s cpu -t 10

# 压力测试 (内存)
./rtos-bench test-stress -s vm -t 10

# 压力测试 (矩阵运算)
./rtos-bench test-stress -s matrix -t 10
```

### 5.4 可用工作负载列表

| 名称 | 说明 | 典型周期 |
|------|------|----------|
| pid | PID 控制器 | 10-100ms |
| ekf | 扩展卡尔曼滤波 | 20-100ms |
| epnp | 相机位姿估计 | 50-200ms |
| icp | 迭代最近点 | 100-500ms |
| cusum | 累积和检测 | 50-200ms |
| ewma | 指数移动平均 | 10-50ms |
| fast | 特征点检测 | 20-100ms |
| modbus | MODBUS 协议 | 10-100ms |
| mqtt | MQTT 协议 | 100-1000ms |
| stub | 空操作 (基准) | any |
| busywait | 忙等待 (基准) | any |

---

## 6. 常见问题

### Q: 可以直接命令行 make 吗？

**可以！** 只需确保：
1. SylixOS 工具链在 PATH 中（如 `loongarch64-sylixos-elf-gcc`）
2. `base` 工程与 `rtos-bench` 在同一目录下

```bash
cd rtos-bench
make clean && make all
```

如果 `base` 工程在其他位置，可以指定：
```bash
make WORKSPACE_base=/path/to/base
```

### Q: 运行时报 "can not find symbol"？

1. 确保执行了 **Clean Build**（不是增量编译）
2. 检查是否缺少源文件
3. 确认 OSAL 实现 (`os_sylixos.c`) 已包含在编译中

### Q: 如何添加自定义工作负载？

参考 `src/cusum_workload.c`，实现 `init`、`exec`、`teardown` 函数，使用 `RTOSBENCH_REGISTER_WORKLOAD` 宏注册。

---

## 文件说明

| 文件 | 用途 | 是否需要修改 |
|------|------|-------------|
| `Makefile` | 主构建文件 | 否 |
| `config.mk` | 工程配置 | 通常不需要 |
| `rtos-bench.mk` | 源文件列表 | 添加新文件时需要 |
| `src/sylixos_stubs.c` | Stub 函数 | 否 |
| `src/cusum_workload.c` | 示例工作负载 | 可选 |

---

## 联系与贡献

- GitHub: https://github.com/hztBUAA/RTOS-Bench
- Issues: 欢迎提交问题和建议
- PR: 欢迎贡献代码
