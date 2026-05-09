#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
翼辉(SylixOS)板卡 test-schedule 自动化测试脚本

按照SOP文档自动测试所有翼辉板卡的test-schedule命令
"""

import os
import sys
import subprocess
import time
from datetime import datetime
from pathlib import Path

# 板卡配置列表
YIHUI_BOARDS = [
    {
        "name": "飞腾派 (Phytium E2000Q)",
        "config": "boards/board_feiteng.yaml",
        "command": "test_schedule_quick",
        "status": "[OK] 已验证 (2026-04-16)",
    },
    {
        "name": "哪吒 (Nezha D1 RISC-V)",
        "config": "boards/board_nezha.yaml",
        "command": "test_schedule_quick",
        "status": "[PENDING] 待验证",
    },
    {
        "name": "工控机 (MH7700 x86_64)",
        "config": "boards/board_gongkong.yaml",
        "command": "test_schedule_quick",
        "status": "[PENDING] 待验证",
    },
    {
        "name": "龙芯 (LS2K1000 LoongArch)",
        "config": "boards/board_loongson.yaml",
        "command": "test_schedule_quick",
        "status": "[PENDING] 待验证",
    },
    {
        "name": "香橙派 (RK3588 ARM64)",
        "config": "boards/board_orangepi.yaml",
        "command": "test_schedule_quick",
        "status": "[PENDING] 待验证",
    },
]

# 脚本目录
SCRIPT_DIR = Path(__file__).parent.absolute()
UTILS_DIR = SCRIPT_DIR.parent


def print_header(text):
    """打印标题"""
    print("\n" + "="*80)
    print(text.center(80))
    print("="*80 + "\n")


def print_board_info(board, index, total):
    """打印板卡信息"""
    print(f"\n[{index}/{total}] 测试板卡: {board['name']}")
    print(f"    配置文件: {board['config']}")
    print(f"    测试命令: {board['command']}")
    print(f"    当前状态: {board['status']}")


def run_telnet_test(config_path, command):
    """
    运行telnet测试

    Args:
        config_path: 板卡配置文件路径
        command: 测试命令 (test_schedule_quick/test_schedule_full)

    Returns:
        (success, output, error)
    """
    # 构建命令 - 使用远程Python脚本版本
    cmd = [
        sys.executable,
        str(SCRIPT_DIR / "telnet_test_remote.py"),
        "-c", str(config_path),
        "-t", command,
    ]

    print(f"\n执行命令: {' '.join(cmd)}\n")

    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=600,  # 10分钟超时
            cwd=str(SCRIPT_DIR),
            encoding='utf-8',
            errors='ignore',
        )

        return (result.returncode == 0, result.stdout, result.stderr)

    except subprocess.TimeoutExpired:
        return (False, "", "测试超时 (600秒)")
    except Exception as e:
        return (False, "", f"执行失败: {str(e)}")


def parse_test_result(output):
    """
    解析测试输出，提取关键信息

    Returns:
        dict with keys: success, score, error_msg
    """
    result = {
        "success": False,
        "score": None,
        "error_msg": None,
    }

    # 查找 Final Score
    for line in output.split('\n'):
        if "Final Score" in line or "final score" in line.lower():
            try:
                # 提取分数 (格式: "Final Score: 100.00/100" 或 "Final Score: 100.00 / 100")
                parts = line.split(':')
                if len(parts) >= 2:
                    score_str = parts[1].strip().split('/')[0].strip()
                    result["score"] = float(score_str)
                    result["success"] = True
            except:
                pass

    # 查找错误信息
    error_keywords = ["error", "fail", "crash", "abort", "timeout", "exception"]
    for line in output.split('\n'):
        line_lower = line.lower()
        if any(kw in line_lower for kw in error_keywords):
            if not result["error_msg"]:
                result["error_msg"] = line.strip()

    return result


def main():
    """主函数"""
    print_header("翼辉(SylixOS)板卡 test-schedule 自动化测试")

    print(f"测试时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"工作目录: {SCRIPT_DIR}")
    print(f"板卡数量: {len(YIHUI_BOARDS)}")

    # 测试结果汇总
    results = []

    # 逐个测试板卡
    for i, board in enumerate(YIHUI_BOARDS, 1):
        print_board_info(board, i, len(YIHUI_BOARDS))

        config_path = SCRIPT_DIR / board["config"]

        # 检查配置文件是否存在
        if not config_path.exists():
            print(f"[X] 配置文件不存在: {config_path}")
            results.append({
                "board": board["name"],
                "success": False,
                "score": None,
                "error": "配置文件不存在",
            })
            continue

        # 运行测试
        print(f"开始测试...")
        start_time = time.time()

        success, stdout, stderr = run_telnet_test(config_path, board["command"])

        elapsed = time.time() - start_time

        # 解析结果
        if success:
            test_result = parse_test_result(stdout)

            if test_result["success"]:
                print(f"[OK] 测试成功! 得分: {test_result['score']}/100")
                print(f"  耗时: {elapsed:.1f}秒")
                results.append({
                    "board": board["name"],
                    "success": True,
                    "score": test_result["score"],
                    "error": None,
                    "elapsed": elapsed,
                })
            else:
                print(f"[WARN] 测试完成但未找到分数")
                if test_result["error_msg"]:
                    print(f"  错误信息: {test_result['error_msg']}")
                results.append({
                    "board": board["name"],
                    "success": False,
                    "score": None,
                    "error": test_result["error_msg"] or "未找到分数",
                    "elapsed": elapsed,
                })
        else:
            print(f"[FAIL] 测试失败")
            if stderr:
                print(f"  错误: {stderr[:200]}")
            results.append({
                "board": board["name"],
                "success": False,
                "score": None,
                "error": stderr or "未知错误",
                "elapsed": elapsed,
            })

        # 打印部分输出（最后20行）
        if stdout:
            lines = stdout.strip().split('\n')
            if len(lines) > 20:
                print(f"\n输出摘要 (最后20行):")
                for line in lines[-20:]:
                    print(f"  {line}")

        # 板卡之间间隔
        if i < len(YIHUI_BOARDS):
            print(f"\n等待5秒后测试下一个板卡...")
            time.sleep(5)

    # 打印汇总报告
    print_header("测试结果汇总")

    success_count = sum(1 for r in results if r["success"])
    fail_count = len(results) - success_count

    print(f"总计: {len(results)} 个板卡")
    print(f"成功: {success_count}")
    print(f"失败: {fail_count}")
    print()

    # 详细结果表格
    print(f"{'板卡':<40} {'状态':<15} {'得分':<15} {'耗时':<10}")
    print("-" * 80)

    for r in results:
        status = "[OK] 成功" if r["success"] else "[FAIL] 失败"
        score = f"{r['score']:.2f}/100" if r["score"] is not None else "N/A"
        elapsed = f"{r.get('elapsed', 0):.1f}s" if r.get('elapsed') else "N/A"

        print(f"{r['board']:<40} {status:<15} {score:<15} {elapsed:<10}")

        if not r["success"] and r.get("error"):
            error_msg = r["error"][:60]
            print(f"  错误: {error_msg}")

    print()

    # 保存结果到文件
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    result_file = SCRIPT_DIR / f"test_results_{timestamp}.txt"

    with open(result_file, 'w', encoding='utf-8') as f:
        f.write("翼辉(SylixOS)板卡 test-schedule 测试结果\n")
        f.write(f"测试时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        f.write("=" * 80 + "\n\n")

        for r in results:
            f.write(f"板卡: {r['board']}\n")
            f.write(f"状态: {'成功' if r['success'] else '失败'}\n")
            f.write(f"得分: {r['score']:.2f}/100\n" if r['score'] is not None else "得分: N/A\n")
            if r.get('elapsed'):
                f.write(f"耗时: {r['elapsed']:.1f}秒\n")
            if not r['success'] and r.get('error'):
                f.write(f"错误: {r['error']}\n")
            f.write("\n")

    print(f"结果已保存到: {result_file}")

    # 返回退出码
    sys.exit(0 if fail_count == 0 else 1)


if __name__ == "__main__":
    main()
