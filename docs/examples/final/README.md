# 最终结果（分层管线）

本目录按三层组织：数据层、算分层、产物层。三层单向依赖，可单独替换。

## 目录

```
final/
├── raw/                    # 同学上传的原始 result.json
├── ssot/                   # 数据层：只有测量值，无分数
│   ├── 7load/
│   └── 9load/
├── baseline/               # 算分基准
│   └── LS2K1000LA_SylixOS.json
├── scored/                 # 算分层：分数 sidecar
│   ├── 7load/
│   └── 9load/
├── template/               # Excel 样式模板
│   └── display_1103.xlsx
├── outputs/                # 产物层
│   ├── 7load/
│   │   ├── scores.csv
│   │   ├── display_7load.xlsx
│   │   └── reports/
│   └── 9load/
├── source/                 # 第一手 Excel 归档
├── SOURCE_DATA.md          # Excel 单元格映射
└── HOW_TO_ADD_DATA.md      # 补数据字段说明
```

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
  --version 9load \
  --ssot   docs/examples/final/ssot/9load \
  --scored docs/examples/final/scored/9load \
  --out    docs/examples/final/outputs/9load
```

输出：

- `scored/9load/*.scores.json`
- `outputs/9load/scores.csv`
- `outputs/9load/reports/*.md`、`*.docx`
- `outputs/9load/display_9load.xlsx`

7 负载把 `9load` 换成 `7load`。

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
- 平均错失率 = 总错失 job / 总 job：7 负载 168，9 负载 216。

## 7 负载与 9 负载

- 调度分：每梯度 21 job 对 27 job。
- 典型负载：7 负载只用 fast、icp、pid、cusum、ewma、modbus、mqtt。
- 当前 SSOT 含 15 块厂商板与 2 块 openEuler 板，不含 RTT。

## 已知缺口

- openEuler 两块板缺 `test-stress.power`，P 与完整总分暂缺，按 partial 权重计算。
- F 全部为占位 60，总分上限 92。
- Excel 模板没有「龙芯派-中移」行，脚本会跳过并提示。
