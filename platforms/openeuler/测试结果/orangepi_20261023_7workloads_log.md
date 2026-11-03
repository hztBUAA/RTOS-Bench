=============================================================
[RTOS-Bench] Comprehensive Test Suite
=============================================================
Output: result_20261023_7workloads.json
Modules: realtime=yes schedule=yes stress=yes cmd=yes workload=yes

>>> Running test-realtime...

=============================================================
[test-realtime] Starting realtime performance benchmark
=============================================================
Mode: Single-core + Multicore tests

the realtime benchmark is running, please wait...
Finish test 1.
Finish test 2.
Finish test 3.
Finish test 4_1, 4_1.
Finish test 4_2, 5_4.
Finish test 5_3.
Finish test 6_1, 7_1.
Finish test 6_3.
Finish test 6_4, 7_2.
mqueue supports blocking on full queue.
Finish test 6_2, 7_4.
Finish test 7_3.
Finish test 8_1, 9_1.
Finish test 8_2, 9_4.
Finish test 9_3.
Finish test 10_1, 11_1.
end ipc_bw task inter core 1
end ipc_bw task inter core 2
end ipc_bw task inter core 4
end ipc_bw task inter core 8
end ipc_bw task on same core
end ipc_bw task on different core
end mem_bw task rd 1
end mem_bw task rd 2
end mem_bw task rd 4
end mem_bw task rd 8
end mem_bw task wr 1
end mem_bw task wr 2
end mem_bw task wr 4
end mem_bw task wr 8
end mem_bw task cp 1
end mem_bw task cp 2
end mem_bw task cp 4
end mem_bw task cp 8
end mem_bw task frd 1
end mem_bw task frd 2
end mem_bw task frd 4
end mem_bw task frd 8
end mem_bw task fwr 1
end mem_bw task fwr 2
end mem_bw task fwr 4
end mem_bw task fwr 8
end mem_bw task fcp 1
end mem_bw task fcp 2
end mem_bw task fcp 4
end mem_bw task fcp 8
end mem_bw task memset 1
end mem_bw task memset 2
end mem_bw task memset 4
end mem_bw task memset 8
end mem_bw task memcpy 1
end mem_bw task memcpy 2
end mem_bw task memcpy 4
end mem_bw task memcpy 8
end task_lt task 1.
end task_lt task 2.
end task_lt task 4.
end task_lt task 8.
系统延迟（单位：us）:
指标       | 立即执行   | 挂起睡眠   | 低优就绪   | 高优恢复
----------------------------------------------------------------------
信号量获取 | 0.000      | 5.000      |            |
信号量释放 | 0.041      |            | 1.708      | 5.375
消息发送   | 0.708      | 5.916      | 2.166      | 5.625
消息接收   | 0.791      | 4.625      | 2.375      | 4.875
互斥锁获取 | 0.041      | 4.333      |            |
互斥锁释放 | 0.000      |            | 1.750      | 4.791
内存块申请 | 0.041      |            |            |
内存块释放 | 0.041      |            |            |
上下文切换延迟    AVG: 1.351
中断软件延迟    MIN: 0.291  MAX: 0.583  AVG: 0.293
系统调用延迟    MIN: 5.833  MAX: 63.875  AVG: 7.031

多核存取性能:
并发度     | 1          | 2          | 4          | 8
----------------------------------------------------------------------
rd         | 4.441      | 10.478     | 10.276     | 10.603
wr         | 1.641      | 1.562      | 1.660      | 1.788
cp         | 0.397      | 0.564      | 0.777      | 0.769
frd        | 17.152     | 32.898     | 33.020     | 32.059
fwr        | 14.963     | 20.153     | 18.766     | 18.389
fcp        | 8.882      | 14.225     | 15.682     | 16.632
memset     | 18.178     | 31.204     | 29.853     | 30.182
memcpy     | 9.101      | 22.211     | 22.126     | 21.333
多核系统服务:
并发度     | 1          | 2          | 4          | 8
任务间通信 | 0.657      | 1.352      | 1.156      | 1.279
创建与删除 | 57.050     | 36.871     | 40.398     | 41.146    （单位为us）
任务在同核与异核上通信比较:
同核通信: 0.693
异核通信: 0.564
（除特殊说明外，上述单位均为GB/s）

=============================================================
[test-realtime] Benchmark completed with code: 0
=============================================================

>>> Running test-schedule (cycles=3 util=30-100 step 10)...
[test-schedule] build Oct 23 2026 23:06:22
[test-schedule] Found 7 industrial workloads (excluded 2 utility workloads)
=============================================================
[Phase 1] Measuring WCET for 7 workloads...
=============================================================
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=15.131 ms avg_latency=5043.695 us/img
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=9.540 ms avg_latency=3180.139 us/img
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=8.646 ms avg_latency=2881.861 us/img
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=7.957 ms avg_latency=2652.417 us/img
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=7.580 ms avg_latency=2526.514 us/img
  [fast]: WCET = 15.228 ms (5 iters)

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=107.414 ms avg_latency=107.414 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=62.556 ms avg_latency=62.556 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=62.331 ms avg_latency=62.331 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=62.283 ms avg_latency=62.283 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=62.283 ms avg_latency=62.283 ms/run
  [icp]: WCET = 107.865 ms (5 iters)
[SCHED-MODBUS] offline frames=6144 total_time=0.976 ms errors=0
[SCHED-MODBUS] offline frames=6144 total_time=1.011 ms errors=0
[SCHED-MODBUS] offline frames=6144 total_time=0.957 ms errors=0
[SCHED-MODBUS] offline frames=6144 total_time=0.957 ms errors=0
[SCHED-MODBUS] offline frames=6144 total_time=0.994 ms errors=0
  [modbus]: WCET = 1.021 ms (5 iters)
[SCHED-MQTT] offline packets=512 bytes=45115 checksum=3283016638 total_time=32.104 ms
[SCHED-MQTT] offline packets=512 bytes=45115 checksum=3283016638 total_time=22.550 ms
[SCHED-MQTT] offline packets=512 bytes=45115 checksum=3283016638 total_time=17.932 ms
[SCHED-MQTT] offline packets=512 bytes=45115 checksum=3283016638 total_time=14.903 ms
[SCHED-MQTT] offline packets=512 bytes=45115 checksum=3283016638 total_time=13.211 ms
  [mqtt]: WCET = 32.211 ms (5 iters)
--- PID Simulation Start ---
[PID] samples=1000000 total_time=42.601 ms avg_latency=0.043 us/op
--- PID Simulation Start ---
[PID] samples=1000000 total_time=20.865 ms avg_latency=0.021 us/op
--- PID Simulation Start ---
[PID] samples=1000000 total_time=17.055 ms avg_latency=0.017 us/op
--- PID Simulation Start ---
[PID] samples=1000000 total_time=14.662 ms avg_latency=0.015 us/op
--- PID Simulation Start ---
[PID] samples=1000000 total_time=12.765 ms avg_latency=0.013 us/op
  [pid]: WCET = 42.643 ms (5 iters)
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.531 ms avg_latency=0.031 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=10.230 ms avg_latency=0.017 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=7.465 ms avg_latency=0.012 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=6.229 ms avg_latency=0.010 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=6.226 ms avg_latency=0.010 us/sample
  [cusum]: WCET = 18.558 ms (5 iters)
[EWMA] loops=100 stream_len_per_loop=5000 total_time=33.021 ms avg_latency=0.066 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=18.979 ms avg_latency=0.038 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=16.743 ms avg_latency=0.033 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=16.039 ms avg_latency=0.032 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=16.041 ms avg_latency=0.032 us/sample
  [ewma]: WCET = 33.034 ms (5 iters)
[Phase 1] 7 workloads measured

=============================================================
[Phase 2] Running Task Sets (U: 30% - 100%, step 10%)
=============================================================

>>> Utilization Gradient: 30% <<<
[test-schedule] uunifast sum=0.3000 target=0.3000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 107.865    | 11.95    | 902.647
pid          | 42.643     | 5.25     | 811.501
ewma         | 33.034     | 4.27     | 773.025
mqtt         | 32.211     | 3.13     | 1028.106
cusum        | 18.558     | 2.01     | 923.724
fast         | 15.228     | 1.76     | 863.583
modbus       | 1.021      | 1.62     | 63.182
Running task set for 3 cycles...
[test-schedule] creating task thread 1/7 (icp)...
[test-schedule] creating task thread 2/7 (pid)...
[test-schedule] creating task thread 3/7 (ewma)...
[test-schedule] creating task thread 4/7 (mqtt)...
[test-schedule] creating task thread 5/7 (cusum)...
[test-schedule] creating task thread 6/7 (fast)...
[test-schedule] creating task thread 7/7 (modbus)...
[test-schedule] all 7 task threads created; entering watchdog
--- PID Simulation Start ---
[EWMA] loops=100 stream_len_per_loop=5000 total_time=83.072 ms avg_latency=0.166 us/sample
[FAST] samples=3 total_time=14.480 ms avg_latency=4826.695 us/img
[PID] samples=1000000 total_time=68.587 ms avg_latency=0.069 us/op

[ICP] Running ICP (point-to-plane)
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=26.197 ms avg_latency=0.044 us/sample
[ICP] samples=1 total_time=141.950 ms avg_latency=141.950 ms/run
[EWMA] loops=100 stream_len_per_loop=5000 total_time=73.997 ms avg_latency=0.148 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=77.037 ms avg_latency=0.077 us/op
[FAST] samples=3 total_time=31.957 ms avg_latency=10652.334 us/img

[ICP] Running ICP (point-to-plane)
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=21.167 ms avg_latency=0.035 us/sample
[ICP] samples=1 total_time=143.992 ms avg_latency=143.992 ms/run
[EWMA] loops=100 stream_len_per_loop=5000 total_time=92.701 ms avg_latency=0.185 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=87.633 ms avg_latency=0.088 us/op
[FAST] samples=3 total_time=40.292 ms avg_latency=13430.668 us/img

[ICP] Running ICP (point-to-plane)
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=27.953 ms avg_latency=0.047 us/sample
[ICP] samples=1 total_time=133.535 ms avg_latency=133.535 ms/run
Gradient 30% complete: MR = 0.0000 (0/21)

>>> Utilization Gradient: 40% <<<
[test-schedule] uunifast sum=0.4000 target=0.4000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 107.865    | 25.04    | 430.847
pid          | 42.643     | 4.99     | 855.289
ewma         | 33.034     | 4.96     | 666.333
mqtt         | 32.211     | 3.09     | 1041.851
cusum        | 18.558     | 0.97     | 1912.878
fast         | 15.228     | 0.81     | 1876.074
modbus       | 1.021      | 0.15     | 692.111
Running task set for 3 cycles...
[test-schedule] creating task thread 1/7 (icp)...
[test-schedule] creating task thread 2/7 (pid)...
[test-schedule] creating task thread 3/7 (ewma)...
[test-schedule] creating task thread 4/7 (mqtt)...
[test-schedule] creating task thread 5/7 (cusum)...
[test-schedule] creating task thread 6/7 (fast)...
[test-schedule] creating task thread 7/7 (modbus)...
[test-schedule] all 7 task threads created; entering watchdog

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=136.690 ms avg_latency=136.690 ms/run
[EWMA] loops=100 stream_len_per_loop=5000 total_time=68.253 ms avg_latency=0.137 us/sample
--- PID Simulation Start ---

[ICP] Running ICP (point-to-plane)
[PID] samples=1000000 total_time=57.154 ms avg_latency=0.057 us/op
[ICP] samples=1 total_time=71.745 ms avg_latency=71.745 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=98.147 ms avg_latency=98.147 ms/run
[EWMA] loops=100 stream_len_per_loop=5000 total_time=65.722 ms avg_latency=0.131 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=78.748 ms avg_latency=0.079 us/op
[FAST] samples=3 total_time=30.175 ms avg_latency=10058.445 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=38.050 ms avg_latency=0.063 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=68.841 ms avg_latency=0.138 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=77.649 ms avg_latency=0.078 us/op
[FAST] samples=3 total_time=54.416 ms avg_latency=18138.654 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=65.027 ms avg_latency=0.108 us/sample
[test-schedule] heartbeat (still running gradient):
    cusum    jobs=2/3  since_progress=1s  stall_window=5s
    fast     jobs=2/3  since_progress=1s  stall_window=5s
[FAST] samples=3 total_time=54.650 ms avg_latency=18216.530 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=68.825 ms avg_latency=0.115 us/sample
Gradient 40% complete: MR = 0.0000 (0/21)

