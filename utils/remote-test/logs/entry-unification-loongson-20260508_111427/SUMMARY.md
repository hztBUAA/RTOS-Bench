# Entry Unification Validation: loongson

- OS: `SylixOS`
- Board IP: `192.168.31.200:23`
- Remote executable/command: `/apps/hzt/loongson-rtos-bench`
- Writable prefix: `/apps/hzt/entry_unify_loongson_20260508_111427`
- Jump host: `10.134.151.45:1026`
- Mode: `basic`

- PASS help: `/apps/hzt/loongson-rtos-bench --help` (help text found)
- PASS export: `/apps/hzt/loongson-rtos-bench export-result -o /apps/hzt/entry_unify_loongson_20260508_111427_probe.json` (no export write error)
- PASS schedule: `/apps/hzt/loongson-rtos-bench test-schedule --quick --cycles 1 --util-start 30 --util-end 30 --util-step 30` (schedule output observed)
- FAIL bad-command: `/apps/hzt/loongson-rtos-bench bad-command` (bad command was not rejected)
