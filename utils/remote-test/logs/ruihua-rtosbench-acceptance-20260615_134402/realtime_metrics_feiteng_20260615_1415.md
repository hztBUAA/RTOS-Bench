# Realtime Metrics - Ruihua Feiteng

- Command: `rtbench_test_realtime`
- Status: PASS
- Return code: 0
- Mode: Single-core tests only
- Evidence: log contains `[test-realtime] Benchmark completed with code: 0` and `0x00000000 (0)`.
- Note: board output reported `mq_open ... Invalid argument` during one subcase, but wrapper completed with benchmark code 0.