>>> Utilization Gradient: 50% <<<
[test-schedule] uunifast sum=0.5000 target=0.5000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 107.865    | 20.01    | 539.001
pid          | 42.643     | 9.58     | 445.022
ewma         | 33.034     | 8.94     | 369.613
mqtt         | 32.211     | 7.50     | 429.388
cusum        | 18.558     | 2.81     | 660.778
fast         | 15.228     | 0.64     | 2382.356
modbus       | 1.021      | 0.52     | 196.757
Running task set for 3 cycles...
[test-schedule] creating task thread 1/7 (icp)...
[test-schedule] creating task thread 2/7 (pid)...
[test-schedule] creating task thread 3/7 (ewma)...
[test-schedule] creating task thread 4/7 (mqtt)...
[test-schedule] creating task thread 5/7 (cusum)...
[test-schedule] creating task thread 6/7 (fast)...
[test-schedule] creating task thread 7/7 (modbus)...
[test-schedule] all 7 task threads created; entering watchdog
--- PID Simulation Start ---
[EWMA] loops=100 stream_len_per_loop=5000 total_time=79.226 ms avg_latency=0.158 us/sample
[PID] samples=1000000 total_time=61.009 ms avg_latency=0.061 us/op

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=144.763 ms avg_latency=144.763 ms/run
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=22.443 ms avg_latency=0.037 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=60.940 ms avg_latency=0.122 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=54.082 ms avg_latency=0.054 us/op

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=70.333 ms avg_latency=70.333 ms/run
[EWMA] loops=100 stream_len_per_loop=5000 total_time=54.899 ms avg_latency=0.110 us/sample
--- PID Simulation Start ---
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=26.804 ms avg_latency=0.045 us/sample
[PID] samples=1000000 total_time=33.923 ms avg_latency=0.034 us/op

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=72.493 ms avg_latency=72.493 ms/run
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=69.643 ms avg_latency=0.116 us/sample
[FAST] samples=3 total_time=55.147 ms avg_latency=18382.391 us/img
[FAST] samples=3 total_time=55.280 ms avg_latency=18426.724 us/img
[test-schedule] heartbeat (still running gradient):
    fast     jobs=2/3  since_progress=0s  stall_window=7s
[FAST] samples=3 total_time=54.679 ms avg_latency=18226.252 us/img
Gradient 50% complete: MR = 0.0000 (0/21)

>>> Utilization Gradient: 60% <<<
[test-schedule] uunifast sum=0.6000 target=0.6000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 107.865    | 30.58    | 352.708
pid          | 42.643     | 13.57    | 314.225
ewma         | 33.034     | 8.95     | 369.248
mqtt         | 32.211     | 2.35     | 1370.334
cusum        | 18.558     | 2.13     | 871.716
fast         | 15.228     | 1.74     | 875.602
modbus       | 1.021      | 0.68     | 149.703
Running task set for 3 cycles...
[test-schedule] creating task thread 1/7 (icp)...
[test-schedule] creating task thread 2/7 (pid)...
[test-schedule] creating task thread 3/7 (ewma)...
[test-schedule] creating task thread 4/7 (mqtt)...
[test-schedule] creating task thread 5/7 (cusum)...
[test-schedule] creating task thread 6/7 (fast)...
[test-schedule] creating task thread 7/7 (modbus)...
[test-schedule] all 7 task threads created; entering watchdog
--- PID Simulation Start ---

[ICP] Running ICP (point-to-plane)
[PID] samples=1000000 total_time=70.466 ms avg_latency=0.070 us/op
[EWMA] loops=100 stream_len_per_loop=5000 total_time=60.510 ms avg_latency=0.121 us/sample
[ICP] samples=1 total_time=121.593 ms avg_latency=121.593 ms/run
--- PID Simulation Start ---
[PID] samples=1000000 total_time=34.277 ms avg_latency=0.034 us/op

[ICP] Running ICP (point-to-plane)
[EWMA] loops=100 stream_len_per_loop=5000 total_time=36.320 ms avg_latency=0.073 us/sample
[ICP] samples=1 total_time=90.048 ms avg_latency=90.048 ms/run
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=14.156 ms avg_latency=0.024 us/sample
[FAST] samples=3 total_time=21.137 ms avg_latency=7045.639 us/img
--- PID Simulation Start ---
[PID] samples=1000000 total_time=33.825 ms avg_latency=0.034 us/op

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=98.651 ms avg_latency=98.651 ms/run
[EWMA] loops=100 stream_len_per_loop=5000 total_time=80.769 ms avg_latency=0.162 us/sample
[FAST] samples=3 total_time=51.891 ms avg_latency=17297.099 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=70.746 ms avg_latency=0.118 us/sample
[FAST] samples=3 total_time=48.560 ms avg_latency=16186.821 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=67.981 ms avg_latency=0.113 us/sample
Gradient 60% complete: MR = 0.0000 (0/21)

>>> Utilization Gradient: 70% <<<
[test-schedule] uunifast sum=0.7000 target=0.7000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 107.865    | 34.10    | 316.337
pid          | 42.643     | 10.35    | 411.893
ewma         | 33.034     | 8.13     | 406.341
mqtt         | 32.211     | 7.46     | 431.676
cusum        | 18.558     | 4.39     | 423.099
fast         | 15.228     | 3.43     | 444.010
modbus       | 1.021      | 2.14     | 47.690
Running task set for 3 cycles...
[test-schedule] creating task thread 1/7 (icp)...
[test-schedule] creating task thread 2/7 (pid)...
[test-schedule] creating task thread 3/7 (ewma)...
[test-schedule] creating task thread 4/7 (mqtt)...
[test-schedule] creating task thread 5/7 (cusum)...
[test-schedule] creating task thread 6/7 (fast)...
[test-schedule] creating task thread 7/7 (modbus)...
[test-schedule] all 7 task threads created; entering watchdog

[ICP] Running ICP (point-to-plane)
--- PID Simulation Start ---
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=22.163 ms avg_latency=0.037 us/sample
[FAST] samples=3 total_time=14.363 ms avg_latency=4787.514 us/img
[EWMA] loops=100 stream_len_per_loop=5000 total_time=65.978 ms avg_latency=0.132 us/sample
[ICP] samples=1 total_time=156.581 ms avg_latency=156.581 ms/run
[PID] samples=1000000 total_time=61.577 ms avg_latency=0.062 us/op

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=71.762 ms avg_latency=71.762 ms/run
--- PID Simulation Start ---
[PID] samples=1000000 total_time=22.818 ms avg_latency=0.023 us/op
[EWMA] loops=100 stream_len_per_loop=5000 total_time=38.245 ms avg_latency=0.076 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=40.699 ms avg_latency=0.068 us/sample
[FAST] samples=3 total_time=26.749 ms avg_latency=8916.418 us/img

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=95.762 ms avg_latency=95.762 ms/run
--- PID Simulation Start ---
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=19.462 ms avg_latency=0.032 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=77.912 ms avg_latency=0.156 us/sample
[PID] samples=1000000 total_time=62.019 ms avg_latency=0.062 us/op
[FAST] samples=3 total_time=19.320 ms avg_latency=6440.042 us/img
Gradient 70% complete: MR = 0.0000 (0/21)

>>> Utilization Gradient: 80% <<<
[test-schedule] uunifast sum=0.8000 target=0.8000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 107.865    | 37.41    | 288.369
pid          | 42.643     | 15.82    | 269.583
ewma         | 33.034     | 15.24    | 216.715
mqtt         | 32.211     | 5.63     | 572.091
cusum        | 18.558     | 3.41     | 544.385
fast         | 15.228     | 2.23     | 682.065
modbus       | 1.021      | 0.26     | 390.222
Running task set for 3 cycles...
[test-schedule] creating task thread 1/7 (icp)...
[test-schedule] creating task thread 2/7 (pid)...
[test-schedule] creating task thread 3/7 (ewma)...
[test-schedule] creating task thread 4/7 (mqtt)...
[test-schedule] creating task thread 5/7 (cusum)...
[test-schedule] creating task thread 6/7 (fast)...
[test-schedule] creating task thread 7/7 (modbus)...
[test-schedule] all 7 task threads created; entering watchdog
--- PID Simulation Start ---

[ICP] Running ICP (point-to-plane)
[PID] samples=1000000 total_time=27.752 ms avg_latency=0.028 us/op
[EWMA] loops=100 stream_len_per_loop=5000 total_time=83.230 ms avg_latency=0.166 us/sample
[ICP] samples=1 total_time=125.501 ms avg_latency=125.501 ms/run
[EWMA] loops=100 stream_len_per_loop=5000 total_time=36.308 ms avg_latency=0.073 us/sample
--- PID Simulation Start ---
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=27.616 ms avg_latency=0.046 us/sample

[ICP] Running ICP (point-to-plane)
[PID] samples=1000000 total_time=69.725 ms avg_latency=0.070 us/op
[ICP] samples=1 total_time=70.760 ms avg_latency=70.760 ms/run
[EWMA] loops=100 stream_len_per_loop=5000 total_time=18.663 ms avg_latency=0.037 us/sample
[FAST] samples=3 total_time=22.436 ms avg_latency=7478.778 us/img
--- PID Simulation Start ---
[PID] samples=1000000 total_time=22.708 ms avg_latency=0.023 us/op

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=100.224 ms avg_latency=100.224 ms/run
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=63.933 ms avg_latency=0.107 us/sample
[FAST] samples=3 total_time=33.943 ms avg_latency=11314.445 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=69.836 ms avg_latency=0.116 us/sample
[FAST] samples=3 total_time=53.939 ms avg_latency=17979.793 us/img
Gradient 80% complete: MR = 0.0000 (0/21)

>>> Utilization Gradient: 90% <<<
[test-schedule] uunifast sum=0.9000 target=0.9000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 107.865    | 33.55    | 321.518
pid          | 42.643     | 20.48    | 208.217
ewma         | 33.034     | 17.40    | 189.863
mqtt         | 32.211     | 11.09    | 290.481
cusum        | 18.558     | 3.88     | 477.797
fast         | 15.228     | 2.91     | 522.542
modbus       | 1.021      | 0.69     | 149.014
Running task set for 3 cycles...
[test-schedule] creating task thread 1/7 (icp)...
[test-schedule] creating task thread 2/7 (pid)...
[test-schedule] creating task thread 3/7 (ewma)...
[test-schedule] creating task thread 4/7 (mqtt)...
[test-schedule] creating task thread 5/7 (cusum)...
[test-schedule] creating task thread 6/7 (fast)...
[test-schedule] creating task thread 7/7 (modbus)...
[test-schedule] all 7 task threads created; entering watchdog
--- PID Simulation Start ---
[EWMA] loops=100 stream_len_per_loop=5000 total_time=82.282 ms avg_latency=0.165 us/sample
[PID] samples=1000000 total_time=66.919 ms avg_latency=0.067 us/op

[ICP] Running ICP (point-to-plane)
[EWMA] loops=100 stream_len_per_loop=5000 total_time=35.372 ms avg_latency=0.071 us/sample
[ICP] samples=1 total_time=94.534 ms avg_latency=94.534 ms/run
--- PID Simulation Start ---
[PID] samples=1000000 total_time=23.797 ms avg_latency=0.024 us/op
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=6.231 ms avg_latency=0.010 us/sample
[FAST] samples=3 total_time=22.232 ms avg_latency=7410.764 us/img
[EWMA] loops=100 stream_len_per_loop=5000 total_time=36.275 ms avg_latency=0.073 us/sample
--- PID Simulation Start ---

[ICP] Running ICP (point-to-plane)
[PID] samples=1000000 total_time=40.042 ms avg_latency=0.040 us/op
[ICP] samples=1 total_time=92.222 ms avg_latency=92.222 ms/run

[ICP] Running ICP (point-to-plane)
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=27.593 ms avg_latency=0.046 us/sample
[ICP] samples=1 total_time=72.063 ms avg_latency=72.063 ms/run
[FAST] samples=3 total_time=16.099 ms avg_latency=5366.278 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=72.038 ms avg_latency=0.120 us/sample
[FAST] samples=3 total_time=55.144 ms avg_latency=18381.418 us/img
Gradient 90% complete: MR = 0.0000 (0/21)

>>> Utilization Gradient: 100% <<<
[test-schedule] uunifast sum=1.0000 target=1.0000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 107.865    | 45.06    | 239.363
pid          | 42.643     | 33.41    | 127.618
ewma         | 33.034     | 8.25     | 400.386
mqtt         | 32.211     | 7.19     | 448.096
cusum        | 18.558     | 5.27     | 351.909
fast         | 15.228     | 0.80     | 1903.906
modbus       | 1.021      | 0.01     | 10000.000
Running task set for 3 cycles...
[test-schedule] creating task thread 1/7 (icp)...
[test-schedule] creating task thread 2/7 (pid)...
[test-schedule] creating task thread 3/7 (ewma)...
[test-schedule] creating task thread 4/7 (mqtt)...
[test-schedule] creating task thread 5/7 (cusum)...
[test-schedule] creating task thread 6/7 (fast)...
[test-schedule] creating task thread 7/7 (modbus)...
[test-schedule] all 7 task threads created; entering watchdog
--- PID Simulation Start ---
[PID] samples=1000000 total_time=37.506 ms avg_latency=0.038 us/op

[ICP] Running ICP (point-to-plane)
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.151 ms avg_latency=0.075 us/op
[ICP] samples=1 total_time=134.280 ms avg_latency=134.280 ms/run
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=22.446 ms avg_latency=0.037 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=26.184 ms avg_latency=0.026 us/op
[EWMA] loops=100 stream_len_per_loop=5000 total_time=33.108 ms avg_latency=0.066 us/sample

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=69.473 ms avg_latency=69.473 ms/run

[ICP] Running ICP (point-to-plane)
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=30.093 ms avg_latency=0.050 us/sample
[ICP] samples=1 total_time=78.691 ms avg_latency=78.691 ms/run
[EWMA] loops=100 stream_len_per_loop=5000 total_time=22.225 ms avg_latency=0.044 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=51.651 ms avg_latency=0.086 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=36.392 ms avg_latency=0.073 us/sample
[FAST] samples=3 total_time=14.736 ms avg_latency=4911.862 us/img
[FAST] samples=3 total_time=53.316 ms avg_latency=17772.127 us/img
[test-schedule] heartbeat (still running gradient):
    fast     jobs=2/3  since_progress=1s  stall_window=5s
    modbus   jobs=0/3  since_progress=5s  stall_window=30s
