#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
简化的单板测试脚本 - 用于调试
"""

import yaml
import subprocess
import tempfile
import time
import sys
from pathlib import Path

def test_board():
    # 读取配置
    config_path = Path("boards/board_feiteng.yaml")
    with open(config_path, 'r', encoding='utf-8') as f:
        config = yaml.safe_load(f)

    print("配置读取成功:")
    print(f"  板卡: {config['board']['name']}")
    print(f"  跳板机: {config['jumphost']['username']}@{config['jumphost']['host']}:{config['jumphost']['port']}")
    print(f"  目标: {config['connection']['ip']}:{config['connection']['port']}")
    print()

    # 创建简单的测试脚本
    test_script = '''#!/usr/bin/env python3
import socket
import time

print("开始连接到板卡...")
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.settimeout(10)
try:
    sock.connect(('192.168.31.204', 23))
    print("连接成功!")
    sock.close()
except Exception as e:
    print(f"连接失败: {e}")
'''

    # 保存到临时文件
    with tempfile.NamedTemporaryFile(mode='w', suffix='.py', delete=False, encoding='utf-8') as f:
        f.write(test_script)
        local_script = f.name

    print(f"临时脚本: {local_script}")

    # 上传到跳板机
    remote_path = f"/tmp/test_{int(time.time())}.py"
    jumphost = f"{config['jumphost']['username']}@{config['jumphost']['host']}"
    jumphost_port = config['jumphost']['port']

    print(f"\n[1/2] 上传脚本到跳板机...")
    scp_cmd = ['scp', '-P', str(jumphost_port), local_script, f"{jumphost}:{remote_path}"]
    print(f"命令: {' '.join(scp_cmd)}")

    result = subprocess.run(scp_cmd, capture_output=True, text=True, timeout=30, encoding='utf-8', errors='ignore')
    if result.returncode != 0:
        print(f"上传失败: {result.stderr}")
        return False

    print("上传成功!")

    # 在跳板机上执行
    print(f"\n[2/2] 在跳板机上执行测试...")
    ssh_cmd = ['ssh', '-p', str(jumphost_port), jumphost, f"python3 {remote_path}"]
    print(f"命令: {' '.join(ssh_cmd)}")

    result = subprocess.run(ssh_cmd, capture_output=True, text=True, timeout=30, encoding='utf-8', errors='ignore')
    print(f"\n输出:\n{result.stdout}")
    if result.stderr:
        print(f"错误:\n{result.stderr}")

    # 清理
    subprocess.run(['ssh', '-p', str(jumphost_port), jumphost, f"rm -f {remote_path}"],
                   capture_output=True, timeout=10)

    import os
    os.unlink(local_script)

    return result.returncode == 0

if __name__ == "__main__":
    try:
        success = test_board()
        sys.exit(0 if success else 1)
    except Exception as e:
        print(f"\n错误: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
