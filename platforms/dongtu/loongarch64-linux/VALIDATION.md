# 东土龙芯（LoongArch64 / Linux musl）板端验收记录

更新时间：2026-10-10 16:37 （每次新增实测结果时更新这一行）

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
| `./rtbench test-schedule --cycles 3` | ✅ | 跑完 |
| `./rtbench test-realtime` | ❌ 段错误 | 在 `Finish test 1.` 之后崩（加 `-m` 同样崩） |
| `./rtbench test-realtime --delay / --cost / --multi-access / --multi-service` | ⏳ 待测 | 用于定位 realtime 崩点 |
| `./rtbench test-stress --job cpu` | ⏳ 待测 | |
| `./rtbench test-stress --job all` | ⏳ 待测 | 全量 |
| `./rtbench test-all --no-realtime` | ⚠️ 卡在 workload 段 | workload 段每负载跑 10 轮，且含 MODBUS/MQTT 真实网络 |
| `./rtbench -A` | ❌ 只跑 stub | 已知 bug：`-A` 未接到 `run_all_workloads()`；用 `-s` 代替 |

## 已知问题

1. **`test-realtime` 段错误**：在 test 1（上下文切换）之后崩。待用分节开关定位到具体子测试。
2. **`test-all` 的 workload 段过长**：每负载 10 轮 + 真实网络，墙钟时间不可控。规避：`--no-workload` 后单独 `./rtbench -s`。
3. **`-A` 未生效**：只运行第 0 个负载（stub）。`-s` 正常（等价于「全部典型负载各一次」）。

## 待补

- `test-realtime` 分节定位结果。
- `test-stress`（`--job cpu` / `--job all`）结果。
- `test-all --no-realtime --no-workload` 导出的 JSON。
- 各命令的截图 / 原始日志。
