# RTOS-Bench Entry Unification Baseline

This memo records the board access information and the end-to-end baselines that
must be re-run after moving platform entries to the shared `rtbench_command`
dispatcher.

## Jump Host

- SSH jump host: `10.134.151.45:1026`
- User: `rtbench`
- Password used by the automation when no key is available: `rtbench`
- Board login: usually `root/root`; try `root/rtbench` if a board differs.
- Canonical source worktree for future changes and commits:
  `C:\Users\hzt\yihui-workspace\rtos-bench\RTOS-Bench`.
  Do not continue split maintenance in `RTOS-Bench-ruihua`.

## Board Access Matrix


| Board      | OS      | Board IP         | FTP    | Shell  | Notes                                                                                         |
| ---------- | ------- | ---------------- | ------ | ------ | --------------------------------------------------------------------------------------------- |
| Loongson   | SylixOS | `192.168.31.200` | `3006` | `3008` | Validated via jump host telnet to `192.168.31.200:23`                                         |
| OrangePi   | SylixOS | `192.168.31.201` | `3009` | `3011` | Historical SylixOS PASS logs exist                                                            |
| Nezha      | SylixOS | `192.168.31.202` | `3012` | `3014` | Historical availability varied                                                                |
| Gongkong   | SylixOS | `192.168.31.203` | `3015` | `3017` | Historical SylixOS PASS logs exist                                                            |
| Feiteng Pi | SylixOS | `192.168.31.204` | `3018` | `3020` | Has dedicated SylixOS Feiteng acceptance notes                                                |
| Feiteng Pi | OneOS   | `192.168.31.205` | `3021` | `3023` | Preserve historical `.out` module flow                                                        |
| Gongkong   | Dongtu  | `192.168.31.206` | `3024` | `3026` | Preserve historical Dongtu deployment flow                                                    |
| OrangePi   | Dongtu  | `192.168.31.207` | `21`   | `23`   | VM shell behind Orange Pi 5 host `192.168.31.241`; `3028` was also noted as SSH in older docs |


Connection must be tested after logging into the jump host first:

```sh
ssh -p1026 rtbench@10.134.151.45
```

Then try both the board-local ports (`192.168.31.x:21/23`) and the campus
forwarded ports (`10.134.151.45:<ftp/telnet>` from the table). As of
2026-05-08, the confirmed connectivity matrix is:

| Board | Internal FTP | Internal Telnet | Forwarded FTP | Forwarded Telnet | Current validation path |
| --- | --- | --- | --- | --- | --- |
| Loongson SylixOS | `192.168.31.200:21` OK | `192.168.31.200:23` OK | `10.134.151.45:3006` refused | `10.134.151.45:3008` OK, but can report `server is full of links` | Prefer `10.134.151.45:3008`; fall back to `192.168.31.200:23` from jump host |
| OrangePi SylixOS | `192.168.31.201:21` no route | `192.168.31.201:23` no route | `10.134.151.45:3009` refused | `10.134.151.45:3011` no route | Not reachable |
| Nezha SylixOS | `192.168.31.202:21` OK | `192.168.31.202:23` OK | `10.134.151.45:3012` refused | `10.134.151.45:3014` OK | Use `10.134.151.45:3014` for telnet; use internal `:21` for FTP if needed |
| Gongkong SylixOS | `192.168.31.203:21` OK | `192.168.31.203:23` OK | `10.134.151.45:3015` OK | `10.134.151.45:3017` OK | Use forwarded `3015/3017` |
| Feiteng SylixOS | `192.168.31.204:21` no route | `192.168.31.204:23` no route | `10.134.151.45:3018` no route | `10.134.151.45:3020` no route | Not reachable |
| Feiteng OneOS | `192.168.31.205:21` no route | `192.168.31.205:23` no route | `10.134.151.45:3021` no route | `10.134.151.45:3023` no route | Not reachable; use serial or restore forwarding |

## Historical Workspaces

