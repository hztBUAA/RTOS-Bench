# 东土/Intewell x86（i386 分区）编译说明

本目录提供**不依赖 Eclipse IDE** 的 x86 RTOS 分区构建流程。目标是把
`generator/` 与 `workloads/` 编译成 Intewell x86 虚拟机可加载的镜像：

```
Debug/make/rtosbench_x86.elf
Debug/make/rtosbench_x86.bin   <-- 上传到 Intewell 管理页的镜像
```

## 1. 适用环境

| 项 | 值 |
|---|---|
| SDK | Intewell Developer **2.3.1**（内含 x86 运行库与 `x86_64-intewell-elf-gcc 9.3.0`） |
| 工具链 | `host/gnu/gcc-9.3.0/x86/bin/x86_64-intewell-elf-*` |
| 目标架构 | **x86 / i386，32 位**（`ARCH := _X86_`） |
| 默认 board | 东土 `kyland/newpre3101_i7` |
| 运行库 | `target/RTCore/lib/gcc-9.3.0/x86/i386/little` |

> 注意：Intewell x86 的 RTOS 分区是 **32 位 i386**，不是 x86_64。
> 宿主是 x86_64 不影响；编译命令使用 `-m elf_i386`。

**2.2.0 的 ARM 版 IDE 只有 `lib64/.../arm64` 运行库，不能链接 x86。**
必须使用带 `target/RTCore/lib/gcc-9.3.0/x86/i386/little` 的 2.3.1 安装包。

## 2. 源码编译 vs 完整链接

- **源码编译（`-c`）**：每个 `.c/.cpp` 只生成 `.o`，检查语法、头文件和 ABI；
  不解析外部符号，也不产生可加载镜像。缺 BSP 库时编译仍可能全绿。
- **完整链接**：把应用对象、`crt0.o`、`board_crt0.o`、`ttosInit.o` 和
  `libvbsp/libcommon/librtl/libtTOS/...` 一起链接，按链接脚本布局内存，
  生成 `.elf` 并转成 `.bin`。只有链接通过，镜像才能被 VM 加载。

本目录的一键脚本做的是**完整链接**。

## 3. 一键构建

```bash
python platforms/dongtu/x86-cli/build_x86.py
# 或指定 SDK / 工程目录
python platforms/dongtu/x86-cli/build_x86.py \
    --sdk C:/Intewell_Developer_V2.3.1_P2_20241225 \
    --project-dir C:/work/dongtu_x86/rtosbench_x86 --clean
```

也可以设置环境变量 `INTEWELL_HOME`。脚本会：

1. 从 `project-template/` 复制最小 x86 TTOS 工程；
2. 把 `@INTEWELL_HOME@` / `@PROJECT_DIR@` 替换为本机路径；
3. 写入 `config_os.mk`（`ARCH := _X86_` + `platforms/dongtu/intewell.mk`）；
4. 复制 `platforms/dongtu/userAppInit.c`；
5. 复制 SDK 的 `target/RTCore/arch/x86/vm/default.mtp`；
6. 生成 `Debug/make/ImgInfoGen`；
7. 调用 SDK 自带 `gnuwin/msys64/usr/bin/make.exe all`。

成功输出：

```
[x86] done
  ELF: .../Debug/make/rtosbench_x86.elf
  BIN: .../Debug/make/rtosbench_x86.bin  <-- upload this image
```

## 4. 上板：管理页上传 + 运行测试（赛迪内网）

### 4.1 地址与端口

| 角色 | 赛迪内网地址 | 原内网地址 | 端口/协议 | 用途 |
|---|---|---|---|---|
| 跳板机 | `10.134.151.45` | — | SSH `1026`（`rtbench`/`rtbench`） | 转发管理页 |
| Intewell 管理页（工控机宿主） | `192.168.31.240` | — | HTTP `5556` | 上传镜像 |
| 本机转发入口 | `127.0.0.1` | — | HTTP `5556` | 浏览器访问 |
| 工控机宿主 SSH | `192.168.31.240` | — | SSH `22`（`kd`/`1`） | 看日志 `/download/log.txt` |
| **RTOS 测试面 vm1** | **`192.168.137.215`** | `192.168.31.206` | telnet `23` | 跑 `rtbench` |

