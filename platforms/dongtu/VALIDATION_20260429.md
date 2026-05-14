# Dongtu Orange Pi Validation - 2026-04-29

## Target

- Platform: Dongtu / Intewell `vm_3588` on Orange Pi
- Branch: `feat/dongtu-orangepi-0428`
- Binary: `vm_3588_hzt_20260429_191126_cmd_wrapper_normalize.bin`
- SHA256: `A407B42CBAE8B80BE5A293D4C203DF48A1CEC43853EEBB9FDBD145230A80D1FB`
- Size: `5234688` bytes
- Deployed as: `download/vm_3588/Debug/make/vm_3588.bin`

## Build And Deploy Evidence

- Build log: `utils/remote-test/logs/dongtu-orangepi-20260428-171416/build_20260429_191126_cmd_wrapper_normalize_serial.log`
- Deploy log: `utils/remote-test/logs/dongtu-orangepi-20260428-171416/deploy_restart_20260429_191126_cmd_wrapper_normalize.log`
- Shell smoke: `rtbench -L` lists 9 industrial workloads plus 2 utility workloads.

## Standard Schedule

Command:

```sh
rtbench test-schedule
```

Result:

- Log: `utils/remote-test/logs/dongtu-orangepi-20260428-171416/telnet_test_schedule_default_20260429_161133_final_standard.stdout.log`
- Status: prompt returned
- Duration: `1959.719` seconds
- Workloads: 9 industrial workloads
- Utilization gradients: 30%, 40%, 50%, 60%, 70%, 80%, 90%, 100%
- Default WCET iterations: 5
- Default cycles: 3
- Jobs per gradient: 27
- Final score: `100.00 / 100`
- Misses: 0 at every gradient

This standalone run used the immediately previous binary
`vm_3588_hzt_20260429_161133_final_no_trace.bin`. The only code change after
that run normalized Dongtu shell-command return handling in `test_cmd`; the
final `191126` binary revalidated the same default schedule path inside
`test-all --no-stress`.

Observed WCET values in the final `test-all --no-stress` schedule run:

| Workload | WCET ms |
| --- | ---: |
| fast | 43.008 |
| epnp | 2.553 |
| ekf | 164.729 |
| icp | 2559.341 |
| modbus | 8105.861 |
| mqtt | 2008.968 |
| pid | 77.400 |
| cusum | 36.018 |
| ewma | 201.370 |

## Test-All Scope

`test-all --no-stress` is the validation scope for this PR because stress is owned separately and currently stalls on Dongtu at `test-stress --job cpu`.

Previous command:

```sh
rtbench test-all --no-stress
```

Previous result:

- Log: `utils/remote-test/logs/dongtu-orangepi-20260428-171416/telnet_test_all_no_stress_20260429_161133_final.stdout.log`
- Status: telnet capture idle timeout, not a reported rtbench failure
- Duration before capture timeout: `1289.351` seconds
- Reason: after realtime, schedule measured Modbus WCET as `33653.292 ms`; at 30% utilization this produced a `403439.838 ms` Modbus period, so 3 cycles can remain silent for longer than the original 1200 second idle capture window.

Long-idle rerun command:

```sh
rtbench test-all --no-stress
```

Capture settings:

- Total timeout: 14400 seconds
- Idle timeout: 3000 seconds
- Final log: `utils/remote-test/logs/dongtu-orangepi-20260428-171416/telnet_test_all_no_stress_20260429_191126_cmdwrap_longidle_20260429_191442.stdout.log`
- Status: prompt returned
- Duration: `1546.986` seconds
- Schedule score: `100.00 / 100`
- `test-cmd`: `5/5` commands supported (`date`, `task`, `pwd`, `ls`, `version`)
- Exported JSON: `utils/remote-test/logs/dongtu-orangepi-20260428-171416/dongtu_rtbench_result_20260429_191126.json`
- JSON parse: passed with `test-schedule`, `test-cmd`, and `typical-workload` all marked `passed`

## Notes

- A first long-idle `test-all --no-stress` run on the `161133` binary completed, but `test-cmd` reported `task` as unsupported even though the command printed its task table. Dongtu's `shell_main_task()` can return a negative value after printing successfully, so the final fix treats mapped Dongtu shell commands as supported when they dispatch to the RTCore shell function. Unknown commands still fail.
- `make -j4 all` can trip Intewell's nested make jobserver on Windows with `invalid --jobserver-auth`; the final rebuild used serial `make all` and produced a valid binary.

## Out Of Scope But Recorded

- `rtbench test-stress --job cpu` stalls at `Running Job: 0/65 -> cpu` on Dongtu.
- The stress issue is intentionally not fixed in this branch.
