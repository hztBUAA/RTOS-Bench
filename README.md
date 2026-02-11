# RTOS-Bench

跨平台周期性实时基准测试框架，支持 Linux / RT-Thread / SylixOS / OneOS / 东土 / 锐华等平台。

## 特性

- **统一入口**：`rtbench` CLI，多平台一致的命令行接口
- **Workload 注册表**：集中管理所有基准测试负载
- **平台抽象层**：POSIX 契约，非 POSIX 平台提供最小垫片
- **开箱即用**：内置计算类（busywait/fast/epnp/ekf/icp/pid/cusum/ewma）和网络类（modbus/mqtt）负载

## 快速开始

### RT-Thread（QEMU aarch64）

```bash
# 编译并运行
./run-rtthread.sh

# 仅编译
./run-rtthread.sh -b

# 在 msh 中运行
msh /> rtbench -b busywait -p 0.5 -t 1 -q
msh /> rtbench -b epnp -p 1 -t 1 -q
msh /> rtbench -L  # 列出所有 workload
```

### SylixOS（QEMU x86_64）

```bash
./run-sylixos.sh      # 启动
./run-sylixos.sh -n   # 无图形模式
```

### OneOS / 东土 / 锐华（POSIX-lite）

```bash
make PLATFORM=oneos CC=arm-none-eabi-gcc CXX=arm-none-eabi-g++
./rtbench -b busywait -p 1 -t 1 -q
```

## 命令行参数

```
rtbench [选项]
  -b, --workload <name>    指定 workload（默认 busywait）
  -p, --period <sec>       周期时间（秒）
  -t, --tasks <n>          运行周期数
  -d, --deadline <sec>     截止时间（默认 = period）
  -f, --priority <prio>    线程优先级
  -c, --cpu <mask>         CPU 亲和性
  -q, --quiet              减少输出
  -A, --all-workloads      顺序运行所有 workload
  -G, --category <cat>     按类别过滤运行
  -L, --list               列出所有 workload
```

## 项目结构

```
rt-bench/
├── generator/              # 核心框架
│   ├── platform/          # 平台抽象层
│   │   ├── linux/
│   │   ├── rt-thread/
│   │   ├── sylixos/
│   │   ├── oneos/
│   │   └── posix-lite/
│   ├── main.c             # Linux/SylixOS 入口
│   ├── rtthread_entry.c   # RT-Thread 入口
│   └── posixlite_entry.c  # POSIX-lite 入口
├── workloads/             # 工作负载
│   ├── CUSUM/             # 变化检测
│   ├── EWMA/              # 指数加权移动平均
│   ├── FAST/              # 角点检测
│   ├── PID/               # PID 控制器
│   ├── EKF/               # 扩展卡尔曼滤波
│   ├── EPNP/              # PnP 位姿估计
│   ├── ICP/               # 点云配准
│   ├── MODBUS/            # Modbus 协议
│   └── MQTT/              # MQTT 协议
├── extern/                # 外部依赖
│   ├── rt-thread/         # RT-Thread 源码
│   ├── toolchains/        # 交叉编译工具链
│   └── yihui/             # SylixOS IDE 和镜像
├── docs/                  # 文档
│   ├── BUILD_GUIDE.md     # 构建与部署指南
│   ├── COMPILE_QUICK_START.md  # 快速编译验证
│   └── RESULTS_SCHEMA.md  # 结果入库参考
├── run-rtthread.sh        # RT-Thread 一键脚本
├── run-sylixos.sh         # SylixOS 启动脚本
├── AGENTS.md              # AI 开发资产（详细指南）
└── CLAUDE.md              # AI 助手快速参考
```

## 文档

| 文档 | 说明 |
|------|------|
| [AGENTS.md](AGENTS.md) | 完整开发资产：新增平台/workload 指南、工具链说明、已验证经验 |
| [docs/BUILD_GUIDE.md](docs/BUILD_GUIDE.md) | 详细构建与部署指南（新人必读） |
| [docs/COMPILE_QUICK_START.md](docs/COMPILE_QUICK_START.md) | 快速编译验证（SylixOS/POSIX-lite） |
| [docs/RESULTS_SCHEMA.md](docs/RESULTS_SCHEMA.md) | 基准测试结果入库参考 |

## 新增 Workload

1. 在 `workloads/<NAME>/` 添加源码，提供 `foo_bench_run()` 入口（`extern "C"`）
2. 在 `workloads/rtbench_workloads.cpp` 注册：
   ```c
   const struct rtosbench_workload rtosbench_foo = {
     .name = "foo",
     .description = "...",
     .init = foo_init,
     .exec = foo_exec,
     .teardown = foo_teardown,
   };
   // 在 register_all_workloads() 中调用 rtosbench_register_workload(&rtosbench_foo);
   ```

## 新增平台

1. 在 `generator/platform/<new>/` 实现：
   - `timer.c` - 定时器
   - `sync.c` - 信号量
   - `scheduler.c` - 优先级/亲和性
   - `timestamp.c` - 时间戳
   - `signal.c` - 信号处理
2. 在 `generator/Makefile` 添加 PLATFORM 分支

## 许可证

MIT License（见 [LICENSE](LICENSE)），子基准套件遵循各自目录下的 LICENSES。

## 参考论文

Nicolella et al., "RT-Bench: An Extensible Benchmark Framework for the Analysis and Management of Real-Time Applications," RTNS 2022. DOI: 10.1145/3534879.3534888
