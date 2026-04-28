# Dongtu vm_3588 Build Validation - 2026-04-28

## Environment

- RTOS-Bench branch: `fix/sylixos-test-schedule0427`
- Dongtu project: `D:\build\workspace\Developer_231Gizwits\IDE\BIN\Intewell_Developer\eclipse\workspace\vm_3588`
- Intewell IDE/toolchain: `C:\Intewell_Developer_V2.2.0_kyland_J6412-64_C2.P2_20260109`
- Build command: `make -j4 all` from `vm_3588\Debug\make`

## Result

Build succeeded with `BUILD_EXIT_CODE=0`.

Generated artifacts:

- `Debug/make/vm_3588.elf` - 22,599,152 bytes
- `Debug/make/vm_3588.bin` - 3,997,696 bytes
- `Debug/make/libvm_3588.a` - 7,272,138 bytes

Local full build log:

- `utils/remote-test/logs/dongtu-vm3588-20260428-165708/build.log`

## Entry Verification

The final ELF contains the shell command and RTOS-Bench entry symbols:

- `_shell_rtbench`
- `shell_cmd_rtbench`
- `rtbench_dongtu_entry`

The final ELF also contains RTOS-Bench command strings including:

- `rtbench`
- `rtbench test-all`
- `rtbench test-schedule`
- `rtbench export-result`

## Known Toolchain Stderr

The Intewell archiver prints:

```text
aarch64-intewell-elf-ar.exe: Unable to load DLL.
```

This appeared on stderr during validation, but make still returned
`BUILD_EXIT_CODE=0` and produced valid ELF/BIN outputs. Treat it as a toolchain
stderr warning unless the make exit code is non-zero or the final artifacts are
missing.
