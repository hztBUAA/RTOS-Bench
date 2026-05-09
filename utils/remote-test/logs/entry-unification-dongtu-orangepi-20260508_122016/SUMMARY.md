# Entry Unification Validation: dongtu-orangepi

- OS: `Dongtu`
- Board IP: `192.168.31.207:23`
- Remote executable/command: `rtbench`
- Writable prefix: `/nfsd/entry_unify_dongtu-orangepi_20260508_122016`
- Jump host: `10.134.151.45:1026`
- Mode: `smoke`

- PASS help: `rtbench --help` (help text found)
- PASS export: `rtbench export-result -o /nfsd/entry_unify_dongtu-orangepi_20260508_122016_probe.json` (no export write error)
- FAIL test-all-smoke: `rtbench test-all --quick --schedule-cycles 1 --util-start 30 --util-end 30 --util-step 30 --stress-job all-quick -o /nfsd/entry_unify_dongtu-orangepi_20260508_122016_quick.json` (test-all result save missing or failed)
- FAIL bad-command: `rtbench bad-command` (bad command was not rejected)
