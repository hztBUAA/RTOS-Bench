#!/usr/bin/env python3
"""
RTOS-Bench 结果格式转换器

将 RTOS-Bench 中间格式 (rtbench_result.json) 展平为标准 性能数据结构.yaml 格式。

Usage:
    python flatten_rtbench_result.py rtbench_result.json --output standard_results.json
    python flatten_rtbench_result.py rtbench_result.json --sw-info sw_info.json --hw-info hw_info.json
"""

import json
import uuid
import argparse
from datetime import datetime
from typing import Any, Dict, List, Tuple, Generator


# 模块到 test_dir 的映射
MODULE_TO_DIR = {
    "test-realtime": "realtime",
    "test-schedule": "schedule",
    "test-stress": "stress",
    "typical-workload": "workload",
}

# 指标优化方向推断规则
OPTIMAL_TYPE_RULES = {
    # 延迟类指标 -> 越小越好
    "latency": "min", "delay": "min", "time": "min", "us": "min", "ms": "min",
    "miss": "min", "fail": "min", "error": "min",
    # 吞吐类指标 -> 越大越好
    "bandwidth": "max", "throughput": "max", "ops": "max", "score": "max",
    "pass": "max", "success": "max", "gb/s": "max", "mb/s": "max",
}


def generate_uuid() -> str:
    """生成唯一的 flow_job_history_id"""
    return str(uuid.uuid4())


def infer_optimal_type(path: str, unit: str = "") -> str:
    """根据路径和单位推断指标优化方向"""
    path_lower = path.lower()
    unit_lower = unit.lower()

    for keyword, opt_type in OPTIMAL_TYPE_RULES.items():
        if keyword in path_lower or keyword in unit_lower:
            return opt_type

    return "max"  # 默认越大越好


def path_to_case_name(path: str) -> str:
    """将路径转换为 test_case 名称"""
    # 移除数组索引，转换为下划线分隔
    import re
    name = re.sub(r'\[(\d+)\]', r'_\1', path)
    name = name.replace('.', '_').replace('-', '_')
    return name.lower()


def extract_metrics(data: Any, prefix: str = "") -> Generator[Tuple[str, Any, str], None, None]:
    """递归提取所有叶子节点指标"""
    if data is None:
        return

    if isinstance(data, dict):
        # 检查是否为叶子对象（包含值和单位）
        if 'unit' in data and any(k in data for k in ['value', 'c1', 'c2', 'c4', 'c8']):
            unit = data.get('unit', '')
            for k, v in data.items():
                if k != 'unit' and isinstance(v, (int, float)):
                    yield (f"{prefix}.{k}" if prefix else k, v, unit)
        else:
            for key, value in data.items():
                new_prefix = f"{prefix}.{key}" if prefix else key
                # 跳过状态和配置字段
                if key in ('status', 'config', 'stage', 'bogo_ops'):
                    continue
                yield from extract_metrics(value, new_prefix)

    elif isinstance(data, list):
        for i, item in enumerate(data):
            if isinstance(item, dict):
                # 尝试用 name/type/operation 作为标识
                item_id = item.get('name') or item.get('type') or item.get('operation') or str(i)
                # 若存在 stage 字段，拼接到 id 中避免重名冲突
                if 'stage' in item:
                    item_id = f"{item_id}_s{item['stage']}"
                item_id = item_id.replace(' ', '_').lower()
                yield from extract_metrics(item, f"{prefix}.{item_id}")
            else:
                yield from extract_metrics(item, f"{prefix}[{i}]")

    elif isinstance(data, (int, float)):
        # 根据路径推断单位
        unit = infer_unit_from_path(prefix)
        yield (prefix, data, unit)

    elif isinstance(data, bool):
        yield (prefix, 1 if data else 0, "bool")


def infer_unit_from_path(path: str) -> str:
    """根据路径推断单位"""
    path_lower = path.lower()
    if 'duration_sec' in path_lower:
        return 'sec'
    elif '_us' in path_lower or 'latency' in path_lower or 'delay' in path_lower:
        return 'us'
    elif '_ms' in path_lower or 'time' in path_lower:
        return 'ms'
    elif 'bandwidth' in path_lower or 'gb' in path_lower:
        return 'GB/s'
    elif 'ops' in path_lower:
        return 'ops'
    elif 'rate' in path_lower:
        return 'ratio'
    elif 'score' in path_lower:
        return 'score'
    return ''


