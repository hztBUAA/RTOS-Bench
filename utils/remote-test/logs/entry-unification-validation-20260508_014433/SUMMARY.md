# Entry Unification Loongson Validation

- Batch: `20260508_014433`
- Board: `loongson` / `192.168.31.200`
- Remote executable: `/apps/hzt/loongson-rtos-bench`
- Log: `loongson_matrix.log`

- PASS: `/apps/hzt/loongson-rtos-bench --help`
- PASS: `/apps/hzt/loongson-rtos-bench export-result -o /apps/hzt/entry_unify_probe_20260508_014433.json`
- PASS: `/apps/hzt/loongson-rtos-bench test-all --quick --schedule-cycles 1 --util-start 30 --util-end 30 --util-step 30 --stress-job all-quick -o /apps/hzt/entry_unify_quick_20260508_014433.json`
- FAIL: `/apps/hzt/loongson-rtos-bench test-schedule --cycles 1 --util-start 30 --util-end 30 --util-step 30`
- FAIL: `/apps/hzt/loongson-rtos-bench bad-command`
