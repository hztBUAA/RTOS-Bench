# 评分与报告生成

本文说明 RTOS-Bench 的出分口径、SSOT JSON 逆生成、自然语言报告导出与元信息补全。

更新时间：2026-11-10

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

### 绝对饱和曲线

子项得分：

```
S = 100 × R / (a + R)，a = 0.25
```

- R：延迟类 = 基准/实测，带宽类 = 实测/基准。
- 基准固定为 SylixOS @ LS2K1000LA，不随同板卡其他系统变化。
- 同一 R 始终得到同一分数，R 越大得分越高，不会出现 0 分。

**当前全部评分入口统一使用 score_calculate.py，仅评分 PID、FAST、ICP、MODBUS、MQTT、CUSUM、EWMA。** 不再使用 min-max 或 60 分保底；四类各占 25%。EKF 历史分类为实时控制，EPnP 为感知计算，两项当前不评分。

固定基准可来自同一工作簿中的 SylixOS 龙芯，或 `--baseline` 指定的 SylixOS + LS2K1000LA JSON。缺少基准则相关得分为 null。原始测量记录保留；SQLite 无规则版本或旧规则版本的缓存分数自动清除重算。缺少类内任务也会标记 incomplete。

### 平均错失率

```
平均错失率 = 总错失 job / 总 job
```

- 当前只评分七项负载；历史九负载结果仅作归档。
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
  → score_calculate.py 统一重算后生成描述 utils/build_reports.py
  → 转算分脚本 schema       utils/ssot_to_score_json.py
  → score_calculate.py 出分（饱和曲线）  utils/score_calculate.py
  → 汇总与导出                            utils/score_calculate.py export
```

说明：

- `score_calculate.py` 只用 `test-stress.power` 作为功耗来源。
- 多核指标由 `multi_core` 结构提供，带宽用实测/基准，任务延迟用基准/实测。
- 哪吒按单核处理，排除多核并重新归一化权重。

## 4. 命令

```bash
# 1. SSOT JSON + 自然语言描述（markdown + docx）
python3 utils/build_reports.py <结果展示表.xlsx> --workloads 7 -o reports/7load

# 2. 若 Excel 不含 SylixOS 龙芯基准，可提供基准 JSON（单系统同样可算分）
python3 utils/build_reports.py <结果展示表.xlsx> --workloads 7 \
    -o reports/7load --baseline baseline.json

# 3. SSOT 转算分脚本 schema
python3 utils/ssot_to_score_json.py reports/7load/ssot -o reports/7load/score_json

# 4. 算分
python3 utils/score_calculate.py --db results.db import-json "reports/7load/score_json/*.json"
python3 utils/score_calculate.py --db results.db list
python3 utils/score_calculate.py --db results.db export \
    --board Phytium-Pi --os OneOS --arch aarch64

# 5. 将当前数据库的评分结果导出为 JSON
python3 utils/score_calculate.py --db results.db export \
    --board Phytium-Pi --os OneOS --arch aarch64 --output score.json

# 6. 元信息补全（网页端无法注入时）
python3 utils/patch_result_meta.py result.json \
    --os-name "RT-Thread" --os-version "5.3.1" \
    --board "Phytium-Pi" --bsp-version "<bsp>"
```

## 5. 已知缺口

- F 全部为占位 60，总分上限 92。
- 缺失功耗等维度时，按 score_calculate.py 的 partial 规则对已有维度重分配权重估算总分；不代表完整验收结果。
- 板卡名与 OS 版本需要与北理工、翼辉最终对齐。
- SSOT JSON 由 Excel 逆生成，不含逐任务统计与 stress/cmd 明细。

## 6. 示例输出

以下文件属于历史九负载示例，仅用于查看原始数据和报告结构；当前七负载分数须按上文命令重算：

- [docs/examples/翼辉工控机_result.json](examples/翼辉工控机_result.json)：SSOT JSON
- [docs/examples/翼辉工控机_测试结果描述.md](examples/翼辉工控机_测试结果描述.md)：markdown
- [docs/examples/翼辉工控机_测试结果描述.docx](examples/翼辉工控机_测试结果描述.docx)：Word
- [docs/examples/final/](examples/final/)：7/9 负载最终得分 CSV 与 SSOT JSON

Word 结构：一级标题（测试结果描述 / 测试评分描述）、二级标题（四个分项）、三级标题（可调度性能），条目用 1) 2) 3) 4) 编号。

历史示例分数不作为当前算分输入。生成报告时忽略 Excel 的旧分数单元格，使用原始测量值与固定基准重新计算。
