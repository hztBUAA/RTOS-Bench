"""
SCP/SFTP 文件传输模块

基于 paramiko 实现上传/下载。
对无 SFTP 的场景（Telnet/Serial），提供 terminal_capture 回退方案。
"""

import json
import re
from typing import Optional

import paramiko


def upload(local_path: str, remote_path: str,
           ip: str, username: str, password: str, port: int = 22):
    """通过 SFTP 上传文件到远端"""
    transport = paramiko.Transport((ip, port))
    transport.connect(username=username, password=password)
    sftp = paramiko.SFTPClient.from_transport(transport)
    try:
        sftp.put(local_path, remote_path)
        print(f"[transfer] 上传完成: {local_path} -> {remote_path}")
    finally:
        sftp.close()
        transport.close()


def download(remote_path: str, local_path: str,
             ip: str, username: str, password: str, port: int = 22):
    """通过 SFTP 从远端下载文件"""
    transport = paramiko.Transport((ip, port))
    transport.connect(username=username, password=password)
    sftp = paramiko.SFTPClient.from_transport(transport)
    try:
        sftp.get(remote_path, local_path)
        print(f"[transfer] 下载完成: {remote_path} -> {local_path}")
    finally:
        sftp.close()
        transport.close()


def extract_json_from_buffer(buffer: str,
                             marker_start: Optional[str] = None) -> Optional[dict]:
    """
    从终端缓冲区中提取 JSON 对象（terminal_capture 回退方案）。

    尝试策略：
    1. 如果指定 marker_start，从 marker 之后提取
    2. 否则尝试匹配最外层 { ... } 块
    """
    text = buffer
    if marker_start:
        idx = text.rfind(marker_start)
        if idx >= 0:
            text = text[idx:]

    # 贪婪匹配最后一个完整 JSON 对象
    matches = re.findall(r'\{[^{}]*(?:\{[^{}]*\}[^{}]*)*\}', text, re.DOTALL)
    for candidate in reversed(matches):
        try:
            return json.loads(candidate)
        except json.JSONDecodeError:
            continue

    return None
