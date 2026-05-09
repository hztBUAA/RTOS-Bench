#!/usr/bin/env python3
"""
简化的Telnet测试脚本 - 用于执行单个板卡的test-schedule命令

Usage:
    python telnet_test.py -c boards/board_feiteng.yaml -t test_schedule_quick
"""

import argparse
import sys
import os
import time
import socket
import yaml
from pathlib import Path


def load_board_config(config_path):
    """加载板卡配置文件"""
    with open(config_path, 'r', encoding='utf-8') as f:
        return yaml.safe_load(f)


def telnet_connect(ip, port, timeout=30):
    """建立Telnet连接"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(timeout)
    sock.connect((ip, port))
    return sock


def telnet_read_until(sock, expected, timeout=10):
    """读取数据直到匹配到期望的字符串"""
    buffer = b""
    start_time = time.time()

    while time.time() - start_time < timeout:
        try:
            sock.settimeout(1)
            data = sock.recv(4096)
            if not data:
                break
            buffer += data
            if expected.encode() in buffer:
                return buffer.decode('utf-8', errors='ignore')
        except socket.timeout:
            if expected.encode() in buffer:
                return buffer.decode('utf-8', errors='ignore')
            continue
        except Exception as e:
            print(f"[错误] 读取数据失败: {e}")
            break

    return buffer.decode('utf-8', errors='ignore')


def telnet_read_available(sock, timeout=1):
    """读取所有可用数据"""
    buffer = b""
    sock.settimeout(timeout)

    try:
        while True:
            data = sock.recv(4096)
            if not data:
                break
            buffer += data
    except socket.timeout:
        pass
    except Exception:
        pass

    return buffer.decode('utf-8', errors='ignore')


def telnet_login_and_execute(ip, port, username, password, command, timeout=600):
    """
    通过Telnet登录并执行命令

    Returns:
        (success, output)
    """
    output_lines = []
    sock = None

    try:
        print(f"[Telnet] 连接到 {ip}:{port}")
        sock = telnet_connect(ip, port)

        # 等待login提示
        print("[Telnet] 等待登录提示...")
        response = telnet_read_until(sock, "login:", timeout=10)
        output_lines.append(response)

        # 发送用户名
        print(f"[Telnet] 发送用户名: {username}")
        sock.sendall((username + "\n").encode('ascii'))
        time.sleep(0.5)

        # 等待password提示
        print("[Telnet] 等待密码提示...")
        response = telnet_read_until(sock, "password:", timeout=10)
        output_lines.append(response)

        # 发送密码
        print("[Telnet] 发送密码")
        sock.sendall((password + "\n").encode('ascii'))
        time.sleep(1)

        # 等待shell提示符
        print("[Telnet] 等待shell提示符...")
        response = telnet_read_until(sock, "#", timeout=10)
        output_lines.append(response)

        # 发送测试命令
        print(f"[Telnet] 执行命令: {command}")
        sock.sendall((command + "\n").encode('ascii'))
        time.sleep(1)

        # 读取输出直到超时或找到结束标记
        print(f"[Telnet] 等待命令完成 (超时: {timeout}秒)...")
        start_time = time.time()

        while time.time() - start_time < timeout:
            try:
                # 尝试读取数据
                data = telnet_read_available(sock, timeout=0.5)
                if data:
                    output_lines.append(data)
                    print(data, end='', flush=True)

                    # 检查是否完成
                    if "Final Score" in data or "final score" in data.lower():
                        print("\n[Telnet] 检测到测试完成标记")
                        time.sleep(2)  # 等待剩余输出
                        data = telnet_read_available(sock, timeout=1)
                        if data:
                            output_lines.append(data)
                            print(data, end='', flush=True)
                        break

                    # 检查是否有错误
                    if any(kw in data.lower() for kw in ["error", "crash", "abort", "exception"]):
                        print("\n[Telnet] 检测到错误信息")

                time.sleep(0.5)

            except Exception as e:
                print(f"\n[Telnet] 读取异常: {e}")
                break

        # 关闭连接
        try:
            sock.sendall(b"exit\n")
            time.sleep(0.5)
        except:
            pass

        if sock:
            sock.close()

        full_output = ''.join(output_lines)

        # 判断是否成功
        success = "Final Score" in full_output or "final score" in full_output.lower()

        return (success, full_output)

    except Exception as e:
        error_msg = f"Telnet连接失败: {str(e)}"
        print(f"\n[错误] {error_msg}")
        if sock:
            sock.close()
        return (False, error_msg)


def main():
    parser = argparse.ArgumentParser(description='Telnet测试脚本')
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

    print(f"板卡: {board['name']}")
    print(f"系统: {board['os']}")
    print(f"架构: {board['arch']}")
    print(f"连接: {conn['ip']}:{conn['port']}")
    print()

    # 获取测试命令
    if args.test not in test_cfg['commands']:
        print(f"错误: 未知的测试命令: {args.test}")
        print(f"可用命令: {', '.join(test_cfg['commands'].keys())}")
        sys.exit(1)

    command = test_cfg['commands'][args.test]
    timeout = test_cfg.get('timeout_sec', 600)

    # 执行测试
    success, output = telnet_login_and_execute(
        ip=conn['ip'],
        port=conn['port'],
        username=conn['username'],
        password=conn['password'],
        command=command,
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
