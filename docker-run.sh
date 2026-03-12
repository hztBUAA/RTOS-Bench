#!/bin/bash
# Run RTOS-Bench Docker container
set -e

IMAGE_NAME="rtos-bench:latest"

usage() {
    echo "Usage: $0 [options]"
    echo "  (no args)     Interactive bash shell"
    echo "  -q            Run QEMU with RT-Thread"
    echo "  -b            Build RT-Thread only"
    echo "  -h            Show help"
    echo ""
    echo "Examples:"
    echo "  $0             # bash shell"
    echo "  $0 -q          # run QEMU interactively"
    echo "  $0 -b          # rebuild rtthread.bin"
}

case "${1:-}" in
    -q) docker run -it --rm "$IMAGE_NAME" ./run-rtthread.sh ;;
    -b) docker run -it --rm "$IMAGE_NAME" ./run-rtthread.sh -b ;;
    -h) usage ;;
    *)  docker run -it --rm "$IMAGE_NAME" ;;
esac
