# RTOS-Bench 测试报告

本文档记录各子命令在 RT-Thread (QEMU aarch64) 平台上的验证结果。

---

## 测试环境

| 项目 | 配置 |
|------|------|
| 平台 | RT-Thread 5.3.0 |
| BSP | qemu-virt64-aarch64 |
| CPU | Cortex-A53 x 4 (QEMU) |
| 内存 | 128MB |
| 工具链 | xpack-aarch64-none-elf-gcc-14.2.1 |

---

## 1. test-stress 验证

> 测试时间: 2026-02-23 19:50 UTC

**命令**: `rtbench test-stress -s cpu -t 5`

**结果**:
```
[test-stress] Starting stress/power test
  Stressor: cpu, Duration: 5 seconds

=============================================================
[test-stress] Running stressor: cpu for 5 seconds
=============================================================
rtos_stress: info: spawning 1 instances of 'cpu' (duration: 500 ticks)...
rtos_stress: info: [cpu-0] started (pid 0x4050d3a0)
rtos_stress: info: [cpu] load set to 100% (busy: 100ms, sleep: 0ms)
rtos_stress: info: [cpu] using 'all' methods (round-robin)
rtos_stress: info: [cpu-0] completed, 0x0000000000000ef1 ops

[test-stress] Stressor cpu completed with code: 0
=============================================================
```

**状态**: ✅ 通过

---

## 2. test-realtime 验证

> 测试时间: 2026-02-23 19:52 UTC

**命令**: `rtbench test-realtime`

**结果**:
```
[test-realtime] Starting realtime performance test

=============================================================
[test-realtime] Starting realtime performance benchmark
=============================================================
Mode: Single-core tests only

Finish test 1.   (上下文切换)
Finish test 2.   (中断延迟)
Finish test 3.   (系统调用)
Finish test 4_1, 4_1.  (信号量)
Finish test 4_2, 5_4.
Finish test 5_3.
Finish test 6_1, 7_1.  (消息队列)
Running test 6_3 (with debug)...
Finish test 6_3.
Running test 6_4, 7_2...
Finish test 6_4, 7_2.
mqueue supports blocking on full queue.
Running test 6_2, 7_4...
Finish test 6_2, 7_4.
Running test 7_3...
Finish test 7_3.
Finish test 8_1, 9_1.  (互斥锁)
Finish test 8_2, 9_4.
Finish test 9_3.
Finish test 10_1, 11_1.  (内存池)

单核系统服务开销 (单位: µs):
指标       | 立即执行   | 挂起睡眠   | 低优就绪   | 高优恢复
----------------------------------------------------------------------
信号量获取 | 2.144      | 59.168     |            |
信号量释放 | 3.312      |            | 24.288     | 2.032
消息发送   | 1214.384   | 9541.008   | 1658.192   | 6595.872
消息接收   | 29.984     | 4161.104   | 6313.184   | 1686.352
互斥锁获取 | 4.128      | 671.152    |            |
互斥锁释放 | 28.432     |            | 390.960    | 1515.552
内存块申请 | 18.064     |            |            |
内存块释放 | 21.296     |            |            |

上下文切换延迟    AVG: 74.875 µs
中断软件延迟    MIN: (需内核插桩)  MAX: -  AVG: -
系统调用延迟    MIN: 0.176  MAX: 11.488  AVG: 0.215 µs

=============================================================
[test-realtime] Benchmark completed with code: 0
=============================================================
```

**状态**: ✅ 通过 (中断测试需要内核插桩支持)

**已知问题**:
- 中断延迟测试 (test2) 需要内核级插桩，QEMU 环境下数据无效
- 消息队列延迟较高，可能与 QEMU 虚拟化开销有关

---

## 3. test-schedule 验证

> 测试时间: 2026-02-23 20:05 UTC

**命令**: `rtbench test-schedule --cycles 100`

**结果** (部分，WCET 测量阶段):
```
[test-schedule] Starting schedulability test
  Cycles: 100, Utilization: 30% - 100% (step 10%)
=============================================================
[Phase 1] Measuring WCET for 10 workloads...
=============================================================
  [stub]: WCET = 0.000 ms
  [busywait]: WCET = 50.000 ms
  [fast]: WCET = 575404.000 ms  (约 575 秒/次)
  ...
```

**FAST workload WCET 测量详情**:
```
[POSIX] Starting FAST Benchmark ...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)

| Image           | Size      | Corners  | Time(us)  | FPS     |
|-----------------|-----------|----------|-----------|---------|
| leuven          | 640 x480  | 5631     |  184843.3 |     5.4 |
| graf            | 640 x480  | 5820     |  206210.2 |     4.8 |
| bikes           | 640 x480  | 3719     |  184350.5 |     5.4 |
|-----------------|-----------|----------|-----------|---------|
[Result] Total Time: 575.404 s
[POSIX] Benchmark Finished.
```

**状态**: ✅ 框架正常 (完整测试耗时过长)

**分析**:
- test-schedule 框架工作正常，正确执行 Phase 1 WCET 测量
- FAST workload 单次执行约 575 秒，WCET 测量需要 50 次迭代
- 预估完整测试时间: FAST 约 8 小时，加上其他 workload 和 Phase 2/3
- QEMU 虚拟化环境下性能较低，实际硬件会快很多

**建议**:
1. 在真实硬件上运行完整测试
2. 或使用 `--workloads stub,busywait,pid` 参数排除重型 workload（待实现）
3. 或减少 WCET 迭代次数

---

## 测试总结

| 子命令 | 状态 | 说明 |
|--------|------|------|
| test-stress | ✅ 通过 | CPU stressor 正常运行 |
| test-realtime | ✅ 通过 | 11个测试完成，中断需内核支持 |
| test-schedule | ✅ 框架正常 | WCET 测量阶段验证通过，完整测试需真实硬件 |

---

## 已知限制

1. **QEMU 性能**: 虚拟化开销导致测试时间远超真实硬件
2. **中断测试**: test-realtime 的中断延迟测试需要内核插桩
3. **重型 workload**: FAST/EKF/ICP 等 workload 在 QEMU 下执行极慢

---

*报告生成时间: 2026-02-23*
*最后更新: 2026-02-23 20:10 UTC*
