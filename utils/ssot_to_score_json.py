#!/usr/bin/env python3
"""把 SSOT JSON 转成 utils/score_caculate.py（博函版）要求的 schema。

主要转换：
- env.board 改为官方板卡名（docs/STRESS_AUTO_TEST.md 对照表）
- test-realtime.multicore 改为 multi_core（memory_bandwidth / ipc_bandwidth / task_latency / core_comm）
- typical-workload 补 avg_time_ms
- power 搬到 modules.test-stress.power

用法：
  python3 utils/ssot_to_score_json.py <ssot_dir> -o <out_dir>
"""

import argparse
import glob
import json
import os
import re

BOARD = {
    "工控机": ("MIC-7700", "x86_64"),
    "香橙派": ("Orange-Pi5", "aarch64"),
    "哪吒": ("DH-1", "riscv64"),
    "龙芯": ("LS2K1000LA", "loongarch64"),
    "飞腾": ("Phytium-Pi", "aarch64"),
}
OS = {
    "翼辉": "SylixOS", "东土": "intewell", "锐华": "rede",
    "中移物联": "OneOS", "欧拉": "openEuler/Linux", "RTT": "RT-Thread",
}
POWER_NAMES = {"static": "standby", "cpu": "cpu", "memory": "memory", "file": "file"}


def canonical_board(name):
    for key, value in BOARD.items():
        if key in name:
            return value
    return name, "aarch64"


def vendor_of(name):
    for key in OS:
        if name.startswith(key):
            return key
    return name


def convert(data):
    board, arch = canonical_board(data["env"]["board"])
    vendor = vendor_of(data["env"]["board"])
    data["env"].update({"board": board, "os_name": OS.get(vendor, vendor), "cpu_type": arch})

    realtime = data["modules"]["test-realtime"]
    multi = realtime.get("multicore") or {}
    memory = [
        {"type": name, "c1": v[0], "c2": v[1], "c4": v[2], "c8": v[3]}
        for name, v in (multi.get("access") or {}).items()
    ]
    service = multi.get("service") or {}
    ipc = {f"c{i + 1}": v for i, v in enumerate(service.get("任务间通信") or [])}
    latency = {f"c{i + 1}": v for i, v in enumerate(service.get("创建与删除") or [])}
    cross = multi.get("cross_core") or {}
    realtime["multi_core"] = {
        "memory_bandwidth": memory,
        "ipc_bandwidth": ipc,
        "task_latency": latency,
        "core_comm": {"intra_core": cross.get("同核通信"), "inter_core": cross.get("异核通信")},
    }
    realtime.pop("multicore", None)

    for wl in data["modules"]["typical-workload"]["workloads"]:
        if wl.get("avg_time_ms") is None:
            wl["avg_time_ms"] = wl.get("exec_time_ms")

    items = (data.get("power") or {}).get("items") or []
    if items:
        data["modules"]["test-stress"] = {
            "power": [
                {"name": POWER_NAMES.get(i["item"], i["item"]), "power_data": i["energy_j"]}
                for i in items if i.get("energy_j") is not None
            ]
        }
    return data


def main():
    parser = argparse.ArgumentParser(description="SSOT -> score_caculate JSON")
    parser.add_argument("ssot_dir")
    parser.add_argument("-o", "--out-dir", required=True)
    args = parser.parse_args()

    os.makedirs(args.out_dir, exist_ok=True)
    count = 0
    for path in glob.glob(os.path.join(args.ssot_dir, "*.json")):
        if path.endswith("index.json"):
            continue
        data = convert(json.load(open(path, encoding="utf-8")))
        name = re.sub(r"[^0-9A-Za-z一-鿿]+", "_", data["env"]["board"] + "_" + data["env"]["os_name"])
        with open(os.path.join(args.out_dir, name + ".json"), "w", encoding="utf-8") as f:
            json.dump(data, f, ensure_ascii=False, indent=2)
        count += 1
    print(f"converted {count} files -> {args.out_dir}")


if __name__ == "__main__":
    main()
