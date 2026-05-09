#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
通过SSH跳板机的Telnet测试脚本 (使用远程Python脚本)

Usage:
    python telnet_test_remote.py -c boards/board_feiteng.yaml -t test_schedule_quick
"""

import argparse
import sys
import time
import yaml
import subprocess
import tempfile
from pathlib import Path


def load_board_config(config_path):
    """加载板卡配置文件"""
    with open(config_path, 'r', encoding='utf-8') as f:
        return yaml.safe_load(f)


def create_remote_script(board_ip, board_port, username, password, command, timeout):
    """创建在跳板机上执行的Python脚本"""
    script = f'''#!/usr/bin/env python3
import socket
import time
import sys

# Telnet协议常量
IAC = 255  # Interpret As Command
DONT = 254
DO = 253
WONT = 252
WILL = 251

def negotiate_telnet(sock, data):
    """处理Telnet协议协商"""
    response = b""
    i = 0
    text = b""

    while i < len(data):
        if data[i] == IAC:
            if i + 2 < len(data):
                cmd = data[i+1]
                opt = data[i+2]

                # 响应协商
                if cmd == DO:
                    response += bytes([IAC, WONT, opt])
                elif cmd == WILL:
                    response += bytes([IAC, DONT, opt])

                i += 3
            else:
                break
        else:
            text += bytes([data[i]])
            i += 1

    if response:
        sock.sendall(response)

    return text

def telnet_test():
    """通过socket实现telnet测试"""
    try:
        print("[1/6] 连接到板卡 {board_ip}:{board_port}")
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(30)
        sock.connect(('{board_ip}', {board_port}))

        # 等待login提示 - 增加等待时间
        print("[2/6] 等待login提示...")
        buffer = b""
        start = time.time()
        time.sleep(1.5)  # 初始等待，参考PowerShell脚本
        while time.time() - start < 15:
            try:
                sock.settimeout(1)
                data = sock.recv(4096)
                text = negotiate_telnet(sock, data)
                buffer += text
                if b"login:" in buffer.lower():
                    break
            except socket.timeout:
                if b"login:" in buffer.lower():
                    break

        # 发送用户名
        print("[3/6] 发送用户名: {username}")
        sock.sendall(b"{username}\\r\\n")
        time.sleep(0.5)  # 参考PowerShell脚本的500ms等待

        # 等待password提示
        print("[4/6] 等待password提示...")
        buffer = b""
        start = time.time()
        while time.time() - start < 10:
            try:
                sock.settimeout(1)
                data = sock.recv(4096)
                text = negotiate_telnet(sock, data)
                buffer += text
                if b"password:" in buffer.lower():
                    break
            except socket.timeout:
                if b"password:" in buffer.lower():
                    break

        # 发送密码
        print("[5/6] 发送密码")
        sock.sendall(b"{password}\\r\\n")
        time.sleep(1)  # 参考PowerShell脚本的1s等待

        # 等待shell提示符
        print("[6/6] 等待shell提示符...")
        buffer = b""
        start = time.time()
        while time.time() - start < 10:
            try:
                sock.settimeout(1)
                data = sock.recv(4096)
                text = negotiate_telnet(sock, data)
                buffer += text
                if b"#" in buffer or b"]#" in buffer:
                    break
            except socket.timeout:
                if b"#" in buffer or b"]#" in buffer:
                    break

        # 清空输入缓冲区
        time.sleep(0.5)
        try:
            sock.settimeout(0.1)
            while True:
                data = sock.recv(4096)
                if not data:
                    break
                negotiate_telnet(sock, data)
        except:
            pass

        # 发送测试命令
        print("\\n[TEST] 执行测试命令: {command}")
        print("="*80)
        sock.settimeout(30)
        sock.sendall(b"{command}\\r\\n")
        time.sleep(2)

        # 读取输出
        print("\\n等待测试完成 (超时: {timeout}秒)...\\n")
        output_lines = []
        start = time.time()
        found_score = False

        while time.time() - start < {timeout}:
            try:
                sock.settimeout(5)
                data = sock.recv(4096)
                if not data:
                    break

                text = data.decode('utf-8', errors='ignore')
                output_lines.append(text)
                print(text, end='', flush=True)

                # 检查是否完成
                if "Final Score" in text or "final score" in text.lower():
                    print("\\n\\n[SUCCESS] 检测到测试完成标记!")
                    found_score = True
                    time.sleep(2)
                    # 读取剩余输出
                    try:
                        sock.settimeout(1)
                        while True:
                            data = sock.recv(4096)
                            if not data:
                                break
                            text = data.decode('utf-8', errors='ignore')
                            output_lines.append(text)
                            print(text, end='', flush=True)
                    except:
                        pass
                    break

            except socket.timeout:
                continue
            except Exception as e:
                print(f"\\n[ERROR] 读取异常: {{e}}")
                break

        # 关闭连接
        try:
            sock.sendall(b"exit\\n")
            time.sleep(0.5)
        except:
            pass
        sock.close()

        if found_score:
            print("\\n[RESULT] 测试成功!")
            return 0
        else:
            print("\\n[RESULT] 测试失败 - 未找到Final Score")
            return 1

    except Exception as e:
        print(f"\\n[ERROR] 测试失败: {{e}}")
        return 1

if __name__ == "__main__":
    sys.exit(telnet_test())
'''
    return script


def execute_via_jumphost(board_ip, board_port, username, password, command,
                         jumphost_ip, jumphost_port, jumphost_user, timeout=600):
    """
    通过SSH跳板机执行telnet命令

    Returns:
        (success, output)
    """
    jumphost = f"{jumphost_user}@{jumphost_ip}"

    print(f"[INFO] 通过SSH跳板机连接到板卡")
    print(f"[INFO] 跳板机: {jumphost}:{jumphost_port}")
    print(f"[INFO] 目标板卡: {board_ip}:{board_port}")
    print(f"[INFO] 执行命令: {command}")
    print()

    try:
        # 创建远程脚本
        remote_script = create_remote_script(board_ip, board_port, username, password, command, timeout)

        # 创建临时文件
        with tempfile.NamedTemporaryFile(mode='w', suffix='.py', delete=False, encoding='utf-8') as f:
            f.write(remote_script)
            local_script = f.name

        print("[1/3] 上传测试脚本到跳板机...")
        remote_path = f"/tmp/telnet_test_{int(time.time())}.py"

        # 上传脚本
        scp_cmd = [
            'scp',
            '-P', str(jumphost_port),
            local_script,
            f"{jumphost}:{remote_path}"
        ]

        result = subprocess.run(scp_cmd, capture_output=True, text=True, timeout=30, encoding='utf-8', errors='ignore')
        if result.returncode != 0:
            print(f"[ERROR] 上传脚本失败: {result.stderr}")
            return (False, result.stderr)

        print("[2/3] 在跳板机上执行测试...")
        print("="*80)

        # 执行远程脚本 - 使用Popen实时输出
        ssh_cmd = [
            'ssh',
            '-p', str(jumphost_port),
            jumphost,
            f"python3 -u {remote_path}"  # -u for unbuffered output
        ]

        process = subprocess.Popen(
            ssh_cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            encoding='utf-8',
            errors='ignore',
            bufsize=1  # Line buffered
        )

        output_lines = []
        try:
            # 实时读取输出
            for line in process.stdout:
                print(line, end='', flush=True)
                output_lines.append(line)

            # 等待进程结束
            process.wait(timeout=timeout + 60)

        except subprocess.TimeoutExpired:
            process.kill()
            print("\n[ERROR] 测试超时")
            return (False, "Timeout")

        output = ''.join(output_lines)

        # 清理远程脚本
        print("\n[3/3] 清理临时文件...")
        subprocess.run(
            ['ssh', '-p', str(jumphost_port), jumphost, f"rm -f {remote_path}"],
            capture_output=True,
            timeout=10
        )

        # 清理本地临时文件
        import os
        os.unlink(local_script)

        # 判断是否成功
        success = process.returncode == 0 and "[SUCCESS]" in output and "Final Score" in output

        return (success, output)

    except subprocess.TimeoutExpired:
        error_msg = f"错误: 测试超时 ({timeout}秒)"
        print(f"\n[ERROR] {error_msg}")
        return (False, error_msg)
    except Exception as e:
        error_msg = f"错误: {str(e)}"
        print(f"\n[ERROR] {error_msg}")
        return (False, error_msg)


def main():
    parser = argparse.ArgumentParser(description='通过SSH跳板机的Telnet测试脚本')
    parser.add_argument('-c', '--config', required=True, help='板卡配置文件路径')
    parser.add_argument('-t', '--test', required=True,
                       help='测试命令 (test_schedule_quick/test_schedule_full)')

    args = parser.parse_args()

    # 加载配置
    config_path = Path(args.config)
    if not config_path.exists():
        print(f"错误: 配置文件不存在: {config_path}")
        sys.exit(1)

    config = load_board_config(config_path)

    # 提取配置信息
    board = config['board']
    conn = config['connection']
    test_cfg = config['test']
    jumphost_cfg = config.get('jumphost', {})

    print(f"板卡: {board['name']}")
    print(f"系统: {board['os']}")
    print(f"架构: {board['arch']}")
    print()

    # 获取测试命令
    if args.test not in test_cfg['commands']:
        print(f"错误: 未知的测试命令: {args.test}")
        print(f"可用命令: {', '.join(test_cfg['commands'].keys())}")
        sys.exit(1)

    command = test_cfg['commands'][args.test]
    timeout = test_cfg.get('timeout_sec', 600)

    # 获取跳板机配置
    if not jumphost_cfg:
        print("错误: 配置文件中缺少 jumphost 配置")
        print("请确保配置文件包含 jumphost 部分")
        sys.exit(1)

    jumphost_ip = jumphost_cfg['host']
    jumphost_port = jumphost_cfg['port']
    jumphost_user = jumphost_cfg['username']

    # 获取板卡连接信息
    board_ip = conn['ip']
    board_port = conn['port']

    print(f"[INFO] 跳板机: {jumphost_user}@{jumphost_ip}:{jumphost_port}")
    print(f"[INFO] 板卡地址: {board_ip}:{board_port}")
    print()

    # 执行测试
    success, output = execute_via_jumphost(
        board_ip=board_ip,
        board_port=board_port,
        username=conn['username'],
        password=conn['password'],
        command=command,
        jumphost_ip=jumphost_ip,
        jumphost_port=jumphost_port,
        jumphost_user=jumphost_user,
        timeout=timeout
    )

    # 输出结果
    print("\n" + "="*80)
    if success:
        print("测试成功!")
        sys.exit(0)
    else:
        print("测试失败!")
        sys.exit(1)


if __name__ == "__main__":
    main()
