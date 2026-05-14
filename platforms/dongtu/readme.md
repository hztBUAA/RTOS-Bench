# Dongtu/Intewell Orange Pi 编译说明

东土平台目录统一使用 `platforms/dongtu`。不要再新建 `platforms/dontu` 或其他拼写目录。

这个目录保存东土/Intewell `vm_3588` 香橙派工程接入 RTOS-Bench 所需的说明和平台文件。Intewell IDE 工程本身仍然由 IDE 创建和维护，RTOS-Bench 源码继续以仓库为唯一真相。

## 文件说明

- `.cproject`：历史 IDE 工程配置参考，用于说明 include/source/filter 的配置方式。
- `intewell_vm3588.mk`：推荐接入方式。由 `vm_3588` 工程本地 `config_os.mk` include，统一带入 RTOS-Bench 源码、include、兼容层和对象列表。
- `compat.c`：东土/Intewell 构建所需的小型 libc/POSIX 兼容补丁。
- `cxx_compat.h`：东土 C++ workload 构建所需兼容头。
- `shell.c`：东土 shell 绑定，负责把 RTOS-Bench 命令接入 Intewell shell。
- `VALIDATION_20260429.md`：香橙派端到端验证记录，包括二进制、部署路径、`test-schedule` 和 `test-all --no-stress` 结果。

## 推荐接入方式

1. 在 Intewell Developer 中创建或打开 `vm_3588` 香橙派工程。
2. 克隆最新 RTOS-Bench 仓库，例如：

   ```sh
   git clone https://github.com/hztBUAA/RTOS-Bench.git
   ```

3. 在 `vm_3588` 工程本地配置中设置 RTOS-Bench 根目录，并 include 平台 mk：

   ```makefile
   RTOS_BENCH_ROOT ?= C:/path/to/RTOS-Bench
   include $(RTOS_BENCH_ROOT)/platforms/dongtu/intewell_vm3588.mk
   ```

4. 保持 IDE 自动生成工程 Makefile 的机制，只把 RTOS-Bench 源码列表和兼容层接入逻辑放在 `intewell_vm3588.mk` 中维护。
5. 在 IDE 中执行 Build Project，或在工程生成的 make 目录中执行串行构建：

   ```sh
   make all
   ```

   不建议默认使用 `make -j4 all`。已验证 Intewell Windows 构建链路可能触发嵌套 make jobserver 的 `invalid --jobserver-auth` 问题。

## 路径变量

- `RTOS_BENCH_ROOT`：RTOS-Bench 仓库根目录，应该由每个同事按本机路径设置。
- `RTBENCH_EXT_OBJ_DIR`：RTOS-Bench 外部对象输出目录，默认 `./rtosbench_ext`。
- `ARCH`：默认 `__ARM64__`，用于 `vm_3588` 工程。

## 验证范围

当前香橙派验证记录见 `VALIDATION_20260429.md`：

- `rtbench -L` 可以列出 9 个工业 workload 和 2 个 utility workload。
- `rtbench test-schedule` 已完成，调度分数 `100.00 / 100`。
- `rtbench test-all --no-stress` 已完成，`test-cmd` 为 `5/5`，导出 JSON 可解析。
- `test-stress --job cpu` 在东土上仍会卡在 `Running Job: 0/65 -> cpu`，当前记录为待后续专项处理。
