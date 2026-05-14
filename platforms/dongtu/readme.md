# Dongtu/Intewell Orange Pi 编译说明

东土平台目录统一使用 `platforms/dongtu`。不要再新建 `platforms/dontu` 或其他拼写目录。

这个目录保存东土/Intewell `vm_3588` 香橙派工程接入 RTOS-Bench 所需的说明和平台文件。Intewell IDE 工程本身仍然由 IDE 创建和维护，RTOS-Bench 源码继续以仓库为唯一真相。

## 文件说明

- `.cproject`：历史 IDE 工程配置参考，用于说明 include/source/filter 的配置方式。
- `config_os.example.mk`：`vm_3588` 工程本地 `config_os.mk` 的示例。新同事可以复制里面的内容到自己的工程根目录。
- `intewell_vm3588.mk`：推荐接入方式。由 `vm_3588` 工程本地 `config_os.mk` include，统一带入 RTOS-Bench 源码、include、兼容层和对象列表。
- `compat.c`：东土/Intewell 构建所需的小型 libc/POSIX 兼容补丁。
- `cxx_compat.h`：东土 C++ workload 构建所需兼容头。
- `shell.c`：东土 shell 绑定，负责把 RTOS-Bench 命令接入 Intewell shell。
- `VALIDATION_20260429.md`：香橙派端到端验证记录，包括二进制、部署路径、`test-schedule` 和 `test-all --no-stress` 结果。

## 新同事接入步骤

1. 在 Intewell Developer 中创建或打开 `vm_3588` 香橙派工程。
2. 克隆最新 RTOS-Bench 仓库，后续都以这个仓库作为唯一源码真相，例如：

   ```sh
   git clone https://github.com/hztBUAA/RTOS-Bench.git
   ```

3. 在 `vm_3588` 工程根目录创建或更新 `config_os.mk`。

   历史实践工程路径示例：

   `D:/build/workspace/Developer_231Gizwits/IDE/BIN/Intewell_Developer/eclipse/workspace/vm_3588/config_os.mk`

   这个文件不是 RTOS-Bench 仓库文件，也不是 `platforms/dongtu` 下的公共源码。它是每个 Intewell 工程本地的构建 hook。

4. 在 `config_os.mk` 中写入以下内容，或者复制 `config_os.example.mk` 后按本机路径修改：

   ```makefile
   RTOS_BENCH_ROOT ?= C:/Users/hzt/yihui-workspace/rtos-bench/RTOS-Bench
   include $(RTOS_BENCH_ROOT)/platforms/dongtu/intewell_vm3588.mk
   ```

   `RTOS_BENCH_ROOT` 必须指向自己机器上的 RTOS-Bench 仓库根目录。如果仓库克隆在其他盘或其他目录，需要改成自己的真实路径。

5. 正常让 Intewell IDE 生成并维护工程 Makefile，不要手改 `Debug/make/makefile`。

   已验证历史工程生成的主 Makefile 中有这个扩展点：

   ```makefile
   -include $(PROJECT_PATH)/config_os.mk
   ```

   因此 IDE Build Project 时，会自动读取工程根目录下的 `config_os.mk`，再通过它 include `platforms/dongtu/intewell_vm3588.mk`。

6. 在 IDE 中执行 Build Project，或在工程生成的 make 目录中执行串行构建：

   ```sh
   make all
   ```

   不建议默认使用 `make -j4 all`。已验证 Intewell Windows 构建链路可能触发嵌套 make jobserver 的 `invalid --jobserver-auth` 问题。

## 路径变量

- `RTOS_BENCH_ROOT`：RTOS-Bench 仓库根目录，应该由每个同事按本机路径设置。
- `RTBENCH_EXT_OBJ_DIR`：RTOS-Bench 外部对象输出目录，默认 `./rtosbench_ext`。
- `ARCH`：默认 `__ARM64__`，用于 `vm_3588` 工程。

`RTOS_BENCH_ROOT` 和 `RTBENCH_EXT_OBJ_DIR` 使用 `?=` 默认赋值，所以可以在 `config_os.mk`、命令行或上层 Makefile 中覆盖。通常新同事只需要改 `RTOS_BENCH_ROOT`。

## 构建链路

东土香橙派接入后的构建链路是：

1. Intewell IDE 生成 `Debug/make/makefile`，并定义 `PROJECT_PATH`、`CC`、`COMPILE_SYMBOL`、`COMPILE_INCLUDE` 等工程变量。
2. 生成的 `Debug/make/makefile` 通过 `-include $(PROJECT_PATH)/config_os.mk` 读取工程本地 hook。
3. `config_os.mk` 设置 `RTOS_BENCH_ROOT`，再 include `$(RTOS_BENCH_ROOT)/platforms/dongtu/intewell_vm3588.mk`。
4. `intewell_vm3588.mk` 把 RTOS-Bench 的 `generator`、`workloads`、`generator/platform/dongtu`、`platforms/dongtu` 兼容文件加入 `C_SRCS`、`CXX_SRCS`、`OBJS` 和 `DEPS`。
5. RTOS-Bench 对象默认输出到 `./rtosbench_ext`，最后和 `vm_3588` 工程自身对象一起参与归档/链接。

## 验证范围

当前香橙派验证记录见 `VALIDATION_20260429.md`：

- `rtbench -L` 可以列出 9 个工业 workload 和 2 个 utility workload。
- `rtbench test-schedule` 已完成，调度分数 `100.00 / 100`。
- `rtbench test-all --no-stress` 已完成，`test-cmd` 为 `5/5`，导出 JSON 可解析。
- `test-stress --job cpu` 在东土上仍会卡在 `Running Job: 0/65 -> cpu`，当前记录为待后续专项处理。
