# 飞腾派 (Phytium Pi) BSP 差异归档

本目录保存**只存在于构建容器 `/work`** 的同事 BSP 差异，以及 RTOS-Bench 本次新增/改写的改动。
SDK（`libraries/phytium_standalone_sdk`）与工具链**不在此归档**（SDK 由 `phytium_standalone_sdk_install.py` 从 gitee 拉取，版本已在下方钉死）。

## 来源

- 同事交付包：`phytium(1).zip`（宿主微信文件目录，约 839 MB），使用其中的 `phytium/` 目录整体覆盖 `rt-thread/bsp/phytium/`。
- **SDK commit（实测，2026-10-16 从容器内 `.git` 复核）**：`8d6aefa63f4b38e72f9a3bd6e62a3ec30da50ae9`（`2026-06-09`，`!222 修复驱动，定时器配置调整，添加GIC MIBIS用例`）。
  - 注意：`phytium_standalone_sdk_install.py` 脚本里 pin 的**默认** commit 是 `20d40083fb3b1b328a2b750938123999d6c12262`，**与同事构建所用不一致**，复现时必须显式切到上表值。
- 目标板：`PHYTIUMPI_FIREFLY_BOARD` / `TARGET_PE2204`，4 核 SMP，UART1 控制台，ELF 入口 `0x80080000`。
- RT-Thread 内核：`RTOS-Bench/extern/rt-thread`，5.3.0，commit `db6c0ddb`。

## 文件清单

| 归档文件（本目录内） | 容器内原始路径（`bsp/phytium/` 下） | 与 git 树的差异 / 说明 |
|---|---|---|
| `aarch64/.config` | `aarch64/.config` | 最终构建配置：含 `USING_SDIF0/1` 双 TF、`USE_SDIF0/1_TF`、`BSP_USING_SDCARD_FATFS`、`BSP_USING_SDIF(_LAYER)`、`RT_USING_SDIO`、`DFS_FD_MAX=128`、`RT_USING_THREADSAFE_PRINTF`、`RT_SDIO_STACK_SIZE=4096`、`RT_MMCSD_STACK_SIZE=4096` |
| `aarch64/SConstruct` | `aarch64/SConstruct` | 同事版：`env.AppendUnique(CPPDEFINES=['RT_THREAD_PLATFORM'])` + `variant_dir='build/...'` + `link.lds` 预处理为 `link_preprocessed.lds` |
| `drv_us_delay.c` | `board/drv_us_delay.c` | 同事新增：`rt_hw_us_delay()`，AArch64 `cntpct` 物理计数器忙等（`rt_hw_get_gtimer_frq()` / `rt_hw_get_cntpct_val()`） |
| `rtbench_cmd.c` | `aarch64/applications/rtbench_cmd.c` | 同事新增：`MSH_CMD_EXPORT_ALIAS(cmd_rtbench, rtos_bench, ...)`，注册 msh 命令 **`rtos_bench`**（不是文档里的 `rtbench`），转发到 `rtosbench_rtthread_entry()` |
| `libraries/port/fboard_port/firefly/mnt_sdcard.c` | `libraries/port/fboard_port/firefly/mnt_sdcard.c` | **RTOS-Bench 改写**：原版硬编码 `FS_SD_DEVICE_NAME "sd00"` + `FS_SD_DEVICE_INDEX 0`，在 RT-Thread 5.x 命名（`<disk>p<partno>`）与 SDIF0/1 归属不确定时挂载失败；现改为遍历候选块设备名（`sd0p0..sd0p3`/`sd0`、`sd1p0..sd1p3`/`sd1`）自发现挂载，并保留 SDIF 强制探测 |
| `libraries/phytium_standalone_sdk/drivers/mmc/fsdif/fsdif_dma.c` | 同 | **RTOS-Bench 补丁**：新增 `#include <string.h>`，修复 GCC 14.2.1 下 `memset` 隐式声明硬报错（仅在启用 SDIF 后暴露） |

> 备注：`fsdif_dma.c` 属于 SDK 目录，但其中这一处改动是 RTOS-Bench 为通过编译所做，故一并归档；复现时在切好 SDK commit 之后再叠加此文件。

## 复现步骤

1. 进入容器 `phytium-build`（镜像 `rtos-bench-utils:phytium`）。
2. 用 `phytium(1).zip` 的 `phytium/` 覆盖 `rt-thread/bsp/phytium/`（**不要** clone SDK / 执行 `scons --attach`）。
3. 把 SDK 切到 commit `8d6aefa63f4b38e72f9a3bd6e62a3ec30da50ae9`：
   ```
   cd rt-thread/bsp/phytium/libraries/phytium_standalone_sdk
   git checkout 8d6aefa63f4b38e72f9a3bd6e62a3ec30da50ae9
   ```
4. 用本目录文件按上表覆盖对应路径（含 `aarch64/.config`、`aarch64/SConstruct`、`board/drv_us_delay.c`、`aarch64/applications/rtbench_cmd.c`、`libraries/port/fboard_port/firefly/mnt_sdcard.c`、`libraries/phytium_standalone_sdk/drivers/mmc/fsdif/fsdif_dma.c`）。
5. 注释 `aarch64/applications/main.c` 里的 `demo_core()`（复现同事版行为，避免启动刷屏）。
6. 在容器内做 LF 副本到 `/work`（Windows 挂载卷上的 CRLF / 软链在容器内会失效），然后编译：
   ```
   cd /work/rt-thread/bsp/phytium/aarch64
   RTT_ROOT=/work/rt-thread RTT_CC=gcc RTT_CC_PREFIX=aarch64-none-elf- \
   RTT_EXEC_PATH=/src/toolchain/bin PATH=/opt/rtt-venv/bin:/src/toolchain/bin:$PATH \
   scons -j4
   ```
   产物：`rtthread_a64.bin` / `.elf` / `.map`（入口 `0x80080000`）。
7. 上板（U-Boot）：
   ```
   tftpboot 0x80080000 aarch64/rtt_test.bin
   dcache flush
   go 0x80080000
   ```

> 工具链：`aarch64-none-elf-gcc-14.2.1-1.1`（Linux x86-64 ELF，容器内路径 `/src/toolchain`）。
> GCC 14 把隐式函数声明当**硬错误**处理——遇到类似 `implicit declaration of function 'xxx'` 时需补齐对应 libc/SDK 头文件，而非降级警告。
