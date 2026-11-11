#!/usr/bin/env python3
"""生成「基准测试数据-展示统计」工作簿。

以 1103 模板为样式基准，只更新汇总页的数据单元格，样式、合并单元格、
列宽、行高保持不变。

用法：
  python3 utils/build_display_workbook.py \
      --template docs/examples/display/template_1103.xlsx \
      --ssot docs/examples/final/ssot/9load \
      -o docs/examples/display/基准测试数据-展示统计.xlsx
"""

import argparse
import json
import os
import re

import openpyxl

BOARD_TYPES = ["工控机", "香橙派", "哪吒", "龙芯", "飞腾"]
VENDOR_PREFIX = [("翼辉", "翼辉"), ("东土", "东土"), ("锐华", "锐华"),
                 ("OneOS", "中移"), ("欧拉", "欧拉"), ("RTT", "RTT")]
CANON_BOARD = {"MIC-7700": "工控机", "Orange-Pi5": "香橙派", "DH-1": "哪吒",
               "LS2K1000LA": "龙芯", "Phytium-Pi": "飞腾"}
OS_VENDOR = {"SylixOS": "翼辉", "Intewell": "东土", "intewell": "东土",
             "ReWorks": "锐华", "rede": "锐华", "OneOS": "中移",
             "openEuler": "欧拉", "openEuler/Linux": "欧拉", "RT-Thread": "RTT"}

# (板卡类型, 厂商) -> 汇总页行号
ROW_MAP = {
    ("香橙派", "翼辉"): 7, ("香橙派", "东土"): 8, ("香橙派", "锐华"): 9, ("香橙派", "中移"): 10,
    ("飞腾", "翼辉"): 14, ("飞腾", "东土"): 15, ("飞腾", "锐华"): 16, ("飞腾", "中移"): 17,
    ("香橙派", "欧拉"): 11, ("飞腾", "欧拉"): 18,
    ("工控机", "翼辉"): 22, ("工控机", "东土"): 23,
    ("哪吒", "翼辉"): 26, ("哪吒", "中移"): 27,
    ("龙芯", "翼辉"): 29, ("龙芯", "锐华"): 30,
}

WORKLOAD_COLS = {
    "pid": "J", "ekf": "K", "fast": "L", "icp": "M", "epnp": "N",
    "modbus": "O", "mqtt": "P", "ewma": "Q", "cusum": "R",
}
POWER_COLS = {"static": "V", "cpu": "W", "memory": "X", "file": "Y"}


def board_type(name):
    for key in BOARD_TYPES:
        if key in name:
            return key
    return CANON_BOARD.get(name)


def vendor(name, os_name=None):
    for prefix, label in VENDOR_PREFIX:
        if name.startswith(prefix):
            return label
    return OS_VENDOR.get(os_name)


def load_ssot(directory):
    data = {}
    for path in sorted(os.listdir(directory)):
        if not path.endswith(".json") or path == "index.json":
            continue
        with open(os.path.join(directory, path), encoding="utf-8") as f:
            doc = json.load(f)
        name = doc.get("env", {}).get("board", "")
        key = (board_type(name), vendor(name, doc.get("env", {}).get("os_name")))
        if key[0] and key[1]:
            data[key] = doc
    return data


def workload_ms(doc):
    out = {}
    for item in doc.get("modules", {}).get("typical-workload", {}).get("workloads", []):
        value = item.get("avg_time_ms")
        if value is None:
            value = item.get("exec_time_ms")
        out[item.get("name")] = value
    return out


def fill_summary(ws, data):
    filled = 0
    skipped = []
    for key, row in ROW_MAP.items():
        doc = data.get(key)
        if not doc:
            continue
        filled += 1
        rt = doc.get("modules", {}).get("test-realtime", {}).get("single_core", {})
        schedule = doc.get("modules", {}).get("test-schedule", {})
        power = {item.get("item"): item.get("energy_j")
                 for item in doc.get("power", {}).get("items", [])}

        loads = workload_ms(doc)
        for name, col in WORKLOAD_COLS.items():
            cell = ws[f"{col}{row}"]
            value = loads.get(name)
            cell.value = value if value is not None else "——"

        ws[f"S{row}"] = rt.get("context_switch", {}).get("avg_us")
        ws[f"T{row}"] = rt.get("interrupt", {}).get("avg_us")
        miss = schedule.get("summary", {}).get("average_miss_rate")
        ws[f"U{row}"] = None if miss is None else round(miss, 3)

        for item, col in POWER_COLS.items():
            ws[f"{col}{row}"] = power.get(item)
    for key in data:
        if key not in ROW_MAP:
            skipped.append(f"{key[1]}-{key[0]}")
    return filled, skipped


def main():
    parser = argparse.ArgumentParser(description="生成展示统计工作簿")
    parser.add_argument("--template", required=True)
    parser.add_argument("--ssot", required=True)
    parser.add_argument("-o", "--output", required=True)
    args = parser.parse_args()

    data = load_ssot(args.ssot)
    wb = openpyxl.load_workbook(args.template)
    filled, skipped = fill_summary(wb["基准测试结果"], data)
    wb.save(args.output)
    print(f"filled {filled} boards -> {args.output}")
    if skipped:
        print("no row in template, skipped: " + ", ".join(skipped))


if __name__ == "__main__":
    main()
