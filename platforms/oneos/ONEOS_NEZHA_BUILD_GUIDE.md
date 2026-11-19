# OneOS 哪吒派 D1H — RTOS-Bench 编译与上板指南

更新时间：2026-11-19 15:30

本指南面向**拿到 RTOS-Bench 源码压缩包**的一方。
本指南不依赖 GitHub。全部步骤离线可做。

---

## 0. 前置条件

需要厂家提供 OneOS V1.6 或更新版本的工程包。包内必须有：

| 目录 | 作用 |
|---|---|
| `projects/d1h-nezha` | 内核镜像工程 |
| `projects/d1h-nezha_out` | 动态模块工程 |
| `oneos-multi-cmake` | 命令行构建脚本 |
| `oneos-multi-tools` 与 RISC-V 工具链 | 编译器与 CMake |

工具链要求：RISC-V 64 位，架构 `riscv64-c900`，`-march=rv64imafdxthead`，`-mabi=lp64d`。

路径限制：**工程路径不要含中文和空格。**

---

## 1. 先分清两个产物

| 产物 | 工程 | 作用 |
|---|---|---|
| `oneos.bin` / `oneos.img` | `projects/d1h-nezha` | 系统镜像。提供 shell、网络、telnet、模块加载器 |
| `d1h-nezha_out.out` | `projects/d1h-nezha_out` | 动态模块。RTOS-Bench 本体 |

RTOS-Bench **编在 `.out` 模块里**。默认不需要重编内核镜像。

只有一种情况要重编镜像：你希望 shell 直接识别 `rtbench` 命令。

---

## 2. 放置源码

设 OneOS 工程根目录为 `<ONEOS_ROOT>`。

把压缩包解压出的 `RTOS-Bench/` 目录复制到 `projects/d1h-nezha_out/` 下。

最终结构：

```
<ONEOS_ROOT>/
├── oneos-multi-cmake/
├── oneos-multi-tools/
└── projects/
    ├── d1h-nezha/                  # 内核镜像工程
    └── d1h-nezha_out/              # 动态模块工程
        ├── CMakeLists.txt          # 下一步修改
        ├── src/                    # 厂家模板自带
        ├── include/                # 厂家模板自带
        └── RTOS-Bench/             # 本工具源码
```

也可以把 `RTOS-Bench/` 放在其它位置。此时第 3 步的路径要写成绝对路径。

---

## 3. 修改 out 工程

打开 `projects/d1h-nezha_out/CMakeLists.txt`。在 `add_subdirectory(RTOS-Bench)` 之前加入两行：

```cmake
set(ONEOS_NEZHA_CXX_LITE ON)
add_subdirectory(src)
add_subdirectory(RTOS-Bench)
```

**顺序很重要。** `ONEOS_NEZHA_CXX_LITE` 必须在 `add_subdirectory(RTOS-Bench)` 之前设置。

模板通常已有 `add_subdirectory(src)`。已存在时不要重复添加。

两个新增行的作用：

| 行 | 作用 |
|---|---|
| `set(ONEOS_NEZHA_CXX_LITE ON)` | 启用哪吒专用编译选项 |
| `add_subdirectory(RTOS-Bench)` | 把 RTOS-Bench 编进本模块 |

`ONEOS_NEZHA_CXX_LITE` 做两件事：

1. 把 test-schedule 的负载集合限制为 `fast,pid,cusum,ewma`。
2. 定义 `EIGEN_NO_IO`，绕开 Eigen 的 C++ IO 路径。

**为什么必须开：** 哪吒 D1H 真实执行 `icp`、`ekf`、`mqtt`、`modbus` 会触发内核崩溃。
用户态看门狗无法拦截该崩溃。

如果对方明确要试全部负载，可以不开该开关。此时请自行承担整板掉线的风险。

可选清理：厂家模板的 `src/hello.c` 会周期打印 `hello`。可以删掉，或改为空函数，减少日志噪声。

---

## 4. 编译动态模块

进入 out 工程，执行厂家构建脚本：

```bash
cd <ONEOS_ROOT>/projects/d1h-nezha_out
bash ../../oneos-multi-cmake/oos.sh build
```

也可以直接调用 CMake：

```bash
cd <ONEOS_ROOT>/projects/d1h-nezha_out
cmake -S . -B build
make -C build -j
```

产物：

```
<ONEOS_ROOT>/projects/d1h-nezha_out/out/d1h-nezha_out.out
```

通过判据：命令返回 0，且文件存在、大小非零。

建议记录产物指纹：

```bash
sha256sum out/d1h-nezha_out.out
ls -l out/d1h-nezha_out.out
```

---

## 5. （可选）编译内核镜像

只在需要 shell 直接执行 `rtbench` 时做本节。

步骤：

1. 复制 `RTOS-Bench/platforms/oneos/rtbench_cmd_stub.c` 到 `projects/d1h-nezha/application/`。
2. 打开该文件，把 `module_name` 改为板端实际路径，例如：

```c
const char *module_name = "/user/ctest.out";
```

3. 编译镜像：

```bash
cd <ONEOS_ROOT>/projects/d1h-nezha
bash ../../oneos-multi-cmake/oos.sh build
```

产物：

```
out/oneos.elf
out/oneos.bin
out/oneos.img
out/oneos_symtbl.txt
```

**路径必须一致。** 镜像里的 `module_name` 必须等于上板时 `ld` 的路径。

---

## 6. 上板

### 6.1 启动与联网

镜像启动后，串口出现 `sh />` 或 `sh /user>` 即为成功。

配置网络：

```
ifconfig
set_if e00 <board-ip> <gateway-ip> 255.255.255.0
default_netif e00
telnetd start
```

### 6.2 准备 TFTP

