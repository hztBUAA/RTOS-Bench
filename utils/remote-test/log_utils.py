"""
日志文件名生成工具 — 使用北京时间戳
"""

from datetime import datetime, timezone, timedelta

_CST = timezone(timedelta(hours=8))


def generate_log_filename(prefix: str = "rtbench_raw") -> str:
    """生成带北京时间戳的日志文件名，如 rtbench_raw_20260319_153045_CST.log"""
    now = datetime.now(_CST)
    return f"{prefix}_{now.strftime('%Y%m%d_%H%M%S')}_CST.log"


def beijing_timestamp() -> str:
    """返回 YYYYMMDD_HHMMSS 格式的北京时间戳字符串"""
    return datetime.now(_CST).strftime("%Y%m%d_%H%M%S")
