# RT-Thread 网络 & Modbus/MQTT Workload 问题分析与修复计划

## Context

用户询问 RT-Thread 下 Modbus 和 MQTT workload 能否正常运行，怀疑 rtconfig 有问题。经过全面排查，发现存在 **三个层面的问题**。

---

## 问题 1：`.config` 与 `rtconfig.h` 不一致（定时炸弹）

**现状**：
- `extern/rt-thread/bsp/qemu-virt64-aarch64/.config` 中网络全部 **关闭**（`# CONFIG_RT_USING_LWIP is not set` 等）
- `extern/rt-thread/bsp/qemu-virt64-aarch64/rtconfig.h` 中网络全部 **开启**（手动修改过）
- SCons 编译读取的是 `rtconfig.h`，所以当前 binary **实际包含网络栈**

**风险**：任何人执行 `scons --menuconfig` 保存后，`rtconfig.h` 会从 `.config` 重新生成，网络支持将被 **全部去除**。

**修复**：将 `.config` 同步为网络开启状态，使其与 `rtconfig.h` 一致。可从 `configs/rtthread_qemu_aarch64.config`（参考配置，网络已开启）覆盖。

**涉及文件**：
- `extern/rt-thread/bsp/qemu-virt64-aarch64/.config` — 需要同步

---

## 问题 2：MQTT workload 使用 `epoll`，RT-Thread 不支持

**现状**：
- `workloads/MQTT/mongoose_config.h` 第 17 行：`#define MG_ENABLE_EPOLL 1`
- Mongoose 启用 epoll 后会 `#include <sys/epoll.h>` 并调用 `epoll_create1`、`epoll_ctl`、`epoll_wait`
- RT-Thread 的 POSIX 层 **没有 epoll 实现**（只有 `poll` 和 `select`）
- `rtconfig.h` 定义了 `RT_USING_POSIX_POLL` 和 `RT_USING_POSIX_SELECT`，但无 epoll

**结果**：MQTT workload 在 RT-Thread 上**编译失败**或链接失败（缺少 `<sys/epoll.h>`、`epoll_*` 符号）。

**修复**：在 `mongoose_config.h` 中根据平台条件禁用 epoll：
```c
#ifdef RT_THREAD_PLATFORM
#define MG_ENABLE_EPOLL 0
#define MG_ENABLE_POLL  1   // 使用 poll() 替代
#else
#define MG_ENABLE_EPOLL 1
#endif
```

**涉及文件**：
- `workloads/MQTT/mongoose_config.h` — 条件禁用 epoll

---

## 问题 3：MQTT workload 依赖外部 broker，QEMU user-mode 网络有限制

**现状**：
- MQTT 连接外部 broker `tcp://44.232.241.40:1883`
- QEMU 使用 `-netdev user`（用户模式 NAT），guest 可以主动外连，但需要 DNS 和路由正常
- 该 IP 是否为公开可用的 MQTT broker（如 test.mosquitto.org 的 IP）需要验证

**结果**：在无外网环境或 broker 下线时，MQTT benchmark 必然失败。

**建议**：可考虑增加 loopback 自测模式（类似 Modbus 的做法），但这是功能增强，非必修项。

---

## 问题 4（次要）：MQTT `mqtt_test()` 函数使用 pthread 但 `MQTT_HAVE_PTHREAD=0`

**现状**：
- `mqtt_bench.c` 第 1-10 行：`RT_THREAD_PLATFORM` 时 `MQTT_HAVE_PTHREAD=0`
- 但 `mqtt_test()` 函数（第 160-180 行）无条件使用 `pthread_create`/`pthread_join`
- workload registry 走的是 `mqtt_bench_run()`（第 184 行），直接调用 `mqtt_thread_entry`，不走 pthread

**结果**：通过 workload registry（`rtbench -b mqtt`）调用没问题。但 MSH 命令 `mqtt_test` 可能编译不过或运行崩溃。

**修复**：给 `mqtt_test()` 加 `#if MQTT_HAVE_PTHREAD` 保护。

**涉及文件**：
- `workloads/MQTT/mqtt_bench.c` — 给 mqtt_test 加条件编译

---

## Modbus workload 状态

Modbus workload **理论上可以正常运行**：
- 使用 vendored nanoMODBUS 库（无外部依赖）
- 连接 `127.0.0.1:5020`（loopback），不需要外网
- pthreads 和 sockets 在 RT-Thread 平台都设为启用（`MDB_HAVE_PTHREAD=1`, `MDB_HAVE_SOCKETS=1`）
- 依赖 RT-Thread 的 POSIX socket 层（lwIP + SAL），当前 `rtconfig.h` 已启用

**前提**：lwIP loopback 需要正常工作（`LWIP_NETIF_LOOPBACK` 在 rtconfig.h 中已定义为 1）。

---

## 修复步骤

### Step 1: 同步 `.config` 使其与 `rtconfig.h` 一致
- 从 `configs/rtthread_qemu_aarch64.config` 复制覆盖 BSP 的 `.config`
- 验证关键网络宏一致

### Step 2: 修复 MQTT mongoose_config.h 的 epoll 问题
- 在 `workloads/MQTT/mongoose_config.h` 中条件化 epoll 设置
- RT-Thread 平台使用 poll 替代 epoll

### Step 3: 修复 mqtt_test() 的条件编译
- `mqtt_bench.c` 中 `mqtt_test()` 函数用 `#if MQTT_HAVE_PTHREAD` 保护

### Step 4: 编译验证
```bash
./run-rtthread.sh -b    # 确认编译通过
```

### Step 5: 运行验证
```bash
# QEMU 中运行
rtbench -b modbus       # 验证 Modbus loopback 测试
rtbench -b mqtt         # 验证 MQTT（需外网 broker 可达）
```

---

## 总结

| 项目 | 当前状态 | 能否跑 | 原因 |
|------|---------|--------|------|
| RTT 网络栈 | rtconfig.h 已启用，.config 未同步 | 当前能编译（有隐患） | .config 与 rtconfig.h 不一致 |
| Modbus workload | 代码无明显问题 | 理论上可以 | 依赖 loopback TCP，已自包含 |
| MQTT workload | **epoll 不兼容** | **编译/运行失败** | mongoose 配置了 epoll，RTT 无 epoll |
| MQTT 外部依赖 | 硬编码外部 broker IP | 看网络环境 | QEMU user-mode NAT 需要宿主机能上网 |
