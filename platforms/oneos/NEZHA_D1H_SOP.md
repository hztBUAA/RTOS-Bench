# OneOS Nezha D1H Bring-up SOP

This SOP records the OneOS Nezha Pi/D1H workflow used to compile RTOS-Bench as
an out module, and the board-level validation flow to use after hardware is
available. The Phytium Pi flow is included as the proven reference.

## Current Status

- Phytium Pi OneOS has been validated on hardware through Telnet/TFTP.
- Nezha D1H has been validated in WSL for full `.out` compilation.
- Nezha D1H hardware runtime validation is still pending.

The Nezha full build compiled the framework plus:

- Workloads: `stub`, `busywait`, `cusum`, `ewma`, `fast`, `epnp`, `ekf`, `icp`,
  `pid`, `modbus`, `mqtt`
- Test modules: `test-schedule`, `test-realtime`, `test-stress`, `test-cmd`
- Registry and dispatch sources: `rtbench_workloads.cpp`,
  `run_all_workloads.cpp`

`workloads/main.cpp` is intentionally excluded because OneOS `.out` modules are
entered through the shell command/module symbol path, not a Linux-style `main()`.

## Build Environment

The Nezha package uses the OneOS multi-CMake command-line workflow rather than
OneOS Studio. In the WSL validation environment:

```bash
export ONEOS_BUILDER=/home/hzt/oneos-benchmark-v1.6/OneOS-Benchmark-V1.6/oneos-multi-tools
export ONEOS_SCRIPTS=/home/hzt/oneos-benchmark-v1.6/OneOS-Benchmark-V1.6/oneos-multi-cmake
export PATH=/home/hzt/oneos-tools/bin:/home/hzt/oneos-tools/cmake-3.27.9-linux-x86_64/bin:$PATH
```

Build image project:

```bash
cd /home/hzt/oneos-benchmark-v1.6/OneOS-Benchmark-V1.6/projects/d1h-nezha
bash ../../oneos-multi-cmake/oos.sh build
```

Build out module:

```bash
cd /home/hzt/oneos-benchmark-v1.6/OneOS-Benchmark-V1.6/projects/d1h-nezha_out
bash ../../oneos-multi-cmake/oos.sh build
```

Expected module:

```text
projects/d1h-nezha_out/out/d1h-nezha_out.out
```

## Required Nezha Compatibility Notes

These changes were required for full build validation:

1. Treat OneOS V2 RISC-V/musl as a V2 libc target.
   - RISC-V musl already defines `clock_t`, `clockid_t`, `timer_t`, pthread
     types, etc.
   - RTOS-Bench must not redefine those types.

2. Use OneOS V2 APIs for RISC-V D1H.
   - `os_get_current_task()` / `os_task_id`
   - `os_tick_get_value()`

3. Prevent C-style `min/max` BSP macros from leaking into C++ workloads.
   - EKF/ECL and EPNP/Eigen use namespaced `math::max`, `matrix::min`, and
     `std::min`.
   - The RTOS-Bench OneOS C++ compatibility header preloads standard headers and
     clears `min/max` before C++ workload headers are parsed.

4. Keep C++ workload files away from OneOS kernel libc response include files.
   - The vendor out template should not pass `processed_kernel_include.txt` and
     `processed_board_include.txt` to C++ files that use libstdc++/sysroot
     headers.
   - Otherwise `pthread`, `timespec`, and `cpu_set_t` are seen from both OneOS
     kernel libc and the toolchain sysroot.

5. Patch vendor BSP headers or wrap them for C++.
   - `sunxi_hal_common.h` and `driver.h` should not define `min/max` under
     `__cplusplus`.
   - Prefer fixing this in the BSP template or carrying a small local patch.

## Phytium Pi Reference Flow

Known board:

```text
Board IP: 192.168.31.205
Telnet: 23
TFTP server: 192.168.31.110
Remote module: /user/phytium_pi_out.out
```

Serial/U-Boot startup:

```text
tftp 0x80100000 oneos.bin
go 0x80100000
```

OneOS shell setup after boot:

```text
set_if e01 192.168.31.205 192.168.31.1 255.255.255.0
default_netif e01
telnetd start
mkdir /user
mount -t fatfs sdmmc0a2 /user
tftp_client 192.168.31.110 get phytium_pi_out.out /user/phytium_pi_out.out
ld /user/phytium_pi_out.out
```

