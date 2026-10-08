#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
sdk_root="${OPENEULER_AARCH64_SDK:-$HOME/opt/openeuler-toolchains/openeuler/aarch64--glibc--stable-2023.11-1}"
toolchain_bin="$sdk_root/bin"

if [[ ! -x "$toolchain_bin/aarch64-buildroot-linux-gnu-gcc" ]]; then
	cat >&2 <<EOF
error: openEuler AArch64 compiler was not found:
  $toolchain_bin/aarch64-buildroot-linux-gnu-gcc
Extract C:\\Users\\hzt\\Downloads\\openeuler.zip inside WSL, or set
OPENEULER_AARCH64_SDK to the extracted toolchain directory.
EOF
	exit 1
fi

export PATH="$toolchain_bin:$PATH"
exec make -C "$script_dir" \
	CROSS_COMPILE=aarch64-buildroot-linux-gnu- \
	-j"$(nproc)" \
	"$@"
