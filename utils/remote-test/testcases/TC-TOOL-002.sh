#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
LOG_DIR="${REPO_ROOT}/logs"
LOG_FILE="${LOG_DIR}/TC-TOOL-002.log"

mkdir -p "${LOG_DIR}"
cd "${REPO_ROOT}"

bash utils/clang/build_clang_arm.sh 2>&1 | tee "${LOG_FILE}"

grep -q "clang version" "${LOG_FILE}"
grep -q "clang --target=arm-none-eabi" "${LOG_FILE}"
grep -q "target artifact: 32-bit ARM EABI relocatable object for Cortex-A53/AArch32" "${LOG_FILE}"
grep -q "build/clang-arm/clang_entry.o" "${LOG_FILE}"
grep -q "clang ARM framework compile success" "${LOG_FILE}"

echo "TC-TOOL-002 log: logs/TC-TOOL-002.log"
