# Entry Unification Validation: loongson

- OS: `SylixOS`
- Board IP: `192.168.31.200:23`
- Remote executable/command: `/apps/hzt/loongson-rtos-bench`
- Writable prefix: `/apps/hzt/entry_unify_loongson_20260508_015829`
- Jump host: `10.134.151.45:1026`

- PASS: `/apps/hzt/loongson-rtos-bench --help`
- PASS: `/apps/hzt/loongson-rtos-bench export-result -o /apps/hzt/entry_unify_loongson_20260508_015829_probe.json`
- FAIL: `/apps/hzt/loongson-rtos-bench test-schedule --cycles 1 --util-start 30 --util-end 30 --util-step 30`
- FAIL: `/apps/hzt/loongson-rtos-bench test-all --quick --no-realtime --no-stress --no-workload --schedule-cycles 1 --util-start 30 --util-end 30 --util-step 30 -o /apps/hzt/entry_unify_loongson_20260508_015829_quick.json`
- FAIL: `/apps/hzt/loongson-rtos-bench bad-command`
