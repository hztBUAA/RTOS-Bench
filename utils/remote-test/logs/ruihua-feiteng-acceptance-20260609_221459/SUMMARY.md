# Ruihua Feiteng Acceptance - 2026-06-09 22:14

Target board:

- Platform: Ruihua/ReWorks on Phytium Pi (AArch64)
- Board IP: `192.168.31.210`
- Telnet port: `23`
- Prompt: `reworks>`
- TFTP image: `/tftp/ruihua-feiteng-reworks-192.168.31.210-20260602.elf`
- Image SHA256: `3a4d5cc813af6614a512206d580d7ee076ae99f3d94b1d62d24473c3c41b3520`

Validation artifacts:

- Raw telnet transcript: `telnet_acceptance_raw.log`
- Machine-readable summary: `acceptance_summary.json`

## Results

- `rtbench_help`: passed, returned to `reworks>`, no module-link error.
- `rtbench_list`: passed, listed `stub`, `busywait`, and `ruihua-smoke`.
- `rtbench_test_cmd`: passed, `10/10 commands supported`.
- `rtbench_test_realtime`: passed, completed with code `0`.
- `rtbench_test_schedule_cycles3`: passed, `Final Score: 100.00 / 100`.
- `rtbench_test_stress`: passed, ran `all-quick`, completed `27 stressor runs`.
- `rtbench_test_all`: passed, returned to `reworks>`, saved `/rtbench_result.json`.

Key `rtbench_test_all` observations:

```text
Modules: realtime=yes schedule=yes stress=yes cmd=yes workload=yes
[test-schedule] ... Final Score: 100.00 / 100
[test-stress] Job 'all-quick' completed: 27 stressor runs
[test-cmd] Result: 10/10 commands supported
[RTOS-Bench] Results saved to: /rtbench_result.json
0x00000000 (0)
reworks>
```

## Notes

- `rtbench_test_stress` and `rtbench_test_all` use the Ruihua shell wrapper defaults:
  - `test-stress --job all-quick`
  - `test-all --schedule-cycles 3 --stress-job all-quick`
- This keeps interactive telnet acceptance bounded while still covering realtime, schedule, stress, command, and workload paths.
