# 东土龙芯（LoongArch64 / Linux musl）板端验收记录

更新时间：2026-10-12 16:18 （每次新增实测结果时更新这一行）

## 环境

- 板卡/系统：东土 Intewell 3.1.2，`Arch: loongarch64`（`root@localhost` shell），IP `192.168.137.114`。
- 产物：`build/rtbench-dongtu-loongarch64`，静态 ELF64 LoongArch（`Type: EXEC`、`NEEDED=0`）。
- 交叉工具链：`loongarch64-unknown-linux-musl`，GCC 14.2.0。

## 最新结论（2026-10-12）

用 **`fix/mqtt1012`** 分支构建的产物，`./rtbench test-all --no-realtime` **完整跑通**（总时长 675s）。

- **`test-schedule`：100.00 / 100**。8 个利用率梯度（30% ~ 100%），miss_rate 全为 0.0（每档 27 jobs = 9 负载 × 3 cycles）。
- **`typical-workload`：passed**。11 个负载各 10 轮**全部 success**，其中 MQTT 10 轮跑完（75.4s）、MODBUS 90.4s。
  - ⇒ **「test-all 跑到 workload 段卡住」的问题已由 `fix/mqtt1012` 解决。**
- `test-stress`：passed（378s）。
- `test-realtime`：skipped（本次用 `--no-realtime`）。
- `test-cmd`：本次 JSON 记 skipped（单独 `test-cmd` 上一轮跑到 10/12）。

产物 sha256：`990818dadd296acc2b8d1b2d7e8d98a7a294e99f5468e8322bb4d4963bd41639`，大小 `52231720` 字节
（源 `fix/mqtt1012 @ 6032acc`；用 `platforms/dongtu/loongarch64-linux/Makefile` 构建，本次同时修正了它的 `PROJECT_ROOT`）。

## 结果矩阵

| 命令 | 结果 | 备注 |
|---|---|---|
| `./rtbench -L` | ✅ | 列出 workload |
| `./rtbench -s` | ✅ | 9 个典型负载各一次，全部完成 |
| `./rtbench test-schedule --cycles 3` | ✅ | Final Score **100.00 / 100** |
| `./rtbench test-stress --job all` | ✅ | `135 stressor runs` |
| `./rtbench test-cmd` | ⚠️ 10/12 | 支持 `date,mkdir,echo,cd,pwd,cp,mv,ls,cat,rm`；不支持 `ps,touch` |
| `./rtbench test-all --no-realtime`（`fix/mqtt1012`） | ✅ | 完整跑完，675s，导出 JSON |
| `./rtbench test-realtime` | ❌ 段错误 | 在 `Finish test 1.` 之后崩（加 `-m` 同样崩） |
| `./rtbench test-realtime --delay / --cost / --multi-access / --multi-service` | ⏳ 待测 | 用于定位 realtime 崩点 |
| `./rtbench -A` | ❌ 只跑 stub | 已知 bug：`-A` 未接到 `run_all_workloads()`；用 `-s` 代替 |

## 问题清单

### 问题 1 ｜ `test-realtime` 段错误 —— 转交 realtime 负责人（未解决）

- 复现：`./rtbench test-realtime`。加 `-m` 同样段错误。
- 现象：输出到 `Finish test 1.` 之后 `Segmentation fault`。test 1（上下文切换）本身通过。
- 待办：用分节开关定位崩点：`--delay`（test1+test2）/ `--cost`（test4~10）/ `--multi-access` / `--multi-service`。

### 问题 2 ｜ `test-all` 的 workload 段卡住 —— 已由 `fix/mqtt1012` 解决 ✅

- 原现象：`./rtbench test-all --no-realtime` 在 MQTT 段长时间不动（最多等半小时）。
- 根因：MQTT 连接关闭时 benchmark 线程不退出（`MG_EV_CLOSE` 未置停止标志），`pthread_join` 挂住。
- 修复：`workloads/MQTT/mqtt_bench.c` 在 `MG_EV_CLOSE` 时置 `g_stop_flag = 1`，线程退出。
- 验证：用修复后的产物，`test-all --no-realtime` 完整跑完（MQTT 10 轮正常结束）。分支：`fix/mqtt1012`。
- 备注：`collect_workload_results()` 对每个负载跑 **10 轮**（`--quick` 5 轮），是设计倍数，不是 bug；修好退出条件后整段时长可接受（本轮 workload 段含 MQTT/MODBUS）。

### 问题 3 ｜ `-A` 未生效 —— 转交 CLI / command 负责人（未解决）

- 现象：`./rtbench -A` 只运行第 0 个负载（`stub`）。
- 根因：`rtbench_command_main` 中 `-A` 只设置 `run_all_workloads = 1`，但 `run_workload_command()` 未读取该标志。只有 `./rtbench -s`（恰好两个参数）才真正调用 `run_all_workloads()`。
- 建议：把 `-A` 接到 `run_all_workloads()`，使其与 `-s` 等价。

## 实测记录

### `test-all --no-realtime`（`fix/mqtt1012`，2026-10-12）

命令：

```sh
./rtbench test-all --no-realtime -o /root/rtbench_mqtt1012_result.json
```

- 总时长 `total_duration_sec = 675`（约 11 分钟），控制台打印 `Results saved to: ...`。
- `test-schedule`：8 档（30,40,50,60,70,80,90,100%），miss_rate 全 0.0。
- `typical-workload`：`stub, busywait, fast, epnp, ekf, icp, modbus, mqtt, pid, cusum, ewma`
  各 `rounds=10`、`success=true`；MQTT `exec_time_ms=75409`、MODBUS `exec_time_ms=90394`。

### `test-all --no-realtime --no-workload`（2026-10-10）

命令：`./rtbench test-all --no-realtime --no-workload -o /root/rtbench_result.json`
结果：完成，落盘 `/root/rtbench_result.json`；`test-stress` 135 stressor runs；`test-cmd` 10/12。

## 待补

- `test-realtime` 分节定位结果（崩在哪一节）。
- 各命令的截图 / 原始日志。