板子从 TFTP 服务器拉取 `.out`。

示例参数：

| 角色 | 示例值 |
|---|---|
| 板卡 | `192.168.31.211` |
| 网关 | `192.168.31.1` |
| TFTP 服务器 | `192.168.31.100` |

板端 `tftp_client` 的文件名相对 TFTP 根目录。

### 6.3 拉取并加载模块

```
mkdir /user
mount -t fatfs <partition> /user
tftp_client <tftp-ip> get d1h-nezha_out.out /user/ctest.out
ld /user/ctest.out
list_lmodule
```

成功标志：

```
TFTP client get file end, err=0
module[/user/ctest.out] loaded
```

**文件名要短。** OneOS shell 单行上限 80 字符。长文件名会被截断。

因此 TFTP 源文件用短名，例如 `ctw.out`。板端目标用固定短名 `/user/ctest.out`。

---

## 7. 运行

### 7.1 内核镜像已带 stub

直接执行：

```
rtbench -L
rtbench -b stub -t 1
rtbench -b busywait -t 1
rtbench test-schedule --cycles 1 --util-start 30 --util-end 30 --util-step 30
```

### 7.2 内核镜像不带 stub

动态模块里的 `SH_CMD_EXPORT` 不一定注册到主 shell。
此时直接敲 `rtbench` 会报 `Command not found`。

用 runner 模块间接调用。runner 做三件事：

1. 用 `os_module_find("/user/ctest.out")` 找到主模块。
2. 用 `os_module_symbol_find_by_handle(handle, "cmd_rtbench_stub", &addr)` 找入口。
3. 构造 `argv` 并调用该入口。

runner 源码模板见：

```
RTOS-Bench/platforms/oneos/0627-acceptance/scripts/8gradient-runner/build_schedfull_runner.sh
```

runner 是另一个 `.out`。用同样方式编译、上传、`ld`。

---

## 8. 首次上板的推荐顺序

不要第一次就跑完整 `test-all`。
长测试会掩盖模块加载、栈空间、网络配置问题。

推荐顺序：

| 序号 | 命令 | 目的 |
|---|---|---|
| 1 | `list_lmodule` | 确认模块已加载 |
| 2 | `rtbench -L` | 确认负载注册 |
| 3 | `rtbench -b stub -t 1` | 最小冒烟 |
| 4 | `rtbench -b busywait -t 1` | 调度冒烟 |
| 5 | `rtbench test-schedule --cycles 1 ...` | 可调度性 |
| 6 | `rtbench test-realtime` | 实时性能 |
| 7 | `rtbench test-stress -s cpu -t 1` | 压力测试 |
| 8 | `rtbench test-all --quick` | 综合 |

每一项通过后，再做下一项。

---

## 9. 常见问题

### 9.1 编译期

| 现象 | 原因 | 处理 |
|---|---|---|
| `Base target ... not found` | `add_subdirectory(RTOS-Bench)` 位置不对 | 确认 `project()` 已在 out 工程根 CMakeLists 调用 |
| C++ 重复定义 `pthread`、`timespec` | C++ 文件混入内核 include response 文件 | 只给 C 文件传 kernel include response |
| `min`/`max` 宏冲突 | BSP 头文件在 `__cplusplus` 下定义宏 | 使用 `oneos_cxx_compat.hpp`，或让厂家修 BSP |
| Eigen 报 `operator<<` 找不到 | 开关未开 | 确认 `ONEOS_NEZHA_CXX_LITE` 已设置 |

### 9.2 运行期

| 现象 | 原因 | 处理 |
|---|---|---|
| `rtbench: Command not found` | 模块未加载，或 `SH_CMD_EXPORT` 未注册 | 先 `list_lmodule`。再考虑 runner 或重编镜像 |
| `Error: Module '/user/xxx.out' not loaded.` | stub 路径与 `ld` 路径不一致 | 统一路径 |
| `ld` 报 `ELF64_R_TYPE ... unrelocaled` | 模块过大或重定位不兼容 | 关闭 `ONEOS_NEZHA_CXX_LITE` 之外的负载，减小模块 |
| telnet 无回显 | telnet 会话被占用或 shell 卡死 | 关掉其它会话。串口执行 `telnetd start`。必要时断电复位 |
| 测试输出在 telnet 看不到 | 模块 `printf` 多走串口 | 接串口查看输出 |
| 运行中整板掉线 | 不安全的负载被真实执行 | 开启 `ONEOS_NEZHA_CXX_LITE` |
| `ping` 通但 shell 不可用 | 网络栈活着不等于 shell 可用 | 以 `help`、`ifconfig` 有回显为准 |

### 9.3 改代码后结果没变

先清理再编译：

```bash
rm -rf out build
bash ../../oneos-multi-cmake/oos.sh build
```

重新上传模块前，先卸载旧模块：

```
unld /user/ctest.out
```

---

## 10. 交付检查清单

编译完成后，建议随产物提供以下信息：

- 源码版本或压缩包名称
- 编译器版本
- `.out` 文件名、大小、SHA256
- 内核镜像名称、大小、SHA256（如有重编）
- 是否开启 `ONEOS_NEZHA_CXX_LITE`
- 上板日志：启动、`list_lmodule`、`rtbench -L`、各测试返回码

---

## 11. 相关文档

| 文档 | 内容 |
|---|---|
| `NEZHA_D1H_SOP.md` | 接入、验收与网络配置 |
| `NEZHA_D1H_SHARED_TFTP_SOP.md` | 多人共享 TFTP 部署 |
| `NEZHA_D1H_WIN_DIRECT_DEBUG_CLOSURE.md` | 编译原理、镜像与模块关系、排错 |
| `0627-acceptance/RESULT.md` | 2026-06-27 验收记录 |
