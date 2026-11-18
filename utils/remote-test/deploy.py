#!/usr/bin/env python3
"""
RTOS-Bench 远程部署与测试脚本

全流程: 上传固件 -> 远程执行 -> 等待完成 -> 下载结果 -> 展平 -> 算分

Usage:
    python utils/remote-test/deploy.py -c board.yaml
    python utils/remote-test/deploy.py -c board.yaml --skip-deploy
    python utils/remote-test/deploy.py -c board.yaml --skip-retrieve
    python utils/remote-test/deploy.py -c board.yaml -o ./my_results/
"""

import argparse
import json
import os
import sys

# 确保 utils/ 在 import 路径中（用于 flatten/score 导入）
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_UTILS_DIR = os.path.dirname(_SCRIPT_DIR)
if _UTILS_DIR not in sys.path:
    sys.path.insert(0, _UTILS_DIR)
if _SCRIPT_DIR not in sys.path:
    sys.path.insert(0, _SCRIPT_DIR)

from config_schema import load_config
from log_utils import generate_log_filename, beijing_timestamp
from dut_connection import SSHConnection, TelnetConnection, SerialConnection
import file_transfer


def _conn_type(cfg: dict) -> str:
    return str(cfg["connection"].get("type", "ssh")).lower()


def _upload_artifact(cfg: dict, local_path: str, remote_path: str) -> bool:
    """按连接类型选择上传通道。返回是否执行了上传。"""
    conn_cfg = cfg["connection"]
    ftp_cfg = cfg.get("ftp") or {}
    conn_type = _conn_type(cfg)

    if conn_type == "ssh":
        file_transfer.upload(
            local_path, remote_path,
            ip=conn_cfg["ip"],
            username=conn_cfg["username"],
            password=conn_cfg["password"],
            port=conn_cfg.get("port", 22),
        )
        return True

    if ftp_cfg.get("enabled"):
        file_transfer.upload_ftp(
            local_path, remote_path,
            ip=conn_cfg["ip"],
            username=ftp_cfg.get("username", conn_cfg["username"]),
            password=ftp_cfg.get("password", conn_cfg["password"]),
            port=ftp_cfg.get("port", 21),
        )
        return True

    print(f"[deploy] 连接类型为 {conn_type}，未启用 FTP，跳过上传")
    print("[deploy] 请手工把文件放到板上，或在配置中加 ftp.enabled: true")
    return False


def _retrieve_result(cfg: dict, dut, result_json_path: str) -> str:
    """按连接类型与 retrieval_method 取回结果。返回本地路径或 None。"""
    conn_cfg = cfg["connection"]
    result_cfg = cfg["result"]
    conn_type = _conn_type(cfg)
    method = result_cfg.get("retrieval_method", "sftp")

    if method == "sftp" and conn_type != "ssh":
        print(f"[deploy] 连接类型为 {conn_type}，SFTP 不可用，改用 terminal_capture")
        method = "terminal_capture"

    if method == "sftp":
        remote_path = result_cfg["remote_path"]
        print(f"[deploy] SFTP 下载结果: {remote_path}")
        file_transfer.download(
            remote_path, result_json_path,
            ip=conn_cfg["ip"],
            username=conn_cfg["username"],
            password=conn_cfg["password"],
            port=conn_cfg.get("port", 22),
        )
        return result_json_path

    if method == "ftp":
        remote_path = result_cfg["remote_path"]
        ftp_cfg = cfg.get("ftp") or {}
        print(f"[deploy] FTP 下载结果: {remote_path}")
        file_transfer.download_ftp(
            remote_path, result_json_path,
            ip=conn_cfg["ip"],
            username=ftp_cfg.get("username", conn_cfg["username"]),
            password=ftp_cfg.get("password", conn_cfg["password"]),
            port=ftp_cfg.get("port", 21),
        )
        return result_json_path

    if method == "terminal_capture":
        print("[deploy] 从终端缓冲区提取 JSON...")
        parsed = file_transfer.extract_json_from_buffer(dut.buffer)
        if parsed:
            with open(result_json_path, "w", encoding="utf-8") as f:
                json.dump(parsed, f, ensure_ascii=False, indent=2)
            print(f"[deploy] 结果已保存: {result_json_path}")
            return result_json_path
        print("[deploy] 警告: 无法从终端缓冲区提取 JSON")
        print("[deploy] 确认 test.command 在测试结束后打印了结果 JSON，例如 cat 结果文件")
        return None

    print(f"[deploy] 未知的 retrieval_method: {method}")
    return None


def _create_output_dir(cfg: dict, override_dir: str = None) -> str:
    board = cfg["board"]
    base = override_dir or cfg["result"].get("local_output_dir", "./results/")
    ts = beijing_timestamp()
    dirname = f"{board['name']}_{board['os']}_{ts}"
    path = os.path.join(base, dirname)
    os.makedirs(path, exist_ok=True)
    return path


def _make_connection(cfg: dict, log_file: str):
    conn_cfg = cfg["connection"]
    conn_type = conn_cfg["type"].lower()

    if conn_type == "ssh":
        return SSHConnection(
            ip=conn_cfg["ip"],
            username=conn_cfg["username"],
            password=conn_cfg["password"],
            port=conn_cfg.get("port", 22),
            log_file=log_file,
        )
    elif conn_type == "telnet":
        return TelnetConnection(
            ip=conn_cfg["ip"],
            username=conn_cfg["username"],
            password=conn_cfg["password"],
            port=conn_cfg.get("port", 23),
            log_file=log_file,
        )
    elif conn_type == "serial":
        return SerialConnection(
            port=conn_cfg["device"],
            baudrate=conn_cfg.get("baudrate", 115200),
            log_file=log_file,
        )
    else:
        raise ValueError(f"不支持的连接类型: {conn_type}")


