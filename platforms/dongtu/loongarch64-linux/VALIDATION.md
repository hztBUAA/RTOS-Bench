# 东土龙芯（LoongArch64 / Linux musl）板端验收记录

更新时间：2026-10-10 16:48 （每次新增实测结果时更新这一行）

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
| `./rtbench test-all --no-realtime --no-workload` | ✅ | 跑完并导出 JSON |
| `./rtbench test-realtime` | ❌ 段错误 | 在 `Finish test 1.` 之后崩（加 `-m` 同样崩） |
| `./rtbench test-realtime --delay / --cost / --multi-access / --multi-service` | ⏳ 待测 | 用于定位 realtime 崩点 |
| `./rtbench test-all`（全量） | ⚠️ workload 段过长 | 每负载 10 轮 + 真实网络，墙钟时间不可控；用 `--no-workload` 规避 |
| `./rtbench -A` | ❌ 只跑 stub | 已知 bug：`-A` 未接到 `run_all_workloads()`；用 `-s` 代替 |

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

## 已知问题

1. **`test-realtime` 段错误**：在 test 1（上下文切换）之后崩。待用分节开关定位到具体子测试。
2. **`test-all` 的 workload 段过长**：每负载 10 轮 + 真实网络。规避：`--no-workload` 后单独 `./rtbench -s`。
3. **`-A` 未生效**：只运行第 0 个负载（stub）。`-s` 正常（等价于「全部典型负载各一次」）。
4. **`test-cmd` 走人工清单**：该 Intewell 未提供 `system()`，命令支持测试按人工结果计（10/12）。

## 待补

- `test-realtime` 分节定位结果。
- 导出 JSON 的 `test-schedule` Final Score（把 `rtbench_result_*.json` 附上）。
- 各命令的截图 / 原始日志。
