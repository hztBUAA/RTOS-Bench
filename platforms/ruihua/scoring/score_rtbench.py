#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
RTOS-Bench 7 负载算分工具（锐华版）

输入：rtbench 结果 JSON（rtbench_test_all 生成，或结果展示表逆生成的 JSON）
输出：控制台分数表 + scores.json + scores.csv

基线已内嵌在 baseline.json，不需要数据库，不需要联网。
只依赖 Python 3 标准库。
"""

from __future__ import annotations

import argparse
import csv
import json
import sys
from pathlib import Path

A = 0.25
F_SCORE = 60.0

BASE_FILE = Path(__file__).resolve().with_name("baseline.json")

# 7 负载口径：EKF / EPnP 不参与评分
WORKLOAD_CATEGORY = {
    "pid": "realtime_control",
    "fast": "perception",
    "icp": "perception",
    "modbus": "communication",
    "mqtt": "communication",
    "ewma": "monitoring",
    "cusum": "monitoring",
}

CATEGORY_ORDER = [
    ("realtime_control", "实时控制"),
    ("perception", "感知"),
    ("communication", "通信"),
    ("monitoring", "运维"),
]

CATEGORY_MEMBERS = {
    "realtime_control": ["pid"],
    "perception": ["fast", "icp"],
    "communication": ["modbus", "mqtt"],
    "monitoring": ["ewma", "cusum"],
}

BASE_RT_WEIGHTS = {
    "context_switch": 0.10,
    "interrupt": 0.10,
    "scheduling": 0.40,
    "multicore": 0.40,
}

POWER_KEYS = ["static", "cpu", "memory", "file"]


def sat(r):
    """绝对饱和曲线：S = 100 * R / (a + R)，a = 0.25。"""
    if r is None:
        return None
    return 100.0 * r / (A + r)


def latency_ratio(measured, baseline):
    if not measured or not baseline:
        return None
    return baseline / measured


def mean(values):
    values = [v for v in values if v is not None]
    return sum(values) / len(values) if values else None


def round2(v):
    return None if v is None else round(v, 2)


def load_json(path):
    with open(path, "r", encoding="utf-8-sig") as f:
        return json.load(f)


def pick_measured_ms(entry):
    """优先 avg_time_ms。

    exec_time_ms 在原始 rtbench JSON 里是多轮总耗时，必须先除以 rounds。
    结果展示表逆生成的 SSOT 没有 rounds 字段，exec_time_ms 本身就是单轮值。
    """
    avg = entry.get("avg_time_ms")
    if avg is not None:
        try:
            avg = float(avg)
            if avg > 0:
                return avg
        except (TypeError, ValueError):
            pass

    exec_ms = entry.get("exec_time_ms")
    if exec_ms is None:
        return None
    try:
        exec_ms = float(exec_ms)
    except (TypeError, ValueError):
        return None
    if exec_ms <= 0:
        return None

    rounds = entry.get("rounds")
    try:
        rounds = int(rounds) if rounds is not None else 0
    except (TypeError, ValueError):
        rounds = 0
    if rounds > 1:
        return exec_ms / rounds
    return exec_ms


def parse_workloads(doc, baseline, warnings):
    mod = doc.get("modules", {}).get("typical-workload", {})
    entries = mod.get("workloads", [])
    measured = {}
    ignored = []

    for entry in entries:
        name = str(entry.get("name", "")).strip().lower()
        if not name:
            continue
        if name not in WORKLOAD_CATEGORY:
            ignored.append(name)
            continue
        value = pick_measured_ms(entry)
        if value is None:
            warnings.append("负载 %s 没有有效耗时，已跳过" % name)
            continue
        measured[name] = value

    if ignored:
        warnings.append("按 7 负载口径忽略: " + ", ".join(sorted(set(ignored))))

    scores = {}
    for name, value in measured.items():
        base = baseline["workloads_ms"].get(name)
        R = latency_ratio(value, base)
        scores[name] = {
            "measured_ms": value,
            "baseline_ms": base,
            "R": R,
            "score": sat(R),
        }
    return measured, scores


def parse_categories(scores, warnings):
    categories = {}
    for cat, label in CATEGORY_ORDER:
        members = CATEGORY_MEMBERS[cat]
        values = [scores[m]["score"] for m in members if m in scores]
        missing = [m for m in members if m not in scores]
        if missing:
            warnings.append("分类 %s 缺少负载: %s" % (label, ", ".join(missing)))
        categories[cat] = {
            "label": label,
            "members": members,
            "score": mean(values),
            "complete": not missing,
        }
    return categories


def parse_schedule(doc):
    mod = doc.get("modules", {}).get("test-schedule", {})
    summary = mod.get("summary", {})
    mr = summary.get("average_miss_rate")
    if mr is None:
        total_jobs = 0
        total_misses = 0
        for g in mod.get("gradients", []):
            total_jobs += int(g.get("total_jobs") or 0)
            total_misses += int(g.get("deadline_misses") or 0)
        if total_jobs:
            mr = total_misses / total_jobs
    if mr is None:
        return None, None
    mr = float(mr)
    return mr, 100.0 * (1.0 - mr)


def parse_realtime(doc, baseline, warnings):
    """按 docs/SCORE-CALCULATE.md §4.1 复算基础实时。

    每个大指标得分 = 其所有子项 item_score 的算术平均。
    多核开销当前无基准数据，不纳入计算；剩余权重归一化。
    """
    rt = doc.get("modules", {}).get("test-realtime", {})
    sc = rt.get("single_core", {})
    base_rt = baseline["realtime_us"]

    items = {}

    ctx = sc.get("context_switch", {}).get("avg_us")
    items["context_switch"] = sat(latency_ratio(ctx, base_rt["context_switch"]))

    intr = sc.get("interrupt", {}).get("avg_us")
    items["interrupt"] = sat(latency_ratio(intr, base_rt["interrupt"]))

    base_cost = base_rt.get("service_cost", {})
    sched_scores = []
    for entry in sc.get("service_cost", []):
        op = entry.get("operation")
        b = base_cost.get(op)
        if not b:
            continue
        for key in ("immediate_us", "suspend_us", "low_prio_us", "high_prio_us"):
            value = sat(latency_ratio(entry.get(key), b.get(key)))
            if value is not None:
                sched_scores.append(value)
    items["scheduling"] = mean(sched_scores)
    items["scheduling_items"] = len(sched_scores)

    if rt.get("multi_core"):
        warnings.append("输入含多核数据，但当前算分口径未纳入多核开销")

    weights = {k: v for k, v in BASE_RT_WEIGHTS.items() if k != "multicore"}
    weighted = 0.0
    total_weight = 0.0
    for key, weight in weights.items():
        value = items.get(key)
        if value is not None:
            weighted += weight * value
            total_weight += weight
    base_score = None if total_weight == 0 else weighted / total_weight

    detail = dict(items)
    detail["weights"] = weights
    detail["partial"] = any(items.get(k) is None for k in weights)
    detail["estimated"] = True
    return base_score, detail


def parse_power(args, doc, baseline, warnings):
    provided = {}
    for key in POWER_KEYS:
        value = getattr(args, "power_" + key)
        if value is not None:
            provided[key] = float(value)

    if not provided:
        mod = doc.get("modules", {})
        for key in POWER_KEYS:
            value = mod.get("power", {}).get(key)
            if value is not None:
                provided[key] = float(value)

    if len(provided) != 4:
        if provided:
            warnings.append("功耗四项不完整，P 不计算")
        return None, {}

    Rs = {}
    scores = {}
    for key in POWER_KEYS:
        Rs[key] = latency_ratio(provided[key], baseline["power_j"].get(key))
        scores[key] = sat(Rs[key])
    return mean(list(scores.values())), {
        "measured_j": provided,
        "baseline_j": baseline["power_j"],
        "R": Rs,
        "item_scores": scores,
    }


def build_result(args):
    doc = load_json(args.input)
    baseline = load_json(BASE_FILE)
    warnings = []

    measured, workload_scores = parse_workloads(doc, baseline, warnings)
    categories = parse_categories(workload_scores, warnings)

    cat_scores = {k: v["score"] for k, v in categories.items()}
    T = mean([v for v in cat_scores.values()])
    complete_cats = all(v["complete"] for v in categories.values())
    if not complete_cats:
        warnings.append("T 为已有分类的平均，结果标记为 partial")

    mr, sched = parse_schedule(doc)
    if sched is None:
        warnings.append("输入没有可调度性结果，调度分缺失")

    base_rt, base_detail = parse_realtime(doc, baseline, warnings)
    if args.base_realtime is not None:
        base_rt = float(args.base_realtime)
        base_detail["override"] = True
        base_detail["estimated"] = False
        warnings.append("基础实时使用命令行覆盖值: %.2f" % base_rt)
    else:
        warnings.append("基础实时为单核三项参考复算值，正式分数以结果表为准；可用 --base-realtime 指定")

    if base_rt is not None and sched is not None:
        RT = 0.5 * base_rt + 0.5 * sched
    else:
        RT = None
        warnings.append("基础实时或调度分缺失，RT 不计算")

    P, power_detail = parse_power(args, doc, baseline, warnings)

    if RT is not None and T is not None and P is not None:
        total = 0.2 * F_SCORE + 0.3 * RT + 0.4 * T + 0.1 * P
    else:
        total = None
        if P is None:
            warnings.append("P 缺失，总分不计算。可用 --power-static/cpu/memory/file 传入焦耳值")

    return {
        "input": str(Path(args.input).resolve()),
        "workload_count": 7,
        "measured_ms": measured,
        "workloads": workload_scores,
        "categories": categories,
        "T": T,
        "T_partial": not complete_cats,
        "sched": {
            "average_miss_rate": mr,
            "score": sched,
        },
        "base_realtime": {
            "score": base_rt,
            "detail": base_detail,
        },
        "RT": RT,
        "P": P,
        "power": power_detail,
        "F": F_SCORE,
        "total": total,
        "warnings": warnings,
    }


def print_result(result):
    def line(label, value):
        text = "-" if value is None else "%.2f" % value
        print("%-10s %10s" % (label, text))

    print("")
    print("RTOS-Bench 7 负载算分")
    print("输入: %s" % result["input"])
    print("-" * 40)
    for cat, label in CATEGORY_ORDER:
        line(label, result["categories"][cat]["score"])
    line("T", result["T"])
    print("-" * 40)
    line("调度", result["sched"]["score"])
    line("基础实时", result["base_realtime"]["score"])
    line("RT", result["RT"])
    print("-" * 40)
    line("P", result["P"])
    line("F", result["F"])
    line("总分", result["total"])
    print("-" * 40)
    for w in result["warnings"]:
        print("注意: %s" % w)
    print("")


def write_outputs(result, out_base):
    json_path = Path(str(out_base) + ".json")
    csv_path = Path(str(out_base) + ".csv")

    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(result, f, ensure_ascii=False, indent=2)

    rows = [
        ("实时控制", result["categories"]["realtime_control"]["score"]),
        ("感知", result["categories"]["perception"]["score"]),
        ("通信", result["categories"]["communication"]["score"]),
        ("运维", result["categories"]["monitoring"]["score"]),
        ("T", result["T"]),
        ("调度", result["sched"]["score"]),
        ("基础实时", result["base_realtime"]["score"]),
        ("RT", result["RT"]),
        ("P", result["P"]),
        ("F", result["F"]),
        ("总分", result["total"]),
    ]
    with open(csv_path, "w", encoding="utf-8-sig", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["指标", "得分"])
        for name, value in rows:
            writer.writerow([name, "" if value is None else "%.2f" % value])

    print("已写出: %s" % json_path)
    print("已写出: %s" % csv_path)


def main(argv=None):
    parser = argparse.ArgumentParser(
        description="RTOS-Bench 7 负载算分工具（内嵌 baseline，无需数据库）"
    )
    parser.add_argument("input", help="rtbench 结果 JSON")
    parser.add_argument("-o", "--out", default="scores", help="输出文件前缀，默认 scores")
    parser.add_argument("--base-realtime", type=float, default=None,
                        help="直接指定基础实时得分，跳过复算")
    parser.add_argument("--power-static", type=float, default=None, help="静态功耗，焦耳")
    parser.add_argument("--power-cpu", type=float, default=None, help="CPU 压力功耗，焦耳")
    parser.add_argument("--power-memory", type=float, default=None, help="内存压力功耗，焦耳")
    parser.add_argument("--power-file", type=float, default=None, help="文件压力功耗，焦耳")
    args = parser.parse_args(argv)

    try:
        result = build_result(args)
    except FileNotFoundError:
        print("错误: 找不到输入文件: %s" % args.input)
        return 2
    except json.JSONDecodeError as exc:
        print("错误: JSON 格式不正确: %s" % exc)
        return 2
    except (KeyError, TypeError, ValueError) as exc:
        print("错误: 输入结构不符合 rtbench 结果格式: %s" % exc)
        return 2

    print_result(result)
    write_outputs(result, args.out)

    # 总分能算出返回 0；缺 P 或缺实时数据返回 2，方便自动化判断
    return 0 if result["total"] is not None else 2


if __name__ == "__main__":
    sys.exit(main())
