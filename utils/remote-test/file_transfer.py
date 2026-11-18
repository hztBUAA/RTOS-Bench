"""
文件传输模块

- SFTP：基于 paramiko，用于 SSH 连接的板卡
- FTP：用于只开放 telnet 的板卡
- terminal_capture：从终端缓冲区提取 JSON，支持任意嵌套
"""

import json
import re
from ftplib import FTP
from typing import Optional

import paramiko

_ANSI_RE = re.compile(r'\x1b\[[0-9;?]*[A-Za-z]')


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


def upload_ftp(local_path: str, remote_path: str,
               ip: str, username: str, password: str, port: int = 21,
               timeout: int = 10):
    """通过 FTP 上传文件到远端。远端目录必须已存在。"""
    ftp = FTP()
    ftp.connect(ip, port, timeout=timeout)
    try:
        ftp.login(username, password)
        with open(local_path, "rb") as f:
            ftp.storbinary(f"STOR {remote_path}", f)
        print(f"[transfer] FTP 上传完成: {local_path} -> {remote_path}")
    finally:
        try:
            ftp.quit()
        except Exception:
            ftp.close()


def download_ftp(remote_path: str, local_path: str,
                 ip: str, username: str, password: str, port: int = 21,
                 timeout: int = 10):
    """通过 FTP 从远端下载文件"""
    ftp = FTP()
    ftp.connect(ip, port, timeout=timeout)
    try:
        ftp.login(username, password)
        with open(local_path, "wb") as f:
            ftp.retrbinary(f"RETR {remote_path}", f.write)
        print(f"[transfer] FTP 下载完成: {remote_path} -> {local_path}")
    finally:
        try:
            ftp.quit()
        except Exception:
            ftp.close()


def _iter_balanced_objects(text: str):
    """按出现顺序产出顶层平衡的 { ... } 片段。忽略字符串内的括号。"""
    stack = []
    in_str = False
    esc = False
    for i, ch in enumerate(text):
        if in_str:
            if esc:
                esc = False
            elif ch == "\\":
                esc = True
            elif ch == '"':
                in_str = False
            continue
        if ch == '"':
            in_str = True
        elif ch == "{":
            stack.append(i)
        elif ch == "}":
            if stack:
                start = stack.pop()
                if not stack:
                    yield text[start:i + 1]


def extract_json_from_buffer(buffer: str,
                             marker_start: Optional[str] = None) -> Optional[dict]:
    """
    从终端缓冲区中提取 JSON 对象。

    支持任意嵌套层级。忽略 ANSI 控制序列。取最后一个能解析的对象。
    """
    if not buffer:
        return None

    text = _ANSI_RE.sub("", buffer)
    if marker_start:
        idx = text.rfind(marker_start)
        if idx >= 0:
            text = text[idx:]

    candidates = list(_iter_balanced_objects(text))
    for candidate in reversed(candidates):
        try:
            return json.loads(candidate)
        except json.JSONDecodeError:
            continue
    return None
