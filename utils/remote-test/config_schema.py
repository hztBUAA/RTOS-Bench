"""
YAML 配置加载与校验
"""

import os
from typing import Any, Dict

import yaml


REQUIRED_SECTIONS = ["board", "connection", "test", "result"]

CONNECTION_DEFAULTS = {
    "type": "ssh",
    "port": 22,
}

RESULT_DEFAULTS = {
    "retrieval_method": "sftp",
    "run_flatten": True,
    "run_score": False,
}


def load_config(yaml_path: str) -> Dict[str, Any]:
    """加载并校验 board 配置 YAML"""
    if not os.path.isfile(yaml_path):
        raise FileNotFoundError(f"配置文件不存在: {yaml_path}")

    with open(yaml_path, "r", encoding="utf-8") as f:
        cfg = yaml.safe_load(f)

    if not isinstance(cfg, dict):
        raise ValueError("配置文件必须是 YAML 字典")

    for section in REQUIRED_SECTIONS:
        if section not in cfg:
            raise ValueError(f"缺少必需配置段: '{section}'")

    # 填充默认值
    conn = cfg["connection"]
    for k, v in CONNECTION_DEFAULTS.items():
        conn.setdefault(k, v)

    result = cfg["result"]
    for k, v in RESULT_DEFAULTS.items():
        result.setdefault(k, v)

    cfg.setdefault("deploy", {})
    cfg["test"].setdefault("timeout_sec", 600)

    return cfg
