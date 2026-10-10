# 东土龙芯（LoongArch64 / Linux musl）命令行交叉构建

本目录用于**在命令行用交叉工具链**把 RTOS-Bench 编成东土龙芯板可运行的产物。

## 与 IDE 路线的区别

| | 本目录（Linux/musl 用户态） | `platforms/dongtu/intewell.mk`（Intewell IDE） |
|---|---|---|
| 产物 | 普通 LoongArch64 Linux ELF | Intewell 分区镜像 `*.bin` |
| 工具链 | `loongarch64-unknown-linux-musl-*` | 厂商 IDE 自带工具链 |
| 构建 | 命令行 `make` | IDE 构建 `Debug/make` |
| 运行 | SCP 上传后**直接执行** | 网页/`rt start` 装载进分区 |

前提：**板子运行 Linux（musl 用户态）**。若板子是 Intewell 分区镜像，请走 `intewell.mk` 那条路。

## 前置：交叉工具链

- 目标三元组：`loongarch64-unknown-linux-musl`
- 宿主：**x86_64-Linux**。它不能在 macOS/arm64 上运行（会 `exec format error`），必须在 x86_64 Linux 上用。
- 版本参考：GCC 14.2.0。
- 来源：东土评测方提供的 `loongarch64-unknown-linux-musl-cross.tgz`（或 musl.cc 同名包）。
- **工具链不进仓库**（体积大）。解压后把它的 `bin/` 加进 `PATH`，或用 `LOONGARCH64_TOOLCHAIN` 指过去。

## 构建

```bash
# 方式一：设环境变量后一键构建
LOONGARCH64_TOOLCHAIN=$HOME/toolchains/loongarch64-unknown-linux-musl-cross \
    ./build-loongarch64.sh

# 方式二：直接 make
CROSS_COMPILE=/path/to/loongarch64-unknown-linux-musl- make
```

产物：`build/rtbench-dongtu-loongarch64`（静态 LoongArch64 ELF）。

## 校验

```bash
CROSS_COMPILE=/path/to/loongarch64-unknown-linux-musl- readelf -h build/rtbench-dongtu-loongarch64
#   期望：Class: ELF64 / Machine: LoongArch / Type: EXEC
CROSS_COMPILE=/path/to/loongarch64-unknown-linux-musl- readelf -d build/rtbench-dongtu-loongarch64 | grep NEEDED
#   期望：无输出（完全静态，不依赖板端 libc/loader）
```

## 板端运行

```bash
# 主机
scp build/rtbench-dongtu-loongarch64 root@<板卡IP>:/root/rtbench
# 板子
chmod +x /root/rtbench
/root/rtbench -L
/root/rtbench -s                                   # 全部典型负载各一次
/root/rtbench test-schedule --cycles 3
/root/rtbench test-stress --job cpu
/root/rtbench test-all --no-realtime -o /root/rtbench_result.json
```

## 构建要点（为什么这样编）

- **源集**：沿用 `platforms/dongtu/intewell.mk` 的清单，但去掉 Intewell SDK 专有的 `shell.c`；入口用 `generator/dongtu_entry.c`（自带 `main()`）。
- **`-DRTBENCH_DONGTU_LINUX`**：在平台头文件里分流「Linux/musl 东土」与「Intewell 东土」的差异（POSIX 网络头 vs lwip、`sched_getcpu()` vs `cpuIDGet()`）。
- **EKF/EPNP**：`main`（≥ PR #46）的 workload registry 无条件引用 EKF/EPNP；本 Makefile 检测到 `workloads/EKF` 就自动纳入（含 Eigen），旧分支没有则跳过。
- **`-static -no-pie`**：自包含，避免依赖板端 rootfs 的 loader/库。
- **`-fpermissive` + `-Wl,--allow-multiple-definition`**：`workloads/EPNP/fix_opengv.h` 的 long-double 桩在 musl 下会与 libc 声明冲突并重复定义，这两个开关放行（桩是等价的）。
- **`-DTEST_SCHEDULE_MAX_PERIOD_NS=10s`**：给可调度性测试的周期封顶，避免低利用率档把墙钟时间拖爆。

## 验收结果

板端各模块的实测结果见 [`VALIDATION.md`](VALIDATION.md)。