def flatten_rtbench_result(
    rtbench_json: Dict,
    sw_info: Dict = None,
    hw_info: Dict = None,
    test_config_info: Dict = None
) -> List[Dict]:
    """
    将 RTOS-Bench 中间格式展平为标准记录列表

    Args:
        rtbench_json: RTOS-Bench 中间格式 JSON
        sw_info: 软件信息（可选，从 rtbench_json 推断）
        hw_info: 硬件信息（可选，从 rtbench_json 推断）
        test_config_info: 测试配置信息（可选）

    Returns:
        标准格式记录列表
    """
    records = []

    # 从 rtbench_json 推断 sw_info
    if sw_info is None:
        env = rtbench_json.get('env', {})
        sw_info = {
            "sdk_type": "RTOS",
            "sdk_version": env.get('os_version', ''),
            "kernel_version": env.get('os_version', ''),
        }

    # 从 rtbench_json 推断 hw_info
    if hw_info is None:
        env = rtbench_json.get('env', {})
        hw_info = {
            "platform_type": "qemu" if "qemu" in env.get('board', '').lower() else "evb",
            "platform_info": {
                "platform_name": env.get('board', ''),
            },
            "cpu_type": env.get('cpu_type', ''),
            "soc_info": {
                "cpu_total_core_num": str(env.get('cpu_core_num', 1)),
                "bit_freq": str(env.get('cpu_freq_mhz', '')),
                "bit_freq_unit": "MHz",
            }
        }

    # 默认测试配置
    if test_config_info is None:
        env = rtbench_json.get('env', {})
        test_config_info = {
            "test_mcpu": env.get('cpu_type', ''),
            "test_cpu_core_num": str(env.get('cpu_core_num', 1)),
            "test_option_alias": "default",
            "test_option_detail": "",
        }

    # 遍历各模块
    modules = rtbench_json.get('modules', {})
    for module_name, module_data in modules.items():
        if module_data is None:
            continue

        test_dir = MODULE_TO_DIR.get(module_name, module_name)

        # 递归提取所有叶子节点指标
        for path, value, unit in extract_metrics(module_data):
            # 跳过非数值类型
            if not isinstance(value, (int, float)):
                continue

            record = {
                "flow_job_history_id": generate_uuid(),
                "sw_info": sw_info,
                "hw_info": hw_info,
                "test_case_info": {
                    "test_suite": "rtbench",
                    "test_dir": test_dir,
                    "test_case": path_to_case_name(path),
                    "test_data_source": "RTOS-Bench"
                },
                "test_config_info": test_config_info,
                "test_result_info": {
                    "test_result": str(value),
                    "test_result_static_info": {
                        "test_unit": unit,
                        "test_run_times": "1",
                        "test_optimal_type": infer_optimal_type(path, unit),
                        "test_result_rawdata": str(value),
                        "test_result_calculate": "direct",
                    },
                    "test_result_valid": "valid",
                    "test_result_type": "daily"
                }
            }
            records.append(record)

    return records


def main():
    parser = argparse.ArgumentParser(
        description='将 RTOS-Bench 中间格式转换为标准性能数据格式'
    )
    parser.add_argument('input', help='输入的 rtbench_result.json 文件')
    parser.add_argument('--output', '-o', default='standard_results.json',
                        help='输出的标准格式 JSON 文件')
    parser.add_argument('--sw-info', help='软件信息 JSON 文件（可选）')
    parser.add_argument('--hw-info', help='硬件信息 JSON 文件（可选）')
    parser.add_argument('--pretty', '-p', action='store_true',
                        help='格式化输出 JSON')

    args = parser.parse_args()

    # 读取输入文件
    with open(args.input, 'r', encoding='utf-8') as f:
        rtbench_json = json.load(f)

    # 读取可选的 sw_info 和 hw_info
    sw_info = None
    hw_info = None

    if args.sw_info:
        with open(args.sw_info, 'r', encoding='utf-8') as f:
            sw_info = json.load(f)

    if args.hw_info:
        with open(args.hw_info, 'r', encoding='utf-8') as f:
            hw_info = json.load(f)

    # 转换
    records = flatten_rtbench_result(rtbench_json, sw_info, hw_info)

    # 输出
    with open(args.output, 'w', encoding='utf-8') as f:
        if args.pretty:
            json.dump(records, f, ensure_ascii=False, indent=2)
        else:
            json.dump(records, f, ensure_ascii=False)

    print(f"转换完成: {len(records)} 条记录 -> {args.output}")


if __name__ == '__main__':
    main()
