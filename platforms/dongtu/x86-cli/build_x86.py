#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Build RTOS-Bench for the Dongtu/Intewell x86 (i386) RTOS partition.

This script reproduces the vendor IDE's generated project outside Eclipse:
  * copy a minimal x86 TTOS application project template
  * point it at this repository and at the installed Intewell SDK
  * wire in RTOS-Bench through platforms/dongtu/intewell.mk
  * invoke the SDK's bundled make and produce Debug/make/rtosbench_x86.{elf,bin}

Tested with Intewell_Developer_V2.3.1 (x86_64-intewell-elf-gcc 9.3.0,
target board kyland/newpre3101_i7, 32-bit i386 partition).

Usage:
    python platforms/dongtu/x86-cli/build_x86.py
    python platforms/dongtu/x86-cli/build_x86.py --sdk C:/Intewell_Developer_V2.3.1_P2_20241225
    python platforms/dongtu/x86-cli/build_x86.py --project-dir build/dongtu_x86/rtosbench_x86 --clean

Environment:
    INTEWELL_HOME   SDK install root (same as --sdk)
"""

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[2]
TEMPLATE = HERE / "project-template"
DEFAULT_SDK = Path(r"C:/Intewell_Developer_V2.3.1_P2_20241225")
PROJECT_NAME = "rtosbench_x86"


def die(msg):
    print("ERROR: " + msg, file=sys.stderr)
    sys.exit(1)


def find_sdk(arg):
    candidates = []
    if arg:
        candidates.append(Path(arg))
    if os.environ.get("INTEWELL_HOME"):
        candidates.append(Path(os.environ["INTEWELL_HOME"]))
    candidates.append(DEFAULT_SDK)
    for c in candidates:
        c = c.resolve()
        if (c / "target/RTCore/arch/x86/vm/default.mtp").is_file() and \
           (c / "host/gnu/gcc-9.3.0/x86/bin/x86_64-intewell-elf-gcc.exe").is_file():
            return c
    die("cannot find an Intewell SDK with x86 support; pass --sdk or set INTEWELL_HOME")


def find_make(sdk):
    make = sdk / "gnuwin/msys64/usr/bin/make.exe"
    if make.is_file():
        return make
    found = shutil.which("make")
    if found:
        return Path(found)
    die("make not found (expected %s)" % make)


def copy_template(project_dir, sdk, board, company):
    if project_dir.exists():
        shutil.rmtree(project_dir)
    shutil.copytree(TEMPLATE, project_dir)

    replacements = {
        b"@INTEWELL_HOME@": sdk.as_posix().encode(),
        b"@PROJECT_DIR@": project_dir.as_posix().encode(),
    }
    for root, _dirs, files in os.walk(project_dir):
        for name in files:
            p = Path(root) / name
            data = p.read_bytes()
            new = data
            for old, val in replacements.items():
                new = new.replace(old, val)
            if new != data:
                p.write_bytes(new)

    v_makefile = project_dir / "Debug/make/v_makefile.mk"
    text = v_makefile.read_text(encoding="utf-8", errors="surrogateescape")
    text = text.replace("CURRENT_BOARD:=newpre3101_i7", "CURRENT_BOARD:=%s" % board)
    text = text.replace("COMPANY:=kyland", "COMPANY:=%s" % company)
    text = text.replace("BOARD_SUFFIX:=newpre3101_i7", "BOARD_SUFFIX:=%s" % board)
    v_makefile.write_text(text, encoding="utf-8", errors="surrogateescape")


def write_project_files(project_dir, sdk):
    # Vendor entry file plus the two x86-specific build fixes.
    (project_dir / "src").mkdir(exist_ok=True)
    shutil.copyfile(REPO / "platforms/dongtu/userAppInit.c", project_dir / "src/userAppInit.c")
    (project_dir / "config_os.mk").write_text(
        "RTOS_BENCH_ROOT := %s\n"
        "ARCH := _X86_\n"
        "include $(RTOS_BENCH_ROOT)/platforms/dongtu/intewell.mk\n"
        "\n"
        "# x86 has no ARM LES shared-memory ABI.  The only use of this macro is\n"
        "# inside a __aarch64__ block; the placeholder only satisfies les.h.\n"
        "USER_OPTION += -DLES_SHARED_MEM_VA=0\n"
        "\n"
        "# 32-bit x86 needs libatomic for __atomic_fetch_nand_*/__atomic_fetch_add_4.\n"
        "LIBS += -latomic\n" % REPO.as_posix(),
        encoding="utf-8",
    )

    # The vendor example's default.mtp has no .sidata section, which makes the
    # final link fail on _sidata_start.  The arch template is the authoritative
    # x86 VM memory map.
    mtp = sdk / "target/RTCore/arch/x86/vm/default.mtp"
    for cfg in ("Debug", "Release"):
        shutil.copyfile(mtp, project_dir / cfg / "default.mtp")

    # The Eclipse IDE normally writes this metadata file before linking; the
    # generated makefile only consumes it (objcopy --add-section .imgattr=...).
    (project_dir / "Debug/make/ImgInfoGen").write_bytes(
        ("CONFIG_IMG_TYPE=0 CONFIG_IMG_NAME=%s CONFIG_IMG_RUN_PLACE=ram "
         "CONFIG_IMG_ALIGN=4096 OS_TYPE_NAME=RTCore TOOLCHAIN_TYPE_NAME=null\n"
         % PROJECT_NAME).encode("ascii")
    )


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--sdk", help="Intewell SDK root (or set INTEWELL_HOME)")
    ap.add_argument("--project-dir", default=str(REPO / "build/dongtu_x86" / PROJECT_NAME),
                    help="generated project directory")
    ap.add_argument("--board", default="newpre3101_i7",
                    help="Intewell x86 board name (default: kyland/newpre3101_i7)")
    ap.add_argument("--company", default="kyland",
                    help="Intewell x86 board vendor directory (default: kyland)")
    ap.add_argument("--clean", action="store_true", help="delete the project directory first")
    args = ap.parse_args()

    sdk = find_sdk(args.sdk)
    project_dir = Path(args.project_dir).resolve()
    board_dir = sdk / "target/RTCore/bsp/board/x86" / args.company / args.board / "partition/ttos"
    if not board_dir.is_dir():
        die("board BSP not found: %s" % board_dir)
    if args.clean and project_dir.exists():
        shutil.rmtree(project_dir)

    print("[x86] SDK       : %s" % sdk)
    print("[x86] repo      : %s" % REPO)
    print("[x86] project   : %s" % project_dir)
    print("[x86] board     : %s/%s" % (args.company, args.board))

    copy_template(project_dir, sdk, args.board, args.company)
    write_project_files(project_dir, sdk)

    make = find_make(sdk)
    env = os.environ.copy()
    env["PATH"] = os.pathsep.join([
        str(sdk / "gnuwin/msys64/usr/bin"),
        str(sdk / "host/bin"),
        env.get("PATH", ""),
    ])
    print("[x86] make      : %s" % make)
    print("[x86] building ...")
    subprocess.run([str(make), "all"], cwd=str(project_dir / "Debug/make"),
                   env=env, check=True)

    elf = project_dir / "Debug/make" / (PROJECT_NAME + ".elf")
    bin_ = project_dir / "Debug/make" / (PROJECT_NAME + ".bin")
    if not elf.is_file() or not bin_.is_file():
        die("build finished but %s/.bin was not produced" % PROJECT_NAME)
    print("[x86] done")
    print("  ELF: %s (%.1f MB)" % (elf, elf.stat().st_size / 1e6))
    print("  BIN: %s (%.1f MB)  <-- upload this image" %
          (bin_, bin_.stat().st_size / 1e6))


if __name__ == "__main__":
    main()
