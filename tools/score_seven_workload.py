#!/usr/bin/env python3
"""七负载版本总分计算（与 docs/SCORE-CALCULATE.md 口径一致）。

输入：结果展示表 Excel（每 20 行一个板卡块）。
输出：每个板卡的总分明细（CSV / XLSX）。

口径：
  RT = 0.5 * 基础实时 + 0.5 * 调度得分
  T  = 0.25 * 实时控制 + 0.25 * 感知 + 0.25 * 通信 + 0.25 * 运维
      子项 R = 基准延迟 / 实测延迟，同板卡族跨系统 min-max 归一化
  P  = 4 项功耗得分的均值
  F  = 60（占位）
  Total = 0.2*F + 0.3*RT + 0.4*T + 0.1*P

用法：
  python3 utils/score_seven_workload.py <结果展示表.xlsx> [-o out.xlsx]
"""

import argparse
import re
import sys

import openpyxl

GENERIC = {"分类", "基", "性能", "功", "耗", "评", "典型", "负载", "实时"}
BOARD_FAMILIES = ["工控机", "香橙派", "哪吒", "龙芯", "飞腾"]
WORKLOAD_SETS = {
    7: ["pid", "fast", "icp", "modbus", "mqtt", "ewma", "cusum"],
    9: ["pid", "ekf", "fast", "icp", "epnp", "modbus", "mqtt", "ewma", "cusum"],
}
CATEGORY_SETS = {
    7: {
        "realtime_control": ["pid"],
        "perception": ["fast", "icp"],
        "communication": ["modbus", "mqtt"],
        "monitoring": ["ewma", "cusum"],
    },
    9: {
        "realtime_control": ["pid", "ekf"],
        "perception": ["fast", "icp", "epnp"],
        "communication": ["modbus", "mqtt"],
        "monitoring": ["ewma", "cusum"],
    },
}
F_SCORE = 60.0
FLOOR_BASE = 60.0
FLOOR_RATIO = 0.4


def _floor(score):
    """仅用于 min-max 分项：保底 60，压缩到 60-100。"""
    return None if score is None else FLOOR_BASE + FLOOR_RATIO * score


def _board_name(ws, start_row):
    for col in (7, 8, 1):
        value = ws.cell(start_row, col).value
        if not value:
            continue
        text = str(value).strip()
        if not text or text.startswith("表") or text in GENERIC:
            continue
        return text
    return ""


def _board_family(name):
    for family in BOARD_FAMILIES:
        if family in name:
            return family
    return name


def _parse_ms(text):
    out = {}
    if not text:
        return out
    for m in re.finditer(r"([A-Za-z][A-Za-z0-9]*)\s+([0-9.]+)\s*ms", str(text)):
        out[m.group(1).lower()] = float(m.group(2))
    return out


def _mean(values):
    values = [v for v in values if v is not None]
    return sum(values) / len(values) if values else None


def parse_blocks(path):
    wb = openpyxl.load_workbook(path, data_only=True)
    records = []
    for sheet in wb.sheetnames:
        ws = wb[sheet]
        for row in range(1, ws.max_row + 1):
            if str(ws.cell(row, 2).value or "").strip() != "调度性能":
                continue
            start = row - 14
            name = _board_name(ws, start)
            measured, baseline = {}, {}
            for r in range(start + 9, start + 13):
                measured.update(_parse_ms(ws.cell(r, 6).value))
                baseline.update(_parse_ms(ws.cell(r, 7).value))
            records.append({
                "sheet": sheet,
                "vendor": sheet.split("-")[0].strip(),
                "name": name,
                "family": _board_family(name),
                "measured": measured,
                "baseline": baseline,
                "sched": ws.cell(row, 5).value,
                "base_rt": ws.cell(start + 13, 5).value,
                "power": [ws.cell(start + 15 + i, 5).value for i in range(4)],
            })
    return records


