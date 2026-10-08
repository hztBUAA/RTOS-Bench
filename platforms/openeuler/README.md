# RTOS-Bench on openEuler AArch64

This platform package builds the RTOS-Bench user-space executable for the two
openEuler targets currently in scope:

| Board profile | CPU/ABI | Compile-time identity | Output |
| --- | --- | --- | --- |
| `feiteng` | Phytium Pi / E2000Q, AArch64 | `OPENEULER_FEITENG` | `build/feiteng/aarch64/rtos-bench-openeuler-feiteng-aarch64` |
| `orangepi` | Orange Pi / RK3588, AArch64 | `OPENEULER_ORANGEPI` | `build/orangepi/aarch64/rtos-bench-openeuler-orangepi-aarch64` |

The board profiles identify the target and keep artifacts separate. Both boards
run the same openEuler/Linux userspace ABI, so the OS adaptation reuses
`generator/platform/linux/`. The board kernel, device tree, boot media and
drivers remain the responsibility of its BSP and are not linked into this
executable.

## Build prerequisites

The compiler package is Linux-hosted. On a Linux development machine, extract
the package locally and run the Bash wrapper directly. On a Windows
development machine, use WSL2; the PowerShell wrapper only forwards the build
request into WSL and does not run the Linux compiler as a Windows executable.

Extract `<path-to-openeuler.zip>` inside the Linux/WSL filesystem, or set
`OPENEULER_AARCH64_SDK` to the extracted directory. The default expected path
is:

```text
$HOME/opt/openeuler-toolchains/openeuler/aarch64--glibc--stable-2023.11-1
```

The compiler prefix is `aarch64-buildroot-linux-gnu-`. This is a Linux/glibc
cross-toolchain, so the result is a Linux ELF for an already-installed
openEuler system; it is not a bare-metal image and does not replace board
firmware.

## Build commands

On Windows, from the repository root in PowerShell:

```powershell
.\platforms\openeuler\build-aarch64.ps1 BOARD=feiteng clean
.\platforms\openeuler\build-aarch64.ps1 BOARD=feiteng

.\platforms\openeuler\build-aarch64.ps1 BOARD=orangepi clean
.\platforms\openeuler\build-aarch64.ps1 BOARD=orangepi
```

The equivalent Linux or WSL commands are:

```bash
./platforms/openeuler/build-aarch64.sh BOARD=feiteng clean
./platforms/openeuler/build-aarch64.sh BOARD=feiteng
./platforms/openeuler/build-aarch64.sh BOARD=orangepi clean
./platforms/openeuler/build-aarch64.sh BOARD=orangepi
```

`clean` is required after changing the board profile, source list, platform
macros or toolchain. The default is a static executable to avoid a glibc
version mismatch between the build sysroot and the board. Use `STATIC=0` only
after checking the board runtime libraries:

```powershell
.\platforms\openeuler\build-aarch64.ps1 BOARD=feiteng STATIC=0
```

`make print-config` reports the selected board, output path and OSAL source.
`check-schedule-sources` fails early if any required schedule source is missing.

## What is linked

The Makefile explicitly links the shared command dispatcher, all workloads, the
Linux platform implementation, the real-time benchmark sources, stress sources
and these five schedule files:

```text
generator/test_schedule.c
generator/test_schedule/sched_workloads.c
generator/test_schedule/sched_compute_wrappers.c
generator/test_schedule/sched_mqtt_wrapper.c
generator/test_schedule/sched_modbus_wrapper.c
```

Pressure tests use the openEuler-specific OSAL already present on `main`:

```text
generator/stress_orig/osal/os_openeuler.c
```

That OSAL first attempts a Linux real-time thread policy and falls back to
ordinary POSIX scheduling when the process lacks the required privilege. The
`test-schedule` implementation remains independent of that pressure-test
fallback.

The small `generator/linux_entry.c` entry provides the common command dispatcher
with the selected board name. It does not duplicate the Linux timer, semaphore,
timestamp, scheduler or signal implementation.

## Deploy and run

After the target board and development host are on the same network, copy the
matching artifact to the openEuler filesystem:

```powershell
scp .\platforms\openeuler\build\feiteng\aarch64\rtos-bench-openeuler-feiteng-aarch64 `
  root@<板卡IP>:/opt/rtos-bench/
```

On the board:

```bash
chmod +x /opt/rtos-bench/rtos-bench-openeuler-feiteng-aarch64
cd /opt/rtos-bench
./rtos-bench-openeuler-feiteng-aarch64 -h
./rtos-bench-openeuler-feiteng-aarch64 -L
./rtos-bench-openeuler-feiteng-aarch64 -b busywait -p 0.5 -t 1 -q
./rtos-bench-openeuler-feiteng-aarch64 test-schedule --cycles 3
```

Use the `orangepi` artifact and output directory for the Orange Pi. The
application is a normal Linux user-space ELF: it is copied and executed after
the board has booted openEuler. It does not use SylixOS `ld`, OneOS module
loading, or a full-system flash step. Flashing is needed only when installing
or replacing the board's openEuler image itself.

## Repository integration boundary

There is intentionally no `generator/platform/openeuler/` directory. openEuler
provides the Linux/POSIX APIs required by the existing Linux platform layer.
The repository changes needed for this integration are:

1. `platforms/openeuler/Makefile`, board profiles and Windows/WSL wrappers;
2. `generator/linux_entry.c` for the common command entry and board metadata;
3. Linux build compatibility fixes in the legacy realtime sources so the full
   command set can be linked with the AArch64 GNU toolchain;
4. selection of `os_openeuler.c` for the stress subsystem;
5. this document and the five-source validation target.

This leaves board-specific boot and image work in the board BSP, where it can be
validated against the actual Phytium Pi and Orange Pi installations.
