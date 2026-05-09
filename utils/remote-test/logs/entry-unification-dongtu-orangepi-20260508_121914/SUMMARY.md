# Entry Unification Validation: dongtu-orangepi

- OS: `Dongtu`
- Board IP: `192.168.31.207:23`
- Remote executable/command: `rtbench`
- Writable prefix: `/nfsd/entry_unify_dongtu-orangepi_20260508_121914`
- Jump host: `10.134.151.45:1026`
- Mode: `basic`

- PASS help: `rtbench --help` (help text found)
- PASS export: `rtbench export-result -o /nfsd/entry_unify_dongtu-orangepi_20260508_121914_probe.json` (no export write error)
- PASS bad-command: `rtbench bad-command` (bad command rejected)
