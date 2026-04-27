# Quick Test Guide - SylixOS test-schedule Fix Verification

## Quick Start

```bash
# 1. Switch to fix branch
cd rtos-bench/RTOS-Bench
git checkout fix/sylixos-test-schedule0427

# 2. Build for all boards (run from workspace root)
cd ../../

# Loongson (loongarch)
cd loongson_base/rtos-bench
make clean && make
cd ../../

# OrangePi (aarch64)
cd feiteng_base_aarch64/rtos-bench  # or orangepi build dir
make clean && make
cd ../../

# Nezha (riscv64)
cd nezha_base/rtos-bench
make clean && make
cd ../../

# Gongkong (x86_64)
cd gongkong_base_x86/rtos-bench
make clean && make
cd ../../

# 3. Run automated tests
cd rtos-bench/RTOS-Bench/utils/remote-test
python telnet_test_remote.py -c boards/board_loongson.yaml -t test_schedule_quick
python telnet_test_remote.py -c boards/board_orangepi.yaml -t test_schedule_quick
python telnet_test_remote.py -c boards/board_nezha.yaml -t test_schedule_quick
python telnet_test_remote.py -c boards/board_gongkong.yaml -t test_schedule_quick
```

## What to Look For

### ✅ Success Indicators

1. **No timer errors**:
   ```
   # Should NOT see:
   [test-schedule] Task X: timer_create failed
   ```

2. **All 9 workloads measured**:
   ```
   [Phase 1] Measuring WCET for 9 workloads...
     [cusum]: WCET = X.XX ms (5 iters)
     [fast]: WCET = X.XX ms (5 iters)
     [epnp]: WCET = X.XX ms (5 iters)
     [ekf]: WCET = X.XX ms (5 iters)
     [icp]: WCET = X.XX ms (5 iters)
     [modbus]: WCET = X.XX ms (5 iters)
     [mqtt]: WCET = X.XX ms (5 iters)
     [pid]: WCET = X.XX ms (5 iters)
     [ewma]: WCET = X.XX ms (5 iters)
   [Phase 1] 9 workloads measured
   ```

3. **Real job execution** (job count > 0):
   ```
   U= 30%: MR=0.0000 (0 misses / 27 jobs)  # 27 > 0 ✓
   U= 40%: MR=0.0000 (0 misses / 27 jobs)
   U= 50%: MR=0.0000 (0 misses / 27 jobs)
   ```

4. **Test completes successfully**:
   ```
   Final Score: 100.00 / 100
   ```

### ❌ Failure Indicators

1. **Timer errors** (should be fixed):
   ```
   [test-schedule] Task X: timer_create failed
   ```

2. **Workload filtering** (should be removed):
   ```
   -> skipped for quick schedule (WCET > 2s)
   [Phase 1] 4 workloads measured  # Should be 9!
   ```

3. **Zero jobs executed** (false positive):
   ```
   U= 30%: MR=0.0000 (0 misses / 0 jobs)  # 0 jobs = BAD!
   ```

## Manual Testing (If Automated Fails)

```bash
# 1. Connect to board via telnet
telnet <board_ip> 23

# 2. Navigate to test directory
cd /apps/rtbench

# 3. Run test manually
./rtbench test-schedule --cycles 3

# 4. Check output for success indicators above
```

## Board IP Addresses

| Board | IP Address | Status |
|-------|-----------|--------|
| 龙芯 LS2K1000 | 192.168.31.201 | Online |
| 香橙派 RK3588 | 192.168.31.202 | Online |
| 哪吒 D1 | 192.168.31.203 | Online |
| 飞腾派 E2000Q | 192.168.31.204 | Offline (needs debug) |
| 工控机 MH7700 | 192.168.31.205 | Online |

## Troubleshooting

### Build Errors

```bash
# If pthread symbols not found, check SylixOS toolchain
# Ensure pthread library is linked in Makefile

# Clean build
make clean
rm -rf build/
make
```

### Test Timeout

```bash
# If test takes too long (>1200s), check:
# 1. Board CPU performance
# 2. Workload WCET values (should be reasonable)
# 3. Network latency (for remote tests)
```

### Feiteng Board Offline

```bash
# From jumphost (rtbench@10.134.151.45)
ping 192.168.31.204
nmap -sn 192.168.31.0/24

# Physical inspection required:
# - Check power LED
# - Check network cable
# - Check switch port
# - Try serial console access
```

## Expected Test Duration

- **Quick mode** (`--cycles 3`): ~5-10 minutes per board
- **Full mode** (`--cycles 100`): ~30-60 minutes per board

## Comparison: Before vs After

### Before Fix
```
[Phase 1] 4 workloads measured  # Only 4-6 workloads
U= 30%: MR=0.0000 (0 misses / 0 jobs)  # 0 jobs = false positive
Final Score: 100.00 / 100  # Meaningless score
```

### After Fix
```
[Phase 1] 9 workloads measured  # All 9 workloads
U= 30%: MR=0.0000 (0 misses / 27 jobs)  # Real execution
Final Score: 100.00 / 100  # Valid score
```

## Next Steps After Verification

1. Confirm all boards pass with 9 workloads and real job execution
2. Document any performance differences between boards
3. Push branch to remote: `git push origin fix/sylixos-test-schedule0427`
4. Create pull request for review
5. Merge to main after approval
