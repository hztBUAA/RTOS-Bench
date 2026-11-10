# 最终结果与 SSOT（2026-11-03）

本目录保留历史 7/9 负载结果与 SSOT JSON。当前 deploy 分支仅支持七负载评分，历史文件不自动更新，旧分数不得直接作为当前报告分数。

## 文件

| 文件 | 说明 |
|---|---|
| `ssot/7load/*.json` | 7 负载 SSOT JSON，含最终分数（含 openEuler 飞腾、香橙派） |
| `ssot/9load/*.json` | 9 负载 SSOT JSON，含最终分数（含 openEuler 飞腾、香橙派） |
| `SOURCE_DATA.md` | 第一手 Excel、单元格映射、更新注意事项 |
| `HOW_TO_ADD_DATA.md` | 补数据的 JSON 字段说明与算分命令 |
| `source/*.xlsx` | 第一手 Excel 源文件 |

## 口径

```
总分 = 0.2×F + 0.3×RT + 0.4×T + 0.1×P
RT   = 0.5×基础实时 + 0.5×调度
T    = 四类负载均值
P    = 四项功耗均值
F    = 60（占位）
```

- 子项使用绝对饱和曲线：`S = 100×R/(a+R)`，`a = 0.25`，基准固定为 SylixOS @ LS2K1000LA。
- 不再使用 min-max，也不再使用 60 分保底。
- 调度分 = 100×(1−平均错失率)，不参与饱和曲线。
- 平均错失率 = 总错失 job / 总 job。7 负载为 168，9 负载为 216。

## 7 负载与 9 负载的差异

- 调度分：7 负载每梯度 21 job，9 负载 27 job，分数不同。
- 典型负载：7 负载只用 fast、icp、pid、cusum、ewma、modbus、mqtt。
- 飞腾族：7 负载含 RTT-飞腾派，9 负载不含。

## 已知缺口

- 欧拉两块板已补原始 JSON，并按新算法生成 7/9 负载 SSOT。
- RTT 的新多核数据含极端值（`ipc c8=0.002`、`task_latency c2=541.9`、`inter_core=0`），
  会改变 RTT 自身的得分；如需剔除，请先确认测量有效性。
- F 全部为占位 60，总分上限 92。
- 欧拉两块板缺 `test-stress.power` 数据，P 与完整总分暂缺。

## 复现

```bash
python3 utils/build_reports.py <结果展示表.xlsx> --workloads 7 -o reports/7load
python3 utils/ssot_to_score_json.py reports/7load/ssot -o reports/7load/score_json
python3 utils/score_calculate.py --db results.db import-json "reports/7load/score_json/*.json"
python3 utils/score_calculate.py --db results.db export --board Phytium-Pi --os RT-Thread --arch aarch64
python3 utils/score_calculate.py score-excel <结果展示表.xlsx> --workloads 7 -o 7load_score.xlsx
```

## 更新流程（给同学与 AI）

1. **放原始数据**：新板卡的 `result.json` 放到 `reports/raw/<板卡>/`；历史数据的结果展示表放到 `reports/excel/`。
2. **生成 SSOT**：`python3 utils/build_reports.py <excel> --workloads 7 -o reports/<ver>`
3. **转算分 JSON**：`python3 utils/ssot_to_score_json.py reports/<ver>/ssot -o reports/<ver>/score_json`
4. **算分**：`python3 utils/score_calculate.py --db results.db import-json "reports/<ver>/score_json/*.json"`，再 `export`。
5. **饱和曲线汇总**：`python3 utils/score_calculate.py score-excel <excel> --workloads 7 -o <out>.xlsx`
6. **更新本目录**：更新七负载结果；九负载文件仅作历史归档。
7. **提交 PR**：写明数据来源、板卡名、OS/BSP 版本、改动原因。

### 约定

- 板卡名用 `MIC-7700`、`Orange-Pi5`、`DH-1`、`LS2K1000LA`、`Phytium-Pi`。
- 子项使用饱和曲线 `S=100R/(a+R)`，a=0.25，基准固定。
- 当前只评分七项任务；PID 属于实时控制，FAST/ICP 属于感知，MODBUS/MQTT 属于通信，CUSUM/EWMA 属于运维。EKF 历史归实时控制，EPnP 归感知，当前均不评分。
- 保留原始数据，SQLite 自动清除旧版本缓存并重算。单系统有基准即可评分；无基准写 null。
- 数据缺失写 `null`，不要填 0。
- 不再使用 min-max 与 60 保底。

### 给 AI 的提示

本目录自包含：CSV 是最终结果，`ssot/` 是输入。让 AI 基于 `ssot/` 重算时，先读 `docs/REPORTING.md` 的口径，再执行上面的命令。任何改动都通过 PR 留痕。
