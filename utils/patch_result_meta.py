#!/usr/bin/env python3
"""补全 result.json 的板卡与 RTOS 元信息。

网页端无法在运行时注入，因此测试结束后用本脚本回填：
  python3 utils/patch_result_meta.py result.json \
      --os-name "RT-Thread" --os-version "5.3.1" --board "飞腾派 PhytiumPi"

默认写回原文件，并保留 .bak 备份。--dry-run 只预览。
"""

import argparse
import json
import shutil
import sys

FIELDS = [
    ("os_name", "--os-name", str),
    ("os_version", "--os-version", str),
    ("board", "--board", str),
    ("bsp_version", "--bsp-version", str),
    ("cpu_type", "--cpu-type", str),
    ("cpu_core_num", "--cpu-core-num", int),
    ("cpu_freq_mhz", "--cpu-freq-mhz", int),
]


def main():
    parser = argparse.ArgumentParser(description="补全 result.json 元信息")
    parser.add_argument("json")
    for name, flag, kind in FIELDS:
        parser.add_argument(flag, dest=name, type=kind, default=None)
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--no-backup", action="store_true")
    args = parser.parse_args()

    try:
        with open(args.json, "r", encoding="utf-8") as f:
            data = json.load(f)
    except (OSError, json.JSONDecodeError) as exc:
        sys.exit(f"读取失败: {exc}")

    env = data.setdefault("env", {})
    updates = {name: getattr(args, name) for name, _, _ in FIELDS if getattr(args, name) is not None}
    if not updates:
        sys.exit("没有要更新的字段。用 --help 查看参数。")

    for key, value in updates.items():
        old = env.get(key)
        env[key] = value
        print(f"{key}: {old!r} -> {value!r}")

    if args.dry_run:
        print("dry-run：未写入")
        return

    if not args.no_backup:
        shutil.copyfile(args.json, args.json + ".bak")
    with open(args.json, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)
    print(f"已写入 {args.json}")


if __name__ == "__main__":
    main()
