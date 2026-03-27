import os
import tempfile
import pandas as pd
from datetime import datetime
import json
import logging
from typing import List, Dict, Optional

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s %(levelname)s %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S"
)

# 合法的 job_type 名称集合
_KNOWN_JOB_TYPES = {'cpu', 'memory', 'file', 'standby'}


def _build_filename(board_name: str, os_name: str, job_type: str, out_path: str) -> str:
    """Build full filename.
    If out_path is a directory or has no extension, create file inside it;
    otherwise treat as full filename.
    """
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    base_name = f"{os_name}_{board_name}_{job_type.upper()}_{timestamp}.xlsx"
    if os.path.isdir(out_path) or out_path.endswith(os.sep):
        os.makedirs(out_path, exist_ok=True)
        return os.path.join(out_path, base_name)
    root, ext = os.path.splitext(out_path)
    if ext == "":
        os.makedirs(out_path, exist_ok=True)
        return os.path.join(out_path, base_name)
    os.makedirs(os.path.dirname(out_path) or '.', exist_ok=True)
    return out_path


def _normalize_job_type(job_type: str) -> str:
    name = str(job_type).lower().strip()

    # 中文别名
    if name in ('待机模式', 'standby'):
        return 'standby'

    # 精确匹配
    if name in _KNOWN_JOB_TYPES:
        return name

    # 模糊匹配
    if 'cpu' in name:
        matched = 'cpu'
    elif 'mem' in name:
        matched = 'memory'
    elif 'file' in name or 'io' in name or 'hdd' in name or 'disk' in name:
        matched = 'file'
    elif 'stand' in name or 'idle' in name:
        matched = 'standby'
    else:
        matched = 'cpu'

    logging.warning(
        f"无法精确识别 job_type='{job_type}'，已 fallback 为 '{matched}'，"
        f"合法值为 {sorted(_KNOWN_JOB_TYPES)}，请确认调用参数。"
    )
    return matched


def _safe_float(value, label: str) -> Optional[float]:
    try:
        return float(value)
    except (TypeError, ValueError):
        logging.warning(f"字段 '{label}' 的值 {value!r} 无法转换为 float，已跳过。")
        return None


def _extract_power_value(summary_info: Dict, data_records: List) -> float:

    for key in ('总功耗(J)', '总功耗', 'total_joules', 'total_joules_j', 'total_joules(J)'):
        if key in summary_info:
            val = _safe_float(summary_info[key], key)
            if val is not None:
                logging.info(f"功耗来源: summary_info['{key}'] = {val} J")
                return val

    # 2. Wh 字段换算
    if '总功耗(Wh)' in summary_info:
        val = _safe_float(summary_info['总功耗(Wh)'], '总功耗(Wh)')
        if val is not None:
            joules = val * 3600.0
            logging.info(f"功耗来源: summary_info['总功耗(Wh)'] = {val} Wh → {joules} J")
            return joules

    # 3. 梯形积分
    try:
        if len(data_records) >= 2:
            total_joules = 0.0
            for i in range(1, len(data_records)):
                t0 = float(data_records[i - 1][0])
                t1 = float(data_records[i][0])
                p0 = float(data_records[i - 1][3])
                p1 = float(data_records[i][3])
                dt = max(0.0, t1 - t0)
                total_joules += (p0 + p1) / 2.0 * dt
            logging.info(f"功耗来源: data_records 梯形积分 = {total_joules:.4f} J")
            return total_joules
    except Exception as e:
        logging.warning(f"梯形积分失败: {e}")

    logging.warning("无法从任何来源提取功耗值，已置为 0.0 J")
    return 0.0


def _merge_power_into_json(result_json: str, power_entry: Dict) -> None:
    logging.info(f"合并功耗数据到 JSON: {result_json}")
    if not os.path.exists(result_json):
        logging.error(f"找不到 JSON 文件：{result_json}")
        raise FileNotFoundError(result_json)

    with open(result_json, 'r', encoding='utf-8') as f:
        data = json.load(f)

    modules = data.setdefault('modules', {})
    test_stress = modules.setdefault('test-stress', {})
    existing = test_stress.get('power')

    if existing is None:
        existing = []
        test_stress['power'] = existing
    elif not isinstance(existing, list):
        logging.warning("modules.test-stress.power 不是数组，已重置为新数组")
        existing = []
        test_stress['power'] = existing

    existing.append(power_entry)

    json_dir = os.path.dirname(os.path.abspath(result_json))
    tmp_fd, tmp_path = tempfile.mkstemp(dir=json_dir, suffix='.tmp')
    try:
        with os.fdopen(tmp_fd, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
        os.replace(tmp_path, result_json)
        logging.info(f"已原子写回 JSON 文件，当前 power 条目数: {len(existing)}")
    except Exception:
        # 清理临时文件，不吞掉原始异常
        try:
            os.unlink(tmp_path)
        except OSError:
            pass
        raise


def export_test_result(
    board_name: str,
    os_name: str,
    job_type: str,
    data_records: List,
    summary_info: Dict,
    excel_path: Optional[str] = None,
    result_json: Optional[str] = None,
    power_unit: str = 'J'
) -> str:
    """Export power sampling and summary to Excel and optionally merge power
    summary into a result JSON.

    Parameters
    ----------
    board_name   : 板卡名称，用于 Excel 文件名
    os_name      : OS 名称，用于 Excel 文件名
    job_type     : 负载类型，合法值：'cpu' / 'memory' / 'file' / 'standby'
                   以及中文别名 '待机模式'
    data_records : 采样记录列表，每行格式：[time_s, voltage_v, current_a, power_w]
    summary_info : 汇总字典，用于 Excel 汇总页及功耗值提取
    excel_path   : Excel 输出路径（目录或完整文件名），默认 'record'
    result_json  : 中间结果 JSON 路径，不为 None 时执行功耗注入
    power_unit   : 功耗单位标注，默认 'J'（焦耳）

    Returns
    -------
    str : 实际写入的 Excel 文件路径
    """
    out_dir = excel_path or 'record'
    filename = _build_filename(board_name, os_name, job_type, out_dir)

    df_data = pd.DataFrame(
        data_records,
        columns=["Time(s)", "Voltage(V)", "Current(A)", "Power(W)"]
    )
    df_summary = pd.DataFrame([summary_info])

    try:
        with pd.ExcelWriter(filename, engine='openpyxl') as writer:
            df_summary.to_excel(writer, sheet_name="测试汇总", index=False)
            df_data.to_excel(writer, sheet_name="详细采样数据", index=False)
        logging.info(f"[Excel Exporter] 结果已保存至: {filename}")
    except Exception as e:
        logging.error(f"[Excel Exporter] 保存 Excel 失败: {e}")
        raise

    if result_json:
        name = _normalize_job_type(job_type)
        power_value = _extract_power_value(summary_info, data_records)

        logging.info(
            f"构建 power entry: name={name}, power_data={power_value:.4f}, "
            f"power_unit={power_unit}"
        )

        entry = {
            "name": name,
            "power_data": round(power_value, 4),
            "power_unit": power_unit,
        }

        try:
            _merge_power_into_json(result_json, entry)
        except Exception as e:
            logging.error(f"合并到 JSON 失败: {e}")
            raise

    return filename
