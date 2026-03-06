#!/bin/bash
# RT-Thread QEMU virt aarch64 (BSP: qemu-virt64-aarch64) 一键测试脚本
# 用法: ./run-rtthread.sh [选项]
#   -b        仅编译，不运行
#   -r        仅运行（使用已有镜像）
#   -t        运行后自动测试 rtbench
#   -h        显示帮助

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
# BSP 目录: "QEMU virt aarch64 (Cortex-A53 x4, 128MB)" 对应 qemu-virt64-aarch64
BSP_DIR="$SCRIPT_DIR/extern/rt-thread/bsp/qemu-virt64-aarch64"
TOOLCHAIN_DIR="$SCRIPT_DIR/extern/toolchains/xpack-aarch64-none-elf-gcc-14.2.1-1.1/bin"

# QEMU 路径（优先使用 brew 安装的版本）
if [ -x "/usr/local/bin/qemu-system-aarch64" ]; then
    QEMU="/usr/local/bin/qemu-system-aarch64"
elif [ -x "/opt/homebrew/bin/qemu-system-aarch64" ]; then
    QEMU="/opt/homebrew/bin/qemu-system-aarch64"
else
    QEMU="qemu-system-aarch64"
fi

BUILD_ONLY=0
RUN_ONLY=0
AUTO_TEST=0

usage() {
    echo "用法: $0 [选项]"
    echo "  -b        仅编译，不运行"
    echo "  -r        仅运行（使用已有镜像）"
    echo "  -t        运行后自动测试 rtbench"
    echo "  -h        显示帮助"
    echo ""
    echo "示例:"
    echo "  $0           # 编译并运行"
    echo "  $0 -b        # 仅编译"
    echo "  $0 -r        # 仅运行"
    echo "  $0 -t        # 运行并自动测试"
}

while getopts "brth" opt; do
    case $opt in
        b) BUILD_ONLY=1 ;;
        r) RUN_ONLY=1 ;;
        t) AUTO_TEST=1 ;;
        h) usage; exit 0 ;;
        *) usage; exit 1 ;;
    esac
done

# 检查依赖
check_dependencies() {
    echo "=== 检查依赖 ==="

    # 检查 QEMU
    if ! command -v "$QEMU" &> /dev/null; then
        echo "错误: 未找到 qemu-system-aarch64"
        echo "请安装: brew install qemu"
        exit 1
    fi
    echo "[OK] QEMU: $QEMU"

    # 检查工具链
    if [ ! -d "$TOOLCHAIN_DIR" ]; then
        echo "错误: 未找到工具链: $TOOLCHAIN_DIR"
        echo "请解压: cd extern/toolchains && tar xzf xpack-aarch64-none-elf-gcc-14.2.1-1.1-darwin-x64.tar.gz"
        exit 1
    fi
    echo "[OK] 工具链: $TOOLCHAIN_DIR"

    # 检查 BSP 目录
    if [ ! -d "$BSP_DIR" ]; then
        echo "错误: 未找到 BSP 目录: $BSP_DIR"
        exit 1
    fi
    echo "[OK] BSP: $BSP_DIR"

    # 检查 scons (仅编译时需要)
    if [ $RUN_ONLY -eq 0 ]; then
        if ! command -v scons &> /dev/null; then
            echo "警告: 未找到 scons，尝试激活 venv..."
            if [ -f "$SCRIPT_DIR/extern/.venv/bin/activate" ]; then
                source "$SCRIPT_DIR/extern/.venv/bin/activate"
            else
                echo "错误: 未找到 scons，请安装: pip install scons"
                exit 1
            fi
        fi
        echo "[OK] scons: $(which scons)"
    fi
}

# 编译 RT-Thread
build_rtthread() {
    echo ""
    echo "=== 编译 RT-Thread ==="

    export RTT_EXEC_PATH="$TOOLCHAIN_DIR"
    export PATH="$RTT_EXEC_PATH:$PATH"

    # 激活 Python 虚拟环境
    if [ -f "$SCRIPT_DIR/extern/.venv/bin/activate" ]; then
        source "$SCRIPT_DIR/extern/.venv/bin/activate"
    fi

    cd "$BSP_DIR"

    echo "工具链: $RTT_EXEC_PATH"
    echo "编译目录: $BSP_DIR"
    echo ""

    scons -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

    if [ $? -ne 0 ]; then
        echo "编译失败!"
        exit 1
    fi

    if [ -f "rtthread.bin" ]; then
        echo ""
        echo "[OK] 编译成功: $BSP_DIR/rtthread.bin"
        ls -la rtthread.bin rtthread.elf
    else
        echo "错误: 未生成 rtthread.bin"
        exit 1
    fi
}

