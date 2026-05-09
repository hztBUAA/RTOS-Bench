#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
通过SSH跳板机的Telnet测试脚本 (使用pexpect)

Usage:
    python telnet_test_jumphost.py -c boards/board_feiteng.yaml -t test_schedule_quick
"""

import argparse
import sys
import time
import yaml
from pathlib import Path

# Windows兼容性处理
try:
    import wexpect
    pexpect = wexpect
    TIMEOUT = wexpect.TIMEOUT
    EOF = wexpect.EOF
except ImportError:
    import pexpect
    TIMEOUT = pexpect.TIMEOUT
    EOF = pexpect.EOF


def load_board_config(config_path):
    """加载板卡配置文件"""
    with open(config_path, 'r', encoding='utf-8') as f:
        return yaml.safe_load(f)


def execute_via_jumphost(board_ip, board_port, username, password, command, timeout=600):
    """
    通过SSH跳板机执行telnet命令

    Returns:
        (success, output)
    """
    # SSH跳板机配置
    jumphost = "rtbench@10.134.151.45"
    jumphost_port = 1026

    print(f"[INFO] 通过SSH跳板机连接到板卡")
    print(f"[INFO] 跳板机: {jumphost}:{jumphost_port}")
    print(f"[INFO] 目标板卡: {board_ip}:{board_port}")
    print(f"[INFO] 执行命令: {command}")
    print()

    output_lines = []

    try:
        # 连接到SSH跳板机
        print("[1/6] 连接到SSH跳板机...")
        ssh_cmd = f"ssh -p {jumphost_port} {jumphost}"
        child = pexpect.spawn(ssh_cmd, encoding='utf-8', timeout=30)
        child.logfile_read = sys.stdout

        # 等待跳板机shell提示符
        print("[2/6] 等待跳板机shell提示符...")
        child.expect(['\\$', '\\#'], timeout=10)

        # 执行telnet命令
        print(f"[3/6] 执行telnet命令...")
        child.sendline(f"telnet {board_ip} {board_port}")
        time.sleep(1)

        # 等待login提示
        print("[4/6] 等待login提示...")
        child.expect('login:', timeout=10)

        # 发送用户名
        print(f"[5/6] 发送用户名: {username}")
        child.sendline(username)
        time.sleep(0.5)

        # 等待password提示
        child.expect('password:', timeout=10)

        # 发送密码
        print(f"[6/6] 发送密码并执行测试命令...")
        child.sendline(password)
        time.sleep(1)

        # 等待shell提示符
        child.expect('#', timeout=10)

        # 发送测试命令
        print(f"\n[TEST] 开始执行测试: {command}")
        print("="*80)
        child.sendline(command)

        # 等待测试完成
        print(f"\n等待测试完成 (超时: {timeout}秒)...\n")
        start_time = time.time()

        while time.time() - start_time < timeout:
            try:
                # 读取输出
                index = child.expect(['Final Score', TIMEOUT], timeout=5)

                if index == 0:
                    # 找到Final Score
                    print("\n[SUCCESS] 检测到测试完成标记!")
                    # 继续读取剩余输出
                    time.sleep(2)
                    try:
                        child.expect(TIMEOUT, timeout=1)
                    except:
                        pass
                    break

            except EOF:
                print("\n[ERROR] 连接意外断开")
                break
            except TIMEOUT:
                # 继续等待
                continue

        # 获取所有输出
        output = child.before + child.after if hasattr(child, 'before') else ""

        # 退出
        try:
            child.sendline("exit")
            time.sleep(0.5)
            child.sendline("exit")
            child.close()
        except:
            pass

        # 判断是否成功
        success = "Final Score" in output or "final score" in output.lower()

        return (success, output)

    except TIMEOUT as e:
        error_msg = f"超时错误: {str(e)}"
        print(f"\n[ERROR] {error_msg}")
        try:
            child.close()
        except:
            pass
        return (False, error_msg)

    except EOF as e:
        error_msg = f"连接断开: {str(e)}"
        print(f"\n[ERROR] {error_msg}")
        return (False, error_msg)

    except Exception as e:
        error_msg = f"执行失败: {str(e)}"
        print(f"\n[ERROR] {error_msg}")
        try:
            child.close()
        except:
            pass
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

    # 根据配置文件中的连接信息，转换为内网IP
    # 10.134.151.3:3020 -> 192.168.31.204:23
    port_mapping = {
        3020: ('192.168.31.204', 23),  # 飞腾派
        3014: ('192.168.31.202', 23),  # 哪吒
        3017: ('192.168.31.203', 23),  # 工控机
        3008: ('192.168.31.200', 23),  # 龙芯
        3011: ('192.168.31.201', 23),  # 香橙派
    }

    external_port = conn['port']
    if external_port in port_mapping:
        board_ip, board_port = port_mapping[external_port]
        print(f"[INFO] 端口映射: {conn['ip']}:{external_port} -> {board_ip}:{board_port}")
        print()
    else:
        print(f"错误: 未知的端口映射: {external_port}")
        sys.exit(1)

    # 执行测试
    success, output = execute_via_jumphost(
        board_ip=board_ip,
        board_port=board_port,
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