[FAST] samples=3 total_time=54.286 ms avg_latency=18095.294 us/img
[test-schedule] heartbeat (still running gradient):
    modbus   jobs=1/3  since_progress=0s  stall_window=30s
[test-schedule] heartbeat (still running gradient):
    modbus   jobs=1/3  since_progress=5s  stall_window=30s
[test-schedule] heartbeat (still running gradient):
    modbus   jobs=2/3  since_progress=0s  stall_window=30s
[test-schedule] heartbeat (still running gradient):
    modbus   jobs=2/3  since_progress=5s  stall_window=30s
[test-schedule] heartbeat (still running gradient):
Gradient 100% complete: MR = 0.0000 (0/21)

=============================================================
[Phase 3] Final Results
=============================================================
U= 30%: MR=0.0000 (0 misses / 21 jobs)
U= 40%: MR=0.0000 (0 misses / 21 jobs)
U= 50%: MR=0.0000 (0 misses / 21 jobs)
U= 60%: MR=0.0000 (0 misses / 21 jobs)
U= 70%: MR=0.0000 (0 misses / 21 jobs)
U= 80%: MR=0.0000 (0 misses / 21 jobs)
U= 90%: MR=0.0000 (0 misses / 21 jobs)
U=100%: MR=0.0000 (0 misses / 21 jobs)

------------------------------------------------------
Average Miss Rate: 0.0000
Final Score: 100.00 / 100
------------------------------------------------------

>>> Running test-stress (job: all)...

=============================================================
[test-stress] Running job: all
=============================================================
Initializing Job: stored_jobfile_cpu.txt...
Running Job: [>                             ] 0/65 (0%) -> cpu
rtos_stress: debug: cpu-load set to 20%
rtos_stress: info: [cpu] load set to 20% (busy: 20ms, sleep: 80ms)
rtos_stress: info: [cpu] using 'all' methods (round-robin)
Running Job: [>                             ] 1/65 (1%) -> matrix
rtos_stress: debug: matrix-size set to 32
rtos_stress: info: [matrix-0] size=32, method=all
Running Job: [>                             ] 2/65 (3%) -> qsort
rtos_stress: debug: qsort-size set to 1024 elements
rtos_stress: info: [qsort-0] sorting 1024 elements (4 KB)
rtos_stress: info: [qsort-0] using 'all' methods
Running Job: [=>                            ] 3/65 (4%) -> atomic
rtos_stress: debug: atomic-threads set to 0
Running Job: [=>                            ] 4/65 (6%) -> bitops
rtos_stress: info: [bitops-0] using 'all' methods
Running Job: [==>                           ] 5/65 (7%) -> bsearch
rtos_stress: debug: bsearch-size set to 1024 elements
rtos_stress: info: [bsearch-0] using 1024 elements (4 KB)
rtos_stress: info: [bsearch-0] using 'all' methods
Running Job: [==>                           ] 6/65 (9%) -> context
Running Job: [===>                          ] 7/65 (10%) -> fp
rtos_stress: info: [fp-0] float/double/long double math stressor
rtos_stress: info: [fp-0] using 'all' methods
Running Job: [===>                          ] 8/65 (12%) -> prime
rtos_stress: info: [prime-0] using 'all' methods
Running Job: [====>                         ] 9/65 (13%) -> stack
rtos_stress: debug: stack-size set to 4096 bytes
rtos_stress: info: [stack-0] started (stack: 4 KB, depth: ~0)
Running Job: [====>                         ] 10/65 (15%) -> str
rtos_stress: debug: str-size set to 1024 bytes
rtos_stress: info: [str-0] string size 1024 bytes
rtos_stress: info: [str-0] using 'all' methods
Running Job: [=====>                        ] 11/65 (16%) -> trig
rtos_stress: info: [trig-0] starting trigonometric stressor
rtos_stress: info: [trig-0] using 'all' methods
Running Job: [=====>                        ] 12/65 (18%) -> vecmath
Running Job: [======>                       ] 13/65 (20%) -> cpu
rtos_stress: debug: cpu-load set to 40%
rtos_stress: info: [cpu] load set to 40% (busy: 40ms, sleep: 60ms)
rtos_stress: info: [cpu] using 'all' methods (round-robin)
Running Job: [======>                       ] 14/65 (21%) -> matrix
rtos_stress: debug: matrix-size set to 64
rtos_stress: info: [matrix-1] size=64, method=all
rtos_stress: info: [matrix-0] size=64, method=all
Running Job: [======>                       ] 15/65 (23%) -> qsort
rtos_stress: debug: qsort-size set to 4096 elements
rtos_stress: info: [qsort-0] sorting 4096 elements (16 KB)
rtos_stress: info: [qsort-0] using 'all' methods
rtos_stress: info: [qsort-1] sorting 4096 elements (16 KB)
rtos_stress: info: [qsort-1] using 'all' methods
Running Job: [=======>                      ] 16/65 (24%) -> atomic
rtos_stress: debug: atomic-threads set to 1
Running Job: [=======>                      ] 17/65 (26%) -> bitops
rtos_stress: info: [bitops-0] using 'all' methods
rtos_stress: info: [bitops-1] using 'all' methods
Running Job: [========>                     ] 18/65 (27%) -> bsearch
rtos_stress: debug: bsearch-size set to 4096 elements
rtos_stress: info: [bsearch-0] using 4096 elements (16 KB)
rtos_stress: info: [bsearch-0] using 'all' methods
rtos_stress: info: [bsearch-1] using 4096 elements (16 KB)
rtos_stress: info: [bsearch-1] using 'all' methods
Running Job: [========>                     ] 19/65 (29%) -> context
Running Job: [=========>                    ] 20/65 (30%) -> fp
rtos_stress: info: [fp-0] float/double/long double math stressor
rtos_stress: info: [fp-0] using 'all' methods
rtos_stress: info: [fp-1] float/double/long double math stressor
Running Job: [=========>                    ] 21/65 (32%) -> prime
rtos_stress: info: [prime-0] using 'all' methods
rtos_stress: info: [prime-1] using 'all' methods
Running Job: [==========>                   ] 22/65 (33%) -> stack
rtos_stress: debug: stack-size set to 8192 bytes
rtos_stress: info: [stack-0] started (stack: 8 KB, depth: ~3)
rtos_stress: info: [stack-1] started (stack: 8 KB, depth: ~3)
Running Job: [==========>                   ] 23/65 (35%) -> str
rtos_stress: debug: str-size set to 4096 bytes
rtos_stress: info: [str-0] string size 4096 bytes
rtos_stress: info: [str-1] string size 4096 bytes
rtos_stress: info: [str-0] using 'all' methods
rtos_stress: info: [str-1] using 'all' methods
Running Job: [===========>                  ] 24/65 (36%) -> trig
rtos_stress: info: [trig-0] starting trigonometric stressor
rtos_stress: info: [trig-0] using 'all' methods
rtos_stress: info: [trig-1] starting trigonometric stressor
rtos_stress: info: [trig-1] using 'all' methods
Running Job: [===========>                  ] 25/65 (38%) -> vecmath
Running Job: [============>                 ] 26/65 (40%) -> cpu
rtos_stress: debug: cpu-load set to 60%
rtos_stress: info: [cpu] load set to 60% (busy: 60ms, sleep: 40ms)
rtos_stress: info: [cpu] using 'all' methods (round-robin)
Running Job: [============>                 ] 27/65 (41%) -> matrix
rtos_stress: debug: matrix-size set to 64
rtos_stress: info: [matrix-2] size=64, method=all
rtos_stress: info: [matrix-1] size=64, method=all
rtos_stress: info: [matrix-0] size=64, method=all
Running Job: [============>                 ] 28/65 (43%) -> qsort
rtos_stress: debug: qsort-size set to 16384 elements
rtos_stress: info: [qsort-0] sorting 16384 elements (64 KB)
rtos_stress: info: [qsort-0] using 'all' methods
rtos_stress: info: [qsort-1] sorting 16384 elements (64 KB)
rtos_stress: info: [qsort-1] using 'all' methods
rtos_stress: info: [qsort-2] sorting 16384 elements (64 KB)
rtos_stress: info: [qsort-2] using 'all' methods
Running Job: [=============>                ] 29/65 (44%) -> atomic
rtos_stress: debug: atomic-threads set to 1
Running Job: [=============>                ] 30/65 (46%) -> bitops
rtos_stress: info: [bitops-0] using 'all' methods
rtos_stress: info: [bitops-2] using 'all' methods
rtos_stress: info: [bitops-1] using 'all' methods
Running Job: [==============>               ] 31/65 (47%) -> bsearch
rtos_stress: debug: bsearch-size set to 16384 elements
rtos_stress: info: [bsearch-0] using 16384 elements (64 KB)
rtos_stress: info: [bsearch-0] using 'all' methods
rtos_stress: info: [bsearch-1] using 16384 elements (64 KB)
rtos_stress: info: [bsearch-2] using 16384 elements (64 KB)
rtos_stress: info: [bsearch-1] using 'all' methods
rtos_stress: info: [bsearch-2] using 'all' methods
Running Job: [==============>               ] 32/65 (49%) -> context
Running Job: [===============>              ] 33/65 (50%) -> fp
rtos_stress: info: [fp-0] float/double/long double math stressor
rtos_stress: info: [fp-0] using 'all' methods
rtos_stress: info: [fp-1] float/double/long double math stressor
rtos_stress: info: [fp-2] float/double/long double math stressor
Running Job: [===============>              ] 34/65 (52%) -> prime
rtos_stress: info: [prime-0] using 'all' methods
rtos_stress: info: [prime-2] using 'all' methods
rtos_stress: info: [prime-1] using 'all' methods
Running Job: [================>             ] 35/65 (53%) -> stack
rtos_stress: debug: stack-size set to 16384 bytes
rtos_stress: info: [stack-0] started (stack: 16 KB, depth: ~11)
rtos_stress: info: [stack-1] started (stack: 16 KB, depth: ~11)
rtos_stress: info: [stack-2] started (stack: 16 KB, depth: ~11)
Running Job: [================>             ] 36/65 (55%) -> str
rtos_stress: debug: str-size set to 8192 bytes
rtos_stress: info: [str-0] string size 8192 bytes
rtos_stress: info: [str-1] string size 8192 bytes
rtos_stress: info: [str-2] string size 8192 bytes
rtos_stress: info: [str-0] using 'all' methods
rtos_stress: info: [str-1] using 'all' methods
rtos_stress: info: [str-2] using 'all' methods
Running Job: [=================>            ] 37/65 (56%) -> trig
rtos_stress: info: [trig-0] starting trigonometric stressor
rtos_stress: info: [trig-0] using 'all' methods
rtos_stress: info: [trig-1] starting trigonometric stressor
rtos_stress: info: [trig-1] using 'all' methods
rtos_stress: info: [trig-2] starting trigonometric stressor
rtos_stress: info: [trig-2] using 'all' methods
Running Job: [=================>            ] 38/65 (58%) -> vecmath
Running Job: [==================>           ] 39/65 (60%) -> cpu
rtos_stress: debug: cpu-load set to 80%
rtos_stress: info: [cpu] load set to 80% (busy: 80ms, sleep: 20ms)
rtos_stress: info: [cpu] using 'all' methods (round-robin)
Running Job: [==================>           ] 40/65 (61%) -> matrix
rtos_stress: debug: matrix-size set to 128
rtos_stress: info: [matrix-0] size=128, method=all
rtos_stress: info: [matrix-1] size=128, method=all
rtos_stress: info: [matrix-2] size=128, method=all
rtos_stress: info: [matrix-3] size=128, method=all
Running Job: [==================>           ] 41/65 (63%) -> qsort
rtos_stress: debug: qsort-size set to 32768 elements
rtos_stress: info: [qsort-0] sorting 32768 elements (128 KB)
rtos_stress: info: [qsort-0] using 'all' methods
rtos_stress: info: [qsort-1] sorting 32768 elements (128 KB)
rtos_stress: info: [qsort-1] using 'all' methods
rtos_stress: info: [qsort-2] sorting 32768 elements (128 KB)
rtos_stress: info: [qsort-2] using 'all' methods
rtos_stress: info: [qsort-3] sorting 32768 elements (128 KB)
rtos_stress: info: [qsort-3] using 'all' methods
Running Job: [===================>          ] 42/65 (64%) -> atomic
rtos_stress: debug: atomic-threads set to 3
Running Job: [===================>          ] 43/65 (66%) -> bitops
rtos_stress: info: [bitops-0] using 'all' methods
rtos_stress: info: [bitops-1] using 'all' methods
rtos_stress: info: [bitops-2] using 'all' methods
rtos_stress: info: [bitops-3] using 'all' methods
Running Job: [====================>         ] 44/65 (67%) -> bsearch
rtos_stress: debug: bsearch-size set to 32768 elements
rtos_stress: info: [bsearch-0] using 32768 elements (128 KB)
rtos_stress: info: [bsearch-0] using 'all' methods
rtos_stress: info: [bsearch-1] using 32768 elements (128 KB)
rtos_stress: info: [bsearch-3] using 32768 elements (128 KB)
rtos_stress: info: [bsearch-2] using 32768 elements (128 KB)
rtos_stress: info: [bsearch-3] using 'all' methods
rtos_stress: info: [bsearch-2] using 'all' methods
rtos_stress: info: [bsearch-1] using 'all' methods
Running Job: [====================>         ] 45/65 (69%) -> context
Running Job: [=====================>        ] 46/65 (70%) -> fp
rtos_stress: info: [fp-0] float/double/long double math stressor
rtos_stress: info: [fp-0] using 'all' methods
rtos_stress: info: [fp-1] float/double/long double math stressor
rtos_stress: info: [fp-2] float/double/long double math stressor
rtos_stress: info: [fp-3] float/double/long double math stressor
Running Job: [=====================>        ] 47/65 (72%) -> prime
rtos_stress: info: [prime-0] using 'all' methods
rtos_stress: info: [prime-1] using 'all' methods
rtos_stress: info: [prime-3] using 'all' methods
rtos_stress: info: [prime-2] using 'all' methods
Running Job: [======================>       ] 48/65 (73%) -> stack
rtos_stress: debug: stack-size set to 32768 bytes
rtos_stress: info: [stack-0] started (stack: 32 KB, depth: ~26)
rtos_stress: info: [stack-1] started (stack: 32 KB, depth: ~26)
rtos_stress: info: [stack-3] started (stack: 32 KB, depth: ~26)
rtos_stress: info: [stack-2] started (stack: 32 KB, depth: ~26)
Running Job: [======================>       ] 49/65 (75%) -> str
rtos_stress: debug: str-size set to 8192 bytes
rtos_stress: info: [str-0] string size 8192 bytes
rtos_stress: info: [str-1] string size 8192 bytes
rtos_stress: info: [str-2] string size 8192 bytes
rtos_stress: info: [str-3] string size 8192 bytes
rtos_stress: info: [str-0] using 'all' methods
rtos_stress: info: [str-2] using 'all' methods
rtos_stress: info: [str-1] using 'all' methods
rtos_stress: info: [str-3] using 'all' methods
Running Job: [=======================>      ] 50/65 (76%) -> trig
rtos_stress: info: [trig-0] starting trigonometric stressor
rtos_stress: info: [trig-0] using 'all' methods
rtos_stress: info: [trig-1] starting trigonometric stressor
rtos_stress: info: [trig-1] using 'all' methods
rtos_stress: info: [trig-2] starting trigonometric stressor
rtos_stress: info: [trig-2] using 'all' methods
rtos_stress: info: [trig-3] starting trigonometric stressor
rtos_stress: info: [trig-3] using 'all' methods
Running Job: [=======================>      ] 51/65 (78%) -> vecmath
Running Job: [========================>     ] 52/65 (80%) -> cpu
rtos_stress: debug: cpu-load set to 100%
rtos_stress: info: [cpu] load set to 100% (busy: 100ms, sleep: 0ms)
rtos_stress: info: [cpu] using 'all' methods (round-robin)
Running Job: [========================>     ] 53/65 (81%) -> matrix
rtos_stress: debug: matrix-size set to 128
rtos_stress: info: [matrix-3] size=128, method=all
rtos_stress: info: [matrix-1] size=128, method=all
rtos_stress: info: [matrix-0] size=128, method=all
rtos_stress: info: [matrix-2] size=128, method=all
rtos_stress: info: [matrix-4] size=128, method=all
Running Job: [========================>     ] 54/65 (83%) -> qsort
rtos_stress: debug: qsort-size set to 65536 elements
rtos_stress: info: [qsort-0] sorting 65536 elements (256 KB)
rtos_stress: info: [qsort-0] using 'all' methods
rtos_stress: info: [qsort-1] sorting 65536 elements (256 KB)
rtos_stress: info: [qsort-1] using 'all' methods
rtos_stress: info: [qsort-2] sorting 65536 elements (256 KB)
rtos_stress: info: [qsort-2] using 'all' methods
rtos_stress: info: [qsort-3] sorting 65536 elements (256 KB)
rtos_stress: info: [qsort-3] using 'all' methods
rtos_stress: info: [qsort-4] sorting 65536 elements (256 KB)
rtos_stress: info: [qsort-4] using 'all' methods
Running Job: [=========================>    ] 55/65 (84%) -> atomic
rtos_stress: debug: atomic-threads set to 3
Running Job: [=========================>    ] 56/65 (86%) -> bitops
rtos_stress: info: [bitops-1] using 'all' methods
rtos_stress: info: [bitops-0] using 'all' methods
rtos_stress: info: [bitops-2] using 'all' methods
rtos_stress: info: [bitops-4] using 'all' methods
rtos_stress: info: [bitops-3] using 'all' methods
Running Job: [==========================>   ] 57/65 (87%) -> bsearch
rtos_stress: debug: bsearch-size set to 65536 elements
rtos_stress: info: [bsearch-0] using 65536 elements (256 KB)
rtos_stress: info: [bsearch-1] using 65536 elements (256 KB)
rtos_stress: info: [bsearch-2] using 65536 elements (256 KB)
rtos_stress: info: [bsearch-0] using 'all' methods
rtos_stress: info: [bsearch-3] using 65536 elements (256 KB)
rtos_stress: info: [bsearch-4] using 65536 elements (256 KB)
rtos_stress: info: [bsearch-2] using 'all' methods
rtos_stress: info: [bsearch-1] using 'all' methods
rtos_stress: info: [bsearch-3] using 'all' methods
rtos_stress: info: [bsearch-4] using 'all' methods
Running Job: [==========================>   ] 58/65 (89%) -> context
Running Job: [===========================>  ] 59/65 (90%) -> fp
rtos_stress: info: [fp-1] float/double/long double math stressor
rtos_stress: info: [fp-0] float/double/long double math stressor
rtos_stress: info: [fp-0] using 'all' methods
rtos_stress: info: [fp-2] float/double/long double math stressor
rtos_stress: info: [fp-3] float/double/long double math stressor
rtos_stress: info: [fp-4] float/double/long double math stressor
Running Job: [===========================>  ] 60/65 (92%) -> prime
rtos_stress: info: [prime-0] using 'all' methods
rtos_stress: info: [prime-1] using 'all' methods
rtos_stress: info: [prime-3] using 'all' methods
rtos_stress: info: [prime-2] using 'all' methods
rtos_stress: info: [prime-4] using 'all' methods
Running Job: [============================> ] 61/65 (93%) -> stack
rtos_stress: debug: stack-size set to 65536 bytes
rtos_stress: info: [stack-0] started (stack: 64 KB, depth: ~56)
rtos_stress: info: [stack-1] started (stack: 64 KB, depth: ~56)
rtos_stress: info: [stack-2] started (stack: 64 KB, depth: ~56)
rtos_stress: info: [stack-4] started (stack: 64 KB, depth: ~56)
rtos_stress: info: [stack-3] started (stack: 64 KB, depth: ~56)
Running Job: [============================> ] 62/65 (95%) -> str
rtos_stress: debug: str-size set to 16384 bytes
rtos_stress: info: [str-0] string size 16384 bytes
rtos_stress: info: [str-1] string size 16384 bytes
rtos_stress: info: [str-2] string size 16384 bytes
rtos_stress: info: [str-3] string size 16384 bytes
rtos_stress: info: [str-4] string size 16384 bytes
rtos_stress: info: [str-0] using 'all' methods
rtos_stress: info: [str-1] using 'all' methods
rtos_stress: info: [str-4] using 'all' methods
rtos_stress: info: [str-2] using 'all' methods
rtos_stress: info: [str-3] using 'all' methods
Running Job: [=============================>] 63/65 (96%) -> trig
rtos_stress: info: [trig-0] starting trigonometric stressor
rtos_stress: info: [trig-0] using 'all' methods
rtos_stress: info: [trig-1] starting trigonometric stressor
rtos_stress: info: [trig-1] using 'all' methods
rtos_stress: info: [trig-2] starting trigonometric stressor
rtos_stress: info: [trig-2] using 'all' methods
rtos_stress: info: [trig-3] starting trigonometric stressor
rtos_stress: info: [trig-3] using 'all' methods
rtos_stress: info: [trig-4] starting trigonometric stressor
rtos_stress: info: [trig-4] using 'all' methods
Running Job: [=============================>] 64/65 (98%) -> vecmath
Running Job: [==============================] 65/65 (100%) -> Finished


