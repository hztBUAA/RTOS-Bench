# Entry Unification Validation: loongson

- OS: `SylixOS`
- Board IP: `192.168.31.200:23`
- Remote executable/command: `/apps/hzt/loongson-rtos-bench`
- Writable prefix: `/apps/hzt/entry_unify_loongson_20260508_022228`
- Jump host: `10.134.151.45:1026`
- Mode: `basic`

- FAIL help: `/apps/hzt/loongson-rtos-bench --help` (prompt not reached before timeout)
- FAIL export: `/apps/hzt/loongson-rtos-bench export-result -o /apps/hzt/entry_unify_loongson_20260508_022228_probe.json` (prompt not reached before timeout)
- FAIL schedule: `/apps/hzt/loongson-rtos-bench test-schedule --quick --cycles 1 --util-start 30 --util-end 30 --util-step 30` (prompt not reached before timeout)
- FAIL bad-command: `/apps/hzt/loongson-rtos-bench bad-command` (prompt not reached before timeout)
