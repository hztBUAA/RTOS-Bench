# Dongtu vm_3588 RTOS-Bench SOP

## Goal

Keep RTOS-Bench as the single source of truth in the GitHub checkout. The
Dongtu `vm_3588` IDE project only keeps board/project configuration and includes
the RTOS-Bench integration make fragment.

## Project Hook

Create `config_os.mk` in the `vm_3588` project root:

```make
RTOS_BENCH_ROOT ?= C:/Users/hzt/yihui-workspace/rtos-bench/RTOS-Bench
include $(RTOS_BENCH_ROOT)/platforms/dongtu/intewell_vm3588.mk
```

Override `RTOS_BENCH_ROOT` from the environment if the checkout lives elsewhere.

## Command-Line Build

Use the Intewell bundled make and toolchain first in `PATH`:

```powershell
$ide = 'C:\Intewell_Developer_V2.2.0_kyland_J6412-64_C2.P2_20260109'
$env:PATH = "$ide\gnuwin\msys64\usr\bin;$ide\host\gnu\bin;$ide\host\bin;$env:PATH"
$env:RTOS_BENCH_ROOT = 'C:/Users/hzt/yihui-workspace/rtos-bench/RTOS-Bench'
cd 'D:\build\workspace\Developer_231Gizwits\IDE\BIN\Intewell_Developer\eclipse\workspace\vm_3588\Debug\make'
& "$ide\gnuwin\msys64\usr\bin\make.exe" -j4 clean_obj
& "$ide\gnuwin\msys64\usr\bin\make.exe" -j4 all
```

Prefer `clean_obj` for command-line rebuilds. The generated `make clean` target
also removes IDE-generated `Debug/config_*.mk`, `Debug/config_*.h`, and
`Debug/imgHeader.h`, so use it only when the IDE will regenerate those files.

## Expected Output

Successful build generates:

- `Debug/make/vm_3588.elf`
- `Debug/make/vm_3588.bin`
- `Debug/make/libvm_3588.a`

Validated on 2026-04-28:

- `vm_3588.elf`: 22,599,152 bytes
- `vm_3588.bin`: 3,997,696 bytes
- `libvm_3588.a`: 7,272,138 bytes

The toolchain may print `aarch64-intewell-elf-ar.exe: Unable to load DLL` on
stderr while still returning `BUILD_EXIT_CODE=0` and producing the final ELF/BIN.
Treat it as a toolchain stderr warning unless the make exit code is non-zero or
the final target is missing.

## Verification

Check that the shell command is linked into the final ELF:

```powershell
$nm = 'C:\Intewell_Developer_V2.2.0_kyland_J6412-64_C2.P2_20260109\host\gnu\gcc-9.3.0\arm64\bin\aarch64-intewell-elf-nm.exe'
$elf = 'D:\build\workspace\Developer_231Gizwits\IDE\BIN\Intewell_Developer\eclipse\workspace\vm_3588\Debug\make\vm_3588.elf'
& $nm $elf 2>$null | Select-String 'shell_cmd_rtbench|rtbench_dongtu_entry'
```

Expected symbols include:

- `_shell_rtbench`
- `shell_cmd_rtbench`
- `rtbench_dongtu_entry`

## Layout

- `platforms/dongtu/intewell_vm3588.mk`: vm_3588 make integration.
- `platforms/dongtu/shell.c`: Dongtu shell command registration.
- `platforms/dongtu/compat.c`: small conservative compatibility shims.
- `generator/platform/dongtu/`: RTOS-Bench platform abstraction implementation
  for timers, sync, scheduler, signals, and timestamp.