========================================================================
 Job: stored_jobfile_cpu.txt [Built-in]
========================================================================
 Stressor    | Stage | Bogo Ops     | Time(s)  | Metric
------------------------------------------------------------------------
 cpu         | 1     | 2800         | 0.46     | N/A
 matrix      | 1     | 6400         | 6.74     | N/A
 qsort       | 1     | 2500         | 3.63     | N/A
 atomic      | 1     | 3000000      | 1.32     | N/A
 bitops      | 1     | 24000        | 4.28     | N/A
 bsearch     | 1     | 3100         | 4.10     | N/A
 context     | 1     | 690000       | 0.20     | N/A
 fp          | 1     | 1050         | 5.77     | N/A
 prime       | 1     | 2700026      | 7.32     | N/A
 stack       | 1     | 6900         | 7.10     | N/A
 str         | 1     | 1300000      | 0.29     | N/A
 trig        | 1     | 210          | 0.80     | N/A
 vecmath     | 1     | 2700         | 2.78     | N/A
 cpu         | 2     | 9800         | 0.36     | N/A
 matrix      | 2     | 5600         | 3.54     | N/A
 qsort       | 2     | 1380         | 1.52     | N/A
 atomic      | 2     | 6600000      | 5.70     | N/A
 bitops      | 2     | 50000        | 4.47     | N/A
 bsearch     | 2     | 2400         | 2.00     | N/A
 context     | 2     | 880000       | 1.80     | N/A
 fp          | 2     | 1460         | 4.02     | N/A
 prime       | 2     | 4200548      | 6.03     | N/A
 stack       | 2     | 13400        | 6.95     | N/A
 str         | 2     | 750000       | 0.36     | N/A
 trig        | 2     | 320          | 0.58     | N/A
 vecmath     | 2     | 3800         | 1.97     | N/A
 cpu         | 3     | 13500        | 0.29     | N/A
 matrix      | 3     | 4800         | 2.06     | N/A
 qsort       | 3     | 300          | 0.79     | N/A
 atomic      | 3     | 5400000      | 5.31     | N/A
 bitops      | 3     | 42000        | 2.51     | N/A
 bsearch     | 3     | 420          | 0.44     | N/A
 context     | 3     | 630000       | 1.90     | N/A
 fp          | 3     | 1200         | 2.25     | N/A
 prime       | 3     | 3901236      | 3.76     | N/A
 stack       | 3     | 11100        | 4.03     | N/A
 str         | 3     | 465000       | 0.19     | N/A
 trig        | 3     | 360          | 0.51     | N/A
 vecmath     | 3     | 3900         | 1.36     | N/A
 cpu         | 4     | 17600        | 0.21     | N/A
 matrix      | 4     | 1120         | 0.56     | N/A
 qsort       | 4     | 180          | 0.90     | N/A
 atomic      | 4     | 5600000      | 9.85     | N/A
 bitops      | 4     | 42000        | 1.92     | N/A
 bsearch     | 4     | 220          | 0.28     | N/A
 context     | 4     | 600000       | 1.60     | N/A
 fp          | 4     | 1280         | 1.82     | N/A
 prime       | 4     | 4400608      | 3.18     | N/A
 stack       | 4     | 10800        | 3.16     | N/A
 str         | 4     | 660000       | 0.18     | N/A
 trig        | 4     | 500          | 0.49     | N/A
 vecmath     | 4     | 5400         | 1.42     | N/A
 cpu         | 5     | 20500        | 0.29     | N/A
 matrix      | 5     | 950          | 0.43     | N/A
 qsort       | 5     | 75           | 0.83     | N/A
 atomic      | 5     | 6000000      | 10.31    | N/A
 bitops      | 5     | 44000        | 1.60     | N/A
 bsearch     | 5     | 100          | 0.23     | N/A
 context     | 5     | 650000       | 1.60     | N/A
 fp          | 5     | 1300         | 1.64     | N/A
 prime       | 5     | 4004700      | 2.34     | N/A
 stack       | 5     | 6750         | 1.73     | N/A
 str         | 5     | 125000       | 0.27     | N/A
 trig        | 5     | 525          | 0.48     | N/A
 vecmath     | 5     | 5500         | 1.16     | N/A