# 运行 RT-Thread
run_rtthread() {
    echo ""
    echo "=== 运行 RT-Thread on QEMU ==="

    cd "$BSP_DIR"

    if [ ! -f "rtthread.bin" ]; then
        echo "错误: 未找到 rtthread.bin，请先编译"
        exit 1
    fi

    # 创建 SD 卡镜像（如果不存在），并格式化为 FAT 文件系统
    if [ ! -f "sd.bin" ]; then
        echo "创建 SD 卡镜像..."
        dd if=/dev/zero of=sd.bin bs=1024 count=65536 2>/dev/null
        mkfs.fat sd.bin >/dev/null 2>&1
    fi

    echo ""
    echo "启动 QEMU..."
    echo "按 Ctrl+A X 退出"
    echo ""
    echo "在 msh 终端中运行:"
    echo "  rtbench test-realtime              # 实时性能测试"
    echo "  rtbench test-schedule --cycles 100 # 可调度性测试"
    echo "  rtbench test-stress -s cpu -t 5    # 压力测试"
    echo "  rtbench test-cmd                   # Shell 命令支持测试"
    echo "  rtbench test-all                   # 运行所有测试"
    echo "  rtbench -b busywait -p 0.5 -t 3 -q # 周期负载"
    echo ""

    if [ $AUTO_TEST -eq 1 ]; then
        # 自动测试模式：使用 expect 或 FIFO
        run_auto_test
    else
        # 交互模式
        "$QEMU" -M virt,gic-version=2 -cpu cortex-a53 -m 128M -smp 4 \
            -kernel rtthread.bin -nographic \
            -drive if=none,file=sd.bin,format=raw,id=blk0 \
            -device virtio-blk-device,drive=blk0,bus=virtio-mmio-bus.0
    fi
}

# 自动测试
run_auto_test() {
    echo "=== 自动测试模式 ==="

    FIFO=$(mktemp -u)
    mkfifo "$FIFO"

    # 启动 QEMU 在后台
    ("$QEMU" -M virt,gic-version=2 -cpu cortex-a53 -m 128M -smp 4 \
        -kernel rtthread.bin -nographic \
        -drive if=none,file=sd.bin,format=raw,id=blk0 \
        -device virtio-blk-device,drive=blk0,bus=virtio-mmio-bus.0 < "$FIFO" 2>&1 &)

    QEMU_PID=$!
    exec 3>"$FIFO"

    echo "等待 RT-Thread 启动..."
    sleep 4

    echo "执行测试命令..."
    echo "" >&3
    sleep 1

    # 显示帮助确认 rtbench 命令已注册
    echo "help rtbench" >&3
    sleep 2

    # 运行 busywait benchmark (-q 静默输出)
    echo "rtbench -p 0.5 -b busywait -t 2 -q" >&3
    sleep 8

    echo "" >&3

    # 运行 stub benchmark
    echo "rtbench -p 0.3 -b stub -t 2 -q" >&3
    sleep 5

    # 实时性能测试
    echo "rtbench test-realtime" >&3
    sleep 10

    # 压力测试（CPU, 5秒）
    echo "rtbench test-stress -s cpu -t 5" >&3
    sleep 8

    # Shell 命令支持测试
    echo "rtbench test-cmd" >&3
    sleep 5

    # 清理
    exec 3>&-
    rm -f "$FIFO"

    echo ""
    echo "=== 测试完成，按 Enter 结束 QEMU ==="
    read -r
    pkill -f "qemu-system-aarch64.*rtthread.bin" 2>/dev/null
}

# 主流程
check_dependencies

if [ $RUN_ONLY -eq 0 ]; then
    build_rtthread
fi

if [ $BUILD_ONLY -eq 0 ]; then
    run_rtthread
fi

echo ""
echo "=== 完成 ==="