def compute(records, workload_count=7):
    workloads = WORKLOAD_SETS[workload_count]
    categories = CATEGORY_SETS[workload_count]
    baseline = {}
    for rec in records:
        for key, value in rec["baseline"].items():
            baseline.setdefault(key, value)

    for rec in records:
        rec["R"] = {
            w: (baseline[w] / rec["measured"][w]
                if w in rec["measured"] and rec["measured"][w] else None)
            for w in workloads
        }

    item_score = {}
    for family in {rec["family"] for rec in records}:
        group = [rec for rec in records if rec["family"] == family]
        for workload in workloads:
            values = [rec["R"][workload] for rec in group if rec["R"][workload] is not None]
            if len(values) < 2 or max(values) == min(values):
                continue
            lo, hi = min(values), max(values)
            for rec in group:
                r = rec["R"][workload]
                if r is not None:
                    item_score[(id(rec), workload)] = max(0.0, min(100.0, (r - lo) / (hi - lo) * 100.0))

    rows = []
    for rec in records:
        cats = {
            cat: _mean([item_score.get((id(rec), w)) for w in members])
            for cat, members in categories.items()
        }
        # 只对 min-max 分项做保底变换。调度分=100*(1-MR)，不参与。
        cats_f = {k: _floor(v) for k, v in cats.items()}
        t_score = _mean(list(cats_f.values()))
        base_rt_f = _floor(rec["base_rt"])
        rt_score = _mean([base_rt_f, rec["sched"]]) if base_rt_f is not None and rec["sched"] is not None else None
        p_score = _floor(_mean([float(v) for v in rec["power"] if v is not None]))
        total = None
        if None not in (rt_score, t_score, p_score):
            total = 0.2 * F_SCORE + 0.3 * rt_score + 0.4 * t_score + 0.1 * p_score
        rows.append({**rec, "categories": cats_f, "base_rt_f": base_rt_f, "T": t_score, "RT": rt_score,
                     "P": p_score, "F": F_SCORE, "total": total})
    return rows


def write_xlsx(rows, path, workload_count=7):
    wb = openpyxl.Workbook()
    ws = wb.active
    ws.title = f"{workload_count}负载总分"
    headers = ["厂商", "板卡", "板卡族", "基础实时", "调度", "RT得分",
               "实时控制", "感知", "通信", "运维", "T得分", "P得分", "F得分", "总分"]
    for col, header in enumerate(headers, 1):
        ws.cell(1, col, header)
    for r, rec in enumerate(rows, 2):
        cats = rec["categories"]
        values = [rec["vendor"], rec["name"], rec["family"], rec.get("base_rt_f"), rec["sched"], rec["RT"],
                  cats["realtime_control"], cats["perception"], cats["communication"], cats["monitoring"],
                  rec["T"], rec["P"], rec["F"], rec["total"]]
        for c, value in enumerate(values, 1):
            cell = ws.cell(r, c, value)
            if isinstance(value, float):
                cell.number_format = "0.00"
    for i in range(1, len(headers) + 1):
        ws.column_dimensions[openpyxl.utils.get_column_letter(i)].width = 12 if i > 2 else 18
    wb.save(path)


def main():
    parser = argparse.ArgumentParser(description="七负载版本总分计算")
    parser.add_argument("excel")
    parser.add_argument("-o", "--output", default=None)
    parser.add_argument("--workloads", type=int, choices=[7, 9], default=7)
    args = parser.parse_args()

    records = parse_blocks(args.excel)
    if not records:
        sys.exit("未找到调度性能数据块，请检查 Excel 格式")
    rows = compute(records, args.workloads)

    print(f"{'厂商':<8}{'板卡':<24}{'RT':>8}{'T':>8}{'P':>8}{'总分':>8}")
    for rec in rows:
        total = f"{rec['total']:.2f}" if rec["total"] is not None else "-"
        print(f"{rec['vendor']:<8}{rec['name'][:22]:<24}{rec['RT']:>8.2f}{rec['T']:>8.2f}{rec['P']:>8.2f}{total:>8}")

    if args.output:
        write_xlsx(rows, args.output, args.workloads)
        print(f"\n已写出: {args.output}")


if __name__ == "__main__":
    main()