------------------------------------------------------------------------
Total Run Time: 2m 37s
Initializing Job: stored_jobfile_memory.txt...
Running Job: [>                             ] 0/30 (0%) -> memcpy
rtos_stress: debug: memcpy-size set to 2048
rtos_stress: info: [memcpy-0] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-0] using 'all' methods
Running Job: [=>                            ] 1/30 (3%) -> stream
rtos_stress: debug: stream-elem set to 2048 elements
rtos_stress: info: [stream-0] target 2048 elements (approx 48 KB total)
rtos_stress: info: [stream-0] ramp: 1024 of 2048 elements
rtos_stress: info: [stream-0] ramp: 2048 of 2048 elements
rtos_stress: info: [stream-0] bandwidth: 24246.60 MB/sec, compute: 1271.22 Mflop/sec
Running Job: [==>                           ] 2/30 (6%) -> vm
rtos_stress: debug: vm-bytes set to 1048576 bytes
rtos_stress: info: [vm-0] allocated 1024 KB buffer
rtos_stress: info: [vm-0] using 'all' methods
Running Job: [===>                          ] 3/30 (10%) -> malloc
rtos_stress: debug: malloc-bytes set to 256
rtos_stress: debug: malloc-max set to 64 slots
rtos_stress: info: [malloc-0] max_slots=64 max_bytes=256
rtos_stress: info: [malloc-0] done
Running Job: [====>                         ] 4/30 (13%) -> memthrash
rtos_stress: info: [memthrash-0] allocated 64 KB
rtos_stress: info: [memthrash-0] using 'all' methods
Running Job: [=====>                        ] 5/30 (16%) -> ptr-chase
rtos_stress: debug: ptr-chase-pages set to 128
rtos_stress: info: [ptr-chase-0] chasing 128 pages (512 KB)
rtos_stress: [ptr-chase-0] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-0] heartbeat: 20000768 ops
rtos_stress: [ptr-chase-0] heartbeat: 30001152 ops
rtos_stress: [ptr-chase-0] heartbeat: 40001536 ops
rtos_stress: [ptr-chase-0] heartbeat: 50001920 ops
rtos_stress: [ptr-chase-0] heartbeat: 60002304 ops
rtos_stress: [ptr-chase-0] heartbeat: 70002688 ops
rtos_stress: [ptr-chase-0] heartbeat: 80003072 ops
rtos_stress: info: [ptr-chase-0] finished. visited 65536 of 65536 pointers (100.00%)
Running Job: [======>                       ] 6/30 (20%) -> memcpy
rtos_stress: debug: memcpy-size set to 2048
rtos_stress: info: [memcpy-0] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-0] using 'all' methods
rtos_stress: info: [memcpy-1] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-1] using 'all' methods
Running Job: [=======>                      ] 7/30 (23%) -> stream
rtos_stress: debug: stream-elem set to 4096 elements
rtos_stress: info: [stream-0] target 4096 elements (approx 96 KB total)
rtos_stress: info: [stream-0] ramp: 1024 of 4096 elements
rtos_stress: info: [stream-1] target 4096 elements (approx 96 KB total)
rtos_stress: info: [stream-1] ramp: 1024 of 4096 elements
rtos_stress: info: [stream-0] ramp: 2048 of 4096 elements
rtos_stress: info: [stream-1] ramp: 2048 of 4096 elements
rtos_stress: info: [stream-0] ramp: 4096 of 4096 elements
rtos_stress: info: [stream-1] ramp: 4096 of 4096 elements
rtos_stress: info: [stream-1] bandwidth: 22421.71 MB/sec, compute: 1175.54 Mflop/sec
rtos_stress: info: [stream-0] bandwidth: 22397.23 MB/sec, compute: 1174.26 Mflop/sec
Running Job: [========>                     ] 8/30 (26%) -> vm
rtos_stress: debug: vm-bytes set to 4194304 bytes
rtos_stress: info: [vm-1] allocated 4096 KB buffer
rtos_stress: info: [vm-1] using 'all' methods
rtos_stress: info: [vm-0] allocated 4096 KB buffer
rtos_stress: info: [vm-0] using 'all' methods
Running Job: [=========>                    ] 9/30 (30%) -> malloc
rtos_stress: debug: malloc-bytes set to 1024
rtos_stress: debug: malloc-max set to 64 slots
rtos_stress: info: [malloc-0] max_slots=64 max_bytes=1024
rtos_stress: info: [malloc-1] max_slots=64 max_bytes=1024
rtos_stress: info: [malloc-1] done
rtos_stress: info: [malloc-0] done
Running Job: [==========>                   ] 10/30 (33%) -> memthrash
rtos_stress: info: [memthrash-0] allocated 64 KB
rtos_stress: info: [memthrash-0] using 'all' methods
rtos_stress: info: [memthrash-1] allocated 64 KB
rtos_stress: info: [memthrash-1] using 'all' methods
Running Job: [===========>                  ] 11/30 (36%) -> ptr-chase
rtos_stress: debug: ptr-chase-pages set to 256
rtos_stress: info: [ptr-chase-0] chasing 256 pages (1024 KB)
rtos_stress: info: [ptr-chase-1] chasing 256 pages (1024 KB)
rtos_stress: [ptr-chase-0] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-0] heartbeat: 20000768 ops
rtos_stress: [ptr-chase-1] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-0] heartbeat: 30001152 ops
rtos_stress: [ptr-chase-0] heartbeat: 40001536 ops
rtos_stress: [ptr-chase-1] heartbeat: 20000768 ops
rtos_stress: [ptr-chase-0] heartbeat: 50001920 ops
rtos_stress: [ptr-chase-0] heartbeat: 60002304 ops
rtos_stress: [ptr-chase-1] heartbeat: 30001152 ops
rtos_stress: info: [ptr-chase-0] finished. visited 131072 of 131072 pointers (100.00%)
rtos_stress: [ptr-chase-1] heartbeat: 40001536 ops
rtos_stress: [ptr-chase-1] heartbeat: 50001920 ops
rtos_stress: [ptr-chase-1] heartbeat: 60002304 ops
rtos_stress: info: [ptr-chase-1] finished. visited 131072 of 131072 pointers (100.00%)
Running Job: [============>                 ] 12/30 (40%) -> memcpy
rtos_stress: debug: memcpy-size set to 2048
rtos_stress: info: [memcpy-0] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-0] using 'all' methods
rtos_stress: info: [memcpy-1] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-1] using 'all' methods
rtos_stress: info: [memcpy-2] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-2] using 'all' methods
Running Job: [=============>                ] 13/30 (43%) -> stream
rtos_stress: debug: stream-elem set to 8192 elements
rtos_stress: info: [stream-2] target 8192 elements (approx 192 KB total)
rtos_stress: info: [stream-2] ramp: 1024 of 8192 elements
rtos_stress: info: [stream-0] target 8192 elements (approx 192 KB total)
rtos_stress: info: [stream-1] target 8192 elements (approx 192 KB total)
rtos_stress: info: [stream-1] ramp: 1024 of 8192 elements
rtos_stress: info: [stream-0] ramp: 1024 of 8192 elements
rtos_stress: info: [stream-2] ramp: 2048 of 8192 elements
rtos_stress: info: [stream-1] ramp: 2048 of 8192 elements
rtos_stress: info: [stream-2] ramp: 4096 of 8192 elements
rtos_stress: info: [stream-1] ramp: 4096 of 8192 elements
rtos_stress: info: [stream-0] ramp: 2048 of 8192 elements
rtos_stress: info: [stream-2] ramp: 8192 of 8192 elements
rtos_stress: info: [stream-1] ramp: 8192 of 8192 elements
rtos_stress: info: [stream-0] ramp: 4096 of 8192 elements
rtos_stress: info: [stream-0] ramp: 8192 of 8192 elements
rtos_stress: info: [stream-1] bandwidth: 22262.08 MB/sec, compute: 1167.17 Mflop/sec
rtos_stress: info: [stream-2] bandwidth: 22194.16 MB/sec, compute: 1163.61 Mflop/sec
rtos_stress: info: [stream-0] bandwidth: 22160.85 MB/sec, compute: 1161.87 Mflop/sec
Running Job: [==============>               ] 14/30 (46%) -> vm
rtos_stress: debug: vm-bytes set to 2097152 bytes
rtos_stress: info: [vm-0] allocated 2048 KB buffer
rtos_stress: info: [vm-0] using 'all' methods
rtos_stress: info: [vm-1] allocated 2048 KB buffer
rtos_stress: info: [vm-1] using 'all' methods
Running Job: [===============>              ] 15/30 (50%) -> malloc
rtos_stress: debug: malloc-bytes set to 2048
rtos_stress: debug: malloc-max set to 64 slots
rtos_stress: info: [malloc-0] max_slots=64 max_bytes=2048
rtos_stress: info: [malloc-1] max_slots=64 max_bytes=2048
rtos_stress: info: [malloc-2] max_slots=64 max_bytes=2048
rtos_stress: info: [malloc-2] done
rtos_stress: info: [malloc-1] done
rtos_stress: info: [malloc-0] done
Running Job: [================>             ] 16/30 (53%) -> memthrash
rtos_stress: info: [memthrash-0] allocated 64 KB
rtos_stress: info: [memthrash-0] using 'all' methods
rtos_stress: info: [memthrash-2] allocated 64 KB
rtos_stress: info: [memthrash-2] using 'all' methods
rtos_stress: info: [memthrash-1] allocated 64 KB
rtos_stress: info: [memthrash-1] using 'all' methods
Running Job: [=================>            ] 17/30 (56%) -> ptr-chase
rtos_stress: debug: ptr-chase-pages set to 512
rtos_stress: info: [ptr-chase-0] chasing 512 pages (2048 KB)
rtos_stress: info: [ptr-chase-2] chasing 512 pages (2048 KB)
rtos_stress: info: [ptr-chase-1] chasing 512 pages (2048 KB)
rtos_stress: [ptr-chase-2] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-2] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-2] finished. visited 262144 of 262144 pointers (100.00%)
rtos_stress: [ptr-chase-1] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-1] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-1] finished. visited 262144 of 262144 pointers (100.00%)
rtos_stress: [ptr-chase-0] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-0] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-0] finished. visited 262144 of 262144 pointers (100.00%)
Running Job: [==================>           ] 18/30 (60%) -> memcpy
rtos_stress: debug: memcpy-size set to 2048
rtos_stress: info: [memcpy-0] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-0] using 'all' methods
rtos_stress: info: [memcpy-1] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-1] using 'all' methods
rtos_stress: info: [memcpy-3] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-3] using 'all' methods
rtos_stress: info: [memcpy-2] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-2] using 'all' methods
Running Job: [===================>          ] 19/30 (63%) -> stream
rtos_stress: debug: stream-elem set to 16384 elements
rtos_stress: info: [stream-0] target 16384 elements (approx 384 KB total)
rtos_stress: info: [stream-0] ramp: 1024 of 16384 elements
rtos_stress: info: [stream-1] target 16384 elements (approx 384 KB total)
rtos_stress: info: [stream-1] ramp: 1024 of 16384 elements
rtos_stress: info: [stream-2] target 16384 elements (approx 384 KB total)
rtos_stress: info: [stream-2] ramp: 1024 of 16384 elements
rtos_stress: info: [stream-0] ramp: 2048 of 16384 elements
rtos_stress: info: [stream-3] target 16384 elements (approx 384 KB total)
rtos_stress: info: [stream-3] ramp: 1024 of 16384 elements
rtos_stress: info: [stream-1] ramp: 2048 of 16384 elements
rtos_stress: info: [stream-2] ramp: 2048 of 16384 elements
rtos_stress: info: [stream-0] ramp: 4096 of 16384 elements
rtos_stress: info: [stream-3] ramp: 2048 of 16384 elements
rtos_stress: info: [stream-1] ramp: 4096 of 16384 elements
rtos_stress: info: [stream-2] ramp: 4096 of 16384 elements
rtos_stress: info: [stream-3] ramp: 4096 of 16384 elements
rtos_stress: info: [stream-0] ramp: 8192 of 16384 elements
rtos_stress: info: [stream-1] ramp: 8192 of 16384 elements
rtos_stress: info: [stream-2] ramp: 8192 of 16384 elements
rtos_stress: info: [stream-3] ramp: 8192 of 16384 elements
rtos_stress: info: [stream-0] ramp: 16384 of 16384 elements
rtos_stress: info: [stream-1] ramp: 16384 of 16384 elements
rtos_stress: info: [stream-2] ramp: 16384 of 16384 elements
rtos_stress: info: [stream-3] ramp: 16384 of 16384 elements
rtos_stress: info: [stream-2] bandwidth: 20667.09 MB/sec, compute: 1083.55 Mflop/sec
rtos_stress: info: [stream-0] bandwidth: 19567.38 MB/sec, compute: 1025.89 Mflop/sec
rtos_stress: info: [stream-3] bandwidth: 19478.97 MB/sec, compute: 1021.26 Mflop/sec
rtos_stress: info: [stream-1] bandwidth: 18964.21 MB/sec, compute: 994.27 Mflop/sec
Running Job: [====================>         ] 20/30 (66%) -> vm
rtos_stress: debug: vm-bytes set to 4194304 bytes
rtos_stress: info: [vm-0] allocated 4096 KB buffer
rtos_stress: info: [vm-0] using 'all' methods
rtos_stress: info: [vm-1] allocated 4096 KB buffer
rtos_stress: info: [vm-1] using 'all' methods
Running Job: [=====================>        ] 21/30 (70%) -> malloc
rtos_stress: debug: malloc-bytes set to 4096
rtos_stress: debug: malloc-max set to 64 slots
rtos_stress: info: [malloc-0] max_slots=64 max_bytes=4096
rtos_stress: info: [malloc-1] max_slots=64 max_bytes=4096
rtos_stress: info: [malloc-3] max_slots=64 max_bytes=4096
rtos_stress: info: [malloc-2] max_slots=64 max_bytes=4096
rtos_stress: info: [malloc-0] done
rtos_stress: info: [malloc-1] done
rtos_stress: info: [malloc-3] done
rtos_stress: info: [malloc-2] done
Running Job: [======================>       ] 22/30 (73%) -> memthrash
rtos_stress: info: [memthrash-0] allocated 64 KB
rtos_stress: info: [memthrash-0] using 'all' methods
rtos_stress: info: [memthrash-1] allocated 64 KB
rtos_stress: info: [memthrash-1] using 'all' methods
rtos_stress: info: [memthrash-3] allocated 64 KB
rtos_stress: info: [memthrash-3] using 'all' methods
rtos_stress: info: [memthrash-2] allocated 64 KB
rtos_stress: info: [memthrash-2] using 'all' methods
Running Job: [=======================>      ] 23/30 (76%) -> ptr-chase
rtos_stress: debug: ptr-chase-pages set to 256
rtos_stress: info: [ptr-chase-0] chasing 256 pages (1024 KB)
rtos_stress: info: [ptr-chase-1] chasing 256 pages (1024 KB)
rtos_stress: info: [ptr-chase-2] chasing 256 pages (1024 KB)
rtos_stress: info: [ptr-chase-3] chasing 256 pages (1024 KB)
rtos_stress: [ptr-chase-1] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-3] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-1] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-1] finished. visited 131072 of 131072 pointers (100.00%)
rtos_stress: [ptr-chase-3] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-3] finished. visited 131072 of 131072 pointers (100.00%)
rtos_stress: [ptr-chase-2] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-0] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-2] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-2] finished. visited 131072 of 131072 pointers (100.00%)
rtos_stress: [ptr-chase-0] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-0] finished. visited 131072 of 131072 pointers (100.00%)
Running Job: [========================>     ] 24/30 (80%) -> memcpy
rtos_stress: debug: memcpy-size set to 2048
rtos_stress: info: [memcpy-0] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-0] using 'all' methods
rtos_stress: info: [memcpy-1] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-1] using 'all' methods
rtos_stress: info: [memcpy-2] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-2] using 'all' methods
rtos_stress: info: [memcpy-3] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-3] using 'all' methods
rtos_stress: info: [memcpy-4] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-4] using 'all' methods
Running Job: [=========================>    ] 25/30 (83%) -> stream
rtos_stress: debug: stream-elem set to 32768 elements
rtos_stress: info: [stream-0] target 32768 elements (approx 768 KB total)
rtos_stress: info: [stream-2] target 32768 elements (approx 768 KB total)
rtos_stress: info: [stream-2] ramp: 2048 of 32768 elements
rtos_stress: info: [stream-3] target 32768 elements (approx 768 KB total)
rtos_stress: info: [stream-3] ramp: 2048 of 32768 elements
rtos_stress: info: [stream-0] ramp: 2048 of 32768 elements
rtos_stress: info: [stream-1] target 32768 elements (approx 768 KB total)
rtos_stress: info: [stream-1] ramp: 2048 of 32768 elements
rtos_stress: info: [stream-0] ramp: 4096 of 32768 elements
rtos_stress: info: [stream-2] ramp: 4096 of 32768 elements
rtos_stress: info: [stream-1] ramp: 4096 of 32768 elements
rtos_stress: info: [stream-3] ramp: 4096 of 32768 elements
rtos_stress: info: [stream-0] ramp: 8192 of 32768 elements
rtos_stress: info: [stream-2] ramp: 8192 of 32768 elements
rtos_stress: info: [stream-1] ramp: 8192 of 32768 elements
rtos_stress: info: [stream-3] ramp: 8192 of 32768 elements
rtos_stress: info: [stream-0] ramp: 16384 of 32768 elements
rtos_stress: info: [stream-1] ramp: 16384 of 32768 elements
rtos_stress: info: [stream-2] ramp: 16384 of 32768 elements
rtos_stress: info: [stream-3] ramp: 16384 of 32768 elements
rtos_stress: info: [stream-1] ramp: 32768 of 32768 elements
rtos_stress: info: [stream-2] ramp: 32768 of 32768 elements
rtos_stress: info: [stream-0] ramp: 32768 of 32768 elements
rtos_stress: info: [stream-3] ramp: 32768 of 32768 elements
rtos_stress: info: [stream-1] bandwidth: 8032.58 MB/sec, compute: 421.14 Mflop/sec
rtos_stress: info: [stream-2] bandwidth: 7729.97 MB/sec, compute: 405.27 Mflop/sec
rtos_stress: info: [stream-0] bandwidth: 6754.06 MB/sec, compute: 354.11 Mflop/sec
rtos_stress: info: [stream-3] bandwidth: 6623.90 MB/sec, compute: 347.28 Mflop/sec
Running Job: [==========================>   ] 26/30 (86%) -> vm
rtos_stress: debug: vm-bytes set to 8388608 bytes
rtos_stress: info: [vm-0] allocated 8192 KB buffer
rtos_stress: info: [vm-0] using 'all' methods
rtos_stress: info: [vm-1] allocated 8192 KB buffer
rtos_stress: info: [vm-1] using 'all' methods
rtos_stress: info: [vm-2] allocated 8192 KB buffer
rtos_stress: info: [vm-2] using 'all' methods
Running Job: [===========================>  ] 27/30 (90%) -> malloc
rtos_stress: debug: malloc-bytes set to 8192
rtos_stress: debug: malloc-max set to 64 slots
rtos_stress: info: [malloc-0] max_slots=64 max_bytes=8192
rtos_stress: info: [malloc-2] max_slots=64 max_bytes=8192
rtos_stress: info: [malloc-1] max_slots=64 max_bytes=8192
rtos_stress: info: [malloc-3] max_slots=64 max_bytes=8192
rtos_stress: info: [malloc-4] max_slots=64 max_bytes=8192
rtos_stress: info: [malloc-0] done
rtos_stress: info: [malloc-4] done
rtos_stress: info: [malloc-1] done
rtos_stress: info: [malloc-2] done
rtos_stress: info: [malloc-3] done
Running Job: [============================> ] 28/30 (93%) -> memthrash
rtos_stress: info: [memthrash-0] allocated 64 KB
rtos_stress: info: [memthrash-0] using 'all' methods
rtos_stress: info: [memthrash-1] allocated 64 KB
rtos_stress: info: [memthrash-1] using 'all' methods
rtos_stress: info: [memthrash-2] allocated 64 KB
rtos_stress: info: [memthrash-3] allocated 64 KB
rtos_stress: info: [memthrash-3] using 'all' methods
rtos_stress: info: [memthrash-2] using 'all' methods
rtos_stress: info: [memthrash-4] allocated 64 KB
rtos_stress: info: [memthrash-4] using 'all' methods
Running Job: [=============================>] 29/30 (96%) -> ptr-chase
rtos_stress: debug: ptr-chase-pages set to 128
rtos_stress: info: [ptr-chase-0] chasing 128 pages (512 KB)
rtos_stress: info: [ptr-chase-1] chasing 128 pages (512 KB)
rtos_stress: info: [ptr-chase-2] chasing 128 pages (512 KB)
rtos_stress: info: [ptr-chase-3] chasing 128 pages (512 KB)
rtos_stress: info: [ptr-chase-4] chasing 128 pages (512 KB)
rtos_stress: [ptr-chase-0] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-4] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-2] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-0] heartbeat: 20000768 ops
rtos_stress: [ptr-chase-4] heartbeat: 20000768 ops
rtos_stress: [ptr-chase-2] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-0] finished. visited 65536 of 65536 pointers (100.00%)
rtos_stress: info: [ptr-chase-4] finished. visited 65536 of 65536 pointers (100.00%)
rtos_stress: info: [ptr-chase-2] finished. visited 65536 of 65536 pointers (100.00%)
rtos_stress: [ptr-chase-3] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-1] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-3] heartbeat: 20000768 ops
rtos_stress: [ptr-chase-1] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-3] finished. visited 65536 of 65536 pointers (100.00%)
rtos_stress: info: [ptr-chase-1] finished. visited 65536 of 65536 pointers (100.00%)
Running Job: [==============================] 30/30 (100%) -> Finished


