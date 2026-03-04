# CLAUDE.md

本文件为 AI 代码助手提供项目上下文。详细开发资产请参阅 [AGENTS.md](AGENTS.md)。

## 项目概览

RTOS-Bench 是一套跨平台工业 RTOS 基准测试框架，基于《工业操作系统通用基准检测指标体系指导书 v1.5》。

## 快速参考

### 常用命令
```bash
# RT-Thread (QEMU aarch64)
./run-rtthread.sh          # 编译并运行
./run-rtthread.sh -b       # 仅编译
./run-rtthread.sh -r       # 仅运行

# msh 内运行测试
rtbench test-realtime                # 实时性能测试
rtbench test-schedule --cycles 100   # 可调度性测试
rtbench test-stress -s cpu -t 5      # 压力测试
rtbench test-cmd                     # Shell 命令支持测试
rtbench test-all                     # 运行所有测试
rtbench -b busywait -p 0.5 -t 1 -q   # 周期负载
rtbench -L                           # 列出所有 workload
```

### 目录结构
- `generator/` - 核心框架与平台抽象层
- `workloads/` - 典型工业负载
- `docs/` - 文档

### 关键文档
- [README.md](README.md) - 快速入门
- [AGENTS.md](AGENTS.md) - 开发约定与详细指南
- [docs/ARCHITECTURE_OVERVIEW.md](docs/ARCHITECTURE_OVERVIEW.md) - 架构设计
- [docs/CMD.md](docs/CMD.md) - Shell 命令支持测试 (test-cmd) 使用指南
- [docs/STRESS.md](docs/STRESS.md) - 压力测试 (test-stress) 使用指南

## 代码修改注意事项

1. **新增 workload**：在 `workloads/<NAME>/` 添加源码，在 `rtbench_workloads.cpp` 注册
2. **新增平台**：在 `generator/platform/<new>/` 实现 timer/sync/scheduler/timestamp/signal
3. **POSIX 契约**：尽量使用标准 POSIX API，RTOS 若不完备则在 `generator/platform/<platform>/` 做最小垫片
4. **禁用旧接口**：不要定义 `benchmark_init/benchmark_execution` 等旧接口，统一通过 workload registry
