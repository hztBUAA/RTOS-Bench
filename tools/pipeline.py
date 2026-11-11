#!/usr/bin/env python3
"""一条命令：SSOT → 算分 → 分数层 → 报告 + Excel。

用法：
  python3 tools/pipeline.py --version 9load \
      --ssot docs/examples/final/ssot/9load \
      --scored docs/examples/final/scored/9load \
      --out docs/examples/final/outputs/9load
"""

import argparse
import copy
import csv
import glob
import importlib.util
import json
import os
import shutil
import sys
import tempfile

import openpyxl

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)


def load(name, filename):
    spec = importlib.util.spec_from_file_location(name, os.path.join(HERE, filename))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


conv = load("conv", "ssot_to_score_json.py")
scorer = load("scorer", "score_caculate.py")
reports = load("reports", "build_reports.py")
display = load("display", "build_display_workbook.py")


def flat_scores(result):
    s = result["scores"]
    return {
        "base_rt": s["realtime"]["details"]["base_realtime"]["score"],
        "sched": s["realtime"]["details"]["schedulability"]["score"],
        "RT": s["realtime"]["score"],
        "T": s["workload"]["score"],
        "P": s["power"]["score"],
        "F": s["function"]["score"],
        "total": result["total_score"],
        "categories": {k: v["score"] for k, v in s["workload"]["details"].items()},
    }


def main():
    ap = argparse.ArgumentParser(description="SSOT 算分与产物生成")
    ap.add_argument("--version", required=True)
    ap.add_argument("--ssot", required=True)
    ap.add_argument("--scored", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--baseline", default=os.path.join(ROOT, "docs/examples/final/baseline/LS2K1000LA_SylixOS.json"))
    ap.add_argument("--template", default=os.path.join(ROOT, "docs/examples/final/template/display_1103.xlsx"))
    args = ap.parse_args()

    os.makedirs(args.scored, exist_ok=True)
    os.makedirs(os.path.join(args.out, "reports"), exist_ok=True)

    tmp = tempfile.mkdtemp(prefix="rtbench-score-")
    try:
        shutil.copy(args.baseline, os.path.join(tmp, "baseline.json"))
        sources = {}
        for path in sorted(glob.glob(os.path.join(args.ssot, "*.json"))):
            if path.endswith("index.json"):
                continue
            doc = json.load(open(path, encoding="utf-8"))
            converted = conv.convert(copy.deepcopy(doc))
            name = os.path.basename(path)
            json.dump(converted, open(os.path.join(tmp, name), "w", encoding="utf-8"), ensure_ascii=False)
            sources[name] = (doc, converted)

        conn = scorer.init_db(os.path.join(tmp, "scores.db"))
        scorer.ingest_json_files(os.path.join(tmp, "*.json"), conn)
        scorer.recompute_all_scores(conn)

        rows = []
        for name, (doc, converted) in sources.items():
            env = converted["env"]
            result = scorer.export_results(env["board"], env["os_name"], env["cpu_type"], conn)
            scores = flat_scores(result)
            sidecar = {
                "schema": "rtbench.scores.v1",
                "board": env["board"],
                "os_name": env["os_name"],
                "arch": env["cpu_type"],
                "source": f"ssot/{args.version}/{name}",
                "algorithm": {"curve": "saturation", "a": 0.25, "baseline": "LS2K1000LA/SylixOS"},
                "scores": scores,
            }
            sidecar_path = os.path.join(args.scored, name.replace(".json", ".scores.json"))
            json.dump(sidecar, open(sidecar_path, "w", encoding="utf-8"), ensure_ascii=False, indent=2)

            blocks = reports.build_narrative(doc, scores)
            base = os.path.splitext(name)[0]
            open(os.path.join(args.out, "reports", base + ".md"), "w", encoding="utf-8").write(
                reports.blocks_to_markdown(blocks))
            reports.write_docx(blocks, os.path.join(args.out, "reports", base + ".docx"))

            c = scores["categories"]
            rows.append([doc["env"].get("board"), env["os_name"], env["cpu_type"],
                         scores["base_rt"], scores["sched"], scores["RT"],
                         c.get("realtime_control"), c.get("perception"),
                         c.get("communication"), c.get("monitoring"),
                         scores["T"], scores["P"], scores["F"], scores["total"]])

        csv_path = os.path.join(args.out, "scores.csv")
        with open(csv_path, "w", encoding="utf-8-sig", newline="") as f:
            w = csv.writer(f)
            w.writerow(["板卡", "OS", "架构", "基础实时", "调度", "RT", "实时控制", "感知",
                        "通信", "运维", "T", "P", "F", "总分"])
            w.writerows(rows)

        data = display.load_ssot(args.ssot)
        wb = openpyxl.load_workbook(args.template)
        filled, skipped = display.fill_summary(wb["基准测试结果"], data)
        xlsx_path = os.path.join(args.out, f"display_{args.version}.xlsx")
        wb.save(xlsx_path)

        reverse = [n for n, (doc, _) in sources.items()
                   if doc.get("meta", {}).get("source") == "excel_reverse"]
        if reverse:
            print(f"warning: {len(reverse)} boards are excel_reverse: " + ", ".join(reverse))
        print(f"scored {len(rows)} boards -> {args.scored}")
        print(f"reports -> {os.path.join(args.out, 'reports')}")
        print(f"csv -> {csv_path}")
        print(f"excel -> {xlsx_path} (filled {filled}, skipped {skipped})")
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    main()
