#!/bin/bash
# Pack RTOS-Bench for deployment
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
OUTPUT_FILE="rtos-bench-docker.tar.gz"

echo "=== Packing RTOS-Bench ==="
echo "Source: $SCRIPT_DIR"
echo "Output: $OUTPUT_FILE"
echo ""

cd "$SCRIPT_DIR/.."

tar czf "$OUTPUT_FILE" \
    --exclude='.git' \
    --exclude='__pycache__' \
    --exclude='*.pyc' \
    --exclude='.vscode' \
    --exclude='.idea' \
    --exclude='*.swp' \
    --exclude='*.swo' \
    --exclude='*~' \
    --exclude='.DS_Store' \
    --exclude='CLAUDE.md' \
    --exclude='AGENTS.md' \
    --exclude='PROGRESS.md' \
    --exclude='docs/agents' \
    --exclude='utils/__pycache__' \
    --exclude='workloads_latest.zip' \
    RTOS-Bench/

echo ""
echo "=== Pack complete ==="
ls -lh "$OUTPUT_FILE"
echo ""
echo "Deploy on new server:"
echo "  tar xzf $OUTPUT_FILE"
echo "  cd RTOS-Bench"
echo "  ./docker-build.sh"
