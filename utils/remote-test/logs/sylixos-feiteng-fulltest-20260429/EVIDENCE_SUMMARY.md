# SylixOS Phytium Pi Full Test Evidence - 2026-04-29

## Artifact

- Source workspace: `C:\Users\hzt\yihui-workspace\feiteng_rtos_bench`
- RTOS-Bench source: symlink to `C:\Users\hzt\yihui-workspace\rtos-bench\RTOS-Bench`
- Build log: `build_feiteng_20260429_192025.log`
- Local binary: `C:\Users\hzt\yihui-workspace\feiteng_rtos_bench\Debug\strip\hzt_feiteng_rtos_bench_20260429_192025`
- Local binary SHA256: `4DC8A6A437F201188B91823CB29A74F4413A4DC9C8EDC7E7A816C5985DE7D58F`
- Remote binary: `/apps/hzt/feiteng-rtos-bench_hzt_20260429_192025`
- Deploy log: `deploy_feiteng_hzt_20260429_192025.log`

## Command

```sh
/apps/hzt/feiteng-rtos-bench_hzt_20260429_192025 test-all -o /apps/hzt/feiteng_testall_full_20260429_192245.json
```

## Result

- Full log: `full_testall_feiteng_20260429_192245.log`
- Script stdout: `run_full_testall_feiteng_20260429_192244.stdout.log`
- Status: FAIL
- JSON export: not generated
- Realtime phase: completed with code 0
- Schedule phase: crashed during Phase 1 WCET measurement before the utilization gradients started

## Crash Point

The failure occurred after these WCET measurements:

- `fast`: `68.106 ms`
- `epnp`: `27.498 ms`
- `ekf`: `1091.893 ms`

The next workload was ICP:

```text
[ICP] Running ICP (point-to-plane)
```

The backtrace then points into the ICP workload, not the entry parser or timer wrapper:

```text
[10] ... IcpPointToPlane::fitStep(...)+3080
[09] ... Icp::fitIterate(...)+476
[08] ... Icp::fit(...)+340
[07] ... icp_bench_run+224
[06] ... run_all_workloads+912
[03] ... test_schedule_run_custom+808
PC = 0x000004000042393c
LR(X30) = 0x000004000042393c
```

## Initial Classification

This is a real-board runtime crash triggered by `test-all` while it is executing the `test-schedule` module. The stack evidence points to the ICP workload implementation on SylixOS Phytium Pi, specifically `IcpPointToPlane::fitStep`, rather than to the framework-level command parser, result export, SylixOS timer creation, or schedule wrapper logic.

The colleague-provided sample showed a different stack in `stress_vm/test6_4`. This run reproduces the broader "full test can crash" class on the same board, but the concrete failing path in this capture is schedule WCET -> ICP.
