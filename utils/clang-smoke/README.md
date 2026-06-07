# clang framework smoke compile

This directory supports the acceptance item **6.1.2 clang/clang++ cross compile tool verification**.

The smoke target verifies only the RTOS-Bench framework-layer minimal closure. It is a bypass compile entry used to prove that clang/clang++ style LLVM cross tooling can compile framework code into an ARM target artifact.

It does **not** cover RTOS/BSP-specific modules. It does **not** compile or link the full realtime, schedulability, stress, or typical workload benchmark modules.

## Files

- `clang_entry.c`: freestanding framework smoke entry for TC-TOOL-002.
- `build_clang_arm.sh`: clang ARM object build script.

## Artifacts

Required artifact:

- `build/clang-arm/clang_entry.o`

Optional artifact, generated only when the local LLVM/lld environment supports a freestanding link:

- `build/clang-arm/rtbench_clang_smoke.elf`

## Run

From the repository root:

```bash
bash utils/clang-smoke/build_clang_arm.sh
```

The script prints the clang version, compiles `utils/clang-smoke/clang_entry.c` with `--target=arm-none-eabi`, checks that `build/clang-arm/clang_entry.o` exists, optionally prints object metadata, and tries an optional ELF link. If `arm-none-eabi` is unavailable in the local clang build, it retries the object compile with `--target=aarch64-none-elf`.

The pass condition is object generation. Optional ELF generation is extra evidence and is not a hard failure condition.