========================================================================
 Job: stored_jobfile_memory.txt [Built-in]
========================================================================
 Stressor    | Stage | Bogo Ops     | Time(s)  | Metric
------------------------------------------------------------------------
 memcpy      | 1     | 10000        | 16.93    | N/A
 stream      | 1     | 52000        | 0.39     | 24246.60 MB/sec, 1271.22 Mflop/sec
 vm          | 1     | 42000384     | 1.14     | N/A
 malloc      | 1     | 50000        | 0.45     | N/A
 memthrash   | 1     | 2300         | 10.66    | N/A
 ptr-chase   | 1     | 86000640     | 1.13     | N/A
 memcpy      | 2     | 20000        | 15.91    | N/A
 stream      | 2     | 41000        | 0.34     | 44818.94 MB/sec, 2349.80 Mflop/sec
 vm          | 2     | 75000320     | 1.00     | N/A
 malloc      | 2     | 100000       | 0.46     | N/A
 memthrash   | 2     | 4400         | 10.72    | N/A
 ptr-chase   | 2     | 124000256    | 1.66     | N/A
 memcpy      | 3     | 30000        | 15.97    | N/A
 stream      | 3     | 27000        | 0.30     | 66617.10 MB/sec, 3492.65 Mflop/sec
 vm          | 3     | 78000128     | 0.98     | N/A
 malloc      | 3     | 150000       | 0.46     | N/A
 memthrash   | 3     | 5400         | 9.84     | N/A
 ptr-chase   | 3     | 82501632     | 1.80     | N/A
 memcpy      | 4     | 40000        | 15.84    | N/A
 stream      | 4     | 10800        | 0.23     | 78677.65 MB/sec, 4124.97 Mflop/sec
 vm          | 4     | 75000320     | 1.05     | N/A
 malloc      | 4     | 200000       | 0.46     | N/A
 memthrash   | 4     | 4400         | 6.50     | N/A
 ptr-chase   | 4     | 80003072     | 0.81     | N/A
 memcpy      | 5     | 50000        | 14.90    | N/A
 stream      | 5     | 4400         | 0.44     | 29140.51 MB/sec, 1527.80 Mflop/sec
 vm          | 5     | 87002112     | 0.79     | N/A
 malloc      | 5     | 250000       | 0.46     | N/A
 memthrash   | 5     | 4500         | 5.32     | N/A
 ptr-chase   | 5     | 130001920    | 0.67     | N/A
