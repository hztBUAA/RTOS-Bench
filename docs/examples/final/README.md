# 最终结果与 SSOT（2026-10-30）

本目录存放 7 负载与 9 负载的最终得分，以及对应 SSOT JSON。

## 文件

| 文件 | 说明 |
|---|---|
| `7load_scores.csv` | 7 负载最终得分（15 块板 + RTT + 欧拉） |
| `9load_scores.csv` | 9 负载最终得分（15 块板） |
| `ssot/7load/*.json` | 7 负载 SSOT JSON，含最终分数 |
| `ssot/9load/*.json` | 9 负载 SSOT JSON，含最终分数 |

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
