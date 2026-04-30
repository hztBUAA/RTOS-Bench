# SylixOS Feiteng Acceptance Record

- Board: SylixOS Feiteng Pi, `192.168.31.204`
- Shell preparation: `shstack 120000`
- Binary: `feiteng-rtos-bench_hzt_exportsanitize_20260430_222216`
- SHA256: `625E1B837833469A82B7DF37A8DE016A6C6056EEC5C5112AC07954CA623865A2`
- Remote path: `/apps/hzt/feiteng-rtos-bench_hzt_exportsanitize_20260430_222216`
- Local build: `C:\Users\hzt\yihui-workspace\feiteng_rtos_bench\Debug\strip\hzt_feiteng_rtos_bench_exportsanitize_20260430_222216`

## Accepted Run

Command:

```sh
./feiteng-rtos-bench_hzt_exportsanitize_20260430_222216 test-all --no-stress --no-workload -o /apps/hzt/feiteng_testall_no_stress_workload_exportsanitize_clean_20260430_222542.json
```

Result:

- Exit status: `0`
- Schedule default config: `cycles=3`, `util_start=30`, `util_end=100`, `util_step=10`
- Gradients: `30, 40, 50, 60, 70, 80, 90, 100`
- Deadline misses: `0 / 216`
- Average miss rate: `0.0`
- Final score: `100.0`
- `test-cmd`: `5 / 11` supported commands, matching SylixOS shell capability observed in prior runs
- Export JSON: parsed successfully with Python `json.loads`

Artifacts:

- Full log: `exportsanitize_clean_testall_no_stress_workload_20260430_222542.log`
- Export JSON: `feiteng_testall_no_stress_workload_exportsanitize_clean_20260430_222542.json`
- Smoke export log: `exportsanitize_testall_export_smoke_20260430_222248.log`
- Smoke export JSON: `feiteng_testall_export_smoke_exportsanitize_20260430_222248.json`

Notes:

- This run validates the framework path used by `test-all` for realtime, default schedule, command probing, and JSON export.
- Stress and standalone workload suites were intentionally excluded in this acceptance run because the current ownership scope is the framework layer and `test-schedule`.
