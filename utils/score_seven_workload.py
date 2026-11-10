#!/usr/bin/env python3
"""Seven-workload Excel adapter for score_caculate.py.

All scores use the fixed-baseline saturation policy; Excel score cells are ignored.
A workbook containing SylixOS + LS2K1000LA or --baseline JSON supplies the reference.
"""

import argparse
import re
import sys

import openpyxl

from copy import deepcopy
from pathlib import Path
import importlib.util
import json


def _load(name, filename):
    spec = importlib.util.spec_from_file_location(name, Path(__file__).with_name(filename))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


scorer = _load("seven_scorer", "score_caculate.py")
excel2json = _load("seven_excel", "excel_to_result_json.py")
schema = _load("seven_schema", "ssot_to_score_json.py")
WORKLOAD_SETS = {7: list(scorer.WORKLOAD_CATEGORY_MAP)}
CATEGORY_SETS = {7: {
    cat: [name for name, category in scorer.WORKLOAD_CATEGORY_MAP.items() if category == cat]
    for cat in scorer.WORKLOAD_CATEGORY_WEIGHTS
}}


def require_seven(workload_count):
    if workload_count != 7:
        raise ValueError("Only the seven-workload scoring policy is supported")


def parse_blocks(path):
    records = []
    for board in excel2json.parse_excel(path, 7):
        raw = schema.convert(excel2json.to_result_json(deepcopy(board), str(path), 7))
        records.append({"vendor": board["vendor"], "name": board["board"],
                        "family": raw["env"]["board"], "score_input": raw})
    return records


def compute(records, workload_count=7, baseline_path=None):
    """Use score_caculate for every score; never reuse Excel score cells."""
    require_seven(workload_count)
    conn = scorer.init_db(":memory:")
    try:
        if baseline_path:
            data = json.loads(Path(baseline_path).read_text(encoding="utf-8-sig"))
            rt = data.get("modules", {}).get("test-realtime", {})
            if "multicore" in rt or not data.get("env", {}).get("cpu_type"):
                data = schema.convert(data)
            if (data["env"].get("board"), data["env"].get("os_name")) != (
                    scorer.CONFIG["baseline_board"], scorer.CONFIG["baseline_os"]):
                raise ValueError("Baseline must be SylixOS + LS2K1000LA")
            error = scorer.validate_json_data(data)
            if error:
                raise ValueError(error)
            scorer._parse_json_data(data, conn, str(baseline_path))
        ordered = sorted(records, key=lambda rec: (
            rec["score_input"]["env"]["board"] != scorer.CONFIG["baseline_board"]
            or rec["score_input"]["env"]["os_name"] != scorer.CONFIG["baseline_os"]))
        for rec in ordered:
            error = scorer.validate_json_data(rec["score_input"])
            if error:
                raise ValueError(error)
            scorer._parse_json_data(rec["score_input"], conn, rec["name"])
        conn.commit()
        scorer.recompute_all_scores(conn)
        rows = []
        for rec in records:
            env = rec["score_input"]["env"]
            result = scorer.export_results(env["board"], env["os_name"], env["cpu_type"], conn)
            scores = result["scores"]
            rt = scores["realtime"]
            rows.append({**rec, "base_rt_f": rt["details"]["base_realtime"]["score"],
                         "sched": rt["details"]["schedulability"]["score"],
                         "RT": rt["score"], "T": scores["workload"]["score"],
                         "P": scores["power"]["score"], "F": scores["function"]["score"],
                         "total": result["total_score"],
                         "categories": {cat: detail["score"] for cat, detail in scores["workload"]["details"].items()},
                         "scoring_rule": result["scoring_rule"], "score_export": result})
        return rows
    finally:
        conn.close()


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
    parser.add_argument("--workloads", type=int, choices=[7], default=7)
    parser.add_argument("--baseline", help="SylixOS + LS2K1000LA reference JSON")
    args = parser.parse_args()

    records = parse_blocks(args.excel)
    if not records:
        sys.exit("未找到调度性能数据块，请检查 Excel 格式")
    rows = compute(records, args.workloads, args.baseline)

    print(f"{'厂商':<8}{'板卡':<24}{'RT':>8}{'T':>8}{'P':>8}{'总分':>8}")
    for rec in rows:
        total = f"{rec['total']:.2f}" if rec["total"] is not None else "-"
        values = ["-" if rec[k] is None else f"{rec[k]:.2f}" for k in ("RT", "T", "P")]
        print(f"{rec['vendor']:<8}{rec['name'][:22]:<24}" + "".join(f"{v:>8}" for v in values) + f"{total:>8}")

    if args.output:
        write_xlsx(rows, args.output, args.workloads)
        print(f"\n已写出: {args.output}")


if __name__ == "__main__":
    main()
