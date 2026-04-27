# SylixOS test-schedule validation - 2026-04-27

All deploy and test logs in this directory were produced after a clean local rebuild at about 2026-04-27 13:44.

## Local binaries used

| Board target | Local artifact | Size | SHA256 |
| --- | --- | ---: | --- |
| Nezha | `C:\Users\hzt\yihui-workspace\nezha-rtos-bench\Debug\strip\nezha-rtos-bench` | 6441912 | `7806D7A9D3CE5CE0819BE1A0F943856998A45D90AEE02CB038E7921C4FC314A5` |
| Feiteng / OrangePi | `C:\Users\hzt\yihui-workspace\feiteng_rtos_bench\Debug\strip\feiteng_rtos_bench` | 6438352 | `3FD1B63CE025E76BD5B005B6C0BDE998621914525DC99E40C7BE9227CA0C53FD` |
| Gongkong | `C:\Users\hzt\yihui-workspace\gongkong_rtos_bench\Debug\strip\gongkong_rtos_bench` | 6872144 | `38AF582720606F66A3AD24CA8B5A022974C2E657905B933101967399C0653A5B` |
| Loongson | `C:\Users\hzt\yihui-workspace\rtos-bench\Debug\strip\rtos-bench` | 7803880 | `88AFE665A69D784F714E35F3FF4D856FC94089A1D00AC667EBFDD8903B5A0FF7` |

Binary string checks before deployment:

- `skipped for quick schedule`: not present in all four rebuilt artifacts.
- `Phase 1] %d workloads measured`: present in all four rebuilt artifacts.

## Deployment status

| Board | IP | Remote path | Status | Log |
| --- | --- | --- | --- | --- |
| Nezha | `192.168.31.202` | `/apps/hzt/nezha-rtos-bench` | FTP uploaded latest binary; telnet refused with `server is full of links` | `deploy_nezha_20260427_141138.log` |
| Gongkong | `192.168.31.203` | `/apps/hzt/gongkong-rtos-bench` | Deployed and chmod verified | `deploy_gongkong_20260427_141142.log` |
| OrangePi | `192.168.31.201` | `/apps/hzt/orangepi-rtos-bench` | Deployed and chmod verified | `deploy_orangepi_20260427_141147.log` |
| Loongson | `192.168.31.200` | `/apps/hzt/loongson-rtos-bench` | Deployed and chmod verified | `deploy_loongson_20260427_141153.log` |
| Feiteng | `192.168.31.204` | `/apps/hzt/feiteng-rtos-bench` | Blocked: board unreachable from jump host | `deploy_feiteng_offline_20260427_141323.log` |

Note: `192.168.31.203` is the SylixOS Gongkong board. `192.168.31.205` is not used for this SylixOS Gongkong deployment.

## Test status with latest deployed binaries

Command used for reachable boards: `test-schedule --cycles 1`.

| Board | Result | Key evidence | Log |
| --- | --- | --- | --- |
| Gongkong | PASS | 9 workloads measured; U=30..100 all `0 misses / 9 jobs`; `Final Score: 100.00 / 100` | `test_gongkong_20260427_141221.log` |
| OrangePi | PASS | 9 workloads measured; U=30..100 all `0 misses / 9 jobs`; `Final Score: 100.00 / 100` | `test_orangepi_20260427_141221.log` |
| Loongson | PASS | 9 workloads measured; U=30..100 all `0 misses / 9 jobs`; `Final Score: 100.00 / 100` | `test_loongson_20260427_141221.log` |
| Nezha | BLOCKED | Latest binary is uploaded, but telnet still refuses connections with `server is full of links` | `runner_nezha_blocked_20260427_141323.log` |
| Feiteng | BLOCKED | Board `192.168.31.204` is unreachable | `deploy_feiteng_offline_20260427_141323.log` |

No passing test log contains these known bad markers:

- `timer_create failed`
- `skipped for quick schedule`
- `Can not find dependent library`
- `[FAIL]`

## Operational notes

- The deployment helper is `utils/remote-test/deploy_via_jumphost.py`.
- Set `RTBENCH_JUMPHOST_PASSWORD` before running it. `RTBENCH_JUMPHOST_HOST`, `RTBENCH_JUMPHOST_PORT`, and `RTBENCH_JUMPHOST_USER` can override the defaults.
- It now points to the rebuilt artifact names produced by `make clean && make all`, avoiding stale strip binaries.
- It accepts `python deploy_via_jumphost.py test <board> <cycles> <log_dir>` for short verification runs with retained logs.
- On `test-schedule --cycles 1`, Phase 2 can still take minutes because each task waits for the periodic release before executing. This is expected for low-utilization task sets with long-period workloads.
