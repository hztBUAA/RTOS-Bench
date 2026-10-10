#!/usr/bin/env bash
# Convenience wrapper for the Dongtu LoongArch64 (Linux/musl) cross build.
#
# The cross toolchain is NOT stored in this repo (it is ~115 MB). Point
# LOONGARCH64_TOOLCHAIN at the extracted toolchain ROOT (the directory that
# contains bin/), or put its bin/ on PATH yourself. Example:
#
#   LOONGARCH64_TOOLCHAIN=$HOME/toolchains/loongarch64-unknown-linux-musl-cross \
#       ./build-loongarch64.sh
set -euo pipefail

here="$(cd "$(dirname "$0")" && pwd)"

if [ -n "${LOONGARCH64_TOOLCHAIN:-}" ]; then
    export PATH="$LOONGARCH64_TOOLCHAIN/bin:$PATH"
fi

exec make -C "$here" "$@"
