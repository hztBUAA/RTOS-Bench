# Entry Unification Validation: nezha-sylixos

- OS: `SylixOS`
- Board IP: `10.134.151.45:3014`
- Remote executable/command: `/apps/hzt/nezha-rtos-bench`
- Writable prefix: `/apps/hzt/entry_unify_nezha-sylixos_20260508_140419`
- Jump host: `10.134.151.45:1026`
- Mode: `basic`

- PASS help: `/apps/hzt/nezha-rtos-bench --help` (help text found)
- PASS export: `/apps/hzt/nezha-rtos-bench export-result -o /apps/hzt/entry_unify_nezha-sylixos_20260508_140419_probe.json` (no export write error)
- PASS bad-command: `/apps/hzt/nezha-rtos-bench bad-command` (bad command rejected)
