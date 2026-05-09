#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
通过SSH跳板机的Telnet测试脚本 (使用subprocess和管道)

Usage:
    python telnet_test_simple.py -c boards/board_feiteng.yaml -t test_schedule_quick
"""

import argparse
import sys
import time
import yaml
import subprocess
import threading
from pathlib import Path


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

    # 构建SSH命令，使用bash脚本自动化telnet交互
    bash_script = f'''
set -e

# 使用expect自动化telnet交互
expect << 'EXPECT_EOF'
set timeout {timeout}

# 执行telnet
spawn telnet {board_ip} {board_port}

# 等待login提示
expect {{
    "login:" {{}}
    timeout {{
        puts "\\n[ERROR] 等待login提示超时"
        exit 1
    }}
}}

# 发送用户名
send "{username}\\r"

# 等待password提示
expect {{
    "password:" {{}}
    timeout {{
        puts "\\n[ERROR] 等待password提示超时"
        exit 1
    }}
}}

# 发送密码
send "{password}\\r"

# 等待shell提示符
expect {{
    "#" {{}}
    timeout {{
        puts "\\n[ERROR] 等待shell提示符超时"
        exit 1
    }}
}}

# 发送测试命令
send "{command}\\r"

# 等待测试完成
expect {{
    "Final Score" {{
        puts "\\n[SUCCESS] 测试完成"
    }}
    timeout {{
        puts "\\n[ERROR] 测试执行超时"
        exit 1
    }}
}}

# 等待额外输出
sleep 2

# 退出
send "exit\\r"
expect eof
EXPECT_EOF
'''

    try:
        print("[1/2] 连接到SSH跳板机并执行测试...")
        print("="*80)

        # 执行SSH命令
        ssh_cmd = [
            'ssh',
            '-p', str(jumphost_port),
            jumphost,
            bash_script
        ]

        result = subprocess.run(
            ssh_cmd,
            capture_output=True,
            text=True,
            timeout=timeout + 60,
            encoding='utf-8',
            errors='ignore'
        )

        output = result.stdout + "\n" + result.stderr
        print(output)

        # 判断是否成功
        success = "[SUCCESS]" in output and "Final Score" in output

        print("\n[2/2] 测试完成")
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
