#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
OUT_DIR="${REPO_ROOT}/build/clang-arm"
SRC="${REPO_ROOT}/generator/clang_entry.c"
OBJ="${OUT_DIR}/clang_entry.o"
ELF="${OUT_DIR}/rtbench_clang_framework.elf"

CLANG="${CLANG:-clang}"
CLANGXX="${CLANGXX:-clang++}"
PRIMARY_TARGET="arm-none-eabi"
PRIMARY_CPU="cortex-a53"
FALLBACK_TARGET="aarch64-none-elf"
TARGET="${PRIMARY_TARGET}"

if ! command -v "${CLANG}" >/dev/null 2>&1; then
    echo "ERROR: clang not found. Please install LLVM/clang." >&2
    exit 1
fi

mkdir -p "${OUT_DIR}"

echo "clang version"
"${CLANG}" --version

if command -v "${CLANGXX}" >/dev/null 2>&1; then
    echo
    echo "clang++ version"
    "${CLANGXX}" --version
else
    echo
    echo "warning: clang++ not found; continuing with clang framework object compile"
fi

echo
echo "clang target architecture: primary ${PRIMARY_TARGET} with -mcpu=${PRIMARY_CPU}"
echo "target artifact: 32-bit ARM EABI relocatable object for Cortex-A53/AArch32"
echo "fallback target: ${FALLBACK_TARGET} for 64-bit AArch64 ELF object if the primary target is unavailable"

compile_object() {
    local target="$1"

    echo
    echo "clang --target=${target} -mcpu=${PRIMARY_CPU} -ffreestanding -fdata-sections -ffunction-sections -Wall -Wextra -c generator/clang_entry.c -o build/clang-arm/clang_entry.o"
    "${CLANG}" --target="${target}" \
        -mcpu="${PRIMARY_CPU}" \
        -ffreestanding \
        -fdata-sections \
        -ffunction-sections \
        -Wall -Wextra \
        -c "${SRC}" \
        -o "${OBJ}"
}

if ! compile_object "${PRIMARY_TARGET}"; then
    echo "warning: ${PRIMARY_TARGET} object compile failed; retrying with ${FALLBACK_TARGET}" >&2
    TARGET="${FALLBACK_TARGET}"
    compile_object "${TARGET}"
fi

test -f "${OBJ}"
echo "generated object: build/clang-arm/clang_entry.o"

echo
if command -v llvm-objdump >/dev/null 2>&1; then
    llvm-objdump -f "${OBJ}" || echo "warning: llvm-objdump failed; continuing"
else
    echo "warning: llvm-objdump not found; skipping object header dump"
fi

if command -v file >/dev/null 2>&1; then
    file "${OBJ}" || echo "warning: file failed; continuing"
else
    echo "warning: file command not found; skipping object file type check"
fi

try_link_elf() {
    local target="$1"
    local link_args=(
        --target="${target}"
        -nostdlib
        -Wl,-e,main
        -Wl,--gc-sections
        "${OBJ}"
        -o "${ELF}"
    )

    if command -v ld.lld >/dev/null 2>&1; then
        link_args=(-fuse-ld=lld "${link_args[@]}")
    fi

    echo
    echo "trying optional ELF link: build/clang-arm/rtbench_clang_framework.elf"
    "${CLANG}" "${link_args[@]}"
}

if try_link_elf "${TARGET}"; then
    echo "generated ELF: build/clang-arm/rtbench_clang_framework.elf"
    if command -v file >/dev/null 2>&1; then
        file "${ELF}" || echo "warning: file failed for optional ELF; continuing"
    fi
else
    echo "warning: optional ELF link failed; object generation is the required pass condition" >&2
    rm -f "${ELF}"
fi

echo
echo "clang ARM framework compile success"
