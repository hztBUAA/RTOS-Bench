# RTOS-Bench on 飞腾派 (Phytium Pi) — RT-Thread 适配与镜像构建

本平台把 RTOS-Bench 与 RT-Thread 5.x 一起编成一块飞腾派（Firefly / PE2204）的裸机镜像
`rtthread_a64.bin`，ELF 入口 `0x80080000`，由 U-Boot 加载运行。

| 项 | 值 |
| --- | --- |
| 目标板 | Phytium Pi / Firefly，`PHYTIUMPI_FIREFLY_BOARD` / `TARGET_PE2204` |
| 内核 | RT-Thread 5.3.0，commit `db6c0ddbf` |
| 平台宏 | `RT_THREAD_PLATFORM`（`generator/platform/rt_thread/`） |
| msh 命令 | `rtos_bench`（**不是** QEMU 文档里的 `rtbench`） |
| 构建产物 | `rtthread_a64.elf` / `rtthread_a64.bin` / `rtthread_a64.map` |

## 1. 构建环境（Ubuntu x86-64）

**宿主必须是 x86-64 的 Linux（推荐 Ubuntu 22.04）**：交叉工具链是 linux-x64 的原生 ELF，
无法在 Windows、macOS 上直接运行；ARM64（含 Apple Silicon 上的 ARM64 虚拟机）同样不满足宿主要求。

```bash
uname -m          # 应为 x86_64

sudo apt-get update
sudo apt-get install -y git build-essential python3 python3-venv python3-pip unzip file
```

创建独立虚拟环境（后续的 `python`/`scons` 都由它提供，避免与系统 Python 冲突）：

```bash
python3 -m venv ~/.venvs/rtthread
```

## 2. 获取源码

### 2.1 RTOS-Bench 仓库

```bash
git clone https://github.com/hztBUAA/RTOS-Bench.git RTOS-Bench
```

### 2.2 rt-thread 源码

```bash
cd RTOS-Bench
git clone https://github.com/RT-Thread/rt-thread.git extern/rt-thread
git -C extern/rt-thread checkout db6c0ddbf
```

### 2.3 飞腾派 BSP

使用团队提供的 `phytium-bsp-20261016.zip`：内含完整 BSP 与 `libraries/phytium_standalone_sdk`
（commit `8d6aefa6`），并已包含 RTOS-Bench 的改动。解压后把其中的 `phytium/` 放到
`extern/rt-thread/bsp/phytium/`，替换仓库自带的默认 BSP：

```bash
unzip phytium-bsp-20261016.zip
rm -rf RTOS-Bench/extern/rt-thread/bsp/phytium
mv phytium RTOS-Bench/extern/rt-thread/bsp/phytium
```

包内 `MANIFEST.md` 列出了该 BSP 相对同事原始版本的改动文件与改动点。**该 BSP 已配置完成**，
不需要再执行 `scons --attach` / `scons --menuconfig`，也不需要手工改 `SConstruct` 或拷贝命令文件。

> 不要使用裁剪版 `phytium.zip`：它不含 SDK、最终 `.config` 与部分驱动。

### 2.4 交叉工具链

`xpack-aarch64-none-elf-gcc`，版本 **14.2.1-1.1**，**linux-x64、relocatable（解压即用）**。
来源：xpack-dev-tools 官方发行版 —— <https://github.com/xpack-dev-tools>
（工具链项目 `aarch64-none-elf-gcc`，对应 release `v14.2.1-1.1`）。

解压到任意目录，记为 `<TOOLCHAIN>/xpack-aarch64-none-elf-gcc-14.2.1-1.1`，其 `bin/` 下为
`aarch64-none-elf-gcc` / `aarch64-none-elf-g++` 等。该包已含 GCC/G++、binutils、newlib、
libstdc++、libsupc++，C++ workload 依赖其中的 G++，无需另装。

## 3. 目录准备

### 3.1 安装 Python 构建依赖

RT-Thread 自带依赖清单（SCons、kconfiglib、PyYAML、requests、tqdm）：

```bash
cd RTOS-Bench
~/.venvs/rtthread/bin/pip install -r extern/rt-thread/tools/requirements.txt
```

### 3.2 建立 rtos-bench 链接

BSP 的 `aarch64/SConscript` 会扫描 `aarch64/` 下的每个子目录，把带 `SConscript` 的目录编入；
RTOS-Bench 正是通过 `aarch64/rtos-bench` 这个链接被接入的，需建立它（指向 RTOS-Bench 仓库根）：

```bash
cd RTOS-Bench
ln -sfn "$PWD" extern/rt-thread/bsp/phytium/aarch64/rtos-bench
readlink -f extern/rt-thread/bsp/phytium/aarch64/rtos-bench   # 应指向 RTOS-Bench 仓库根
```

