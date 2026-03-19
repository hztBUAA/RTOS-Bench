# RT-Thread BSP 配置机制详解

本文档记录 RT-Thread BSP（Board Support Package）的配置体系，以 `qemu-virt64-aarch64` BSP 为例，解释 Kconfig / `.config` / `rtconfig.h` 三者的关系，以及 RTOS-Bench 项目中 `configs/` 参考文件的运作方式。

---

## 1. 三个核心文件

RT-Thread 的编译配置体系有三个文件，各司其职：

| 文件 | 格式 | 角色 | 谁读它 |
|------|------|------|--------|
| `Kconfig` | Kconfig 语法 | 菜单模板——定义有哪些选项、依赖关系、默认值 | menuconfig 工具 |
| `.config` | `CONFIG_XXX=y` | 用户的选择——记录每个选项的开关状态 | SCons 构建系统 |
| `rtconfig.h` | C 头文件 `#define` | 编译宏——让 gcc 知道哪些功能开启了 | gcc 编译器 |

### 1.1 Kconfig——菜单模板

分布在各目录下，定义配置选项。以 `bsp/qemu-virt64-aarch64/drivers/Kconfig` 为例：

```kconfig
config BSP_USING_VIRTIO_NET
    bool "Using VirtIO NET"          # menuconfig 中显示为勾选框
    select RT_USING_VIRTIO           # 勾选后自动连带开启
    select RT_USING_VIRTIO_NET       # 勾选后自动连带开启
    default y                        # 默认勾选
```

这段代码**不产生任何编译结果**，它只是定义了一个菜单项。Kconfig 是 RT-Thread 源码自带的，我们不修改。

BSP 顶层 `Kconfig` 通过 `source` 引入子目录：

```kconfig
# bsp/qemu-virt64-aarch64/Kconfig
mainmenu "RT-Thread Project Configuration"
source "$(RTT_DIR)/Kconfig"          # RT-Thread 内核选项
source "$(BSP_DIR)/drivers/Kconfig"  # BSP 板级驱动选项
```

### 1.2 `.config`——用户的选择

Kconfig 格式，和 Linux 内核的 `.config` 同源：

```ini
CONFIG_RT_USING_VIRTIO_NET=y            # 布尔值，启用
CONFIG_RT_LWIP_MEM_ALIGNMENT=8          # 数值
CONFIG_RT_CONSOLE_DEVICE_NAME="uart0"   # 字符串
# CONFIG_RT_USING_WIFI is not set       # 明确禁用（注释格式）
```

### 1.3 `rtconfig.h`——编译器看到的宏

`.config` 的 C 头文件映射，转换规则：

| `.config` | `rtconfig.h` |
|-----------|-------------|
| `CONFIG_XXX=y` | `#define XXX` |
| `CONFIG_XXX=数字` | `#define XXX 数字` |
| `CONFIG_XXX="字符串"` | `#define XXX "字符串"` |
| `# CONFIG_XXX is not set` | （不出现） |

示例：

```c
#define RT_USING_VIRTIO_NET
#define BSP_USING_VIRTIO_NET
#define RT_LWIP_MEM_ALIGNMENT 8
#define RT_CONSOLE_DEVICE_NAME "uart0"
```

---

## 2. 三者的生成关系

```
Kconfig（菜单模板）
    │
    │  menuconfig 读取，展示 TUI 菜单
    │  用户勾选/取消
    │
    ▼  保存时同时写出两个文件：
┌───────────┐         ┌────────────┐
│  .config  │────────>│ rtconfig.h │
│ (Kconfig) │ 自动转换 │  (C 宏)    │
└───────────┘         └────────────┘
    │                       │
    │ SCons 读它             │ gcc 读它
    │ 决定编译哪些 .c 文件    │ 决定 #ifdef 走哪个分支
    ▼                       ▼
构建系统选源文件          编译器选代码路径
```

**正常流程**下，开发者运行 `scons --menuconfig`，工具自动保证 `.config` 和 `rtconfig.h` 一致。

---

## 3. SCons 如何使用这些配置

SCons 同时依赖两个文件：

- 读 `.config` 决定哪些目录/源文件参与编译（如是否编译 `virtio_net.c`）
- 编译时 gcc 通过 `#include <rtconfig.h>` 读取宏定义

两者必须一致。如果 `.config` 说启用 VirtIO NET 但 `rtconfig.h` 缺少对应宏：

1. SCons 把 `virtio_net.c` 加入编译列表
2. gcc 编译时发现 `rtconfig.h` 里没有 `#define RT_USING_VIRTIO_NET`
3. 驱动内部 `#ifdef RT_USING_VIRTIO_NET` 保护的初始化代码被跳过
4. 编译通过但网卡驱动没有注册到系统

---

## 4. RTOS-Bench 的 configs/ 机制

### 4.1 为什么需要 configs/

`extern/rt-thread/` 是 `install.sh` clone 下来的外部代码，BSP 目录里的 `.config` 和 `rtconfig.h` 不在我们的 git 管理范围。所以项目维护了 `configs/` 目录作为**参考副本**：

