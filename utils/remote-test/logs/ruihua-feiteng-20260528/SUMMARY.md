# Ruihua Feiteng ReWorks Validation

Date: 2026-05-28

## Target

- Board: Ruihua Feiteng / Phytium Pi
- RTOS: ReWorks 6.1.1 ARM
- Project: `C:\rtos\6.1.1-ARM\workspace\feiteng4rtos`
- TFTP image path: `C:\rtos\6.1.1-ARM\workspace\feiteng4rtos\gnuaarch64\FTE2000_SMP-64\reworks.elf`
- Telnet: `192.168.2.100:23`

## Build

The self-boot ReWorks image built successfully with RTOS-Bench integrated via
the project-side `gnuaarch64/user.mk` hook.

Latest verified image SHA256:

```text
7A66E3FB278846EBC58F7FDCA2F75900403C56D4F0236790DD0EAF921BF2BEB3
```

## Shell Commands Verified

```text
rtbench_help
rtbench_list
rtbench_test_schedule_cycles3
```

## Full Schedule Result

The full schedulability smoke for this target ran all default utilization
gradients from 30% to 100% with 3 cycles per gradient:

```text
[test-schedule] Found 1 industrial workloads (excluded 2 utility workloads)
[ruihua-smoke]: WCET = 50.000 ms (5 iters)
U= 30%: MR=0.0000 (0 misses / 3 jobs)
U= 40%: MR=0.0000 (0 misses / 3 jobs)
U= 50%: MR=0.0000 (0 misses / 3 jobs)
U= 60%: MR=0.0000 (0 misses / 3 jobs)
U= 70%: MR=0.0000 (0 misses / 3 jobs)
U= 80%: MR=0.0000 (0 misses / 3 jobs)
U= 90%: MR=0.0000 (0 misses / 3 jobs)
U=100%: MR=0.0000 (0 misses / 3 jobs)
Average Miss Rate: 0.0000
Final Score: 100.00 / 100
```

The ReWorks shell returned to `reworks>` after the run.

## Notes

- The generic POSIX `periodic_benchmark()` lifecycle was unsafe in the tested
  ReWorks telnet shell. Ruihua workload commands now use a finite direct
  periodic runner that preserves workload selection, period spacing, and
  activation count without relying on POSIX signal/atexit cleanup.
- `test-schedule` uses the platform timer abstraction and passed after replacing
  Ruihua's `SIGEV_THREAD` timer usage with a pthread-backed timer.
- Realtime, stress, and full industrial workload suites are not claimed as
  validated in this log.