> 若源码曾经过 Windows 拷贝，确认源文件为 LF 行尾（CRLF 会让 scons 解析出错）。

## 4. 编译

```bash
cd RTOS-Bench/extern/rt-thread/bsp/phytium/aarch64

export RTT_ROOT="$PWD/../../.."
export RTT_CC=gcc
export RTT_CC_PREFIX=aarch64-none-elf-
export RTT_EXEC_PATH="<TOOLCHAIN>/xpack-aarch64-none-elf-gcc-14.2.1-1.1/bin"
export PATH="$HOME/.venvs/rtthread/bin:$RTT_EXEC_PATH:$PATH"

# 自检：file 应显示 x86-64 ELF，-dumpmachine 应为 aarch64-none-elf
file "$RTT_EXEC_PATH/aarch64-none-elf-gcc"
aarch64-none-elf-gcc -dumpmachine
command -v scons

scons --useconfig=.config   # 由 .config 重新生成 rtconfig.h（首次必跑）
scons -j4                   # 小内存环境改用 -j2
```

产物：`rtthread_a64.elf` / `rtthread_a64.bin` / `rtthread_a64.map`。改变工具链、架构或重要配置后，
先 `scons -c` 清理再全量编译。

> 不要 `source aarch64/thread-env.sh` 或 `aarch64/smart-env.sh`：它们指向包内并不存在的旧
> GCC 10.3 / musl 工具链，会覆盖 `RTT_EXEC_PATH` 与 `PATH`，导致编译失败。

## 5. 校验产物

```bash
cd RTOS-Bench/extern/rt-thread/bsp/phytium/aarch64

file rtthread_a64.elf
aarch64-none-elf-readelf -h rtthread_a64.elf | grep -E 'Machine|Entry point'
aarch64-none-elf-nm rtthread_a64.elf | grep -E 'rtosbench_rtthread_entry|rtosbench_register_rtos_workloads|__fsym_rtos_bench'
ls -lh rtthread_a64.elf rtthread_a64.bin rtthread_a64.map
```

验收应同时满足：scons 退出码为 0、ELF 为 AArch64、ELF/BIN/MAP 均已生成、RTOS-Bench 入口与
shell 注册符号确实存在。**仅生成 `rtthread_a64.bin` 不能证明 RTOS-Bench 已参与链接。**

## 6. 上板运行

U-Boot 下（示例经 TFTP 加载）：

```text
tftpboot 0x80080000 aarch64/rtthread_a64.bin
dcache flush
go 0x80080000
```

进入 msh 后执行 `rtos_bench` 子命令，例如：

```text
rtos_bench test-stress --job file
rtos_bench test-all
```

## 7. 注意事项与排查

- **改过 `.config` 后必须重跑 `scons --useconfig=.config` 再编译**，否则 `rtconfig.h`
  不更新，改动不生效（scons 只读 `rtconfig.h`）。
- **GCC 14 把隐式函数声明当硬错误**：出现 `implicit declaration of function 'xxx'`
  需补齐对应 libc/SDK 头文件，而非降级成警告。
- 工具链必须是 **linux-x64** 版本；构建宿主需 **x86-64 Linux**。
- BSP 已随包提供 SDK，`aarch64/SConstruct` 的 `install_phytium_sdk()` 检测到后会自动跳过；
  若 SDK 缺失，它会用 `libraries/phytium_standalone_sdk_install.py` 从 gitee 拉取
  （该脚本默认 pin 的 commit 与本平台所用不一致，需另行核对）。
- BSP 已内置 `SConstruct` 的 `RT_THREAD_PLATFORM` 平台宏与 `applications/rtbench_cmd.c`
  命令入口，无需手工添加。

| 现象 | 首先检查 |
| --- | --- |
| `Exec format error` / GCC 无法运行 | 宿主是否为 x86-64 Linux；工具链是否为 linux-x64 版 |
| `aarch64-none-elf-gcc: not found` | `RTT_EXEC_PATH` 与 shell `PATH` 是否都已设置 |
| 找不到 SDK 的 Kconfig / `fparameters.h` | `bsp/phytium/libraries/phytium_standalone_sdk/` 是否完整 |
| 缺 `yaml` / `kconfiglib` / `SCons` | 是否装了 `extern/rt-thread/tools/requirements.txt` 且用同一 venv |
| `pthread.h` / socket 接口缺失 | `.config` 是否被改动；重跑 `scons --useconfig=.config` |
| 构建成功但无 `rtbench` 注册符号 | `aarch64/rtos-bench` 链接是否指向仓库根；`RT_THREAD_PLATFORM` 是否生效 |
| C++ 编译进程被 OOM 杀死 | 降低并行度（`-j2`），检查宿主内存 |
