# 最终结果与 SSOT（2026-10-30）

本目录存放 7 负载与 9 负载的最终得分，以及对应 SSOT JSON。

## 文件

| 文件 | 说明 |
|---|---|
| `7load_scores.csv` | 7 负载最终得分（15 块板 + RTT + 欧拉） |
| `9load_scores.csv` | 9 负载最终得分（15 块板） |
| `ssot/7load/*.json` | 7 负载 SSOT JSON，含最终分数 |
| `ssot/9load/*.json` | 9 负载 SSOT JSON，含最终分数 |
| `SOURCE_DATA.md` | 第一手 Excel、单元格映射、更新注意事项 |
| `source/*.xlsx` | 第一手 Excel 源文件 |

## 口径

```
总分 = 0.2×F + 0.3×RT + 0.4×T + 0.1×P
RT   = 0.5×基础实时 + 0.5×调度
T    = 四类负载均值
P    = 四项功耗均值
F    = 60（占位）
```

- 只对 min-max 分项做 `60 + 0.4×原分`：基础实时、T 的四类、P。
- 调度分 = 100×(1−平均错失率)，不做保底。
- 基础实时使用 `score_caculate.py`（含博函 Base_RT 修复）。
- 平均错失率 = 总错失 job / 总 job。7 负载为 168，9 负载为 216。

## 7 负载与 9 负载的差异

- 调度分：7 负载每梯度 21 job，9 负载 27 job，分数不同。
- 典型负载：7 负载只用 fast、icp、pid、cusum、ewma、modbus、mqtt。
- 飞腾族：7 负载含 RTT-飞腾派，9 负载不含。

## 已知缺口

- 欧拉两块板无原始明细，基础实时与 T 沿用云端汇总值，仅作参考。
- 欧拉的功耗分由云端 P 经 60 保底换算，待补原始数据。
- RTT 的新多核数据含极端值（`ipc c8=0.002`、`task_latency c2=541.9`、`inter_core=0`），
  会改变飞腾族 min-max；如需剔除，请先确认测量有效性。
- F 全部为占位 60，总分上限 92。

## 复现

```bash
python3 utils/build_reports.py <结果展示表.xlsx> --workloads 7 -o reports/7load
python3 utils/ssot_to_score_json.py reports/7load/ssot -o reports/7load/score_json
python3 utils/score_caculate.py --db results.db import-json "reports/7load/score_json/*.json"
python3 utils/score_caculate.py --db results.db export --board Phytium-Pi --os RT-Thread --arch aarch64
python3 utils/score_seven_workload.py <结果展示表.xlsx> --workloads 7 -o 7load_score.xlsx
```

## 更新流程（给同学与 AI）

1. **放原始数据**：新板卡的 `result.json` 放到 `reports/raw/<板卡>/`；历史数据的结果展示表放到 `reports/excel/`。
2. **生成 SSOT**：`python3 utils/build_reports.py <excel> --workloads 7|9 -o reports/<ver>`
3. **转算分 JSON**：`python3 utils/ssot_to_score_json.py reports/<ver>/ssot -o reports/<ver>/score_json`
4. **算分**：`python3 utils/score_caculate.py --db results.db import-json "reports/<ver>/score_json/*.json"`，再 `export`。
5. **保底与汇总**：`python3 utils/score_seven_workload.py <excel> --workloads 7|9 -o <out>.xlsx`
6. **更新本目录**：替换 `7load_scores.csv` / `9load_scores.csv` 与 `ssot/` 下的 JSON。
7. **提交 PR**：写明数据来源、板卡名、OS/BSP 版本、改动原因。

### 约定

- 板卡名用 `MIC-7700`、`Orange-Pi5`、`DH-1`、`LS2K1000LA`、`Phytium-Pi`。
- 只对 min-max 分项做 `60 + 0.4×原分`，调度分与 F 不参与。
- 7 负载与 9 负载分开计算，不能共用调度分。
- 数据缺失写 `null`，不要填 0。

### 给 AI 的提示

本目录自包含：CSV 是最终结果，`ssot/` 是输入。让 AI 基于 `ssot/` 重算时，先读 `docs/REPORTING.md` 的口径，再执行上面的命令。任何改动都通过 PR 留痕。
