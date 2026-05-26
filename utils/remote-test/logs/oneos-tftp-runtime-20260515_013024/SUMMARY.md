# OneOS TFTP Runtime Validation

- Batch: `20260515_013024`
- Board: `192.168.31.205:23`
- TFTP source: `192.168.31.110:/tftp/phytium_pi_out.out`
- Remote module: `/user/phytium_pi_out.out`
- Local module: `C:\OneOSStudio\workspace\phytium_pi_out\out\phytium_pi_out.out`
- Log: `C:\Users\hzt\yihui-workspace\rtos-bench\RTOS-Bench\utils\remote-test\logs\oneos-tftp-runtime-20260515_013024\terminal.log`

- PASS pwd: `pwd`
- PASS list-before: `list_lmodule`
- PASS unload: `unld /user/phytium_pi_out.out`
- PASS tftp-get: `tftp_client 192.168.31.110 get phytium_pi_out.out /user/phytium_pi_out.out`
- PASS ls-user: `ls /user`
- PASS load: `ld /user/phytium_pi_out.out`
- PASS list-after: `list_lmodule`
- PASS help: `rtbench --help`
- PASS list-workloads: `rtbench -L`
- PASS workload-stub: `rtbench -b stub -t 1`
- PASS workload-busywait: `rtbench -b busywait -t 1`
- PASS schedule-quick: `rtbench test-schedule --cycles 1 --util-start 30 --util-end 30 --util-step 30`
