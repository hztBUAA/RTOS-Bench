#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
通过跳板机部署RTOS-Bench到SylixOS板卡

使用SSH跳板机 -> FTP方式上传文件
"""

import paramiko
import os
import sys
import time
import re

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
if hasattr(sys.stderr, "reconfigure"):
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))


def resolve_local_file(path):
    """Resolve local build artifacts from the workspace, regardless of cwd."""
    if os.path.isabs(path):
        return path

    roots = []
    env_root = os.environ.get("YIHUI_WORKSPACE")
    if env_root:
        roots.append(env_root)
    roots.extend([
        os.getcwd(),
        SCRIPT_DIR,
        os.path.abspath(os.path.join(SCRIPT_DIR, "..", "..", "..", "..")),
    ])

    seen = set()
    for root in roots:
        root = os.path.abspath(root)
        if root in seen:
            continue
        seen.add(root)
        candidate = os.path.join(root, path)
        if os.path.exists(candidate):
            return candidate

    return os.path.abspath(path)

# 跳板机配置
JUMPHOST = {
    "host": os.environ.get("RTBENCH_JUMPHOST_HOST", "10.134.151.45"),
    "port": int(os.environ.get("RTBENCH_JUMPHOST_PORT", "1026")),
    "username": os.environ.get("RTBENCH_JUMPHOST_USER", "rtbench"),
    "password": os.environ.get("RTBENCH_JUMPHOST_PASSWORD", ""),
}

# 板卡配置
BOARDS = {
    "nezha": {
        "ip": "192.168.31.202",
        "ftp_port": 21,
        "telnet_port": 23,
        "local_binary": "nezha-rtos-bench/Debug/strip/nezha-rtos-bench",
        "remote_dir": "/apps/hzt/",
        "remote_name": "nezha-rtos-bench",
        "test_cmd": "/apps/hzt/nezha-rtos-bench test-schedule --cycles 3"
    },
    "gongkong": {
        # SOP 中 SylixOS 工控机是 192.168.31.203；192.168.31.205 是 OneOS 飞腾派。
        "ip": "192.168.31.203",
        "ftp_port": 21,
        "telnet_port": 23,
        "local_binary": "gongkong_rtos_bench/Debug/strip/gongkong_rtos_bench",
        "remote_dir": "/apps/hzt/",
        "remote_name": "gongkong-rtos-bench",
        "test_cmd": "/apps/hzt/gongkong-rtos-bench test-schedule --cycles 3"
    },
    "feiteng": {
        "ip": "192.168.31.204",
        "ftp_port": 21,
        "telnet_port": 23,
        "local_binary": "feiteng_rtos_bench/Debug/strip/feiteng_rtos_bench",
        "remote_dir": "/apps/hzt/",
        "remote_name": "feiteng-rtos-bench",
        "test_cmd": "/apps/hzt/feiteng-rtos-bench test-schedule --cycles 3"
    },
    "orangepi": {
        "ip": "192.168.31.201",
        "ftp_port": 21,
        "telnet_port": 23,
        # 香橙派和飞腾派都是aarch64，使用同一个二进制
        "local_binary": "feiteng_rtos_bench/Debug/strip/feiteng_rtos_bench",
        "remote_dir": "/apps/hzt/",
        "remote_name": "orangepi-rtos-bench",
        "test_cmd": "/apps/hzt/orangepi-rtos-bench test-schedule --cycles 3"
    },
    "loongson": {
        "ip": "192.168.31.200",
        "ftp_port": 21,
        "telnet_port": 23,
        "local_binary": "rtos-bench/Debug/strip/rtos-bench",
        "remote_dir": "/apps/hzt/",
        "remote_name": "loongson-rtos-bench",
        "test_cmd": "/apps/hzt/loongson-rtos-bench test-schedule --cycles 3"
    }
}

REMOTE_NAME_SUFFIX = os.environ.get("RTBENCH_REMOTE_SUFFIX", "")
if REMOTE_NAME_SUFFIX:
    for _board in BOARDS.values():
        _board["remote_name"] = f"{_board['remote_name']}{REMOTE_NAME_SUFFIX}"
        _remote_path = f"{_board['remote_dir'].rstrip('/')}/{_board['remote_name']}"
        _board["test_cmd"] = f"{_remote_path} test-schedule --cycles 3"


PROMPT_RE = r"(\[root@sylixos:.*\]#|sh\s+/\w+>)\s*$"


def connect_jumphost():
    """连接跳板机。"""
    if not JUMPHOST["password"]:
        raise RuntimeError("Set RTBENCH_JUMPHOST_PASSWORD before connecting to the jump host")

    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh.connect(
        JUMPHOST["host"],
        port=JUMPHOST["port"],
        username=JUMPHOST["username"],
        password=JUMPHOST["password"],
        timeout=30,
    )
    return ssh


def read_until(channel, patterns, timeout=30, echo=True, log_handle=None):
    """从交互 channel 读取，直到匹配任一正则或超时。"""
    compiled = [re.compile(pattern, re.I | re.M) for pattern in patterns]
    deadline = time.time() + timeout
    output = ""

    while time.time() < deadline:
        if channel.recv_ready():
            chunk = channel.recv(4096).decode("utf-8", errors="replace")
            output += chunk
            if echo:
                print(chunk, end="", flush=True)
            if log_handle:
                log_handle.write(chunk)
                log_handle.flush()
            if any(pattern.search(output) for pattern in compiled):
                return output, True
        else:
            if channel.exit_status_ready():
                break
            time.sleep(0.05)

    return output, False


def send_line(channel, text):
    channel.send(text + "\r")
    time.sleep(0.15)


def open_board_telnet(ssh, board, timeout=20):
    """通过跳板机 telnet 到板卡并登录 root/root。"""
    channel = ssh.get_transport().open_session()
    channel.get_pty(width=180, height=60)
    channel.exec_command(f"telnet {board['ip']} {board['telnet_port']}")

    output, matched = read_until(
        channel,
        [r"login:", r"password:", PROMPT_RE, r"server is full", r"Connection closed"],
        timeout=timeout,
    )
    if "server is full" in output or "Connection closed" in output:
        channel.close()
        raise RuntimeError("telnet server refused connection (server is full or closed)")

    if re.search(r"login:", output, re.I):
        send_line(channel, "root")
        output, _ = read_until(channel, [r"password:", PROMPT_RE], timeout=8)

    if re.search(r"password:", output, re.I):
        send_line(channel, "root")
        output, matched = read_until(channel, [PROMPT_RE], timeout=12)

    if not matched and not re.search(PROMPT_RE, output, re.I | re.M):
        channel.close()
        raise RuntimeError("telnet login did not reach shell prompt")

    return channel


def run_board_commands(ssh, board, commands, timeout_per_command=30):
    """在板卡 shell 中执行短命令。"""
    channel = open_board_telnet(ssh, board)
    try:
        for command in commands:
            print(f"\n>>> {command}")
            send_line(channel, command)
            _, matched = read_until(channel, [PROMPT_RE], timeout=timeout_per_command)
            if not matched:
                raise TimeoutError(f"command timed out: {command}")
    finally:
        try:
            send_line(channel, "exit")
            time.sleep(0.2)
        finally:
            channel.close()


def deploy_to_board(board_name):
    """部署到指定板卡"""
    if board_name not in BOARDS:
        print(f"[ERROR] Unknown board: {board_name}")
        print(f"Available boards: {', '.join(BOARDS.keys())}")
        return False

    board = BOARDS[board_name]
    local_file = resolve_local_file(board["local_binary"])
    remote_dir = board["remote_dir"]
    remote_name = board["remote_name"]
    remote_path = f"{remote_dir}{remote_name}"

    print(f"\n{'='*60}")
    print(f"部署 {board_name} 板卡")
    print(f"{'='*60}")
    print(f"  本地文件: {local_file}")
    print(f"  远程路径: {remote_path}")
    print(f"  板卡IP: {board['ip']}")

    # 检查本地文件
    if not os.path.exists(local_file):
        print(f"  [ERROR] 本地文件不存在!")
        return False

    file_size = os.path.getsize(local_file)
    print(f"  文件大小: {file_size / 1024 / 1024:.2f} MB")

    ssh = None
    try:
        # 连接跳板机
        print(f"\n[1/4] 连接跳板机 {JUMPHOST['host']}:{JUMPHOST['port']}...")
        ssh = connect_jumphost()
        print(f"  [OK] 跳板机连接成功")

        # 上传文件到跳板机
        print(f"\n[2/4] 上传文件到跳板机...")
        sftp = ssh.open_sftp()
        temp_path = f"/tmp/{board_name}_rtos_bench"
        sftp.put(local_file, temp_path)
        sftp.chmod(temp_path, 0o755)
        print(f"  [OK] 已上传到: {temp_path}")

        # 检查板卡连通性
        print(f"\n[3/4] 检查板卡连通性...")
        stdin, stdout, stderr = ssh.exec_command(
            f"ping -c 1 -W 2 {board['ip']} > /dev/null 2>&1 && echo OK || echo FAIL",
            timeout=10
        )
        ping_result = stdout.read().decode().strip()
        if ping_result != "OK":
            print(f"  [ERROR] 板卡 {board['ip']} 不可达")
            ssh.close()
            return False
        print("  [OK] 板卡可达")

        # 通过FTP上传到板卡
        print(f"\n[4/4] FTP上传到板卡...")

        # 使用curl上传
        ftp_cmd = (
            f"curl -T {temp_path} 'ftp://{board['ip']}{remote_path}' "
            "--user root:root --connect-timeout 10 --max-time 180 "
            "--ftp-create-dirs --fail --show-error"
        )
        print(f"  命令: {ftp_cmd}")

        stdin, stdout, stderr = ssh.exec_command(ftp_cmd, timeout=240)
        stdout_text = stdout.read().decode('utf-8', errors='replace')
        stderr_text = stderr.read().decode('utf-8', errors='replace')
        exit_status = stdout.channel.recv_exit_status()

        if exit_status == 0:
            print(f"  [OK] FTP上传成功!")
            if stdout_text:
                print(stdout_text, end="")
            if stderr_text:
                print(stderr_text)

            # 设置执行权限并核对文件。跳板机没有 expect，所以直接驱动 telnet。
            try:
                run_board_commands(ssh, board, [f"chmod 755 {remote_path}", f"ll {remote_path}"])
            except Exception as telnet_error:
                print(f"  [WARN] FTP已成功，但telnet权限/核对步骤失败: {telnet_error}")

            list_cmd = (
                f"curl 'ftp://{board['ip']}{remote_dir}' --user root:root "
                "--connect-timeout 10 --max-time 30 --fail --show-error"
            )
            stdin, stdout, stderr = ssh.exec_command(list_cmd, timeout=45)
            listing = stdout.read().decode('utf-8', errors='replace')
            list_err = stderr.read().decode('utf-8', errors='replace')
            if listing:
                print("  FTP目录核对:")
                print(listing)
            if list_err:
                print(list_err)

            # 清理临时文件
            ssh.exec_command(f"rm -f {temp_path}")
            print(f"\n[SUCCESS] 部署完成!")
            print(f"  测试命令: {board['test_cmd']}")

            ssh.close()
            return True
        else:
            print(f"  [ERROR] FTP上传失败 (exit={exit_status})")
            if stderr_text:
                print(f"  错误: {stderr_text[:200]}")
            ssh.close()
            return False

    except Exception as e:
        print(f"  [ERROR] 部署失败: {e}")
        import traceback
        traceback.print_exc()
        return False
    finally:
        if ssh:
            ssh.close()


def run_test_on_board(board_name, cycles=None, log_dir=None):
    """在板卡上运行测试"""
    if board_name not in BOARDS:
        print(f"[ERROR] Unknown board: {board_name}")
        return False

    board = BOARDS[board_name]
    test_cmd = board["test_cmd"]
    if cycles is not None:
        test_cmd = re.sub(r"--cycles\s+\d+", f"--cycles {cycles}", test_cmd)
        if "--cycles" not in test_cmd:
            test_cmd = f"{test_cmd} --cycles {cycles}"

    print(f"\n{'='*60}")
    print(f"运行测试: {board_name}")
    print(f"命令: {test_cmd}")
    print(f"{'='*60}")

    ssh = None
    channel = None
    try:
        # 连接跳板机
        ssh = connect_jumphost()
        channel = open_board_telnet(ssh, board)

        log_name = f"test_{board_name}_{time.strftime('%Y%m%d_%H%M%S')}.log"
        if log_dir:
            os.makedirs(log_dir, exist_ok=True)
            log_name = os.path.join(log_dir, log_name)
        print(f"执行测试中，请等待... 日志: {log_name}")
        send_line(channel, test_cmd)

        output = ""
        final_seen = False
        timeout_seconds = 1800
        deadline = time.time() + timeout_seconds
        bad_markers = [
            "timer_create failed",
            "skipped for quick schedule",
            "Can not find dependent library",
        ]

        with open(log_name, "w", encoding="utf-8") as log_handle:
            log_handle.write(f">>> {test_cmd}\n")
            while time.time() < deadline:
                if channel.recv_ready():
                    chunk = channel.recv(4096).decode("utf-8", errors="replace")
                    output += chunk
                    print(chunk, end="", flush=True)
                    log_handle.write(chunk)
                    log_handle.flush()
                    if "Final Score" in chunk:
                        final_seen = True
                    if final_seen and re.search(PROMPT_RE, output, re.I | re.M):
                        break
                    if re.search(PROMPT_RE, output, re.I | re.M) and not final_seen:
                        break
                else:
                    time.sleep(0.1)

        marker_hits = [marker for marker in bad_markers if marker in output]
        if marker_hits:
            print(f"\n[FAIL] 发现异常标记: {', '.join(marker_hits)}")
        if not final_seen:
            print("\n[FAIL] 未看到 Final Score，测试可能超时或提前失败")

        return final_seen and not marker_hits

    except Exception as e:
        print(f"[ERROR] 测试失败: {e}")
        return False
    finally:
        if channel:
            try:
                send_line(channel, "exit")
            except Exception:
                pass
            channel.close()
        if ssh:
            ssh.close()


def main():
    if len(sys.argv) < 2:
        print("用法:")
        print("  python deploy_via_jumphost.py deploy <board>   - 部署到板卡")
        print("  python deploy_via_jumphost.py test <board>     - 运行测试")
        print("  python deploy_via_jumphost.py all <board>      - 部署并测试")
        print(f"\n可用板卡: {', '.join(BOARDS.keys())}")
        sys.exit(1)

    action = sys.argv[1]
    board = sys.argv[2] if len(sys.argv) > 2 else "nezha"
    cycles = None
    log_dir = None
    if len(sys.argv) > 3:
        try:
            cycles = int(sys.argv[3])
        except ValueError:
            log_dir = sys.argv[3]
    if len(sys.argv) > 4:
        log_dir = sys.argv[4]

    if action == "deploy":
        success = deploy_to_board(board)
        sys.exit(0 if success else 1)
    elif action == "test":
        success = run_test_on_board(board, cycles=cycles, log_dir=log_dir)
        sys.exit(0 if success else 1)
    elif action == "all":
        if deploy_to_board(board):
            run_test_on_board(board, cycles=cycles, log_dir=log_dir)
    else:
        print(f"未知操作: {action}")
        sys.exit(1)


if __name__ == "__main__":
    main()