- Dongtu/Intewell historical workspace:
`D:\build\workspace\Developer_231Gizwits\IDE\BIN\Intewell_Developer\eclipse\workspace`
- Dongtu SOP:
`D:\build\workspace\Developer_231Gizwits\IDE\BIN\Intewell_Developer\eclipse\workspace\DEPLOY_SOP.md`
- OneOS historical workspaces:
`C:\OneOSStudio\workspace`,
`C:\OneOSStudio\workspace\phytium_pi_out`
- OneOS/SylixOS board testing SOP backup:
`C:\OneOSStudio\workspace\phytium_pi_out\RTOS-Bench_backup_20260429_200245\docs\SOP.md`

Dongtu SOP confirms the practiced Orange Pi 5 Intewell topology:

- Ubuntu host: `192.168.31.241`
- RTOS VM: `192.168.31.207`
- RTOS VM shell: telnet `192.168.31.207:23`
- RTOS VM FTP: `192.168.31.207:21`
- Eclipse projects: `os` and `vm_3588`
- Build artifacts:
`os/Release/merge/config.bin`,
`vm_3588/Debug/make/vm_3588.bin`
- Deployment helper scripts expected on jump host:
`ssh_cmd.py`, `scp_upload.py`, `telnet_vm.py`
- Restart flow:
`rt stop`, `rt start`, `rt status`, `rt ifconfig`

## Current Known-Good Loongson Binary

- Remote executable: `/apps/hzt/loongson-rtos-bench`
- Do not rely on the default root output path on SylixOS. The Loongson board can
create `/rtbench_*.json` as a zero-byte file but cannot write content there.
- Use an explicit writable result path:

```sh
/apps/hzt/loongson-rtos-bench test-all --quick --schedule-cycles 1 --util-start 30 --util-end 30 --util-step 30 --stress-job all-quick -o /apps/hzt/loongson_fullquick_<batch>.json
```

Latest observed result before entry unification:

- `Final Score: 100.00 / 100`
- `[RTOS-Bench] Results saved to: /apps/hzt/loongson_fullquick_20260508_005922.json`
- JSON size: `8909 B`

## Historical Evidence To Preserve

- SylixOS entry validation:
`utils/remote-test/logs/sylixos-entry-validation-20260427-final/SUMMARY.md`
- SylixOS schedule validation:
`utils/remote-test/logs/sylixos-test-schedule-20260427/SUMMARY.md`
- SylixOS full `test-all` baseline:
`utils/remote-test/logs/sylixos-full-testall-20260428-default3/full_testall_summary_20260428_124312.json`
- SylixOS Feiteng acceptance:
`utils/remote-test/logs/sylixos-feiteng-fulltest-20260429/FEITENG_ACCEPTANCE_20260430_222542.md`
- Board metadata:
`utils/remote-test/boards/BOARDS.md`
- Git history to inspect for OneOS/Dongtu baseline recovery:
`origin/fix/oneos-entry-api`, `origin/feat/dongtu-orangepi-0428`,
`origin/feat/multi-platform-entry-0420`.

Historical full-run timing matters for validation timeouts:

- SylixOS Loongson full baseline:
  `/apps/hzt/loongson-rtos-bench-default3 test-all -o /apps/hzt/loongson_testall_full_20260428_124312.json`
  finished with `[BOARD_RESULT] loongson: PASS` and `Results saved to`.
- SylixOS Gongkong full baseline:
  `/apps/hzt/gongkong-rtos-bench-default3 test-all -o /apps/hzt/gongkong_testall_full_20260428_124312.json`
  finished with `[BOARD_RESULT] gongkong: PASS` and `Results saved to`.
- Dongtu OrangePi historical commit:
  `2fdbfa7 feat(dongtu): validate Orange Pi schedule entry flow`.
  `rtbench test-schedule` took `1959.719` seconds and returned
  `Final Score: 100.00 / 100`.
- Dongtu `test-all --no-stress` historical acceptance used a 14400 second total
  timeout and 3000 second idle timeout; it completed in `1546.986` seconds.
- OneOS `.out` split-module historical commit:
  `37a4d46 feat(oneos): ... 按照.out进行模块分离式的编译`.
  The kernel-side `rtbench` command loads `/user/phytium_pi_out.out` and looks
  up module symbol `cmd_rtbench_stub`.

## Required Post-Migration Checks

For every migrated platform entry:

