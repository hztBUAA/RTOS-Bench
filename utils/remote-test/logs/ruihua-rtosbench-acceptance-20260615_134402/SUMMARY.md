# Ruihua Feiteng RTOS-Bench Acceptance Summary

## Environment

- Host: Windows, workspace `C:\Users\hzt\yihui-workspace`
- Board: Ruihua/ReWorks on Phytium Pi, reached from `rtbench` host by telnet `192.168.31.210:23`
- RTOS: Ruihua ReWorks 6.1.1 ARM
- TFTP host: `rtbench@10.134.151.45:1026`
- TFTP image used: `/tftp/ruihua-feiteng-reworks-192.168.31.210-20260602.elf`
- Local accepted binary archive: `C:\Users\hzt\yihui-workspace\rtos-bench-artifacts\ruihua\feiteng\20260615_134402\reworks.elf`
- Accepted binary SHA256: `90BCF57E9B4FBABC6FE1FDC13E9E2D165A6DD475089608CBFC1598740F000DD5`
- Log directory: `utils/remote-test/logs/ruihua-rtosbench-acceptance-20260615_134402`

## Commands Executed

- Entry validation: `rtbench_list`
- P0 schedule: `rtbench_test_schedule_cycles3`
- P1 realtime: `rtbench_test_realtime`
- P2 workloads serial: `rtbench_fast`, `rtbench_epnp`, `rtbench_ekf`, `rtbench_icp`, `rtbench_modbus`, `rtbench_mqtt`, `rtbench_pid`, `rtbench_cusum`, `rtbench_ewma`
- P3 stress: `rtbench_test_stress`
- P4 test-all quick: `rtbench_test_all_quick`
- Export result: `rtbench_export_result_default`
- Result retrieval: `cat /rtbench_result.json`
- Report generation: `python utils/flatten_rtbench_result.py feiteng_export_20260615_1425.json --output feiteng_flattened_20260615_1425.json`

## Test Matrix

| Priority | Scope | Meaning | Command | Status | Return Code | Log File | Result File |
|---|---|---|---|---|---:|---|---|
| P0 | test-schedule | 调度测试 | `rtbench_test_schedule_cycles3` | PASS | 0 | `test_schedule_feiteng_20260615_1414.log` | n/a |
| P1 | test-realtime | 实时性能测试 | `rtbench_test_realtime` | PASS | 0 | `test_realtime_feiteng_20260615_1415.log` | `realtime_metrics_feiteng_20260615_1415.md` |
| P2 | workloads serial | 9 个 workload 串行测试 | `rtbench_<workload>` | PASS | 0 | `workloads_serial_feiteng_20260615_1418.log` | `workloads_matrix_feiteng_20260615_1418.md` |
| P3 | test-stress | 压力测试 | `rtbench_test_stress` | PASS | 0 | `test_stress_feiteng_20260615_1419.log` | n/a |
| P4 | test-all quick + export | quick 全链路与结果导出 | `rtbench_test_all_quick`; `rtbench_export_result_default` | PASS | 0 | `runner_testall_feiteng_20260615_1422.log`; `export_result_feiteng_20260615_1425.log` | `feiteng_export_20260615_1425.json`; `feiteng_flattened_20260615_1425.json` |

## Workloads Matrix

See `workloads_matrix_feiteng_20260615_1418.md`. All nine workloads passed with return code 0:

- `fast`
- `epnp`
- `ekf`
- `icp`
- `modbus`
- `mqtt`
- `pid`
- `cusum`
- `ewma`

## Result Files

- Board JSON recovered from `/rtbench_result.json`: `feiteng_export_20260615_1425.json`
- Test-all JSON copy: `feiteng_testall_20260615_1422.json`
- Flattened report: `feiteng_flattened_20260615_1425.json`
- Flattened record count: 188

## Remaining Risks

No blocking risks for current acceptance scope.

Operational notes:

- The accepted binary is archived on the Windows host, not committed into the Git repository.
- Do not overwrite shared `/tftp/reworks.elf`; Ruihua Feiteng uses the dedicated `ruihua-feiteng-*` TFTP image.
