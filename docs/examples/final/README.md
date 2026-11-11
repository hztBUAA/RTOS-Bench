# 最终结果（分层管线）

本目录按三层组织：数据层、算分层、产物层。三层单向依赖，可单独替换。

## 目录

```
final/
├── raw/                    # 同学上传的原始 result.json
├── ssot/                   # 数据层：只有测量值，无分数
│   └── 7load/
├── baseline/               # 算分基准
│   └── LS2K1000LA_SylixOS.json
├── scored/                 # 算分层：分数 sidecar
│   └── 7load/
├── template/               # Excel 样式模板
│   └── display_1103.xlsx
├── outputs/                # 产物层
│   └── 7load/
│       ├── scores.csv
│       ├── display_7load.xlsx
│       └── reports/
├── source/                 # 第一手 Excel 归档
├── SOURCE_DATA.md          # Excel 单元格映射
└── HOW_TO_ADD_DATA.md      # 补数据字段说明
```

## 数据来源

| 来源 | meta.source | 说明 |
|---|---|---|
| 板卡原生输出 | `rtbench_raw` | 字段完整，含 task_stats、test-stress、test-cmd |
| Excel 逆推 | `excel_reverse` | 只有汇总字段，缺 task_stats 等 |

当前 15 块厂商板为 `excel_reverse`，openEuler 两块为 `rtbench_raw`。算分够用，但不是忠实的原始记录。厂商补原生 `result.json` 后可替换。

## 三层关系

```
SSOT（测量值） ──读──▶ 算分脚本 ──写──▶ scored（分数）
                            │
                    outputs（报告 + Excel）读 SSOT 和 scored
```

- `ssot/` 不含 `scores`，只存测量值。
- `scored/` 每块板一个 `.scores.json`，含 `board/os/arch/source/algorithm/scores`。
- 算分脚本不写 SSOT。

## 一条命令

```bash
python3 tools/pipeline.py \
  --version 7load \
  --ssot   docs/examples/final/ssot/7load \
  --scored docs/examples/final/scored/7load \
  --out    docs/examples/final/outputs/7load
```

输出：

- `scored/7load/*.scores.json`
- `outputs/7load/scores.csv`
- `outputs/7load/reports/*.md`、`*.docx`
- `outputs/7load/display_7load.xlsx`

7 负载把 `7load` 换成 `7load`。

## 更新流程（给同学）

1. 改 `ssot/<version>/<板卡>.json` 中 `modules` 与 `power` 的测量值。
2. 在分支上运行 `tools/pipeline.py`，把 `scored/` 与 `outputs/` 的更新一并提交。
3. 提 PR 到 main，标题写「data: 更新 <板卡> 结果」。
4. 全量重跑会更新所有板的报告与 Excel，属于正常；并行 PR 的冲突在合并时解决。
5. CI 会重算并比对 `scored/` 与 `scores.csv`，不一致则失败。

## 口径

```
总分 = 0.2×F + 0.3×RT + 0.4×T + 0.1×P
RT   = 0.5×基础实时 + 0.5×调度
T    = 四类负载均值
P    = 四项功耗均值
F    = 60（占位）
```

- 子项用绝对饱和曲线：`S = 100×R/(a+R)`，`a = 0.25`，基准固定 SylixOS @ LS2K1000LA。
- 不再使用 min-max，也不再使用 60 分保底。
- 调度分 = 100×(1−平均错失率)，不参与曲线。
- 平均错失率 = 总错失 job / 总 job：7 负载 168。

## 已知缺口

- openEuler 两块板缺 `test-stress.power`，P 与完整总分暂缺，按 partial 权重计算。
- F 全部为占位 60，总分上限 92。
- Excel 模板没有「龙芯派-中移」行，脚本会跳过并提示。
