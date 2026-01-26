# 工作负载说明与测试指引

本目录包含已经打包进 rt-bench 的典型负载，统一通过 `workload_registry` 注册，可在 Linux/SylixOS/RT-Thread 上使用：

- `fast`：FAST 角点检测（C）
- `epnp`：Perspective-n-Point 求解（C++/Eigen）
- `ekf`：飞行数据集 EKF 重放（C++）
- `icp`：点云 ICP 对齐（C++）
- `modbus`：本地 Modbus TCP 回环（C）（当前在 RT-Thread 未启用，需 POSIX socket/pthread 支持）
- `mqtt`：GeoLife 轨迹 MQTT 发布（C）（当前在 RT-Thread 未启用，需 POSIX socket/pthread 支持）
- `pid`：PID 控制器合成数据基准（C++）
- `cusum`：CUSUM 均值漂移/台阶检测（C）
- `ewma`：EWMA 平滑与残差阈值告警（C）

## 运行方式（统一）

使用 rt-bench 入口，选择负载名即可：

```sh
# Linux/SylixOS
./rt-bench -p 1 -t 1 -b <workload>

# RT-Thread (msh)
rtbench -p 1 -t 1 -b <workload>
```

- `-p`：周期（秒，浮点），建议 0.5~1s。
- `-t`：迭代次数，测试用 1 先确认可运行，再增大。
- `-b`：负载名称：`fast|epnp|ekf|icp|modbus|mqtt|pid|cusum|ewma`。
- 可按需添加 `-c` 绑定核心，`-f` 设置优先级（RT-Thread SCHED_FIFO）。

## 各负载备注与推荐参数

- `fast`：内部默认循环 1000 次；需要 `workloads/FAST/include_imgs` 中的内置数据，其他参数无需设置。
  - 推荐：`-p 1 -t 1 -b fast`
- `epnp`：默认迭代 1000 次，固定 100 点；需要 Eigen；运行时间较长。
  - 推荐：`-p 1 -t 1 -b epnp`
- `ekf`：重放 `iris_gps.h` 数据集，无额外参数，执行时长取决于数据量。
  - 推荐：`-p 1 -t 1 -b ekf`
- `icp`：点云对齐一次性运行；依赖内置 suzanne 模型。
  - 推荐：`-p 1 -t 1 -b icp`
- `modbus`：本地服务+客户端回环，使用 127.0.0.1:5020；需要网络栈（未在 RT-Thread/SylixOS 打包）。
  - 推荐：`-p 1 -t 1 -b modbus`
- `mqtt`：向 `broker.emqx.io:1883` 发布 GeoLife 轨迹；需要可访问公网的网络（未在 RT-Thread/SylixOS 打包）。
  - 推荐：`-p 1 -t 1 -b mqtt`（若离线环境请跳过或改用本地 broker）
- `pid`：合成数据 100000 次循环；CPU 计算型。
  - 推荐：`-p 1 -t 1 -b pid`
- `cusum`：合成 6000 点序列，t=2000 处均值台阶，t≥4000 线性漂移；累积和越阈值计为告警。
  - 推荐：`-p 0.5 -t 1 -b cusum`
- `ewma`：合成正弦基线+噪声，t=1200/2500 注入尖峰，t=3600 起 40 点掉线置零；|残差| 超阈值计数。
  - 推荐：`-p 0.5 -t 1 -b ewma`

## 构建与平台差异

- 默认 `generator/Makefile` 设置 `RTOS_WORKLOADS=1`，Linux/SylixOS 会自动编译本目录的源码（需要 `-lstdc++`）。
- RT-Thread 通过 `extern/rt-thread/bsp/qemu-virt64-aarch64/applications/SConscript` 自动包含本目录，无需 BSP 再维护负载源码。
- 所有负载统一走 `workload_registry`，不再支持旧的 `benchmark_registry` 覆盖模式。

## 测试建议（待执行）

- Linux 快速冒烟：`make -C generator PLATFORM=linux` 后，分别运行 `-b fast/epnp/ekf/icp/modbus/pid/cusum/ewma`，MQTT 需联网。
- RT-Thread：`run-rtthread.sh -b` 编译后，msh 中 `rtbench -p 1 -t 1 -b fast` 等逐一验证。
- 记录性能：每个负载至少跑一次，保留 `timing.csv`，并注明参数、平台、核心配置。
