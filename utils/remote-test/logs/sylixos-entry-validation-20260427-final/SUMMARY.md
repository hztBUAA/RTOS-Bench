# SylixOS Entry / Schedule Validation Summary

Validation batch: `20260427_233446`

Branch: `fix/sylixos-test-schedule0427`

## Verdict

The full remote acceptance batch used locally built SylixOS binaries deployed with unique remote names ending in `-r4` and passed on the reachable boards:

| Board | Result | Entry checks | `test-all --quick` export | JSON parse | `test-schedule --cycles 3` |
| --- | --- | ---: | --- | --- | --- |
| `gongkong` | PASS | 10/10 | PASS | PASS | PASS |
| `orangepi` | PASS | 10/10 | PASS | PASS | PASS |
| `loongson` | PASS | 10/10 | PASS | PASS | PASS |

Skipped by current board availability: Feiteng/Phytium Pi and Nezha.

The formal schedule acceptance command was **not** quick:

```sh
test-schedule --cycles 3
```

It ran the eight default utilization gradients from 30% to 100%:

```c
TEST_SCHEDULE_UTIL_START = 30
TEST_SCHEDULE_UTIL_END   = 100
TEST_SCHEDULE_UTIL_STEP  = 10
```

All three boards finished with `Final Score: 100.00 / 100` and `Average Miss Rate: 0.0000`.

After this batch, one help-text-only cleanup changed top-level `--help` schedule default descriptions from hard-coded literals to the same `TEST_SCHEDULE_*` macros already used by the parser and `test-schedule --help`. The three binaries were rebuilt locally as `r5`; remote redeployment could not be rerun in the current shell because `RTBENCH_JUMPHOST_PASSWORD` was not present. The r4 remote acceptance remains the behavior baseline for schedule/test-all execution.

## Runtime Snapshot

| Board | Full validation | Before schedule | Schedule approx | Max period | Max period workload | `test-all --quick` duration |
| --- | ---: | ---: | ---: | ---: | --- | ---: |
| `gongkong` | 35.76 min | 5.87 min | 29.89 min | 95,817.204 ms | `modbus` @ 30% | 347 sec |
| `orangepi` | 34.29 min | 7.71 min | 26.58 min | 77,546.884 ms | `modbus` @ 40% | 454 sec |
| `loongson` | 50.33 min | 10.01 min | 40.32 min | 131,862.636 ms | `ekf` @ 30% | 595 sec |

Loongson is the main time sink because its measured schedule workload costs are much larger, especially EKF/ICP class workloads. The period matrix and per-workload timing are recorded in:

```text
utils/remote-test/logs/sylixos-entry-validation-20260427-final/outputs/sylixos_schedule_validation_20260427.xlsx
```

## Quick vs Non-Quick

The `test-all` smoke command used for entry/export validation was:

```sh
test-all --quick --schedule-cycles 1 --util-start 30 --util-end 30 --util-step 30 --stress-job all-quick -o <json>
```

This intentionally reduces:

- schedule to one 30% gradient and one cycle;
- stress to `all-quick`;
- workload collection rounds from 10 to 5;
- quick workload collection uses schedule wrappers for bounded MQTT/MODBUS execution.

This path validates entry parsing, module orchestration, export-result, fetched JSON, and end-to-end command stability. It is **not** the final schedule acceptance path.

The separate acceptance run used:

```sh
test-schedule --cycles 3
```

without `--quick`, across all eight formal utilization gradients.

A naked `test-schedule` or naked `test-all` remains functionally valid, but uses `TEST_SCHEDULE_CYCLES = 10000`, which is impractical for this board set. Based on the measured `cycles=3` runtime, Loongson would scale to an extremely long wall time if left unconstrained.

## Wrapper Effect

`generator/test_schedule.c` uses `sched_get_wrapper(wl->name)` and dispatches `wrapper->quick_exec()` when a schedule wrapper exists. Today this affects MQTT and MODBUS. That means schedule feasibility measures bounded network workload representatives instead of allowing network timeout/retry behavior to dominate WCET and period assignment.

This does not invalidate schedule discrimination among CPU/perception/control workloads, and WCET/runtime use the same dispatch path. It does mean full network throughput/timeout behavior is intentionally outside the schedule score and should be validated separately if needed.

## Why The Later `test-all` Crash Appeared

The earlier successful `test-all --quick` run did use a locally rebuilt binary, but it was a single pass and did not expose the SylixOS/AArch64 string-stressor crash. The later rerun hit the existing code path during `test-stress all-quick`:

- log evidence: `entry_validation_orangepi_20260427_231518.log`
- failing stage: `Running Job ... -> str`
- backtrace: PC in SylixOS libc `strcmp`, LR in RTOS-Bench `stress_osal_strcmp`
- register evidence: `X0 == X1`

The corresponding source path is:

- `generator/stress_orig/common/stress_stored_job.c`: `all-quick` includes `str --ops 130000 -c 1 --str-size 1024`
- `generator/stress_orig/stressor/stress-str.c`: the workload calls `stress_osal_strcmp(str1, str1)` and related cases
- `generator/stress_orig/osal/os_sylixos.c`: `stress_osal_strcmp` previously delegated directly to libc `strcmp`

The fix replaces the SylixOS OSAL `strcmp`/`strncmp` wrappers with small local safe implementations that handle identical pointers and null guards before walking bytes.

One more apparent issue after that was not an ELF build corruption: after force-stopping the failed run and overwriting the same remote executable names, the next deployment hit SylixOS loader/FPU-type/module-init corruption. Local ELF inspection still showed the expected `hard-float` symbol state. Deploying the same rebuilt binaries under unique `-r4` names removed that failure mode, so the final logs use the `-r4` remote names.

## Key Artifacts

- Runner log: `runner_entry_validation_r4_20260427_233446.log`
- Latest local rebuild logs:
  - `build_gongkong_r5.log`
  - `build_orangepi_r5.log`
  - `build_loongson_r5.log`
- Board logs:
  - `entry_validation_gongkong_20260427_233446.log`
  - `entry_validation_orangepi_20260427_233446.log`
  - `entry_validation_loongson_20260427_233446.log`
- Summary JSON: `entry_validation_summary_20260427_233446.json`
- Export JSON:
  - `gongkong_export_20260427_233446.json`
  - `orangepi_export_20260427_233446.json`
  - `loongson_export_20260427_233446.json`
- `test-all` JSON:
  - `gongkong_testall_20260427_233446.json`
  - `orangepi_testall_20260427_233446.json`
  - `loongson_testall_20260427_233446.json`
- Workbook: `outputs/sylixos_schedule_validation_20260427.xlsx`
