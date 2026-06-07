# clang framework compile validation

This directory supports acceptance item **6.1.2 clang/clang++ cross compile tool verification**.

The validation target verifies the RTOS-Bench framework-layer minimal closure. The compiled entry is `generator/clang_entry.c`, which sits beside other platform entries such as `generator/oneos_entry.c`, `generator/rtthread_entry.c`, and `generator/sylixos_entry.c`.

This validation does **not** cover RTOS/BSP-specific modules. It does **not** compile or link the full realtime, schedulability, stress, or typical workload benchmark modules.

## Target Architecture

The primary clang target is:

- target triple: `arm-none-eabi`
- CPU: `cortex-a53`
- output type: 32-bit ARM EABI relocatable object, suitable as compile evidence for Cortex-A53/AArch32

If the local clang build cannot compile `arm-none-eabi`, the script retries the object compile with:

- fallback target triple: `aarch64-none-elf`
- output type: 64-bit AArch64 ELF relocatable object

The acceptance priority is the primary `arm-none-eabi` path. The fallback is only for local LLVM distributions that do not expose the primary bare-metal ARM target.

## Files

- `generator/clang_entry.c`: framework validation entry for TC-TOOL-002.
- `utils/clang/build_clang_arm.sh`: clang ARM object build script.

## Artifacts

Required artifact:

- `build/clang-arm/clang_entry.o`

Optional artifact, generated only when the local LLVM/lld environment supports a freestanding link:

- `build/clang-arm/rtbench_clang_framework.elf`

## Run

From the repository root:

```bash
bash utils/clang/build_clang_arm.sh
```

The script prints the clang and clang++ versions, compiles `generator/clang_entry.c` with `--target=arm-none-eabi -mcpu=cortex-a53`, checks that `build/clang-arm/clang_entry.o` exists, optionally prints object metadata, and tries an optional ELF link.

The pass condition is object generation. Optional ELF generation is extra evidence and is not a hard failure condition.
