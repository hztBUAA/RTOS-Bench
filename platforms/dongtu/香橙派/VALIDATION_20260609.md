# Dongtu Orange Pi Validation - 2026-06-09

## Summary

Dongtu Orange Pi completed an end-to-end RTOS-Bench acceptance run with:

- Windows/Intewell command-line build: passed.
- Direct SSH control of the outer Linux host: passed.
- Deployment of the freshly built `vm_3588.bin` to the board: passed.
- Inner RTOS telnet startup: passed.
- `rtbench test-all`: passed, including realtime, schedule, stress, cmd, workload, and result export.
- Result JSON fetch and parse: passed.

The final acceptance logs are under:

`utils/remote-test/logs/dongtu-orangepi-acceptance-20260609_2212/`

## Board And Network

- Outer Linux host: `192.168.31.241`
- Outer login: `ssh root@192.168.31.241`
- Outer RT control command: `/usr/bin/rt`
- Inner RTOS shell: `telnet 192.168.31.207 23`
- Inner RTOS prompt: `vm1 [ /nfsd/ ]#`

The outer Linux host does not run RTOS-Bench directly. It controls the Intewell VM/RTOS image:

```sh
rt status
rt stop
rt start
rt ifconfig
```

If `192.168.31.207:23` is unreachable, check `rt status` first. A `stop` status means the inner RTOS is not running; run `rt start` and then re-check `rt ifconfig`.

## Fixed Image Paths

Local Windows build output:

`D:/build/workspace/Developer_231Gizwits/IDE/BIN/Intewell_Developer/eclipse/workspace/vm_3588/Debug/make/vm_3588.bin`

Outer board active images:

- `/download/vm_3588.bin`
- `/download/config.bin`

Final deployed `vm_3588.bin`:

- Local SHA256: `4fd1b734f6f372a9211477f387b1673f94a343a3189b062621917fe62931d012`
- Board active SHA256: `4fd1b734f6f372a9211477f387b1673f94a343a3189b062621917fe62931d012`
- Named board copy: `/download/vm_3588_hzt_acceptance_20260609_2248_export_path.bin`
- Pre-deploy board backup: `/download/vm_3588_backup_before_20260609_2248_export_path.bin`
- `config.bin` SHA256: `ebdf06c6930bc266f5ec2fe54d752d80eb9c86d9dd09e891e58b5da013c21c6a`

## Build

Build directory:

`D:/build/workspace/Developer_231Gizwits/IDE/BIN/Intewell_Developer/eclipse/workspace/vm_3588/Debug/make`

Important environment:

- MSYS tools: `C:/Intewell_Developer_V2.2.0_kyland_J6412-64_C2.P2_20260109/gnuwin/msys64/usr/bin`
- GCC tools: `C:/Intewell_Developer_V2.2.0_kyland_J6412-64_C2.P2_20260109/host/gnu/gcc-9.3.0/arm64/bin`
- `RTOS_BENCH_ROOT=C:/Users/hzt/yihui-workspace/rtos-bench/RTOS-Bench`

Final build logs:

- `utils/remote-test/logs/dongtu-orangepi-acceptance-20260609_2212/build_all_dongtu_export_path_20260609.log`
- `utils/remote-test/logs/dongtu-orangepi-acceptance-20260609_2212/build_artifacts_dongtu_export_path_20260609.txt`

The final build exited with `BUILD_EXIT_CODE=0`.

## Deployment

Deployment was performed through `root@192.168.31.241`:

1. Stop RTOS: `rt stop`
2. Back up `/download/vm_3588.bin`
3. Upload fresh `vm_3588.bin`
4. Copy it to `/download/vm_3588.bin`
5. Start RTOS: `rt start`
6. Confirm `rt ifconfig` reports `192.168.31.207`

Deployment logs:

- `utils/remote-test/logs/dongtu-orangepi-acceptance-20260609_2212/deploy_export_path_vm3588_to_outer.log`
- `utils/remote-test/logs/dongtu-orangepi-acceptance-20260609_2212/outer_rt_start_export_path.log`

## Test-All Acceptance

Command executed in the inner RTOS shell:

```sh
rtbench test-all
```

Final result:

- Exit code: `0`
- Realtime: passed
- Schedule: passed, final score `100.00 / 100`
- Stress: passed with `all-quick`, `27` stressor runs
- Command test: passed
- Workload smoke: completed
- Result export: `/nfsd/rtbench_result.json`
- Outer fetch path: `/nfs_root/vm1/rtbench_result.json`
- Result JSON parse: passed

Final successful logs:

- `utils/remote-test/logs/dongtu-orangepi-acceptance-20260609_2212/telnet_test_all_final_success.log`
- `utils/remote-test/logs/dongtu-orangepi-acceptance-20260609_2212/test_all_final_summary.txt`
- `utils/remote-test/logs/dongtu-orangepi-acceptance-20260609_2212/fetch_final_result_json.log`
- `utils/remote-test/logs/dongtu-orangepi-acceptance-20260609_2212/rtbench_result_final_success.json`

## Code Notes

Dongtu now uses bounded defaults for bare `rtbench test-all`:

- Schedule defaults to quick parameters on `DONGTU_PLATFORM`.
- Stress defaults to `all-quick` on `DONGTU_PLATFORM`.
- Default result export path is `/nfsd/rtbench_result.json`, which maps to `/nfs_root/vm1/rtbench_result.json` on the outer Linux host.

These defaults keep the default command suitable for board acceptance while preserving explicit overrides such as `--schedule-cycles`, `--util-start`, `--util-end`, `--util-step`, and `--stress-job`.