```
configs/
├── rtthread_qemu_aarch64.config       ← .config 的参考副本
├── rtthread_qemu_aarch64_rtconfig.h   ← rtconfig.h 的参考副本
└── bsp_files/
    └── rtbench_cmd.c                  ← msh 命令入口（部署到 BSP applications/）
```

### 4.2 install.sh 部署流程

`install.sh` 中 `setup_rtthread()` 函数的工作：

```bash
# 1. clone RT-Thread（固定版本）
git clone ... "$RTT_DIR"
git checkout "$RTT_COMMIT"

# 2. 创建软链接让 BSP 能找到 rtos-bench 源码
ln -sf "$SCRIPT_DIR" "$BSP_DIR/rtos-bench"

# 3. 部署配置文件（无条件覆盖）
cp configs/rtthread_qemu_aarch64.config     → BSP/.config
cp configs/rtthread_qemu_aarch64_rtconfig.h → BSP/rtconfig.h
cp configs/bsp_files/rtbench_cmd.c          → BSP/applications/
```

每次 `install.sh` 都**无条件覆盖**，不检查目标是否已存在。

### 4.3 跳过了 menuconfig

`install.sh` 直接部署预制的配置文件，**没有运行 menuconfig**。这意味着：

- 不走自动生成流程，两个配置文件的一致性由我们自己保证
- 修改配置时，必须同时检查 `.config` 和 `rtconfig.h` 是否对应
- 推荐做法：在 BSP 目录跑一次 `scons --menuconfig` 调好，然后把生成的两个文件都拷回 `configs/`

---

## 5. 故障案例：VirtIO NET 网卡丢失

### 5.1 现象

用户通过 `install.sh` 安装后，QEMU 中运行 RT-Thread：

```
msh /> ifconfig
# 无输出，找不到网卡

msh /> modbus_test
[E/sal.skt] not find network interface device by protocol family(2).
[E/sal.skt] SAL socket protocol family input failed, return error -3.
```

### 5.2 根因

`configs/` 中两个文件不一致：

```
configs/.config        →  CONFIG_BSP_USING_VIRTIO_NET=y     ✅ 有
configs/rtconfig.h     →  (缺 #define BSP_USING_VIRTIO_NET)  ❌ 没有
                          (缺 #define RT_USING_VIRTIO_NET)   ❌ 没有
```

原因是有人手动编辑过 `configs/rtconfig.h`（改了别的内容），没有从 menuconfig 重新生成，导致 VirtIO NET 相关的宏丢失。`.config` 没被动过所以还是对的。

### 5.3 修复

从已验证可工作的 BSP `rtconfig.h` 回写到 `configs/rtconfig_qemu_aarch64_rtconfig.h`，关键补回的宏：

```c
#define RT_USING_VIRTIO_NET          // 内核层 VirtIO 网卡驱动
#define BSP_USING_VIRTIO_NET         // BSP 层板级网卡初始化
#define RT_USING_LWIP_VER_NUM 0x20102
#define RT_LWIP_MEM_ALIGNMENT 8      // aarch64 需要 8 字节对齐（原来写的 4）
#define RT_LWIP_NETIF_NAMESIZE 6
#define PTHREAD_NUM_MAX 32           // 原来是 8，不够用
```

### 5.4 验证

修复后 QEMU 中：

```
msh /> ifconfig
network interface device: virtio (Default)
MTU: 1500
MAC: 52 54 00 12 34 56
FLAGS: UP LINK_UP INTERNET_UP DHCP_ENABLE ETHARP BROADCAST IGMP
ip address: 10.0.2.15
```

Modbus: 20 requests, 0 errors。MQTT: 617 messages sent。

---

## 6. 配置变更操作规范

### 修改配置的推荐流程

```bash
# 1. 在 BSP 目录运行 menuconfig
cd extern/rt-thread/bsp/qemu-virt64-aarch64
scons --menuconfig

# 2. 在 TUI 中修改选项，保存退出
#    此时 .config 和 rtconfig.h 自动同步

# 3. 验证编译
cd /path/to/RTOS-Bench
./run-rtthread.sh -b

# 4. 验证运行
./run-rtthread.sh

# 5. 回写到 configs/（两个文件都要拷）
cp extern/rt-thread/bsp/qemu-virt64-aarch64/.config    configs/rtthread_qemu_aarch64.config
cp extern/rt-thread/bsp/qemu-virt64-aarch64/rtconfig.h configs/rtthread_qemu_aarch64_rtconfig.h

# 6. 提交
git add configs/
git commit -m "fix(configs): sync from menuconfig - 描述改了什么"
```

### 注意事项

- **永远不要只改一个文件**。`.config` 和 `rtconfig.h` 必须成对同步
- **优先用 menuconfig**，而不是手动编辑 `rtconfig.h`
- **RT-Thread 版本已锁定**。`install.sh` 中 `RTT_COMMIT` 固定了上游 commit，避免上游更新导致 Kconfig 选项变化与我们的配置文件不兼容