```sh
rtbench --help
rtbench export-result -o <writable-dir>/probe.json
rtbench test-all --quick --schedule-cycles 1 --util-start 30 --util-end 30 --util-step 30 --stress-job all-quick -o <writable-dir>/quick.json
rtbench test-schedule --cycles 1 --util-start 30 --util-end 30 --util-step 30
rtbench bad-command
```

Acceptance requires preserving the historical board behavior or documenting a
known pre-existing board/BSP limitation. Do not remove a platform's old entry
logic from review context until its real-board logs meet or exceed the baseline.

## Validation Runs

- SylixOS Nezha, framework/basic checks:
  `utils/remote-test/logs/entry-unification-nezha-sylixos-20260508_140419/SUMMARY.md`
  passed `--help`, explicit `export-result -o /apps/hzt/...json`, and bad-command rejection through `10.134.151.45:3014`.
- SylixOS Nezha, schedule acceptance:
  `utils/remote-test/logs/entry-unification-nezha-sylixos-20260508_140501/SUMMARY.md`
  passed `/apps/hzt/nezha-rtos-bench test-schedule --cycles 3` with `Final Score: 100.00 / 100`.
  Manual telnet entry for later verification:
  `proxychains telnet 10.134.151.45 3014`, executable
  `/apps/hzt/nezha-rtos-bench`.
- SylixOS Nezha, latest rebuilt entry-unification binary:
  local source project `C:\Users\hzt\yihui-workspace\nezha-rtos-bench`,
  local unstripped artifact `Debug\nezha-rtos-bench`, SHA256
  `D430CBDC10D9A27E619E85776F5533C29C9EB0EF99EE98277269B196983956E6`.
  The stripped artifact generated during the same build had SHA256
  `B51B6171376F6776809D76E2EB6F826FF735927BC7FFA3E935AB822D660957BA`.
  The same-name
  remote path `/apps/hzt/nezha-rtos-bench` was overwritten, but the SylixOS
  loader still reported a stale same-name module/dependency error. Use the
  confirmed unique latest runtime path `/apps/hzt/nezha-rtos-bench-r7`, which
  prints `test-all --help` and reports default output
  `/apps/hzt/rtbench_result.json`.
  Full `test-all` on r7 is not yet accepted: the 2026-05-08 LAVA run timed out
  after 10800 seconds while still in `test-realtime` before entering
  `test-schedule`; `/apps/hzt/nezha_testall_full_r7_20260508.json` was not
  created. As of 2026-05-09, Nezha telnet returns `server is full of links`
  while FTP remains reachable, likely due to stale telnet sessions after the
  timeout. Treat this as a board/session blocker until telnet is released or
  the board is restarted.
- SylixOS Nezha r7 after board restart, bounded end-to-end smoke:
  `utils/remote-test/logs/entry-unification-nezha-r7-smoke-20260509_141438/SUMMARY.md`.
  The run used `/apps/hzt/nezha-rtos-bench-r7 test-all --quick --schedule-cycles 1 --util-start 30 --util-end 30 --util-step 30 --stress-job all-quick -o /apps/hzt/nezha_r7_smoke_20260509_141438_quick.json`.
  It completed `test-realtime`, quick `test-schedule`, stress, command probing,
  and typical workloads, then saved an 8864-byte JSON result to `/apps/hzt`.
  This restores the historical quick end-to-end baseline after reboot; full
  non-quick `test-all` still needs a longer dedicated run.
- SylixOS Gongkong, framework/basic checks:
  `utils/remote-test/logs/entry-unification-gongkong-sylixos-20260508_140428/SUMMARY.md`
  passed `--help`, explicit `export-result -o /apps/hzt/...json`, and bad-command rejection through `10.134.151.45:3017`.
- SylixOS Gongkong, schedule acceptance:
  `utils/remote-test/logs/entry-unification-gongkong-sylixos-20260508_145755/SUMMARY.md`
  passed `/apps/hzt/gongkong-rtos-bench test-schedule --cycles 3` with `Final Score: 100.00 / 100`.
- Dongtu OrangePi, schedule acceptance:
  `utils/remote-test/logs/entry-unification-dongtu-orangepi-20260508_122132/SUMMARY.md`
  passed `rtbench test-schedule --cycles 3` with `Final Score: 100.00 / 100`.

