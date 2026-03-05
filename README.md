# RTOS-Bench

**跨平台工业 RTOS 基准测试框架**

基于《工业操作系统通用基准检测指标体系指导书 v1.5》，提供实时性能、可调度性、功耗三大类测试能力。

---

## 快速开始

### 1. 环境安装（一键配置）

```bash
git clone https://github.com/hztBUAA/RTOS-Bench.git
cd RTOS-Bench
./install.sh
```

脚本会自动安装：
- aarch64 交叉编译工具链
- SCons 构建系统
- RT-Thread 源码（shallow clone）
- 配置 BSP 软链接

### 2. 编译运行

```bash
# 编译并启动 QEMU
./run-rtthread.sh

# 仅编译
./run-rtthread.sh -b

# 仅运行（使用已编译的镜像）
./run-rtthread.sh -r
```

### 3. 执行测试

在 RT-Thread msh 命令行中：

```bash
# 实时性能测试 - 测量上下文切换、IPC 延迟等
msh /> rtbench test-realtime

# 可调度性测试 - UUniFast 任务集验证
msh /> rtbench test-schedule --cycles 100

# 功耗压力测试 - CPU/内存压力
msh /> rtbench test-stress -s cpu -t 10

# Shell 命令支持测试
msh /> rtbench test-cmd

# 运行所有测试
msh /> rtbench test-all

# 运行单个 workload
msh /> rtbench -b busywait -p 0.5 -t 100 -q
msh /> rtbench -L  # 列出所有 workload
```

---

## 测试能力

| 子命令 | 功能 | 测量指标 |
|--------|------|----------|
| `test-realtime` | 实时性能测试 | 上下文切换、系统调用、信号量/互斥锁/消息队列延迟 |
| `test-schedule` | 可调度性验证 | 截止时间错失率 (Miss Rate)、调度评分 |
| `test-stress` | 功耗压力测试 | CPU/内存压力，配合外部功耗仪使用 |
| `test-cmd` | Shell 命令支持测试 | Shell 命令注册与执行能力 |
| `test-all` | 综合测试 | 运行所有测试子命令并汇总结果 |
| `-b <workload>` | 周期负载执行 | 任务响应时间、吞吐量 |

---

## 项目结构

```
RTOS-Bench/
├── generator/                 # 核心框架
│   ├── test_realtime.c/.h    # 实时性测试入口
│   ├── test_schedule.c/.h    # 可调度性测试
│   ├── test_stress.c/.h      # 压力测试入口
│   ├── test_cmd.c/.h         # Shell 命令支持测试
│   ├── realtime_orig/        # 实时性测试实现
│   ├── stress_orig/          # stress-ng 移植
│   ├── workload_registry.c   # 负载注册表
│   ├── rtthread_entry.c      # RT-Thread msh 入口
│   └── platform/             # 平台抽象层
│       ├── rt-thread/
│       ├── linux/
│       ├── sylixos/
│       ├── dongtu/
│       ├── ruihua/
│       └── oneos/
│
├── workloads/                 # 典型工业负载
│   ├── FAST/                 # 角点检测
│   ├── EPNP/                 # 位姿估计
│   ├── EKF/                  # 卡尔曼滤波
│   ├── PID/                  # PID 控制
│   ├── CUSUM/, EWMA/         # 异常检测
│   └── MODBUS/, MQTT/        # 工业通信
│
├── docs/                      # 文档
│   ├── ARCHITECTURE_OVERVIEW.md  # 架构概览
│   ├── SCHEDULE.md           # 可调度性测试详解
│   ├── REALTIME.md           # 实时性测试详解
│   ├── STRESS.md             # 压力测试详解
│   ├── CMD.md                # Shell 命令支持测试
│   └── TEST_REPORT.md        # 测试报告
│
├── install.sh                 # 一键环境配置
└── run-rtthread.sh            # RT-Thread 编译运行脚本
```

---

## 文档索引

| 文档 | 说明 | 适用读者 |
|------|------|----------|
| [docs/ARCHITECTURE_OVERVIEW.md](docs/ARCHITECTURE_OVERVIEW.md) | 框架架构、跨平台策略、组件关系 | 新开发者必读 |
| [docs/SCHEDULE.md](docs/SCHEDULE.md) | 可调度性测试原理与使用 | 测试执行者 |
| [docs/REALTIME.md](docs/REALTIME.md) | 实时性能测试原理与使用 | 测试执行者 |
| [docs/STRESS.md](docs/STRESS.md) | 压力测试原理与使用 | 测试执行者 |
| [docs/CMD.md](docs/CMD.md) | Shell 命令支持测试使用指南 | 测试执行者 |
| [docs/TEST_REPORT.md](docs/TEST_REPORT.md) | 已验证的测试报告 | 参考 |
| [AGENTS.md](AGENTS.md) | AI 开发资产（详细约定） | AI 辅助开发 |

---

## 平台支持

| 平台 | 状态 | 入口文件 | 构建系统 |
|------|------|----------|----------|
| **RT-Thread** | ✅ 完整支持 | `rtthread_entry.c` | SCons |
| **Linux** | ✅ 支持 | `main.c` | Makefile |
| **SylixOS** | ⚠️ 需 Windows IDE | `sylixos_entry.c` | RealEvo |
| **OneOS** | ⚠️ 部分支持 | `oneos_entry.c` | SCons |
| **东土 (Dongtu)** | ⚠️ 待验证 | `dongtu_entry.c` | 厂商 IDE |
| **锐华 (Ruihua)** | ⚠️ 待验证 | `ruihua_entry.c` | 厂商 IDE |

详见 [docs/ARCHITECTURE_OVERVIEW.md](docs/ARCHITECTURE_OVERVIEW.md) 中的跨平台编译章节。

---

## 命令行参数

```
rtbench [子命令] [选项]

子命令:
  test-realtime [--multicore]     实时性能测试
  test-schedule [--cycles N]      可调度性测试
  test-stress -s <stressor> -t N  压力测试
  test-cmd                        Shell 命令支持测试
  test-all [-o <file>]            运行所有测试

通用选项:
  -b, --workload <name>    指定 workload
  -p, --period <sec>       周期时间（秒，支持小数）
  -t, --tasks <n>          执行周期数
  -q, --quiet              安静模式
  -L, --list               列出所有 workload
  -A, --all-workloads      运行所有 workload
```

---

## 贡献指南

### 新增 Workload

1. 在 `workloads/<NAME>/` 添加源码
2. 在 `workloads/rtbench_workloads.cpp` 注册：
   ```c
   const struct rtosbench_workload rtosbench_foo = {
       .name = "foo",
       .description = "...",
       .init = foo_init,
       .exec = foo_exec,
       .teardown = foo_teardown,
   };
   // 在 register_all_workloads() 中调用
   rtosbench_register_workload(&rtosbench_foo);
   ```

### 新增平台

1. 在 `generator/platform/<new>/` 实现平台抽象层
2. 创建 `generator/<new>_entry.c` 入口文件
3. 集成到对应构建系统

详见 [docs/ARCHITECTURE_OVERVIEW.md](docs/ARCHITECTURE_OVERVIEW.md)。

---

## 许可证

MIT License - 见 [LICENSE](LICENSE)

子基准套件遵循各自目录下的许可证。

---

## 参考

- Nicolella et al., "RT-Bench: An Extensible Benchmark Framework for the Analysis and Management of Real-Time Applications," RTNS 2022
- 工业操作系统通用基准检测指标体系指导书 v1.5
