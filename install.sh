#!/bin/bash
# RTOS-Bench 环境安装脚本
# 用途: 一键配置 RT-Thread (QEMU aarch64) 开发环境
# 用法: ./install.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXTERN_DIR="$SCRIPT_DIR/extern"
TOOLCHAIN_DIR="$EXTERN_DIR/toolchains"
RTT_DIR="$EXTERN_DIR/rt-thread"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

info() { echo -e "${GREEN}[INFO]${NC} $1"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
error() { echo -e "${RED}[ERROR]${NC} $1"; exit 1; }

# ============================================================================
# 1. 检查系统依赖
# ============================================================================
check_system_deps() {
    info "检查系统依赖..."

    local missing=""

    # 必需工具
    for cmd in git python3 wget tar; do
        if ! command -v $cmd &> /dev/null; then
            missing="$missing $cmd"
        fi
    done

    if [ -n "$missing" ]; then
        error "缺少必需工具:$missing\n请先安装: sudo apt install$missing"
    fi

    # 检查 QEMU
    if ! command -v qemu-system-aarch64 &> /dev/null; then
        warn "未安装 qemu-system-aarch64，将尝试安装..."
        if command -v apt &> /dev/null; then
            sudo apt update && sudo apt install -y qemu-system-arm
        elif command -v dnf &> /dev/null; then
            sudo dnf install -y qemu-system-aarch64
        elif command -v pacman &> /dev/null; then
            sudo pacman -S qemu-system-aarch64
        else
            warn "请手动安装 QEMU: qemu-system-aarch64"
        fi
    fi

    info "系统依赖检查完成"
}

# ============================================================================
# 2. 安装 Python 依赖 (SCons)
# ============================================================================
install_python_deps() {
    info "配置 Python 环境..."

    # 检查是否已有 scons
    if command -v scons &> /dev/null; then
        info "SCons 已安装: $(scons --version | head -1)"
        return 0
    fi

    # 创建虚拟环境
    local VENV_DIR="$EXTERN_DIR/.venv"
    if [ ! -d "$VENV_DIR" ]; then
        info "创建 Python 虚拟环境..."
        python3 -m venv "$VENV_DIR"
    fi

    # 激活并安装 scons
    source "$VENV_DIR/bin/activate"
    pip install --upgrade pip
    pip install scons

    info "SCons 安装完成: $(scons --version | head -1)"
}

# ============================================================================
# 3. 下载 aarch64 工具链
# ============================================================================
install_toolchain() {
    info "检查 aarch64 工具链..."

    local TOOLCHAIN_NAME="xpack-aarch64-none-elf-gcc-14.2.1-1.1"
    local TOOLCHAIN_PATH="$TOOLCHAIN_DIR/$TOOLCHAIN_NAME"

    if [ -d "$TOOLCHAIN_PATH" ]; then
        info "工具链已存在: $TOOLCHAIN_PATH"
        return 0
    fi

    mkdir -p "$TOOLCHAIN_DIR"
    cd "$TOOLCHAIN_DIR"

    # 检测系统架构
    local ARCH=$(uname -m)
    local DOWNLOAD_URL=""

    case "$ARCH" in
        x86_64)
            DOWNLOAD_URL="https://github.com/xpack-dev-tools/aarch64-none-elf-gcc-xpack/releases/download/v14.2.1-1.1/xpack-aarch64-none-elf-gcc-14.2.1-1.1-linux-x64.tar.gz"
            ;;
        aarch64)
            DOWNLOAD_URL="https://github.com/xpack-dev-tools/aarch64-none-elf-gcc-xpack/releases/download/v14.2.1-1.1/xpack-aarch64-none-elf-gcc-14.2.1-1.1-linux-arm64.tar.gz"
            ;;
        *)
            error "不支持的架构: $ARCH"
            ;;
    esac

    info "下载工具链 (约 200MB)..."
    local TARBALL="toolchain.tar.gz"
    wget -q --show-progress -O "$TARBALL" "$DOWNLOAD_URL"

    info "解压工具链..."
    tar xf "$TARBALL"
    rm "$TARBALL"

    # 验证
    if [ -x "$TOOLCHAIN_PATH/bin/aarch64-none-elf-gcc" ]; then
        info "工具链安装成功"
        "$TOOLCHAIN_PATH/bin/aarch64-none-elf-gcc" --version | head -1
    else
        error "工具链安装失败"
    fi

    cd "$SCRIPT_DIR"
}