------------------------------------------------------------------------
Total Run Time: 2m 17s
Initializing Job: stored_jobfile_file.txt...
Running Job: [>                             ] 0/40 (0%) -> hdd
rtos_stress: info: [hdd-0] testing file '0.tmp', size 64 KB
Running Job: [>                             ] 1/40 (2%) -> open
rtos_stress: debug: open-max set to 4
rtos_stress: info: [open] 5 open methods active
rtos_stress: info: [open-0] opening up to 4 files per round (base dir: "")
Running Job: [=>                            ] 2/40 (5%) -> copy-file
rtos_stress: debug: copy-file-bytes set to 16384 bytes
rtos_stress: info: [copy-file-0] file size 16384 bytes, chunk 4096, verify every 4 ops
rtos_stress: info: [copy-file-0] fsync available
rtos_stress: info: [copy-file-0] pre-allocating 16384 bytes...
rtos_stress: info: [copy-file-0] pre-alloc done, starting copy loop
Running Job: [==>                           ] 3/40 (7%) -> unlink
rtos_stress: info: [unlink-0] starting with 32 files
rtos_stress: SUMMARY: [unlink-0] fd limit hit 0 times, final limit=32/32
Running Job: [===>                          ] 4/40 (10%) -> fstat
rtos_stress: debug: fstat-files set to 2
rtos_stress: info: [fstat-0] created 2/2 files
Running Job: [===>                          ] 5/40 (12%) -> dentry
rtos_stress: debug: dentries set to 2
rtos_stress: info: [dentry-0] using 2 dentries in stress_dentry_0_dir
Running Job: [====>                         ] 6/40 (15%) -> rename
rtos_stress: info: [rename-0] using dir 'R0', file size 16 bytes
Running Job: [=====>                        ] 7/40 (17%) -> pipe
rtos_stress: info: [pipe-0] blocking mode chunk=512 run_id=0
rtos_stress: info: [pipe-0] throughput: 247561.35 KB/sec
Running Job: [======>                       ] 8/40 (20%) -> hdd
rtos_stress: info: [hdd-0] testing file '0.tmp', size 64 KB
Running Job: [======>                       ] 9/40 (22%) -> open
rtos_stress: debug: open-max set to 4
rtos_stress: info: [open-0] opening up to 4 files per round (base dir: "")
rtos_stress: info: [open-1] opening up to 4 files per round (base dir: "")
Running Job: [=======>                      ] 10/40 (25%) -> copy-file
rtos_stress: debug: copy-file-bytes set to 32768 bytes
rtos_stress: info: [copy-file-0] file size 32768 bytes, chunk 4096, verify every 4 ops
rtos_stress: info: [copy-file-1] file size 32768 bytes, chunk 4096, verify every 4 ops
rtos_stress: info: [copy-file-0] fsync available
rtos_stress: info: [copy-file-0] pre-allocating 32768 bytes...
rtos_stress: info: [copy-file-1] fsync available
rtos_stress: info: [copy-file-1] pre-allocating 32768 bytes...
rtos_stress: info: [copy-file-0] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-1] pre-alloc done, starting copy loop
Running Job: [========>                     ] 11/40 (27%) -> unlink
rtos_stress: info: [unlink-0] starting with 32 files
rtos_stress: info: [unlink-1] starting with 32 files
rtos_stress: SUMMARY: [unlink-0] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-1] fd limit hit 0 times, final limit=32/32
Running Job: [=========>                    ] 12/40 (30%) -> fstat
rtos_stress: debug: fstat-files set to 4
rtos_stress: info: [fstat-0] created 4/4 files
Running Job: [=========>                    ] 13/40 (32%) -> dentry
rtos_stress: debug: dentries set to 4
rtos_stress: info: [dentry-0] using 4 dentries in stress_dentry_0_dir
rtos_stress: info: [dentry-1] using 4 dentries in stress_dentry_1_dir
Running Job: [==========>                   ] 14/40 (35%) -> rename
rtos_stress: info: [rename-0] using dir 'R0', file size 16 bytes
rtos_stress: info: [rename-1] using dir 'R1', file size 16 bytes
Running Job: [===========>                  ] 15/40 (37%) -> pipe
rtos_stress: info: [pipe-0] blocking mode chunk=4096 run_id=1
rtos_stress: info: [pipe-1] blocking mode chunk=4096 run_id=2
rtos_stress: info: [pipe-0] throughput: 1015154.32 KB/sec
rtos_stress: info: [pipe-1] throughput: 1094568.40 KB/sec
Running Job: [============>                 ] 16/40 (40%) -> hdd
rtos_stress: info: [hdd-0] testing file '0.tmp', size 64 KB
Running Job: [============>                 ] 17/40 (42%) -> open
rtos_stress: debug: open-max set to 6
rtos_stress: info: [open-0] opening up to 6 files per round (base dir: "")
rtos_stress: info: [open-2] opening up to 6 files per round (base dir: "")
rtos_stress: info: [open-1] opening up to 6 files per round (base dir: "")
Running Job: [=============>                ] 18/40 (45%) -> copy-file
rtos_stress: debug: copy-file-bytes set to 65536 bytes
rtos_stress: info: [copy-file-0] file size 65536 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-1] file size 65536 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-2] file size 65536 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-0] fsync available
rtos_stress: info: [copy-file-0] pre-allocating 65536 bytes...
rtos_stress: info: [copy-file-1] fsync available
rtos_stress: info: [copy-file-1] pre-allocating 65536 bytes...
rtos_stress: info: [copy-file-2] fsync available
rtos_stress: info: [copy-file-2] pre-allocating 65536 bytes...
rtos_stress: info: [copy-file-0] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-1] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-2] pre-alloc done, starting copy loop
Running Job: [==============>               ] 19/40 (47%) -> unlink
rtos_stress: info: [unlink-0] starting with 32 files
rtos_stress: info: [unlink-2] starting with 32 files
rtos_stress: info: [unlink-1] starting with 32 files
rtos_stress: SUMMARY: [unlink-0] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-2] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-1] fd limit hit 0 times, final limit=32/32
Running Job: [===============>              ] 20/40 (50%) -> fstat
rtos_stress: debug: fstat-files set to 6
rtos_stress: info: [fstat-0] created 6/6 files
Running Job: [===============>              ] 21/40 (52%) -> dentry
rtos_stress: debug: dentries set to 4
rtos_stress: info: [dentry-0] using 4 dentries in stress_dentry_0_dir
rtos_stress: info: [dentry-1] using 4 dentries in stress_dentry_1_dir
rtos_stress: info: [dentry-2] using 4 dentries in stress_dentry_2_dir
Running Job: [================>             ] 22/40 (55%) -> rename
rtos_stress: info: [rename-0] using dir 'R0', file size 16 bytes
rtos_stress: info: [rename-1] using dir 'R1', file size 16 bytes
Running Job: [=================>            ] 23/40 (57%) -> pipe
rtos_stress: info: [pipe-0] blocking mode chunk=32768 run_id=3
rtos_stress: info: [pipe-2] blocking mode chunk=32768 run_id=4
rtos_stress: info: [pipe-1] blocking mode chunk=32768 run_id=5
rtos_stress: info: [pipe-2] throughput: 447727.52 KB/sec
rtos_stress: info: [pipe-0] throughput: 694210.90 KB/sec
rtos_stress: info: [pipe-1] throughput: 479809.32 KB/sec
Running Job: [==================>           ] 24/40 (60%) -> hdd
rtos_stress: info: [hdd-0] testing file '0.tmp', size 128 KB
Running Job: [==================>           ] 25/40 (62%) -> open
rtos_stress: debug: open-max set to 8
rtos_stress: info: [open-0] opening up to 8 files per round (base dir: "")
rtos_stress: info: [open-1] opening up to 8 files per round (base dir: "")
rtos_stress: info: [open-2] opening up to 8 files per round (base dir: "")
rtos_stress: info: [open-3] opening up to 8 files per round (base dir: "")
Running Job: [===================>          ] 26/40 (65%) -> copy-file
rtos_stress: debug: copy-file-bytes set to 131072 bytes
rtos_stress: info: [copy-file-0] file size 131072 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-1] file size 131072 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-2] file size 131072 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-3] file size 131072 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-0] fsync available
rtos_stress: info: [copy-file-0] pre-allocating 131072 bytes...
rtos_stress: info: [copy-file-2] fsync available
rtos_stress: info: [copy-file-1] fsync available
rtos_stress: info: [copy-file-3] fsync available
rtos_stress: info: [copy-file-3] pre-allocating 131072 bytes...
rtos_stress: info: [copy-file-1] pre-allocating 131072 bytes...
rtos_stress: info: [copy-file-2] pre-allocating 131072 bytes...
rtos_stress: info: [copy-file-0] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-3] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-1] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-2] pre-alloc done, starting copy loop
Running Job: [====================>         ] 27/40 (67%) -> unlink
rtos_stress: info: [unlink-1] starting with 32 files
rtos_stress: info: [unlink-0] starting with 32 files
rtos_stress: info: [unlink-3] starting with 32 files
rtos_stress: info: [unlink-2] starting with 32 files
rtos_stress: SUMMARY: [unlink-1] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-3] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-2] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-0] fd limit hit 0 times, final limit=32/32
Running Job: [=====================>        ] 28/40 (70%) -> fstat
rtos_stress: debug: fstat-files set to 8
rtos_stress: info: [fstat-0] created 8/8 files
Running Job: [=====================>        ] 29/40 (72%) -> dentry
rtos_stress: debug: dentries set to 8
rtos_stress: info: [dentry-0] using 8 dentries in stress_dentry_0_dir
rtos_stress: info: [dentry-1] using 8 dentries in stress_dentry_1_dir
rtos_stress: info: [dentry-2] using 8 dentries in stress_dentry_2_dir
Running Job: [======================>       ] 30/40 (75%) -> rename
rtos_stress: info: [rename-1] using dir 'R1', file size 16 bytes
rtos_stress: info: [rename-0] using dir 'R0', file size 16 bytes
rtos_stress: info: [rename-2] using dir 'R2', file size 16 bytes
Running Job: [=======================>      ] 31/40 (77%) -> pipe
rtos_stress: info: [pipe-0] blocking mode chunk=32768 run_id=6
rtos_stress: info: [pipe-1] blocking mode chunk=32768 run_id=7
rtos_stress: info: [pipe-2] blocking mode chunk=32768 run_id=8
rtos_stress: info: [pipe-3] blocking mode chunk=32768 run_id=9
rtos_stress: info: [pipe-3] throughput: 342826.64 KB/sec
rtos_stress: info: [pipe-2] throughput: 493516.87 KB/sec
rtos_stress: info: [pipe-0] throughput: 488584.39 KB/sec
rtos_stress: info: [pipe-1] throughput: 394422.72 KB/sec
Running Job: [========================>     ] 32/40 (80%) -> hdd
rtos_stress: info: [hdd-0] testing file '0.tmp', size 128 KB
Running Job: [========================>     ] 33/40 (82%) -> open
rtos_stress: debug: open-max set to 10
rtos_stress: info: [open-0] opening up to 10 files per round (base dir: "")
rtos_stress: info: [open-2] opening up to 10 files per round (base dir: "")
rtos_stress: info: [open-1] opening up to 10 files per round (base dir: "")
rtos_stress: info: [open-3] opening up to 10 files per round (base dir: "")
rtos_stress: info: [open-4] opening up to 10 files per round (base dir: "")
Running Job: [=========================>    ] 34/40 (85%) -> copy-file
rtos_stress: debug: copy-file-bytes set to 262144 bytes
rtos_stress: info: [copy-file-0] file size 262144 bytes, chunk 4096, verify every 16 ops
rtos_stress: info: [copy-file-1] file size 262144 bytes, chunk 4096, verify every 16 ops
rtos_stress: info: [copy-file-2] file size 262144 bytes, chunk 4096, verify every 16 ops
rtos_stress: info: [copy-file-4] file size 262144 bytes, chunk 4096, verify every 16 ops
rtos_stress: info: [copy-file-3] file size 262144 bytes, chunk 4096, verify every 16 ops
rtos_stress: info: [copy-file-0] fsync available
rtos_stress: info: [copy-file-0] pre-allocating 262144 bytes...
rtos_stress: info: [copy-file-4] fsync available
rtos_stress: info: [copy-file-4] pre-allocating 262144 bytes...
rtos_stress: info: [copy-file-3] fsync available
rtos_stress: info: [copy-file-3] pre-allocating 262144 bytes...
rtos_stress: info: [copy-file-1] fsync available
rtos_stress: info: [copy-file-1] pre-allocating 262144 bytes...
rtos_stress: info: [copy-file-2] fsync available
rtos_stress: info: [copy-file-2] pre-allocating 262144 bytes...
rtos_stress: info: [copy-file-0] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-3] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-2] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-4] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-1] pre-alloc done, starting copy loop
Running Job: [==========================>   ] 35/40 (87%) -> unlink
rtos_stress: info: [unlink-2] starting with 32 files
rtos_stress: info: [unlink-0] starting with 32 files
rtos_stress: info: [unlink-4] starting with 32 files
rtos_stress: info: [unlink-1] starting with 32 files
rtos_stress: info: [unlink-3] starting with 32 files
rtos_stress: SUMMARY: [unlink-0] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-1] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-2] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-4] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-3] fd limit hit 0 times, final limit=32/32
Running Job: [===========================>  ] 36/40 (90%) -> fstat
rtos_stress: debug: fstat-files set to 10
rtos_stress: info: [fstat-0] created 10/10 files
Running Job: [===========================>  ] 37/40 (92%) -> dentry
rtos_stress: debug: dentries set to 8
rtos_stress: info: [dentry-0] using 8 dentries in stress_dentry_0_dir
rtos_stress: info: [dentry-1] using 8 dentries in stress_dentry_1_dir
rtos_stress: info: [dentry-2] using 8 dentries in stress_dentry_2_dir
rtos_stress: info: [dentry-3] using 8 dentries in stress_dentry_3_dir
Running Job: [============================> ] 38/40 (95%) -> rename
rtos_stress: info: [rename-0] using dir 'R0', file size 16 bytes
rtos_stress: info: [rename-2] using dir 'R2', file size 16 bytes
rtos_stress: info: [rename-1] using dir 'R1', file size 16 bytes
Running Job: [=============================>] 39/40 (97%) -> pipe
rtos_stress: info: [pipe-0] blocking mode chunk=32768 run_id=10
rtos_stress: info: [pipe-1] blocking mode chunk=32768 run_id=11
rtos_stress: info: [pipe-2] blocking mode chunk=32768 run_id=12
rtos_stress: info: [pipe-3] blocking mode chunk=32768 run_id=13
rtos_stress: info: [pipe-4] blocking mode chunk=32768 run_id=14
rtos_stress: info: [pipe-4] throughput: 621356.06 KB/sec
rtos_stress: info: [pipe-1] throughput: 605435.54 KB/sec
rtos_stress: info: [pipe-2] throughput: 523583.98 KB/sec
rtos_stress: info: [pipe-0] throughput: 622075.14 KB/sec
rtos_stress: info: [pipe-3] throughput: 424301.43 KB/sec
Running Job: [==============================] 40/40 (100%) -> Finished


========================================================================
 Job: stored_jobfile_file.txt [Built-in]
========================================================================
 Stressor    | Stage | Bogo Ops     | Time(s)  | Metric
------------------------------------------------------------------------
 hdd         | 1     | 480          | 6.88     | N/A
 open        | 1     | 2400         | 0.72     | N/A
 copy-file   | 1     | 2400         | 7.47     | N/A
 unlink      | 1     | 2400         | 0.33     | N/A
 fstat       | 1     | 6000         | 0.15     | N/A
 dentry      | 1     | 3000         | 0.91     | N/A
 rename      | 1     | 6000         | 6.23     | N/A
 pipe        | 1     | 49756        | 0.20     | 247561.35 KB/sec
 hdd         | 2     | 240          | 2.08     | N/A
 open        | 2     | 2400         | 0.39     | N/A
 copy-file   | 2     | 2400         | 9.33     | N/A
 unlink      | 2     | 2400         | 0.24     | N/A
 fstat       | 2     | 6000         | 0.15     | N/A
 dentry      | 2     | 3000         | 0.28     | N/A
 rename      | 2     | 6000         | 3.15     | N/A
 pipe        | 2     | 52874        | 0.20     | 2109722.72 KB/sec
 hdd         | 3     | 160          | 1.42     | N/A
 open        | 3     | 2400         | 0.22     | N/A
 copy-file   | 3     | 2400         | 4.21     | N/A
 unlink      | 3     | 2400         | 0.25     | N/A
 fstat       | 3     | 6000         | 0.15     | N/A
 dentry      | 3     | 3000         | 0.20     | N/A
 rename      | 3     | 6000         | 3.16     | N/A
 pipe        | 3     | 5094         | 0.20     | 1621747.75 KB/sec
 hdd         | 4     | 160          | 0.81     | N/A
 open        | 4     | 2400         | 0.16     | N/A
 copy-file   | 4     | 2400         | 8.14     | N/A
 unlink      | 4     | 2400         | 1.19     | N/A
 fstat       | 4     | 6000         | 0.15     | N/A
 dentry      | 4     | 3000         | 0.14     | N/A
 rename      | 4     | 6000         | 2.13     | N/A
 pipe        | 4     | 5413         | 0.20     | 1719350.63 KB/sec
 hdd         | 5     | 160          | 0.92     | N/A
 open        | 5     | 2400         | 0.13     | N/A
 copy-file   | 5     | 2400         | 5.68     | N/A
 unlink      | 5     | 2400         | 0.22     | N/A
 fstat       | 5     | 6000         | 0.15     | N/A
 dentry      | 5     | 3000         | 0.12     | N/A
 rename      | 5     | 6000         | 2.13     | N/A
 pipe        | 5     | 8778         | 0.20     | 2796752.16 KB/sec
