# CLAUDE.md

本文件为 AI 代码助手提供项目上下文。详细开发资产请参阅 [AGENTS.md](AGENTS.md)。

## 项目概览

RTOS-Bench 是一套跨平台周期性实时基准测试框架，支持 Linux / RT-Thread / SylixOS / OneOS 等平台。

## 快速参考

### 常用命令
```bash
# RT-Thread (QEMU aarch64)
./run-rtthread.sh          # 编译并运行
./run-rtthread.sh -b       # 仅编译
./run-rtthread.sh -r       # 仅运行

# SylixOS (QEMU x86_64)
./run-sylixos.sh           # 启动
./run-sylixos.sh -n        # 无图形模式

# msh 内运行测试
rtbench -b busywait -p 0.5 -t 1 -q
rtbench -L                 # 列出所有 workload
```

### 目录结构
- `generator/` - 核心框架与平台抽象层
- `workloads/` - 所有打包的工作负载
- `docs/` - 构建指南与文档

### 关键文档
- [AGENTS.md](AGENTS.md) - 完整开发资产（新增平台/workload 指南）
- [docs/BUILD_GUIDE.md](docs/BUILD_GUIDE.md) - 构建与部署详细指南
- [docs/COMPILE_QUICK_START.md](docs/COMPILE_QUICK_START.md) - 快速编译验证
- [docs/RESULTS_SCHEMA.md](docs/RESULTS_SCHEMA.md) - 结果入库参考

## 代码修改注意事项

1. **新增 workload**：在 `workloads/<NAME>/` 添加源码，在 `rtbench_workloads.cpp` 注册
2. **新增平台**：在 `generator/platform/<new>/` 实现 timer/sync/scheduler/timestamp/signal
3. **POSIX 契约**：尽量使用标准 POSIX API，RTOS 若不完备则在平台目录做最小垫片
4. **禁用旧接口**：不要定义 `benchmark_init/benchmark_execution` 等旧接口，统一通过 workload registry
