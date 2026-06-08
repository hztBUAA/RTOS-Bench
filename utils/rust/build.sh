#!/usr/bin/env bash
set -euo pipefail

TOOLCHAIN="1.96.0"
TARGET="aarch64-unknown-none"
MODE="demo"
declare -a CARGO_ARGS=()

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
TARGET_DIR="${CARGO_TARGET_DIR:-${SCRIPT_DIR}/target}"
cd "${SCRIPT_DIR}"

if ! command -v rustup >/dev/null 2>&1; then
    echo "error: rustup is required to install/use Rust ${TOOLCHAIN}" >&2
    exit 1
fi

while (($# > 0)); do
    case "$1" in
        --all)
            MODE="all"
            ;;
        --demo)
            MODE="demo"
            ;;
        --lib)
            MODE="lib"
            ;;
        *)
            CARGO_ARGS+=("$1")
            ;;
    esac
    shift
done

if ! rustup toolchain list | grep -Eq "^${TOOLCHAIN}(-|$)"; then
    rustup toolchain install "${TOOLCHAIN}"
fi

if ! rustup target list --toolchain "${TOOLCHAIN}" --installed | grep -qx "${TARGET}"; then
    rustup target add "${TARGET}" --toolchain "${TOOLCHAIN}"
fi

PROFILE="debug"
for arg in "${CARGO_ARGS[@]}"; do
    case "${arg}" in
        --release)
            PROFILE="release"
            ;;
    esac
done

if [[ "${MODE}" == "lib" ]]; then
    cargo "+${TOOLCHAIN}" build --target "${TARGET}" --lib "${CARGO_ARGS[@]}"
    echo "built: ${TARGET_DIR}/${TARGET}/${PROFILE}/librtos_rust_bench.a"
elif [[ "${MODE}" == "demo" ]]; then
    cargo "+${TOOLCHAIN}" build --target "${TARGET}" --bin rtos-rust-bench-demo "${CARGO_ARGS[@]}"
    echo "built: ${TARGET_DIR}/${TARGET}/${PROFILE}/rtos-rust-bench-demo"
else
    cargo "+${TOOLCHAIN}" build --target "${TARGET}" --lib "${CARGO_ARGS[@]}"
    cargo "+${TOOLCHAIN}" build --target "${TARGET}" --bin rtos-rust-bench-demo "${CARGO_ARGS[@]}"
    echo "built: ${TARGET_DIR}/${TARGET}/${PROFILE}/librtos_rust_bench.a"
    echo "built: ${TARGET_DIR}/${TARGET}/${PROFILE}/rtos-rust-bench-demo"
fi