------------------------------------------------------------------------
Total Run Time: 1m 11s

[test-stress] Job 'all' completed: 135 stressor runs
=============================================================

>>> Running test-cmd...
Fri Jan  1 12:11:30 UTC 2021
    PID TTY          TIME CMD
    612 pts/0    00:00:00 bash
    975 pts/0    00:07:36 rtos-bench-open
   2226 pts/0    00:00:00 ps
/rtosbench
result_20261023.json                   rtos-bench-openeuler-orangepi-aarch64-vxworks-7wl-fix
rtos-bench-openeuler-orangepi-aarch64  test_cmd_dir
HELLO
=======================================================================
>> Result: Command 'date' finished.
>> Result: Command 'ps' finished.
>> Result: Command 'mkdir test_cmd_dir' finished.
>> Result: Command 'cd .' finished.
>> Result: Command 'pwd' finished.
>> Result: Command 'echo 'HELLO' > ./test_cmd_dir/original.txt' finished.
>> Result: Command 'touch ./test_cmd_dir/original.txt' finished.
>> Result: Command 'cp ./test_cmd_dir/original.txt ./test_cmd_dir/backup.txt' finished.
>> Result: Command 'mv ./test_cmd_dir/backup.txt ./test_cmd_dir/final_target.txt' finished.
>> Result: Command 'ls' finished.
>> Result: Command 'cat ./test_cmd_dir/original.txt' finished.
>> Result: Command 'rm -r ./test_cmd_dir' finished.
=======================================================================
Supported commands: date ps mkdir cd pwd echo touch cp mv ls cat rm
Unsupported commands:
[test-cmd] Result: 12/12 commands supported

>>> Running typical workloads...
  Running workload: stub
    stub: 0.002 ms total, 0.000 ms avg
  Running workload: busywait
    busywait: 41.668 ms total, 4.167 ms avg
  Running workload: fast
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=20.289 ms avg_latency=6763.140 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=15.765 ms avg_latency=5254.917 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=17.141 ms avg_latency=5713.792 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=14.188 ms avg_latency=4729.473 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=15.351 ms avg_latency=5117.028 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=20.262 ms avg_latency=6753.931 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=23.208 ms avg_latency=7735.987 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=22.455 ms avg_latency=7485.015 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=20.601 ms avg_latency=6867.001 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=16.971 ms avg_latency=5657.112 us/img
[POSIX Thread][FAST] Benchmark thread finished.
    fast: 189.603 ms total, 18.960 ms avg
  Running workload: icp
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=84.494 ms avg_latency=84.494 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=90.927 ms avg_latency=90.927 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=78.498 ms avg_latency=78.498 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=64.984 ms avg_latency=64.984 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=195.518 ms avg_latency=195.518 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=203.138 ms avg_latency=203.138 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=73.308 ms avg_latency=73.308 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=195.470 ms avg_latency=195.470 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=203.043 ms avg_latency=203.043 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=194.555 ms avg_latency=194.555 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
    icp: 1398.399 ms total, 139.840 ms avg
  Running workload: modbus
[POSIX Thread][MODBUS][Server] Creating benchmark thread...
[POSIX Thread][MODBUS][Client] Creating benchmark thread...
[Client] Waiting for server...
[Client] Connecting to 127.0.0.1:5020...
[Client] Connected!
[Client] Modbus client created

=== R/W Benchmark Result ===
Time:       8.607 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.46
[MODBUS] samples=4 total_time=8607.176 ms avg_latency=2151.794 ms/request
[Client] Finished
[POSIX Thread][MODBUS][Client] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Creating benchmark thread...
[POSIX Thread][MODBUS][Client] Creating benchmark thread...
[Client] Waiting for server...
[Client] Connecting to 127.0.0.1:5020...
[Client] Connected!
[Client] Modbus client created

=== R/W Benchmark Result ===
Time:       8.598 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.47
[MODBUS] samples=4 total_time=8598.079 ms avg_latency=2149.520 ms/request
[Client] Finished
[POSIX Thread][MODBUS][Client] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Creating benchmark thread...
[POSIX Thread][MODBUS][Client] Creating benchmark thread...
[Client] Waiting for server...
[Client] Connecting to 127.0.0.1:5020...
[Client] Connected!
[Client] Modbus client created

=== R/W Benchmark Result ===
Time:       8.599 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.47
[MODBUS] samples=4 total_time=8598.636 ms avg_latency=2149.659 ms/request
[Client] Finished
[POSIX Thread][MODBUS][Client] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Creating benchmark thread...
[POSIX Thread][MODBUS][Client] Creating benchmark thread...
[Client] Waiting for server...
[Client] Connecting to 127.0.0.1:5020...
[Client] Connected!
[Client] Modbus client created

=== R/W Benchmark Result ===
Time:       8.598 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.47
[MODBUS] samples=4 total_time=8598.110 ms avg_latency=2149.527 ms/request
[Client] Finished
[POSIX Thread][MODBUS][Client] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Creating benchmark thread...
[POSIX Thread][MODBUS][Client] Creating benchmark thread...
[Client] Waiting for server...
[Client] Connecting to 127.0.0.1:5020...
[Client] Connected!
[Client] Modbus client created

=== R/W Benchmark Result ===
Time:       8.599 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.47
[MODBUS] samples=4 total_time=8598.704 ms avg_latency=2149.676 ms/request
[Client] Finished
[POSIX Thread][MODBUS][Client] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Creating benchmark thread...
[POSIX Thread][MODBUS][Client] Creating benchmark thread...
[Client] Waiting for server...
[Client] Connecting to 127.0.0.1:5020...
[Client] Connected!
[Client] Modbus client created

=== R/W Benchmark Result ===
Time:       8.598 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.47
[MODBUS] samples=4 total_time=8598.082 ms avg_latency=2149.521 ms/request
[Client] Finished
[POSIX Thread][MODBUS][Client] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Creating benchmark thread...
[POSIX Thread][MODBUS][Client] Creating benchmark thread...
[Client] Waiting for server...
[Client] Connecting to 127.0.0.1:5020...
[Client] Connected!
[Client] Modbus client created

=== R/W Benchmark Result ===
Time:       8.599 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.47
[MODBUS] samples=4 total_time=8598.763 ms avg_latency=2149.691 ms/request
[Client] Finished
[POSIX Thread][MODBUS][Client] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Creating benchmark thread...
[POSIX Thread][MODBUS][Client] Creating benchmark thread...
[Client] Waiting for server...
[Client] Connecting to 127.0.0.1:5020...
[Client] Connected!
[Client] Modbus client created

=== R/W Benchmark Result ===
Time:       8.598 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.47
[MODBUS] samples=4 total_time=8598.087 ms avg_latency=2149.522 ms/request
[Client] Finished
[POSIX Thread][MODBUS][Client] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Creating benchmark thread...
[POSIX Thread][MODBUS][Client] Creating benchmark thread...
[Client] Waiting for server...
[Client] Connecting to 127.0.0.1:5020...
[Client] Connected!
[Client] Modbus client created

=== R/W Benchmark Result ===
Time:       8.599 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.47
[MODBUS] samples=4 total_time=8598.584 ms avg_latency=2149.646 ms/request
[Client] Finished
[POSIX Thread][MODBUS][Client] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Creating benchmark thread...
[POSIX Thread][MODBUS][Client] Creating benchmark thread...
[Client] Waiting for server...
[Client] Connecting to 127.0.0.1:5020...
[Client] Connected!
[Client] Modbus client created

=== R/W Benchmark Result ===
Time:       8.599 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.47
[MODBUS] samples=4 total_time=8598.640 ms avg_latency=2149.660 ms/request
[Client] Finished
[POSIX Thread][MODBUS][Client] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Benchmark thread finished.
    modbus: 96012.763 ms total, 9601.276 ms avg
  Running workload: mqtt
[POSIX Thread][MQTT] Creating benchmark thread...
[MQTT] Thread Started...
[MQTT] Connecting to tcp://44.232.241.40:1883 (Raw TCP Mode)...
[MQTT] Socket Created
[MQTT] Error: socket error
[POSIX Thread][MQTT] Benchmark thread finished.
[POSIX Thread][MQTT] Creating benchmark thread...
[MQTT] Thread Started...
[MQTT] Connecting to tcp://44.232.241.40:1883 (Raw TCP Mode)...
[MQTT] Socket Created
[MQTT] Connection timeout after 3000.000 ms
[POSIX Thread][MQTT] Benchmark thread finished.
[POSIX Thread][MQTT] Creating benchmark thread...
[MQTT] Thread Started...
[MQTT] Connecting to tcp://44.232.241.40:1883 (Raw TCP Mode)...
[MQTT] Socket Created
[MQTT] Error: socket error
[POSIX Thread][MQTT] Benchmark thread finished.
[POSIX Thread][MQTT] Creating benchmark thread...
[MQTT] Thread Started...
[MQTT] Connecting to tcp://44.232.241.40:1883 (Raw TCP Mode)...
[MQTT] Socket Created
[MQTT] Connection timeout after 3000.000 ms
[POSIX Thread][MQTT] Benchmark thread finished.
[POSIX Thread][MQTT] Creating benchmark thread...
[MQTT] Thread Started...
[MQTT] Connecting to tcp://44.232.241.40:1883 (Raw TCP Mode)...
[MQTT] Socket Created
[MQTT] Error: socket error
[POSIX Thread][MQTT] Benchmark thread finished.
[POSIX Thread][MQTT] Creating benchmark thread...
[MQTT] Thread Started...
[MQTT] Connecting to tcp://44.232.241.40:1883 (Raw TCP Mode)...
[MQTT] Socket Created
[MQTT] Connection timeout after 3000.000 ms
[POSIX Thread][MQTT] Benchmark thread finished.
[POSIX Thread][MQTT] Creating benchmark thread...
[MQTT] Thread Started...
[MQTT] Connecting to tcp://44.232.241.40:1883 (Raw TCP Mode)...
[MQTT] Socket Created
[MQTT] Error: socket error
[POSIX Thread][MQTT] Benchmark thread finished.
[POSIX Thread][MQTT] Creating benchmark thread...
[MQTT] Thread Started...
[MQTT] Connecting to tcp://44.232.241.40:1883 (Raw TCP Mode)...
[MQTT] Socket Created
[MQTT] Connection timeout after 3000.000 ms
[POSIX Thread][MQTT] Benchmark thread finished.
[POSIX Thread][MQTT] Creating benchmark thread...
[MQTT] Thread Started...
[MQTT] Connecting to tcp://44.232.241.40:1883 (Raw TCP Mode)...
[MQTT] Socket Created
[MQTT] Error: socket error
[POSIX Thread][MQTT] Benchmark thread finished.
[POSIX Thread][MQTT] Creating benchmark thread...
[MQTT] Thread Started...
[MQTT] Connecting to tcp://44.232.241.40:1883 (Raw TCP Mode)...
[MQTT] Socket Created
[MQTT] Connection timeout after 3000.000 ms
[POSIX Thread][MQTT] Benchmark thread finished.
    mqtt: 16022.973 ms total, 1602.297 ms avg
  Running workload: pid
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=59.457 ms avg_latency=0.059 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=54.576 ms avg_latency=0.055 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=54.587 ms avg_latency=0.055 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=54.608 ms avg_latency=0.055 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=54.579 ms avg_latency=0.055 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=22.767 ms avg_latency=0.023 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=21.469 ms avg_latency=0.021 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=35.511 ms avg_latency=0.036 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=25.175 ms avg_latency=0.025 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=24.059 ms avg_latency=0.024 us/op
[POSIX Thread][PID] Benchmark thread finished.
    pid: 409.209 ms total, 40.921 ms avg
  Running workload: cusum
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=4.039 ms avg_latency=0.007 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.525 ms avg_latency=0.031 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.540 ms avg_latency=0.031 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.502 ms avg_latency=0.031 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.528 ms avg_latency=0.031 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.507 ms avg_latency=0.031 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.533 ms avg_latency=0.031 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.503 ms avg_latency=0.031 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.526 ms avg_latency=0.031 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.498 ms avg_latency=0.031 us/sample
    cusum: 172.105 ms total, 17.211 ms avg
  Running workload: ewma
[EWMA] loops=100 stream_len_per_loop=5000 total_time=24.375 ms avg_latency=0.049 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=66.840 ms avg_latency=0.134 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=66.773 ms avg_latency=0.134 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=66.763 ms avg_latency=0.134 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=66.760 ms avg_latency=0.134 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=66.774 ms avg_latency=0.134 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=66.778 ms avg_latency=0.134 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=66.831 ms avg_latency=0.134 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=66.762 ms avg_latency=0.134 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=66.779 ms avg_latency=0.134 us/sample
    ewma: 627.107 ms total, 62.711 ms avg

=============================================================
[RTOS-Bench] Results saved to: result_20261023_7workloads.json
=============================================================