> `192.168.137.215` 是 vm1 测试面；`192.168.31.240:5556` 是宿主管理页。
> 不要 telnet 管理页，也不要把镜像上传到 vm1。

### 4.2 打开管理页（Windows PowerShell）

```powershell
ssh -L 5556:192.168.31.240:5556 rtbench@10.134.151.45 -p 1026
# 密码：rtbench
```

保持 SSH 窗口不关，浏览器打开：

```text
http://127.0.0.1:5556
```

### 4.3 上传并生效镜像

```text
实时虚拟机
  -> rtos_bench
  -> 进入右侧配置/编辑页面
  -> 镜像
  -> 上传 Debug/make/rtosbench_x86.bin   （注意是 .bin，不是 .elf）
  -> 生效
  -> 点击生效
```

上传后等待 vm1 重启；测试面 IP 为 `192.168.137.215`。
可在宿主 `kd@192.168.31.240` 上看启动日志：

```sh
tail -f /download/log.txt
```

### 4.4 跑测试

```bash
telnet 192.168.137.215 23
```

出现 `vm1`/Intewell shell 后执行（`rtbench` 是 `platforms/dongtu/shell.c`
注册的 shell 命令，**不要**写成 `./rtbench`）：

```bash
# 全量验收
rtbench test-all -o /apps/rtosbench/rtbench_result.json
cat /apps/rtosbench/rtbench_result.json

# 仅连通性冒烟（正式验收不要加 --quick）
rtbench test-all --quick -o /apps/rtosbench/rtbench_result.json
```

若 `/apps/rtosbench/` 不存在或不可写，换 `/user/`、`/tmp/` 等可写目录，并同步
`utils/remote-test/boards/board_intewell_gongkong.yaml` 的 `test.command`
与 `result.remote_path`。

### 4.5 自动化执行（deploy.py）

镜像上传必须手工走管理页；上传并生效后：

```bash
python utils/remote-test/deploy.py \
  -c utils/remote-test/boards/board_intewell_gongkong.yaml --skip-deploy
```

`deploy.py` 通过 telnet 发送 `test.command`，用 `terminal_capture` 从终端
输出中提取 JSON；结果默认落到 `results/intewell_gongkong/`。

> 当前状态：**编译/链接已在 Windows 本机验证通过**；上板 IP/端口依据仓库内
> `utils/remote-test/boards/` 与 `platforms/dongtu/readme.md` 的赛迪内网映射整理。
> 首次现场执行请确认 vm1 telnet 登录方式与 `/apps/rtosbench` 可写性，确认后把
> `board_intewell_gongkong.yaml` 头部状态从「待确认」改为「已验证」。

## 5. 已知问题（都已在本流程中处理）

| 现象 | 原因 / 处理 |
|---|---|
| `#error "Dongtu SDK header defining LES_SHARED_MEM_VA is not included"` | 该宏只在 `__aarch64__` 的 LES 绑定函数里使用；x86 下用 `-DLES_SHARED_MEM_VA=0` 占位 |
| `undefined reference to __atomic_fetch_nand_*` / `__atomic_fetch_add_4` | 32 位 x86 需要 `-latomic` |
| `undefined reference to _sidata_start` | 工程 `default.mtp` 缺 `.sidata`；必须使用 `arch/x86/vm/default.mtp` |
| `objcopy: cannot open: ImgInfoGen` | IDE 才生成的镜像元数据；脚本会生成 `Debug/make/ImgInfoGen` |
| `ar: ./src/userAppInit.o: No such file` | 干净构建时 `Debug/make/src` 不存在；模板的 `subdir.mk` 已加 `mkdir -p` |
| 用 ARM IDE 打开/编译 x86 工程失败 | x86 与 ARM 的 IDE/BSP/宏不同，不要混用 |

## 6. 手动（IDE）流程

仍可按 [`../readme.md`](../readme.md) 的步骤在 x86 Intewell IDE 中新建项目、
复制 `config_os.mk` 与 `userAppInit.c` 后构建；本目录的 `project-template/`
可作为“现有项目导入”的起点（把 `@INTEWELL_HOME@`/`@PROJECT_DIR@` 替换为本机路径）。
