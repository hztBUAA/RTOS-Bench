# SylixOS test-schedule final validation summary

Date: 2026-04-27
Branch: `fix/sylixos-test-schedule`
Validation command: `test-schedule --cycles 1`

## Source changes validated

- `generator/test_schedule.c`: use schedule-specific workload wrappers for WCET measurement and runtime execution.
- `generator/test_schedule/sched_mqtt_wrapper.c`: cap MQTT quick workload runtime at 2 seconds to avoid indefinite waits.
- `generator/platform/sylixos/version_override.c`: export SylixOS module version `0.0.0` from a text section so SylixOS 3.9.0 boards accept binaries built with the local 3.2.8 SDK headers.
- `platforms/sylixos/rtos-bench.mk`: include the SylixOS version override and schedule wrapper sources.
- `generator/sylixos_entry.c`: make command parsing explicit; help/invalid/unknown options print usage and return without launching tests.
- `docs/ENTRY_ARGUMENTS_SOP.md`: document the cross-platform entry parser SOP.

## Latest local binaries

| Board target | Local binary | Bytes | SHA256 |
| --- | --- | ---: | --- |
| OrangePi / Feiteng | `C:\Users\hzt\yihui-workspace\feiteng_rtos_bench\Debug\strip\feiteng_rtos_bench` | 6441784 | `E9C4AF5E7ECC941D7E39714BCD76F36CFD9FAE54F63F1A62352DD28782080D39` |
| Gongkong | `C:\Users\hzt\yihui-workspace\gongkong_rtos_bench\Debug\strip\gongkong_rtos_bench` | 6904960 | `72A0B650C49501B1A99E25693A4761BC75AFC2612FAB98975BBD4374E6404BC0` |
| Nezha | `C:\Users\hzt\yihui-workspace\nezha-rtos-bench\Debug\strip\nezha-rtos-bench` | 6472320 | `435D841C546E9F00EAB10504239692EFBAB2BE7E8507253196C841939DAC2C9A` |
| Loongson | `C:\Users\hzt\yihui-workspace\rtos-bench\Debug\strip\rtos-bench` | 7810480 | `00168FDA96C9D16876590C3D457C051DE3A4C5B84A281BA2289991DC8B58B2A9` |

Build logs:

- `build_latest_orangepi_feiteng_20260427_171321.log`: first parallel ARM64 build hit a make/mkdir race.
- `build_latest_orangepi_feiteng_retry_20260427_171615.log`: sequential retry succeeded.
- `build_latest_gongkong_20260427_171321.log`: succeeded.
- `build_latest_nezha_20260427_171321.log`: succeeded.
- `build_latest_loongson_20260427_171321.log`: succeeded.

## Deployment status

| Board | Status | Evidence |
| --- | --- | --- |
| OrangePi | Uploaded latest binary to `/apps/hzt/orangepi-rtos-bench` | `deploy_latest2_orangepi_20260427_171925.log` |
| Gongkong | Uploaded latest binary to `/apps/hzt/gongkong-rtos-bench` | `deploy_latest2_gongkong_20260427_171925.log` |
| Loongson | Uploaded latest binary to `/apps/hzt/loongson-rtos-bench` | `deploy_latest2_loongson_20260427_171925.log` |
| Nezha | Uploaded latest binary, but telnet refused validation with `server is full of links` | `deploy_latest2_nezha_20260427_171925.log`, `test_latest_cycles1_nezha_attempt_20260427_175249.log` |
| Feiteng | Latest binary exists locally, deployment blocked because `192.168.31.204` is unreachable | `deploy_latest2_feiteng_20260427_175249.log` |

## cycles=1 validation results

The runnable boards were executed from unique remote filenames after `dlconfig refresh` to avoid SylixOS loader/share-cache reuse by basename.

| Board | Remote unique path | Result |
| --- | --- | --- |
| OrangePi | `/apps/hzt/orangepi-rtos-bench-latest-20260427_172334` | PASS, `Average Miss Rate: 0.0000`, `Final Score: 100.00 / 100` |
| Gongkong | `/apps/hzt/gongkong-rtos-bench-latest-20260427_172334` | PASS, `Average Miss Rate: 0.0000`, `Final Score: 100.00 / 100` |
| Loongson | `/apps/hzt/loongson-rtos-bench-latest-20260427_172334` | PASS, `Average Miss Rate: 0.0000`, `Final Score: 100.00 / 100` |

Result logs:

- `test_latest_cycles1_orangepi_20260427_172334.log`
- `test_latest_cycles1_gongkong_20260427_172334.log`
- `test_latest_cycles1_loongson_20260427_172334.log`

Entry parser prechecks recorded in each validation log:

- `--help` prints top-level usage.
- `test-schedule --help` prints test-schedule usage.
- `test -schedule --help` prints `[rtbench] Unknown option or command: test` and does not run the benchmark.
- `test-schedule --cycles abc` prints `[rtbench] Invalid cycles: abc` and does not run the benchmark.

No final `cycles=1` result log contains the known bad markers `OS-version`, `Can not find dependent library`, or `timer_create failed`.

## cycles=2 boundary validation

After the `cycles=1` milestone, the runnable boards were re-tested with `test-schedule --cycles 2` using the same latest deployed binaries, unique remote filenames, and `dlconfig refresh` precheck flow.

Batch: `20260427_191911`

| Board | Remote unique path | Result |
| --- | --- | --- |
| OrangePi | `/apps/hzt/orangepi-rtos-bench-cycles2-20260427_191911` | PASS, `Average Miss Rate: 0.0000`, `Final Score: 100.00 / 100` |
| Gongkong | `/apps/hzt/gongkong-rtos-bench-cycles2-20260427_191911` | PASS, `Average Miss Rate: 0.0000`, `Final Score: 100.00 / 100` |
| Loongson | `/apps/hzt/loongson-rtos-bench-cycles2-20260427_191911` | PASS, `Average Miss Rate: 0.0000`, `Final Score: 100.00 / 100` |

Result logs:

- `test_latest_cycles2_orangepi_20260427_191911.log`
- `test_latest_cycles2_gongkong_20260427_191911.log`
- `test_latest_cycles2_loongson_20260427_191911.log`

Each completed utilization gradient reported `MR=0.0000 (0/18)`. No final `cycles=2` result log contains the known bad markers `OS-version`, `Can not find dependent library`, or `timer_create failed`.

## Loader/cache evidence

The earlier successful validation was based on rebuilt board binaries, not a deliberately old binary. The risk was that SylixOS could keep stale module state when repeatedly using the same remote basename.

Relevant code evidence:

- Local SDK header `k_kernel.h` stamps modules as SylixOS `3.2.8`, while validation boards report SylixOS `3.9.0`.
- SylixOS loader accepts SO module version `0.0.0`, so `version_override.c` exports that compatible value.
- SylixOS loader code paths can find loaded modules by basename/path and can reuse loader/share state. Final validation therefore copies the deployed binary to a unique remote filename and runs `dlconfig refresh` before execution.

## Full/default cycles note

`TEST_SCHEDULE_CYCLES` defaults to `10000`. With the current WCET-derived periods, a default run is not practical in this validation window. For example, the latest `cycles=1` Loongson log shows 30% utilization periods up to `169231.598 ms`; a single gradient at `cycles=10000` would take about 19.6 days, before the remaining gradients.

This commit therefore marks the latest-code SylixOS `test-schedule --cycles 1` milestone. Boundary validation should increase `--cycles` gradually and record the largest completed value, or adjust the schedule model/period cap before treating the default `10000` cycles as a formal end-to-end run.
