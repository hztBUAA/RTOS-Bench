# Entry Unification Validation: gongkong-sylixos

- OS: `SylixOS`
- Board IP: `10.134.151.45:3017`
- Remote executable/command: `/apps/hzt/gongkong-rtos-bench`
- Writable prefix: `/apps/hzt/entry_unify_gongkong-sylixos_20260509_141352`
- Jump host: `10.134.151.45:1026`
- Mode: `smoke`

- PASS help: `/apps/hzt/gongkong-rtos-bench --help` (help text found)
- PASS test-all-smoke: `/apps/hzt/gongkong-rtos-bench test-all --quick --schedule-cycles 1 --util-start 30 --util-end 30 --util-step 30 --stress-job all-quick -o /apps/hzt/entry_unify_gongkong-sylixos_20260509_141352_quick.json` (result json saved)
- PASS bad-command: `/apps/hzt/gongkong-rtos-bench bad-command` (bad command rejected)
