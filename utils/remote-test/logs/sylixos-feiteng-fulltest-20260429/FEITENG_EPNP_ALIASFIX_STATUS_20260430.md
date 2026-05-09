# Feiteng SylixOS epnpaliasfix validation status

Date: 2026-04-30

## Binary

- Local binary: `C:\Users\hzt\yihui-workspace\feiteng_rtos_bench\Debug\strip\hzt_feiteng_rtos_bench_epnpaliasfix_20260430_0316`
- Remote binary: `/apps/hzt/feiteng-rtos-bench_hzt_epnpaliasfix_20260430_0316`
- SHA256: `041D7B567FD82885319813720DD536B269648B39EEDCC85CD77B25CA0314877C`
- Size: `6406536`

## Code evidence

- Feiteng crashed in standalone `-b epnp -t 1` at:
  - `workloads/EPNP/methods.cpp:275`
  - expression: `translation = -rotation * translation;`
  - symbolized stack: `opengv::absolute_pose::epnp(...) -> epnp_bench_run -> workload_exec -> periodic_benchmark -> main`
- Fix applied locally:
  - replaced the self-referential Eigen matrix-vector expression with an equivalent explicit 3x3 temporary multiply.
  - no workload is skipped or reduced.
- Build template alignment:
  - `platforms/sylixos/rtos-bench.mk` now includes the Eigen/OpenGV compatibility macros already used by the Feiteng project makefile.

## Logs

- Clean build log: `build_feiteng_clean_eigen_retry_j1_20260430_031211.log`
- Alias-fix incremental build log: `build_feiteng_epnp_aliasfix_20260430_031539.log`
- Deploy log: `deploy_feiteng_epnpaliasfix_20260430_0316.log`
- `-L` pass log: `list_workloads_epnpaliasfix_20260430_031823.log`
- Original ePnP crash evidence: `single_workload_epnp_wcetthread_clean_20260430_030917.log`
- `addr2line` evidence was generated from local unstripped binary `C:\Users\hzt\yihui-workspace\feiteng_rtos_bench\Debug\feiteng_rtos_bench`.

## Current blocker

After attempting to copy the 6MB binary to a short alias on the board, the board-side `cp` operation did not return within 120 seconds. Since then:

- `ping 192.168.31.204` succeeds.
- Telnet reaches the server but remains stuck after password, then reports `server is full of links`.
- FTP directory listing times out.

The board needs a power/reboot reset before validation can continue.

## 2026-04-30 afternoon update

Vendor-facing short report created:

- `FEITENG_VENDOR_REPORT_WECHAT_20260430.md`

Code-side diagnostic candidate built:

- Local binary: `C:\Users\hzt\yihui-workspace\feiteng_rtos_bench\Debug\strip\hzt_feiteng_rtos_bench_testallinline_20260430_140712`
- SHA256: `F82A151803192AF3D23A516D48FB11CDF5E3DE5617745F478ACC1832A0C1D4A0`
- Size: `6406312`
- Build result: `make -j4 all` succeeded.

Candidate change:

- SylixOS `test-all` now runs the suite directly on the entry path instead of creating an additional long-lived 4MB pthread wrapper.
- This keeps `test-all` closer to standalone `test-realtime` / `test-schedule` execution and removes one extra `pthread_create -> API_VmmStackAlloc` pressure point.
- No realtime, schedule, stress, export, or workload business logic is skipped or reduced.

Deployment status:

- Upload target planned: `/apps/hzt/feiteng-rtos-bench_hzt_testallinline_20260430_140712`
- FTP upload attempt timed out with no data returned.
- Native FTP attempt reached `220 SylixOS FTP server ready` and `331 Password required`, then hung after password.
- Telnet still reports `server is full of links`.
- Ping remains OK, so the board is powered/network-reachable, but shell/FTP services still need a reset.

## Next validation after reboot

1. Verify `/apps/hzt/feiteng-rtos-bench_hzt_epnpaliasfix_20260430_0316 -L`.
2. Run `/apps/hzt/feiteng-rtos-bench_hzt_epnpaliasfix_20260430_0316 -b epnp -t 1`.
3. Run `/apps/hzt/feiteng-rtos-bench_hzt_epnpaliasfix_20260430_0316 test-schedule --cycles 1 --util-start 30 --util-end 30`.
4. If the above passes, run standard `test-schedule`.
5. Deploy and verify `/apps/hzt/feiteng-rtos-bench_hzt_testallinline_20260430_140712`.
6. Compare standalone `test-realtime` with:
   - `test-all --no-schedule --no-stress --no-cmd --no-workload`
   - `test-all --no-stress --no-workload`
   - full `test-all -o /apps/hzt/feiteng_testall_inline.json`
7. Record whether removing the wrapper thread changes the `API_VmmStackAlloc` failure behavior.
