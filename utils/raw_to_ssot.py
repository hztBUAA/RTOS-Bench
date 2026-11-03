#!/usr/bin/env python3
"""Convert a raw rtbench result into the final SSOT JSON schema.

The target result is scored with score_caculate.py before conversion. Unlike
ssot_to_score_json.py, this tool intentionally reads the modern multi_core
layout produced by current rtbench builds.
"""

import argparse
import copy
import importlib.util
import json
from pathlib import Path


HERE = Path(__file__).resolve().parent


def _load_module(name, filename):
    spec = importlib.util.spec_from_file_location(name, HERE / filename)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


scorer = _load_module("score_caculate", "score_caculate.py")
ssot_schema = _load_module("ssot_to_score_json", "ssot_to_score_json.py")

POWER_NAMES = {
    "standby": ("static", "static_power"),
    "cpu": ("cpu", "cpu_power"),
    "memory": ("memory", "memory_power"),
    "file": ("file", "file_power"),
}


def _read_json(path):
    with Path(path).open(encoding="utf-8-sig") as stream:
        return json.load(stream)


def _write_json(path, data):
    target = Path(path)
    target.parent.mkdir(parents=True, exist_ok=True)
    with target.open("w", encoding="utf-8") as stream:
        json.dump(data, stream, ensure_ascii=False, indent=2)
        stream.write("\n")


def _convert_multicore_to_ssot(realtime):
    modern = realtime.get("multi_core")
    if not isinstance(modern, dict):
        return

    access = {}
    for entry in modern.get("memory_bandwidth") or []:
        if not isinstance(entry, dict) or not entry.get("type"):
            continue
        access[entry["type"]] = [
            entry.get(key)
            for key in scorer.MULTICORE_CONCURRENCIES
        ]

    ipc = modern.get("ipc_bandwidth") or {}
    task_latency = modern.get("task_latency") or {}
    core_comm = modern.get("core_comm") or {}

    realtime["multicore"] = {
        "access": access,
        "service": {
            "任务间通信": [
                ipc.get(key)
                for key in scorer.MULTICORE_CONCURRENCIES
            ],
            "创建与删除": [
                task_latency.get(key)
                for key in scorer.MULTICORE_CONCURRENCIES
            ],
        },
        "cross_core": {
            "同核通信": core_comm.get("intra_core"),
            "异核通信": core_comm.get("inter_core"),
        },
    }
    realtime.pop("multi_core", None)


def _build_power_items(raw_modules, export):
    stress = raw_modules.get("test-stress") or {}
    entries = stress.get("power") or []
    power_details = (
        (export.get("scores") or {}).get("power", {}).get("details", {})
    )
    items = []

    for entry in entries:
        if not isinstance(entry, dict):
            continue
        mapped = POWER_NAMES.get(str(entry.get("name", "")).strip().lower())
        if mapped is None:
            continue
        item_name, detail_name = mapped
        detail = (power_details.get(detail_name) or {}).get("power_data") or {}
        items.append({
            "item": item_name,
            "energy_j": entry.get("power_data"),
            "score": detail.get("score"),
        })
    return items


def build_ssot(raw_data, baseline_path, board, os_name, workload_count):
    baseline = _read_json(baseline_path)
    baseline_rt = (
        ((baseline.get("modules") or {}).get("test-realtime") or {})
    )
    if isinstance(baseline_rt, dict) and "multi_core" not in baseline_rt:
        baseline = ssot_schema.convert(copy.deepcopy(baseline))

    error = scorer.validate_json_data(baseline)
    if error:
        raise ValueError(f"baseline JSON is invalid: {error}")

    target = copy.deepcopy(raw_data)
    modules = target.get("modules") or {}
    env = target.setdefault("env", {})
    env["board"] = board
    env["os_name"] = os_name

    error = scorer.validate_json_data(target)
    if error:
        raise ValueError(f"target JSON is invalid: {error}")

    conn = scorer.init_db(":memory:")
    try:
        scorer._parse_json_data(
            copy.deepcopy(baseline), conn, Path(baseline_path).name
        )
        conn.commit()
        info = scorer._parse_json_data(target, conn, "raw-result.json")
        conn.commit()

        arch = info["arch"]
        scorer.normalize_items_for_board(info["board_model"], arch, conn)
        power_scores = scorer.normalize_power_for_board(
            info["board_model"], arch, conn
        )
        scorer.recalc_scores_for_target(
            info["board_model"], info["os_name"], arch, conn, power_scores
        )
        export = scorer.export_results(
            info["board_model"], info["os_name"], arch, conn
        )
    finally:
        conn.close()

    realtime = export["scores"]["realtime"]
    workload = export["scores"]["workload"]
    power = export["scores"]["power"]
    function = export["scores"]["function"]

    target["meta"] = {
        **(target.get("meta") or {}),
        "source": "rtbench_raw",
        "workload_count": workload_count,
    }

    target_realtime = modules.get("test-realtime") or {}
    if isinstance(target_realtime, dict):
        _convert_multicore_to_ssot(target_realtime)

    for entry in (
        (modules.get("typical-workload") or {}).get("workloads") or []
    ):
        if isinstance(entry, dict) and entry.get("avg_time_ms") is None:
            entry["avg_time_ms"] = entry.get("exec_time_ms")

    target["power"] = {
        "items": _build_power_items(modules, export),
    }
    stress = modules.get("test-stress")
    if isinstance(stress, dict):
        summary = {"status": stress.get("status", "from_raw")}
        if stress.get("power"):
            summary["power"] = stress["power"]
        modules["test-stress"] = summary

    test_cmd = modules.get("test-cmd")
    if isinstance(test_cmd, dict):
        modules["test-cmd"] = {"status": test_cmd.get("status", "from_raw")}

    target["scores"] = {
        "base_rt": realtime["details"]["base_realtime"]["score"],
        "sched": realtime["details"]["schedulability"]["score"],
        "RT": realtime["score"],
        "T": workload["score"],
        "P": power["score"],
        "F": function["score"],
        "total": export["total_score"],
        "categories": {
            category: details["score"]
            for category, details in workload["details"].items()
        },
        "average_miss_rate": realtime["details"]["schedulability"].get(
            "average_mr"
        ),
    }
    return target


def main():
    parser = argparse.ArgumentParser(
        description="Convert a raw rtbench JSON to the final SSOT schema"
    )
    parser.add_argument("raw_json")
    parser.add_argument("--baseline", required=True)
    parser.add_argument("--board", required=True)
    parser.add_argument("--os", dest="os_name", default="openEuler")
    parser.add_argument("--workloads", type=int, choices=(7, 9), required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    data = build_ssot(
        _read_json(args.raw_json),
        args.baseline,
        args.board,
        args.os_name,
        args.workloads,
    )
    data["meta"]["source_file"] = Path(args.raw_json).name
    _write_json(args.output, data)


if __name__ == "__main__":
    main()
