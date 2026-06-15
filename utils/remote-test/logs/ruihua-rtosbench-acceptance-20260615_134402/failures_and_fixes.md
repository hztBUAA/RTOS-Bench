# Failures and Fixes

## Failure 1

- Scope: host automation / P1 log collection
- Command: `rtbench_test_realtime`
- Return code: host wrapper timed out before cleanup
- Log: `test_realtime_feiteng_20260615_1415.partial.log`
- Symptom: the host SSH process did not exit cleanly, but the board log showed `[test-realtime] Benchmark completed with code: 0` and `0x00000000 (0)`.
- Root cause: transient SSH/session cleanup issue in the host-to-rtbench automation path, not a board-side realtime test failure.
- Files changed: none.
- Patch summary: fetched the remote telnet log separately, normalized it to `test_realtime_feiteng_20260615_1415.log`, and recorded rc=0 based on the board shell return value.
- Validation command: log inspection of `test_realtime_feiteng_20260615_1415.log`.
- Validation result: PASS.

## Failure 2

- Scope: result collection
- Command: `cat /rtbench_result.json`
- Return code: 0
- Log: `probe_filecmds/cat__rtbench_result.json.log`
- Symptom: raw telnet output inserted line wraps/control bytes, so direct JSON parsing failed.
- Root cause: telnet shell output formatting, not invalid board result content.
- Files changed: none.
- Patch summary: stripped telnet control characters and hard line wraps from the captured JSON text before parsing.
- Validation command: parse `feiteng_export_20260615_1425.json` with Python `json.loads`, then run `utils/flatten_rtbench_result.py`.
- Validation result: PASS; flattened report contains 188 records.
