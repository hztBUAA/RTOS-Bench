#!/bin/bash
# Build RTOS-Bench Docker image
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
IMAGE_NAME="rtos-bench"
TAG="latest"

echo "=== Building ${IMAGE_NAME}:${TAG} ==="
echo "Build context: ${SCRIPT_DIR}"
echo "This may take a while (~2.5GB context)..."
echo ""

docker build -t "${IMAGE_NAME}:${TAG}" "$SCRIPT_DIR"

echo ""
echo "=== Build complete ==="
docker images "${IMAGE_NAME}:${TAG}"
echo ""
echo "Usage:"
echo "  docker run -it --rm ${IMAGE_NAME}:${TAG}              # interactive shell"
echo "  docker run -it --rm ${IMAGE_NAME}:${TAG} ./run-rtthread.sh  # run QEMU"
