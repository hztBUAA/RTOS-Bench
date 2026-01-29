# 编译与快速验证指南（SylixOS / POSIX-lite RTOS）

面向刚克隆仓库、已安装对应交叉工具链的同事，说明如何在命令行编译并跑通 rt-bench。

## 前置
- 已克隆：`git clone ... && cd rt-bench`
- 已能调用交叉工具链（例如 `sylixos-gcc`、`arm-none-eabi-gcc` 等）。
- 如有 SDK/sysroot，请记录其 include/lib 路径。

常用环境变量（可一次性导出）：
```
export CC=arm-none-eabi-gcc CXX=arm-none-eabi-g++ AR=arm-none-eabi-ar STRIP=arm-none-eabi-strip
# 若有 sysroot：
export CFLAGS="--sysroot=/path/to/sysroot -I/path/to/sdk/include"
export LDFLAGS="--sysroot=/path/to/sysroot -L/path/to/sdk/lib"
```

## 目标一：SylixOS 版本
1) 最小框架（更快验证）：  
   `make PLATFORM=sylixos RTOS_WORKLOADS=0 CC=... CXX=...`
2) 全部内置 workload：  
   `make PLATFORM=sylixos CC=... CXX=...`
3) 产物：`generator/rtbench`。拷到 SylixOS 板端/镜像运行：  
   `./rtbench -b busywait -p 1 -t 1 -q`
4) 如需本机 QEMU 运行，可参考仓库脚本 `run-sylixos.sh`（需准备 `sylixos_boot.qcow2` / `sylixos_main.qcow2`）。

## 目标二：POSIX-lite RTOS（OneOS / Dongtu / Ruihua）
1) 先用最小集验证（推荐）：  
   - OneOS:  `make PLATFORM=oneos RTOS_WORKLOADS=0 CC=... CXX=...`  
   - Dongtu: `make PLATFORM=dongtu RTOS_WORKLOADS=0 CC=... CXX=...`  
   - Ruihua: `make PLATFORM=ruihua RTOS_WORKLOADS=0 CC=... CXX=...`
   产物同样是 `generator/rtbench`，入口为 `posixlite_entry.c`，不依赖 argp/perf。
2) 若确认 libc/pthread/timer/sem/libstdc++ 可用，再去掉 `RTOS_WORKLOADS=0` 打包全部 workload：  
   `make PLATFORM=oneos CC=... CXX=...`
3) 运行示例（板端）：  
   `./rtbench -b busywait -p 1 -t 1 -q`  
   列出工作负载：`./rtbench -L`

## 常用开关与运行参数
- `PLATFORM=`：`sylixos` / `oneos` / `dongtu` / `ruihua`
- `RTOS_WORKLOADS=0`：只编译框架 + stub/busywait，便于快速连通或资源紧张时使用
- 运行参数：`-b <workload> -p <period_sec> -t <tasks> [-f <prio>] [-c <cpu>] [-A|-G <cat>] -q`

## 建议的最小验证流程
1) `make PLATFORM=<目标> RTOS_WORKLOADS=0 ...`
2) 目标板/镜像运行：`./rtbench -b busywait -p 1 -t 3 -q`
3) 如成功，再尝试 `RTOS_WORKLOADS=1`，运行 `fast` 或 `epnp` 等。

## 常见适配点（若报错）
- 定时器：若不支持 `timer_create/SIGEV_THREAD`，在 `generator/platform/posix-lite/timer.c` 改为 RTOS 软定时器。
- 信号量/优先级：`sem_*` 或 `pthread_setschedparam` 缺失时，用 RTOS 原生 API 替换 `sync.c` / `scheduler.c`。
- C++/网络 workload 依赖缺库：先退回 `RTOS_WORKLOADS=0` 或只启用简单 workload，再逐步补齐依赖。

## 若只能用厂商 IDE（无命令行工具链暴露）
思路：在 IDE 里手工创建工程，照 Makefile 选源文件、宏和头文件，输出名建议仍为 `rtbench`。

### SylixOS（RealEvo IDE 等）
1) 新建 C/C++ 可执行工程，CPU/ABI 选 SylixOS 对应模板。  
2) 添加编译宏：`PLATFORM=sylixos`（会自动等价于 `-DSYLIXOS_PLATFORM`），可选 `RTOS_WORKLOADS=0` 若只要框架。  
3) 加入源码（与 Makefile 对齐）：  
   - 核心：`generator/periodic_benchmark.c`, `generator/logging.c`, `generator/memory_watcher.c`  
   - Workload registry：`generator/workload_registry.c`, `generator/workload_stub.c`, `generator/workload_busywait.c`  
   - 入口/辅助：`generator/main.c`, `generator/get_cpu_timestamp.c`, `generator/performance_counters.c`, `generator/performance_sampler.c`  
   - 平台层（SylixOS）：`generator/platform/sylixos/{timer.c,sync.c,scheduler.c,timestamp.c,signal.c}`  
   - 如需全部内置 workload，再加入 `workloads/` 目录下所有 `.c/.cpp`。  
4) 头文件搜索路径：`generator/`，`workloads/`（若启用），必要时 SDK include 路径。  
5) 链接选项：`-lrt -lm -pthread`，若 IDE 需要，关闭 `--wrap`（SylixOS 已在平台层处理）。  
6) 构建后将可执行拷入 SylixOS 设备/镜像运行：`./rtbench -b busywait -p 1 -t 1 -q`。

### POSIX-lite（OneOS / Dongtu / Ruihua，厂商自带 IDE）
1) 新建 C/C++ 可执行工程，目标 ABI 选择对应板卡。  
2) 编译宏：`PLATFORM=oneos` 或 `PLATFORM=dongtu` 或 `PLATFORM=ruihua`；若只要框架，可再加 `RTOS_WORKLOADS=0`。  
3) 源码列表：  
   - 核心：`generator/periodic_benchmark.c`, `generator/logging.c`, `generator/memory_watcher.c`  
   - Workload registry：`generator/workload_registry.c`, `generator/workload_stub.c`, `generator/workload_busywait.c`  
   - 入口：`generator/posixlite_entry.c`  
   - 平台层（POSIX-lite）：`generator/platform/posix-lite/{timer.c,sync.c,scheduler.c,timestamp.c,signal.c}`  
   - 如需打包全部 workload，添加 `workloads/` 下所有 `.c/.cpp`。  
4) 头文件路径：`generator/`，`workloads/`（若启用），以及厂商 SDK include。  
5) 链接选项：`-lm -pthread`，如需 C++ workload，加 `-lstdc++`（或 IDE 的等价勾选）。  
6) 运行：`./rtbench -b busywait -p 1 -t 1 -q`，或 `./rtbench -L` 查看列表。

提示：IDE 若不支持一次性添加大批源文件，可先仅添加上述“核心 + 平台 + 入口”子集（即 RTOS_WORKLOADS=0 配置），确认能运行后再批量加入 `workloads/` 目录。
