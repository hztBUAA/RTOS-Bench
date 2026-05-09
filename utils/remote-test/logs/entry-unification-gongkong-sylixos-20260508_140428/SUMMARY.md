# Entry Unification Validation: gongkong-sylixos

- OS: `SylixOS`
- Board IP: `10.134.151.45:3017`
- Remote executable/command: `/apps/hzt/gongkong-rtos-bench`
- Writable prefix: `/apps/hzt/entry_unify_gongkong-sylixos_20260508_140428`
- Jump host: `10.134.151.45:1026`
- Mode: `basic`

- PASS help: `/apps/hzt/gongkong-rtos-bench --help` (help text found)
- PASS export: `/apps/hzt/gongkong-rtos-bench export-result -o /apps/hzt/entry_unify_gongkong-sylixos_20260508_140428_probe.json` (no export write error)
- PASS bad-command: `/apps/hzt/gongkong-rtos-bench bad-command` (bad command rejected)