def run(cfg_path: str,
        skip_deploy: bool = False,
        skip_retrieve: bool = False,
        output_dir: str = None):
    """主编排流程"""
    # 1. 加载配置
    cfg = load_config(cfg_path)
    board = cfg["board"]
    conn_cfg = cfg["connection"]
    test_cfg = cfg["test"]
    result_cfg = cfg["result"]

    print(f"[deploy] 板卡: {board['name']} | OS: {board['os']} | 架构: {board['arch']}")

    # 2. 创建输出目录
    out_dir = _create_output_dir(cfg, output_dir)
    print(f"[deploy] 输出目录: {out_dir}")

    # 3. 日志文件
    log_name = generate_log_filename()
    log_path = os.path.join(out_dir, log_name)

    # 4. 上传固件（可选）
    deploy_cfg = cfg.get("deploy", {})
    if not skip_deploy and deploy_cfg.get("local_binary"):
        local_bin = deploy_cfg["local_binary"]
        remote_dir = deploy_cfg.get("remote_dir", "/tmp/")
        remote_bin = remote_dir.rstrip("/") + "/" + os.path.basename(local_bin)
        print(f"[deploy] 上传固件: {local_bin} -> {remote_bin}")
        _upload_artifact(cfg, local_bin, remote_bin)

    # 5. 建立 DUT 连接
    if _conn_type(cfg) == "telnet" and conn_cfg.get("port") == 22:
        print("[deploy] 警告: telnet 常用端口是 23，当前配置为 22")
    print(f"[deploy] 连接 DUT ({conn_cfg['type']}:{conn_cfg.get('ip', '')}:{conn_cfg.get('port', '')})...")
    dut = _make_connection(cfg, log_path)

    try:
        # 6. 发送测试命令
        cmd = test_cfg["command"]
        print(f"[deploy] 发送命令: {cmd}")
        dut.send_cmd(cmd)

        # 7. 等待结束信号
        end_regex = test_cfg["end_regex"]
        timeout = test_cfg.get("timeout_sec", 600)
        print(f"[deploy] 等待结束信号 (timeout={timeout}s)...")
        dut.wait_for_regex(end_regex, timeout=timeout)
        print("[deploy] 测试完成信号已收到")

    finally:
        # 8. 断开连接（日志自动关闭）
        dut.disconnect()
        print(f"[deploy] 连接已断开，日志: {log_path}")

    # 9. 获取结果
    result_json_path = os.path.join(out_dir, "rtbench_result.json")

    if not skip_retrieve:
        result_json_path = _retrieve_result(cfg, dut, result_json_path)

    # 10. 展平（可选）
    flatten_path = None
    if result_cfg.get("run_flatten") and result_json_path and os.path.isfile(result_json_path):
        try:
            from flatten_rtbench_result import flatten_rtbench_result

            with open(result_json_path, "r", encoding="utf-8") as f:
                rtbench_data = json.load(f)

            records = flatten_rtbench_result(rtbench_data)
            flatten_path = os.path.join(out_dir, "standard_results.json")
            with open(flatten_path, "w", encoding="utf-8") as f:
                json.dump(records, f, ensure_ascii=False, indent=2)
            print(f"[deploy] 展平完成: {len(records)} 条记录 -> {flatten_path}")
        except Exception as e:
            print(f"[deploy] 展平失败: {e}")

    # 11. 算分（可选）
    score_path = None
    if result_cfg.get("run_score") and result_json_path and os.path.isfile(result_json_path):
        try:
            from score_caculate import score_single_json

            score_result = score_single_json(result_json_path)
            score_path = os.path.join(out_dir, "score_report.json")
            with open(score_path, "w", encoding="utf-8") as f:
                json.dump(score_result, f, ensure_ascii=False, indent=2)
            total = score_result.get("total_score")
            print(f"[deploy] 算分完成: total_score={total} -> {score_path}")
        except Exception as e:
            print(f"[deploy] 算分失败: {e}")

    # 12. 摘要
    print("\n" + "=" * 50)
    print(f"  板卡:     {board['name']} ({board['os']})")
    print(f"  输出目录: {out_dir}")
    print(f"  日志:     {log_name}")
    if result_json_path and os.path.isfile(result_json_path):
        print(f"  结果:     rtbench_result.json")
    if flatten_path:
        print(f"  展平:     standard_results.json")
    if score_path:
        print(f"  评分:     score_report.json")
    print("=" * 50)

    return out_dir


def main():
    parser = argparse.ArgumentParser(
        description="RTOS-Bench 远程部署与测试脚本"
    )
    parser.add_argument("-c", "--config", required=True,
                        help="板卡配置 YAML 文件路径")
    parser.add_argument("--skip-deploy", action="store_true",
                        help="跳过固件上传")
    parser.add_argument("--skip-retrieve", action="store_true",
                        help="跳过结果下载")
    parser.add_argument("-o", "--output", default=None,
                        help="自定义输出目录")

    args = parser.parse_args()
    run(args.config,
        skip_deploy=args.skip_deploy,
        skip_retrieve=args.skip_retrieve,
        output_dir=args.output)


if __name__ == "__main__":
    main()