## Runtime Expectations

Do not classify full validation as failed with short smoke timeouts. Historical
logs show that complete board checks are long-running:

- SylixOS Loongson entry validation: about `50.33 min` total.
- SylixOS Loongson `test-all --quick ...`: about `595 sec`.
- SylixOS Loongson standalone `test-schedule --cycles 3`: about `40.32 min`.
- Dongtu OrangePi standalone `rtbench test-schedule`: `1959.719 sec`.
- Dongtu OrangePi `rtbench test-all --no-stress`: `1546.986 sec` with a
  `14400 sec` total timeout and `3000 sec` idle timeout.

For SylixOS, reuse the historical validation runner settings:

```sh
python utils/remote-test/scripts/run_sylixos_entry_validation.py \
  --boards loongson,orangepi,gongkong \
  --log-dir utils/remote-test/logs/<batch> \
  --schedule-cycles 3 \
  --schedule-timeout-sec 7200
```

The `test-all --quick` command is an entry/export smoke check. The formal
schedule acceptance remains non-quick `test-schedule --cycles 3`.

## OneOS Notes

The practiced OneOS flow uses a split kernel/module design:

- Kernel workspace: `C:\OneOSStudio\workspace\phytium_pi`
- Module workspace: `C:\OneOSStudio\workspace\phytium_pi_out`
- Module path on board: `/user/phytium_pi_out.out`
- Kernel command stub: `phytium_pi/application/rtbench_cmd_stub.c`
- Runtime command flow:

```sh
unld /user/phytium_pi_out.out
ld /user/phytium_pi_out.out
rtbench test-schedule --cycles 3
```

The kernel stub resolves `cmd_rtbench_stub` from `/user/phytium_pi_out.out`, so
the OneOS entry must keep exporting that symbol even after migrating to the
shared dispatcher.

### 2026-05-15 OneOS V1.5 TFTP Acceptance

- Local module: `C:\OneOSStudio\workspace\phytium_pi_out\out\phytium_pi_out.out`
- SHA256: `758C5DB290FADADFB3813D1813002A813E98753D1ED51C41B6951A0C87391B5A`
- TFTP source: `192.168.31.110:/tftp/phytium_pi_out.out`
- Board module: `/user/phytium_pi_out.out`
- Validation log:
  `utils/remote-test/logs/oneos-tftp-runtime-20260515_013024/terminal.log`
- Summary:
  `utils/remote-test/logs/oneos-tftp-runtime-20260515_013024/SUMMARY.md`

Validated commands on the Phytium Pi OneOS board:

```sh
unld /user/phytium_pi_out.out
tftp_client 192.168.31.110 get phytium_pi_out.out /user/phytium_pi_out.out
ld /user/phytium_pi_out.out
rtbench --help
rtbench -L
rtbench -b stub -t 1
rtbench -b busywait -t 1
rtbench test-schedule --cycles 1 --util-start 30 --util-end 30 --util-step 30
```

Acceptance result: `stub`, `busywait`, workload listing, and the schedulability
smoke all passed. The schedulability run measured all 9 industrial workloads and
completed with `Final Score: 100.00 / 100`.

Root-cause notes for the OneOS V1.5 fix:

- `periodic_benchmark` must not register `atexit()` from the `.out` module. On
  this image the call printed `exec atexit...` and blocked before the workload
  reached timer setup. The benchmark now relies on its explicit cleanup path on
  OneOS.
- OneOS must use the lightweight RTOS `cpu_set_t` mask used by other POSIX-lite
  RTOS targets. The POSIX `<sched.h>` CPU macros blocked setup before
  `Execution environment setup complete`.
- OneOS kernel timers were replaced by a task-based timer shim for RTOS-Bench
  period callbacks. This avoids missing callbacks from dynamically loaded
  modules and lets both workload execution and `test-schedule` advance.
- The validation script now requires shell prompts for PASS, stops on first
  command failure, supports both jump-host direct TCP and forwarded telnet
  ports, and does not send `exit` at the end because that can leave the board
  telnet service without a fresh prompt for the next run.