# ============================================================================
# 4. 克隆/更新 RT-Thread
# ============================================================================
setup_rtthread() {
    info "配置 RT-Thread..."

    if [ -d "$RTT_DIR" ]; then
        info "RT-Thread 已存在，检查更新..."
        cd "$RTT_DIR"
        git fetch origin --depth=1 2>/dev/null || true
        cd "$SCRIPT_DIR"
    else
        info "克隆 RT-Thread (shallow clone)..."
        mkdir -p "$EXTERN_DIR"
        git clone --depth=1 https://github.com/RT-Thread/rt-thread.git "$RTT_DIR"
    fi

    # 创建软链接到 rtos-bench
    local BSP_DIR="$RTT_DIR/bsp/qemu-virt64-aarch64"
    local LINK_PATH="$BSP_DIR/rtos-bench"

    if [ ! -L "$LINK_PATH" ]; then
        info "创建软链接: BSP -> rtos-bench"
        ln -sf "$SCRIPT_DIR" "$LINK_PATH"
    fi

    # 复制 BSP 配置文件
    local CONFIG_SRC="$SCRIPT_DIR/configs/rtthread_qemu_aarch64.config"
    local CONFIG_DST="$BSP_DIR/.config"
    if [ -f "$CONFIG_SRC" ] && [ ! -f "$CONFIG_DST" ]; then
        info "复制 BSP 配置..."
        cp "$CONFIG_SRC" "$CONFIG_DST"
    fi

    info "RT-Thread 配置完成"
}

# ============================================================================
# 5. 验证安装
# ============================================================================
verify_installation() {
    info "验证安装..."

    local errors=0

    # 检查工具链
    local GCC="$TOOLCHAIN_DIR/xpack-aarch64-none-elf-gcc-14.2.1-1.1/bin/aarch64-none-elf-gcc"
    if [ -x "$GCC" ]; then
        info "✓ 工具链: $($GCC --version | head -1)"
    else
        warn "✗ 工具链未找到"
        errors=$((errors + 1))
    fi

    # 检查 SCons
    if command -v scons &> /dev/null || [ -x "$EXTERN_DIR/.venv/bin/scons" ]; then
        info "✓ SCons: 已安装"
    else
        warn "✗ SCons 未安装"
        errors=$((errors + 1))
    fi

    # 检查 QEMU
    if command -v qemu-system-aarch64 &> /dev/null; then
        info "✓ QEMU: $(qemu-system-aarch64 --version | head -1)"
    else
        warn "✗ QEMU 未安装"
        errors=$((errors + 1))
    fi

    # 检查 RT-Thread
    if [ -d "$RTT_DIR/bsp/qemu-virt64-aarch64" ]; then
        info "✓ RT-Thread BSP: 已配置"
    else
        warn "✗ RT-Thread BSP 未找到"
        errors=$((errors + 1))
    fi

    return $errors
}

# ============================================================================
# 6. 打印使用说明
# ============================================================================
print_usage() {
    echo ""
    echo "============================================================"
    echo "  RTOS-Bench 环境配置完成!"
    echo "============================================================"
    echo ""
    echo "快速开始:"
    echo "  1. 编译并运行:"
    echo "     ./run-rtthread.sh"
    echo ""
    echo "  2. 仅编译:"
    echo "     ./run-rtthread.sh -b"
    echo ""
    echo "  3. 在 msh 中执行测试:"
    echo "     msh /> rtbench test-realtime"
    echo "     msh /> rtbench test-schedule --cycles 100"
    echo "     msh /> rtbench test-stress -s cpu -t 10"
    echo "     msh /> rtbench -b busywait -p 0.5 -t 100 -q"
    echo ""
    echo "文档:"
    echo "  - README.md              # 快速入门"
    echo "  - docs/ARCHITECTURE_OVERVIEW.md  # 架构概览"
    echo "  - docs/TEST_REPORT.md    # 测试报告"
    echo ""
    echo "============================================================"
}

# ============================================================================
# 主流程
# ============================================================================
main() {
    echo "============================================================"
    echo "  RTOS-Bench 环境安装脚本"
    echo "============================================================"
    echo ""

    check_system_deps
    install_python_deps
    install_toolchain
    setup_rtthread

    echo ""
    if verify_installation; then
        print_usage
    else
        warn "部分组件安装失败，请检查上述输出"
        exit 1
    fi
}

# 执行
main "$@"
