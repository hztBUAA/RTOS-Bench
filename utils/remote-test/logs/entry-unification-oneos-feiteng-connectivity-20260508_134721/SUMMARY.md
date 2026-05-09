# Entry Unification Validation: oneos-feiteng Connectivity

- Board IP: `192.168.31.205`
- Expected FTP/Telnet: `21` / `23` through jump host
- Historical forwarded ports checked from Windows: `10.134.151.45:3021`, `10.134.151.45:3023`
- New local module built: `C:\OneOSStudio\workspace\phytium_pi_out\out\phytium_pi_out.out`
- Remote module target: `/user/phytium_pi_out.out`

## Result

- PASS build: OneOS CMake build completed and included `rtbench_command.c`.
- FAIL deploy/connectivity: jump host cannot reach OneOS board.
- `ping -c 1 -W 2 192.168.31.205`: no reply.
- TCP `192.168.31.205:21`: failed.
- TCP `192.168.31.205:23`: failed.
- TCP `127.0.0.1:3021` on jump host: failed.
- TCP `127.0.0.1:3023` on jump host: failed.
- TCP `10.134.151.45:3021` from jump host: failed.
- TCP `10.134.151.45:3023` from jump host: failed.

OneOS end-to-end validation is blocked until the Feiteng OneOS board or port
forwarding is restored. The historical validation flow remains:

```sh
ld /user/phytium_pi_out.out
rtbench --help
rtbench test-schedule --cycles 1 --util-start 30 --util-end 30 --util-step 30
rtbench bad-command
```
