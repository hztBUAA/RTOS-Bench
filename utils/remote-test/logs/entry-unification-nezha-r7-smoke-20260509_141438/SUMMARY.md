# Nezha r7 Smoke Validation

- Remote executable: `/apps/hzt/nezha-rtos-bench-r7`
- Batch: `20260509_141438`
- Log: `C:\Users\hzt\yihui-workspace\rtos-bench\RTOS-Bench\utils\remote-test\logs\entry-unification-nezha-r7-smoke-20260509_141438\terminal.log`

- NOTE help/export: SylixOS telnet delayed the first command output until the next prompt, so the automation marked these two probe commands as incomplete even though the help text was printed. The end-to-end smoke acceptance is based on the completed `test-all --quick` JSON below.
- PASS test-all-quick: `/apps/hzt/nezha-rtos-bench-r7 test-all --quick --schedule-cycles 1 --util-start 30 --util-end 30 --util-step 30 --stress-job all-quick -o /apps/hzt/nezha_r7_smoke_20260509_141438_quick.json` (ok)
- PASS bad-command: `/apps/hzt/nezha-rtos-bench-r7 bad-command` (ok)
- PASS ftp-result-check: `/apps/hzt/nezha_r7_smoke_20260509_141438_quick.json` exists and is 8864 bytes.
