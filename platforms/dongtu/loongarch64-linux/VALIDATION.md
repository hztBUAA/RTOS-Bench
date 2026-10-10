# 东土龙芯（LoongArch64 / Linux musl）板端验收记录

更新时间：2026-10-10 17:46 （每次新增实测结果时更新这一行）

## 环境

- 板卡/系统：东土 Intewell 3.1.2，`Arch: loongarch64`（`root@localhost` shell），IP `192.168.137.114`。
- 产物：`build/rtbench-dongtu-loongarch64`，静态 ELF64 LoongArch（`Type: EXEC`、`NEEDED=0`）。
- 参考 sha256（main 上 `ce842f2` 源码构建）：`44f4e1ef90452b9a44b3e2024b6c4c9b8016d8e58a77319fa4665b34c3ea8a01`，大小 `52232664` 字节。
- 交叉工具链：`loongarch64-unknown-linux-musl`，GCC 14.2.0。

## 结果矩阵

| 命令 | 结果 | 备注 |
|---|---|---|
| `./rtbench -L` | ✅ | 列出 workload |
| `./rtbench -s` | ✅ | 9 个典型负载各一次，全部完成 |
| `./rtbench test-schedule --cycles 3` | ✅ | 跑完（Final Score 见导出 JSON） |
| `./rtbench test-stress --job all` | ✅ | `135 stressor runs`，总耗时约 5s |
| `./rtbench test-cmd` | ⚠️ 10/12 | 支持 `date,mkdir,echo,cd,pwd,cp,mv,ls,cat,rm`；不支持 `ps,touch` |
| `./rtbench test-all --no-realtime --no-workload` | ✅ | 跑完并导出 JSON（本次验收采用） |
| `./rtbench test-realtime` | ❌ 段错误 | 在 `Finish test 1.` 之后崩（加 `-m` 同样崩） |
| `./rtbench test-realtime --delay / --cost / --multi-access / --multi-service` | ⏳ 待测 | 用于定位 realtime 崩点 |
| `./rtbench test-all`（带 workload 段） | ❌ 不可用 | 每负载 10 轮，实测两次都在 MQTT 段长时间刷屏；用 `--no-workload` 规避 |
| `./rtbench -A` | ❌ 只跑 stub | 已知 bug：`-A` 未接到 `run_all_workloads()`；用 `-s` 代替 |

## 问题清单（需转交对应同学）

### 问题 1 ｜ `test-realtime` 段错误 —— 转交 realtime 负责人

- 复现：`./rtbench test-realtime`。加 `-m` 同样段错误。
- 现象：输出到 `Finish test 1.` 之后 `Segmentation fault`。test 1（上下文切换）本身通过。
- 待办：用分节开关定位崩点。
  - `--delay` = test1 + test2（中断延迟）
  - `--cost` = test4 ~ test10
  - `--multi-access` / `--multi-service`
- 证据：控制台截图（见下或另附）。

### 问题 2 ｜ `test-all` 的 workload 段每负载 ×10，过长到不可用 —— 转交 workload 负责人

- 复现：`./rtbench test-all --no-realtime`（含 workload 段）。两次复现，均停在 MQTT 段长时间刷屏。
- 根因：`collect_workload_results()` 对每个负载调用 `exec()` **10 次**（`--quick` 为 5 次）。而 `exec` 就等于该负载的 `xxx_test()`，即**等于 `-s` 的内容跑 10 遍**。
- 量级：MQTT 单轮约 6.7s、MODBUS 单轮约 8s（均为真实 TCP），再叠加 EKF/EPNP/ICP 重计算 → 整段十几到几十分钟。
- 定性：**不是死锁**。每一轮都完整结束（`Benchmark Fininished` → `thread finished`），只是被放大 10 倍。
- 影响：`test-all` 在这块板上带 workload 段不可用。
- 规避：`--no-workload`，workload 用 `./rtbench -s` 单独跑。
- 建议：把默认轮数 **10 → 1**（与 `-s` 对齐），或在 `test-all` 中默认跳过网络类负载。

### 问题 3 ｜ `-A` 未生效 —— 转交 CLI / command 负责人

- 现象：`./rtbench -A` 只运行第 0 个负载（`stub`）。
- 根因：`rtbench_command_main` 中 `-A` 只设置 `run_all_workloads = 1`，但 `run_workload_command()` 未读取该标志。只有 `./rtbench -s`（恰好两个参数）才真正调用 `run_all_workloads()`。
- 建议：把 `-A` 接到 `run_all_workloads()`，使其与 `-s` 等价。

## 实测记录

### `test-all --no-realtime --no-workload`

命令：

```sh
./rtbench test-all --no-realtime --no-workload -o /root/rtbench_result.json
```

结果：完成，控制台打印 `[RTOS-Bench] Results saved to: /root/rtbench_result.json`。
板端另存副本：`/root/rtbench_result_2026_1010_1645_no_realtime_no_workload.json`。

- `test-stress`：`Job 'all' completed: 135 stressor runs`，`Total Run Time: 0m 5s`。
- `test-cmd`：`Note: The Intewell provided does not support command execution 'system()'.`
  人工清单：支持 `date, mkdir, echo, cd, pwd, cp, mv, ls, cat, rm`；不支持 `ps, touch` →
  `Result: 10/12 commands supported`。
- `test-schedule`：跑完（Final Score 从 JSON 中读取，待补）。

## 待补

- `test-realtime` 分节定位结果。
- 导出 JSON 的 `test-schedule` Final Score（把 `rtbench_result_*.json` 附上）。
- 各命令的截图 / 原始日志。
