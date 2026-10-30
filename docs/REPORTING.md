# 评分与报告生成

本文说明 RTOS-Bench 的出分口径、SSOT JSON 逆生成、自然语言报告导出与元信息补全。

更新时间：2026-10-29

## 1. 评分口径

总分：

```
总分 = 0.2×F + 0.3×RT + 0.4×T + 0.1×P
```

| 分项 | 计算 | 说明 |
|---|---|---|
| F | 60（占位） | 基础功能，满分 100。当前无实测数据，由翼辉侧工具提供 |
| RT | 0.5×基础实时 + 0.5×调度 | 调度分 = 100×(1−平均截止时间错失率) |
| T | 4 类负载得分的均值 | 实时控制、感知、通信、运维 |
| P | 4 项功耗得分的均值 | 静态、CPU、内存、文件 |

因为 F=60，总分上限为 92。

### min-max 归一化

- 子项得分 = (R − Rmin) / (Rmax − Rmin) × 100，裁剪到 [0,100]。
- R：延迟类 = 基准/实测，带宽类 = 实测/基准。
- Rmin/Rmax 取同一板卡族、所有已测系统的极值。
- 板卡族：工控机、香橙派、哪吒、龙芯、飞腾。

### 60 分保底

只对经过 min-max 的分项做映射：

```
新分 = 60 + 0.4 × 原分
```

- 作用对象：基础实时、T 的四类、P。
- 不作用：F（占位 60）、调度分（不参与 min-max）。
- 效果：min-max 分项落在 60–100。

### 平均错失率

```
平均错失率 = 总错失 job / 总 job
```

- 9 负载：8 个梯度 × 27 job = 216。
- 7 负载：8 个梯度 × 21 job = 168。
- 不是各梯度错失率的平均值。

## 2. 板卡命名

与压力/功耗测试配置一致：

| 中文 | 配置名 |
|---|---|
| 工控机 | MIC-7700 |
| 香橙派 | Orange-Pi5 |
| 哪吒 | DH-1 |
| 龙芯 | LS2K1000LA |
| 飞腾派 | Phytium-Pi |

OS 名称：SylixOS、Intewell、ReWorks、OneOS、RT-Thread、openEuler。

OS 与 BSP 版本在 `utils/board_meta.json` 维护。ReWorks 已填 V6.1.1 与三块板的 BSP 版本，其他厂商留空待补。

## 3. 流程

```
结果展示表 Excel
  → 逆生成 SSOT JSON        utils/excel_to_result_json.py
  → 生成描述                utils/build_reports.py
  → 转算分脚本 schema       utils/ssot_to_score_json.py
  → score_caculate.py 出分  utils/score_caculate.py
  → 60 保底与汇总           utils/score_seven_workload.py
```

说明：

- `score_caculate.py` 只用 `test-stress.power` 作为功耗来源。
- 多核指标由 `multi_core` 结构提供，带宽用实测/基准，任务延迟用基准/实测。
- 哪吒按单核处理，排除多核并重新归一化权重。

## 4. 命令

```bash
# 1. SSOT JSON + 自然语言描述（markdown + docx）
python3 utils/build_reports.py <结果展示表.xlsx> --workloads 9 -o reports/9load

# 2. 可选：用外部算分结果覆盖描述中的分数
python3 utils/build_reports.py <结果展示表.xlsx> --workloads 9 \
    -o reports/9load --scores-json scores.json

# 3. SSOT 转算分脚本 schema
python3 utils/ssot_to_score_json.py reports/9load/ssot -o reports/9load/score_json

# 4. 算分
python3 utils/score_caculate.py --db results.db import-json "reports/9load/score_json/*.json"
python3 utils/score_caculate.py --db results.db list
python3 utils/score_caculate.py --db results.db export \
    --board Phytium-Pi --os OneOS --arch aarch64

# 5. 7/9 负载总分与 60 保底
python3 utils/score_seven_workload.py <结果展示表.xlsx> --workloads 9 -o 9load_score.xlsx

# 6. 元信息补全（网页端无法注入时）
python3 utils/patch_result_meta.py result.json \
    --os-name "RT-Thread" --os-version "5.3.1" \
    --board "Phytium-Pi" --bsp-version "<bsp>"
```

## 5. 已知缺口

- F 全部为占位 60，总分上限 92。
- 欧拉两块板缺功耗，RTT 缺功耗，暂不出总分。
- 板卡名与 OS 版本需要与北理工、翼辉最终对齐。
- SSOT JSON 由 Excel 逆生成，不含逐任务统计与 stress/cmd 明细。

## 6. 示例输出

以翼辉工控机为例（9 负载，60 保底）：

- [docs/examples/翼辉工控机_result.json](examples/翼辉工控机_result.json)：SSOT JSON
- [docs/examples/翼辉工控机_测试结果描述.md](examples/翼辉工控机_测试结果描述.md)：markdown
- [docs/examples/翼辉工控机_测试结果描述.docx](examples/翼辉工控机_测试结果描述.docx)：Word
- [docs/examples/final/](examples/final/)：7/9 负载最终得分 CSV 与 SSOT JSON

Word 结构：一级标题（测试结果描述 / 测试评分描述）、二级标题（四个分项）、三级标题（可调度性能），条目用 1) 2) 3) 4) 编号。

示例中的分数为 60 保底后的结果，输入 JSON 由结果展示表逆生成。
