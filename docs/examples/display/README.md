# 基准测试数据展示统计

本目录生成《基准测试数据-展示统计》工作簿。

## 文件

| 文件 | 说明 |
|---|---|
| `template_1103.xlsx` | 1103 版模板，样式基准 |
| `基准测试数据-展示统计.xlsx` | 生成结果 |
| `../../../utils/build_display_workbook.py` | 生成脚本 |

## 生成方式

以模板为样式来源，只更新「基准测试结果」页的数据单元格，样式、合并单元格、列宽、行高全部保持不变。

```bash
python3 utils/build_display_workbook.py \
  --template docs/examples/display/template_1103.xlsx \
  --ssot docs/examples/final/ssot/9load \
  -o docs/examples/display/基准测试数据-展示统计.xlsx
```

数据来源为该板卡的 SSOT JSON：

- 典型负载：`modules.typical-workload.workloads[].avg_time_ms`
- 上下文切换、中断：`modules.test-realtime.single_core`
- 平均错失率：`modules.test-schedule.summary.average_miss_rate`
- 功耗：`power.items[].energy_j`

## 校验

生成后逐格比对模板：

- 样式差异 0
- 合并单元格一致
- 维度一致（A1:Y31、A1:BJ98）

数值上与模板的差异来自数据更新，例如锐华两行旧值为占位值，已替换为实测值。平均错失率按模板习惯保留 3 位小数。

模板中没有「龙芯派-中移」这一行，脚本会跳过并在日志中提示，不会自行增行，以免破坏版式。

## 说明

「单核和多核实时性能结果」页当前保留模板内容，样式与结构不变。该页的逐项明细填充需要单独的行列映射，尚未纳入本脚本。
