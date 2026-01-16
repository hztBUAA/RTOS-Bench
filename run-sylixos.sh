#!/bin/bash
# SylixOS QEMU 启动脚本
# 用法: ./run-sylixos.sh [选项]
#   -n  不使用图形界面 (nographic)
#   -s  启用共享目录

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SYLIXOS_DIR="$SCRIPT_DIR/extern/yihui/SylixOS IDE 6.5.0_professional/VMware/SylixOSx86"

BOOT_IMG="$SYLIXOS_DIR/sylixos_boot.qcow2"
MAIN_IMG="$SYLIXOS_DIR/sylixos_main.qcow2"

if [ ! -f "$BOOT_IMG" ] || [ ! -f "$MAIN_IMG" ]; then
    echo "Error: SylixOS images not found."
    echo "Please run: cd '$SYLIXOS_DIR' && unzip '../SylixOS VMware.zip'"
    echo "Then: qemu-img convert -f vmdk -O qcow2 x86_boot.vmdk sylixos_boot.qcow2"
    echo "      qemu-img convert -f vmdk -O qcow2 x86_main.vmdk sylixos_main.qcow2"
    exit 1
fi

QEMU_OPTS="-m 512M"
QEMU_OPTS="$QEMU_OPTS -hda $BOOT_IMG"
QEMU_OPTS="$QEMU_OPTS -hdb $MAIN_IMG"
QEMU_OPTS="$QEMU_OPTS -net nic -net user,hostfwd=tcp::2222-:22"

NOGRAPHIC=0
SHARE=0

while getopts "ns" opt; do
    case $opt in
        n) NOGRAPHIC=1 ;;
        s) SHARE=1 ;;
        *) echo "Usage: $0 [-n] [-s]"; exit 1 ;;
    esac
done

if [ $NOGRAPHIC -eq 1 ]; then
    QEMU_OPTS="$QEMU_OPTS -nographic"
fi

if [ $SHARE -eq 1 ]; then
    QEMU_OPTS="$QEMU_OPTS -virtfs local,path=$SCRIPT_DIR,mount_tag=rtbench,security_model=mapped"
    echo "Shared directory enabled. In SylixOS, mount with:"
    echo "  mount -t 9p -o trans=virtio rtbench /mnt"
fi

echo "Starting SylixOS..."
echo "SSH access: ssh -p 2222 root@localhost"
echo "Press Ctrl+A X to exit QEMU (nographic mode)"
echo ""

qemu-system-x86_64 $QEMU_OPTS