Phytium smoke validation:

```text
list_lmodule
rtbench --help
rtbench -L
rtbench -b stub -t 1
rtbench -b busywait -t 1
rtbench -b epnp -t 1
```

Phytium acceptance command used for reduced typical workload validation:

```text
rtbench test-all --no-realtime --no-schedule --no-stress --no-cmd
```

If `rtbench -b stub -t 1` stops after `exec atexit`, the loaded `.out` is likely
an old build. Rebuild, upload the new `.out`, then unload and reload:

```text
unld /user/phytium_pi_out.out
unld /user/phytium_pi_out.out
tftp_client 192.168.31.110 get phytium_pi_out.out /user/phytium_pi_out.out
ld /user/phytium_pi_out.out
```

Successful workload runs print `Execution environment setup complete`,
`Job completed`, `Cleaning up job environment`, and return to `sh /user>`.

## Nezha D1H Hardware Validation Plan

Use the Phytium process as the baseline, with Nezha-specific image/module names:

```text
Image project: d1h-nezha
Out project: d1h-nezha_out
Expected module: d1h-nezha_out.out
Suggested remote module: /user/d1h-nezha_out.out
```

Board startup will depend on the vendor D1H bootloader configuration. Use the
vendor image load address and image name from the Nezha package. After OneOS
boots, configure networking, mount `/user`, fetch the module, and load it:

```text
set_if <netif> <board-ip> <gateway-ip> <netmask>
default_netif <netif>
telnetd start
mkdir /user
mount -t fatfs <storage-partition> /user
tftp_client <tftp-server-ip> get d1h-nezha_out.out /user/d1h-nezha_out.out
ld /user/d1h-nezha_out.out
```

If the image-side shell command stub is used, ensure it looks up the exact module
path loaded above. For example, if the stub still searches
`/user/phytium_pi_out.out`, either keep that remote module name or update the
stub to `/user/d1h-nezha_out.out`.

Minimum runtime validation:

```text
list_lmodule
rtbench --help
rtbench -L
rtbench -b stub -t 1
rtbench -b busywait -t 1
rtbench -b epnp -t 1
rtbench -b ekf -t 1
rtbench -b icp -t 1
rtbench test-schedule --cycles 1 --util-start 30 --util-end 30 --util-step 30
```

Then run the reduced typical workload acceptance:

```text
rtbench test-all --no-realtime --no-schedule --no-stress --no-cmd
```

Keep `test-realtime`, `test-schedule`, and `test-stress` as separate runs during
initial hardware bring-up so long-running stages do not hide workload/module
load failures.

## Campus Router / Shared Lab Network SOP

Goal: anyone on the same lab network can Telnet to the board and use TFTP to
deploy modules.

1. Put the TFTP server, host PC, and board on the same L2 network.
   - Example subnet: `192.168.31.0/24`
   - Router/gateway: `192.168.31.1`
   - TFTP server: `192.168.31.110`
   - Phytium: `192.168.31.205`
   - Reserve a stable Nezha IP, for example `192.168.31.206`.

2. Configure the board with a static IP after every boot unless the BSP stores
   network config persistently:

```text
set_if <netif> <board-ip> 192.168.31.1 255.255.255.0
default_netif <netif>
telnetd start
```

3. Validate from a developer machine:

```text
ping <board-ip>
telnet <board-ip> 23
```

4. Validate TFTP from the board:

```text
tftp_client 192.168.31.110 get <module>.out /user/<module>.out
```

5. For campus networks with upstream routing/NAT, keep the board network behind
   the lab router and expose only the router/VPN if remote access is required.
   Do not expose Telnet directly on the campus public network.

## Acceptance Evidence To Collect

For each board run, collect:

- Image name and SHA256
- `.out` name, size, and SHA256
- Serial boot log
- Telnet transcript covering module unload/load and smoke commands
- `rtbench -L` output
- Results for `stub`, `busywait`, `epnp`, `ekf`, `icp`
- Reduced `test-all --no-realtime --no-schedule --no-stress --no-cmd` output

Store logs under `utils/remote-test/logs/<board>-<date>/` and summarize the
artifact paths in `SUMMARY.md`.
