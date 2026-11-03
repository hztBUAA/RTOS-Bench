[root@openEuler-phytium rtosbench]# ./rtos-bench-openeuler-feiteng-aarch64-vxworks-7wl-fix test-all --multicore -o result_20261023_7workloads.json

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
信号量获取 | 0.020      | 4.360      |            |
信号量释放 | 0.040      |            | 1.480      | 4.820
消息发送   | 2.100      | 6.480      | 2.480      | 6.040
消息接收   | 2.180      | 4.860      | 2.740      | 5.600
互斥锁获取 | 0.040      | 4.080      |            |
互斥锁释放 | 0.040      |            | 1.520      | 4.620
内存块申请 | 0.040      |            |            |
内存块释放 | 0.060      |            |            |
上下文切换延迟    AVG: 1.867
中断软件延迟    MIN: 0.160  MAX: 0.360  AVG: 0.184
系统调用延迟    MIN: 9.020  MAX: 26.200  AVG: 9.430

多核存取性能:
并发度     | 1          | 2          | 4          | 8
----------------------------------------------------------------------
rd         | 1.169      | 2.333      | 2.336      | 2.341
wr         | 0.272      | 0.347      | 0.341      | 0.344
cp         | 0.287      | 0.431      | 0.427      | 0.428
frd        | 8.962      | 17.892     | 17.886     | 16.244
fwr        | 12.538     | 11.292     | 12.560     | 11.733
fcp        | 4.193      | 6.388      | 6.397      | 6.119
memset     | 11.497     | 13.343     | 11.846     | 11.706
memcpy     | 4.560      | 6.249      | 6.341      | 6.550
多核系统服务:
并发度     | 1          | 2          | 4          | 8
任务间通信 | 0.359      | 0.583      | 0.405      | 0.354
创建与删除 | 128.745    | 102.083    | 132.458    | 115.220   （单位为us）
任务在同核与异核上通信比较:
同核通信: 0.344
异核通信: 0.283
（除特殊说明外，上述单位均为GB/s）

=============================================================
[test-realtime] Benchmark completed with code: 0
=============================================================

>>> Running test-schedule (cycles=3 util=30-100 step 10)...
[test-schedule] build Oct 23 2026 23:06:07
[test-schedule] Found 7 industrial workloads (excluded 2 utility workloads)
=============================================================
[Phase 1] Measuring WCET for 7 workloads...
=============================================================
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=13.036 ms avg_latency=4345.207 us/img
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=12.691 ms avg_latency=4230.333 us/img
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=12.986 ms avg_latency=4328.800 us/img
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=12.955 ms avg_latency=4318.453 us/img
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=12.921 ms avg_latency=4306.953 us/img
  [fast]: WCET = 13.126 ms (5 iters)

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=351.065 ms avg_latency=351.065 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=344.319 ms avg_latency=344.319 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=344.809 ms avg_latency=344.809 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=345.154 ms avg_latency=345.154 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=344.477 ms avg_latency=344.477 ms/run
  [icp]: WCET = 353.336 ms (5 iters)
[SCHED-MODBUS] offline frames=6144 total_time=0.750 ms errors=0
[SCHED-MODBUS] offline frames=6144 total_time=0.756 ms errors=0
[SCHED-MODBUS] offline frames=6144 total_time=0.749 ms errors=0
[SCHED-MODBUS] offline frames=6144 total_time=0.777 ms errors=0
[SCHED-MODBUS] offline frames=6144 total_time=0.749 ms errors=0
  [modbus]: WCET = 0.786 ms (5 iters)
[SCHED-MQTT] offline packets=512 bytes=45115 checksum=3283016638 total_time=39.565 ms
[SCHED-MQTT] offline packets=512 bytes=45115 checksum=3283016638 total_time=39.509 ms
[SCHED-MQTT] offline packets=512 bytes=45115 checksum=3283016638 total_time=39.530 ms
[SCHED-MQTT] offline packets=512 bytes=45115 checksum=3283016638 total_time=39.511 ms
[SCHED-MQTT] offline packets=512 bytes=45115 checksum=3283016638 total_time=39.501 ms
  [mqtt]: WCET = 39.712 ms (5 iters)
--- PID Simulation Start ---
[PID] samples=1000000 total_time=23.991 ms avg_latency=0.024 us/op
--- PID Simulation Start ---
[PID] samples=1000000 total_time=24.000 ms avg_latency=0.024 us/op
--- PID Simulation Start ---
[PID] samples=1000000 total_time=23.997 ms avg_latency=0.024 us/op
--- PID Simulation Start ---
[PID] samples=1000000 total_time=24.003 ms avg_latency=0.024 us/op
--- PID Simulation Start ---
[PID] samples=1000000 total_time=24.003 ms avg_latency=0.024 us/op
  [pid]: WCET = 24.015 ms (5 iters)
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.743 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.710 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.672 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.709 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.673 ms avg_latency=0.043 us/sample
  [cusum]: WCET = 25.774 ms (5 iters)
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.510 ms avg_latency=0.069 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.424 ms avg_latency=0.069 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.499 ms avg_latency=0.069 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.478 ms avg_latency=0.069 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.395 ms avg_latency=0.069 us/sample
  [ewma]: WCET = 34.522 ms (5 iters)
[Phase 1] 7 workloads measured

=============================================================
[Phase 2] Running Task Sets (U: 30% - 100%, step 10%)
=============================================================

>>> Utilization Gradient: 30% <<<
[test-schedule] uunifast sum=0.3000 target=0.3000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 353.336    | 11.80    | 2994.006
mqtt         | 39.712     | 9.71     | 408.805
ewma         | 34.522     | 4.17     | 828.636
cusum        | 25.774     | 3.06     | 841.098
pid          | 24.015     | 0.92     | 2603.150
fast         | 13.126     | 0.26     | 5078.829
modbus       | 0.786      | 0.07     | 10000.000
Running task set for 3 cycles...
[test-schedule] creating task thread 1/7 (icp)...
[test-schedule] creating task thread 2/7 (mqtt)...
[test-schedule] creating task thread 3/7 (ewma)...
[test-schedule] creating task thread 4/7 (cusum)...
[test-schedule] creating task thread 5/7 (pid)...
[test-schedule] creating task thread 6/7 (fast)...
[test-schedule] creating task thread 7/7 (modbus)...
[test-schedule] all 7 task threads created; entering watchdog
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.441 ms avg_latency=0.069 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.983 ms avg_latency=0.043 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.502 ms avg_latency=0.069 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.687 ms avg_latency=0.043 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.476 ms avg_latency=0.069 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.669 ms avg_latency=0.043 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.869 ms avg_latency=0.076 us/op

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=358.332 ms avg_latency=358.332 ms/run
[test-schedule] heartbeat (still running gradient):
    icp      jobs=1/3  since_progress=1s  stall_window=8s
    pid      jobs=1/3  since_progress=2s  stall_window=7s
    fast     jobs=0/3  since_progress=5s  stall_window=15s
    modbus   jobs=0/3  since_progress=5s  stall_window=30s
[FAST] samples=3 total_time=19.429 ms avg_latency=6476.480 us/img
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.865 ms avg_latency=0.076 us/op

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=352.554 ms avg_latency=352.554 ms/run
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.845 ms avg_latency=0.076 us/op

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=353.333 ms avg_latency=353.333 ms/run
[test-schedule] heartbeat (still running gradient):
    fast     jobs=1/3  since_progress=4s  stall_window=15s
    modbus   jobs=1/3  since_progress=0s  stall_window=30s
[FAST] samples=3 total_time=19.410 ms avg_latency=6469.860 us/img
[test-schedule] heartbeat (still running gradient):
    fast     jobs=2/3  since_progress=4s  stall_window=15s
    modbus   jobs=1/3  since_progress=5s  stall_window=30s
[FAST] samples=3 total_time=19.381 ms avg_latency=6460.207 us/img
[test-schedule] heartbeat (still running gradient):
    modbus   jobs=2/3  since_progress=0s  stall_window=30s
[test-schedule] heartbeat (still running gradient):
    modbus   jobs=2/3  since_progress=5s  stall_window=30s
[test-schedule] heartbeat (still running gradient):
Gradient 30% complete: MR = 0.0000 (0/21)

>>> Utilization Gradient: 40% <<<
[test-schedule] uunifast sum=0.4000 target=0.4000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 353.336    | 12.17    | 2903.858
mqtt         | 39.712     | 11.30    | 351.522
ewma         | 34.522     | 7.22     | 478.418
cusum        | 25.774     | 3.70     | 696.372
pid          | 24.015     | 2.67     | 900.304
fast         | 13.126     | 2.60     | 504.482
modbus       | 0.786      | 0.35     | 225.364
Running task set for 3 cycles...
[test-schedule] creating task thread 1/7 (icp)...
[test-schedule] creating task thread 2/7 (mqtt)...
[test-schedule] creating task thread 3/7 (ewma)...
[test-schedule] creating task thread 4/7 (cusum)...
[test-schedule] creating task thread 5/7 (pid)...
[test-schedule] creating task thread 6/7 (fast)...
[test-schedule] creating task thread 7/7 (modbus)...
[test-schedule] all 7 task threads created; entering watchdog
[FAST] samples=3 total_time=19.759 ms avg_latency=6586.173 us/img
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.018 ms avg_latency=0.188 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.672 ms avg_latency=0.043 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.871 ms avg_latency=0.076 us/op
[FAST] samples=3 total_time=19.920 ms avg_latency=6640.160 us/img
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.385 ms avg_latency=0.189 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.669 ms avg_latency=0.043 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=93.992 ms avg_latency=0.188 us/sample
[FAST] samples=3 total_time=19.744 ms avg_latency=6581.387 us/img
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.915 ms avg_latency=0.076 us/op
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.674 ms avg_latency=0.043 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.884 ms avg_latency=0.076 us/op

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=357.159 ms avg_latency=357.159 ms/run
[test-schedule] heartbeat (still running gradient):
    icp      jobs=1/3  since_progress=1s  stall_window=8s

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=351.782 ms avg_latency=351.782 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=352.316 ms avg_latency=352.316 ms/run
Gradient 40% complete: MR = 0.0000 (0/21)

>>> Utilization Gradient: 50% <<<
[test-schedule] uunifast sum=0.5000 target=0.5000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 353.336    | 15.18    | 2328.361
mqtt         | 39.712     | 11.71    | 339.263
ewma         | 34.522     | 10.22    | 337.645
cusum        | 25.774     | 6.08     | 423.652
pid          | 24.015     | 2.89     | 831.711
fast         | 13.126     | 2.73     | 480.049
modbus       | 0.786      | 1.19     | 66.055
Running task set for 3 cycles...
[test-schedule] creating task thread 1/7 (icp)...
[test-schedule] creating task thread 2/7 (mqtt)...
[test-schedule] creating task thread 3/7 (ewma)...
[test-schedule] creating task thread 4/7 (cusum)...
[test-schedule] creating task thread 5/7 (pid)...
[test-schedule] creating task thread 6/7 (fast)...
[test-schedule] creating task thread 7/7 (modbus)...
[test-schedule] all 7 task threads created; entering watchdog
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.014 ms avg_latency=0.188 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.837 ms avg_latency=0.043 us/sample
[FAST] samples=3 total_time=19.397 ms avg_latency=6465.667 us/img
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.085 ms avg_latency=0.188 us/sample
--- PID Simulation Start ---
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=26.107 ms avg_latency=0.044 us/sample
[PID] samples=1000000 total_time=75.901 ms avg_latency=0.076 us/op
[FAST] samples=3 total_time=19.456 ms avg_latency=6485.213 us/img
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.125 ms avg_latency=0.188 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.687 ms avg_latency=0.043 us/sample
[FAST] samples=3 total_time=19.403 ms avg_latency=6467.807 us/img
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.847 ms avg_latency=0.076 us/op

[ICP] Running ICP (point-to-plane)
--- PID Simulation Start ---
[PID] samples=1000000 total_time=78.709 ms avg_latency=0.079 us/op
[ICP] samples=1 total_time=356.351 ms avg_latency=356.351 ms/run

[ICP] Running ICP (point-to-plane)
[test-schedule] heartbeat (still running gradient):
    icp      jobs=1/3  since_progress=2s  stall_window=6s
[ICP] samples=1 total_time=352.228 ms avg_latency=352.228 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=352.999 ms avg_latency=352.999 ms/run
Gradient 50% complete: MR = 0.0000 (0/21)

>>> Utilization Gradient: 60% <<<
[test-schedule] uunifast sum=0.6000 target=0.6000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 353.336    | 18.17    | 1944.139
mqtt         | 39.712     | 17.22    | 230.636
ewma         | 34.522     | 8.96     | 385.492
cusum        | 25.774     | 8.27     | 311.660
pid          | 24.015     | 3.94     | 610.060
fast         | 13.126     | 2.43     | 540.832
modbus       | 0.786      | 1.02     | 77.151
Running task set for 3 cycles...
[test-schedule] creating task thread 1/7 (icp)...
[test-schedule] creating task thread 2/7 (mqtt)...
[test-schedule] creating task thread 3/7 (ewma)...
[test-schedule] creating task thread 4/7 (cusum)...
[test-schedule] creating task thread 5/7 (pid)...
[test-schedule] creating task thread 6/7 (fast)...
[test-schedule] creating task thread 7/7 (modbus)...
[test-schedule] all 7 task threads created; entering watchdog
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.732 ms avg_latency=0.043 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.481 ms avg_latency=0.069 us/sample
[FAST] samples=3 total_time=19.412 ms avg_latency=6470.527 us/img
--- PID Simulation Start ---
[PID] samples=1000000 total_time=23.994 ms avg_latency=0.024 us/op
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.684 ms avg_latency=0.043 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=93.982 ms avg_latency=0.188 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.671 ms avg_latency=0.043 us/sample
[FAST] samples=3 total_time=19.412 ms avg_latency=6470.773 us/img
--- PID Simulation Start ---
[PID] samples=1000000 total_time=24.002 ms avg_latency=0.024 us/op
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.209 ms avg_latency=0.188 us/sample
[FAST] samples=3 total_time=19.437 ms avg_latency=6478.840 us/img
--- PID Simulation Start ---
[PID] samples=1000000 total_time=24.018 ms avg_latency=0.024 us/op

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=149.243 ms avg_latency=149.243 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=147.627 ms avg_latency=147.627 ms/run
[test-schedule] heartbeat (still running gradient):
    icp      jobs=2/3  since_progress=0s  stall_window=5s

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=147.621 ms avg_latency=147.621 ms/run
Gradient 60% complete: MR = 0.0000 (0/21)

>>> Utilization Gradient: 70% <<<
[test-schedule] uunifast sum=0.7000 target=0.7000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 353.336    | 17.00    | 2079.005
mqtt         | 39.712     | 16.70    | 237.796
ewma         | 34.522     | 14.58    | 236.844
cusum        | 25.774     | 8.07     | 319.451
pid          | 24.015     | 7.89     | 304.433
fast         | 13.126     | 3.57     | 367.809
modbus       | 0.786      | 2.20     | 35.660
Running task set for 3 cycles...
[test-schedule] creating task thread 1/7 (icp)...
[test-schedule] creating task thread 2/7 (mqtt)...
[test-schedule] creating task thread 3/7 (ewma)...
[test-schedule] creating task thread 4/7 (cusum)...
[test-schedule] creating task thread 5/7 (pid)...
[test-schedule] creating task thread 6/7 (fast)...
[test-schedule] creating task thread 7/7 (modbus)...
[test-schedule] all 7 task threads created; entering watchdog
--- PID Simulation Start ---
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.065 ms avg_latency=0.188 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=26.982 ms avg_latency=0.045 us/sample
[FAST] samples=3 total_time=19.889 ms avg_latency=6629.733 us/img
[PID] samples=1000000 total_time=87.384 ms avg_latency=0.087 us/op
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.039 ms avg_latency=0.188 us/sample
--- PID Simulation Start ---
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=26.105 ms avg_latency=0.044 us/sample
[PID] samples=1000000 total_time=75.799 ms avg_latency=0.076 us/op
[FAST] samples=3 total_time=19.534 ms avg_latency=6511.480 us/img
[EWMA] loops=100 stream_len_per_loop=5000 total_time=60.078 ms avg_latency=0.120 us/sample
--- PID Simulation Start ---
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=26.115 ms avg_latency=0.044 us/sample
[PID] samples=1000000 total_time=75.892 ms avg_latency=0.076 us/op
[FAST] samples=3 total_time=19.470 ms avg_latency=6489.900 us/img

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=355.257 ms avg_latency=355.257 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=351.561 ms avg_latency=351.561 ms/run
[test-schedule] heartbeat (still running gradient):
    icp      jobs=2/3  since_progress=0s  stall_window=6s

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=352.078 ms avg_latency=352.078 ms/run
Gradient 70% complete: MR = 0.0000 (0/21)

>>> Utilization Gradient: 80% <<<
[test-schedule] uunifast sum=0.8000 target=0.8000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 353.336    | 26.14    | 1351.929
mqtt         | 39.712     | 18.98    | 209.196
ewma         | 34.522     | 16.50    | 209.200
cusum        | 25.774     | 11.46    | 224.921
pid          | 24.015     | 2.75     | 872.066
fast         | 13.126     | 2.71     | 484.519
modbus       | 0.786      | 1.46     | 53.918
Running task set for 3 cycles...
[test-schedule] creating task thread 1/7 (icp)...
[test-schedule] creating task thread 2/7 (mqtt)...
[test-schedule] creating task thread 3/7 (ewma)...
[test-schedule] creating task thread 4/7 (cusum)...
[test-schedule] creating task thread 5/7 (pid)...
[test-schedule] creating task thread 6/7 (fast)...
[test-schedule] creating task thread 7/7 (modbus)...
[test-schedule] all 7 task threads created; entering watchdog
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=49.530 ms avg_latency=0.083 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=95.156 ms avg_latency=0.190 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=45.057 ms avg_latency=0.075 us/sample
[FAST] samples=3 total_time=20.345 ms avg_latency=6781.540 us/img
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.024 ms avg_latency=0.188 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=26.180 ms avg_latency=0.044 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.007 ms avg_latency=0.188 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.941 ms avg_latency=0.076 us/op
[FAST] samples=3 total_time=19.446 ms avg_latency=6481.893 us/img

[ICP] Running ICP (point-to-plane)
[FAST] samples=3 total_time=20.358 ms avg_latency=6785.973 us/img
[ICP] samples=1 total_time=356.067 ms avg_latency=356.067 ms/run
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.907 ms avg_latency=0.076 us/op
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.913 ms avg_latency=0.076 us/op

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=352.294 ms avg_latency=352.294 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=352.184 ms avg_latency=352.184 ms/run
Gradient 80% complete: MR = 0.0000 (0/21)

>>> Utilization Gradient: 90% <<<
[test-schedule] uunifast sum=0.9000 target=0.9000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 353.336    | 28.28    | 1249.478
mqtt         | 39.712     | 27.63    | 143.741
ewma         | 34.522     | 20.26    | 170.376
cusum        | 25.774     | 7.78     | 331.440
pid          | 24.015     | 3.26     | 736.656
fast         | 13.126     | 2.27     | 577.915
modbus       | 0.786      | 0.52     | 149.960
Running task set for 3 cycles...
[test-schedule] creating task thread 1/7 (icp)...
[test-schedule] creating task thread 2/7 (mqtt)...
[test-schedule] creating task thread 3/7 (ewma)...
[test-schedule] creating task thread 4/7 (cusum)...
[test-schedule] creating task thread 5/7 (pid)...
[test-schedule] creating task thread 6/7 (fast)...
[test-schedule] creating task thread 7/7 (modbus)...
[test-schedule] all 7 task threads created; entering watchdog
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.260 ms avg_latency=0.189 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.683 ms avg_latency=0.043 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.380 ms avg_latency=0.189 us/sample
[FAST] samples=3 total_time=19.883 ms avg_latency=6627.747 us/img
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.014 ms avg_latency=0.188 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.680 ms avg_latency=0.043 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.822 ms avg_latency=0.076 us/op
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.674 ms avg_latency=0.043 us/sample
[FAST] samples=3 total_time=19.433 ms avg_latency=6477.527 us/img

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=148.944 ms avg_latency=148.944 ms/run
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.755 ms avg_latency=0.076 us/op
[FAST] samples=3 total_time=19.439 ms avg_latency=6479.827 us/img
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.850 ms avg_latency=0.076 us/op

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=147.689 ms avg_latency=147.689 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=147.466 ms avg_latency=147.466 ms/run
Gradient 90% complete: MR = 0.0000 (0/21)

>>> Utilization Gradient: 100% <<<
[test-schedule] uunifast sum=1.0000 target=1.0000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 353.336    | 46.07    | 766.931
mqtt         | 39.712     | 21.06    | 188.551
ewma         | 34.522     | 12.35    | 279.476
cusum        | 25.774     | 11.92    | 216.313
pid          | 24.015     | 5.31     | 452.476
fast         | 13.126     | 2.64     | 497.302
modbus       | 0.786      | 0.65     | 120.404
Running task set for 3 cycles...
[test-schedule] creating task thread 1/7 (icp)...
[test-schedule] creating task thread 2/7 (mqtt)...
[test-schedule] creating task thread 3/7 (ewma)...
[test-schedule] creating task thread 4/7 (cusum)...
[test-schedule] creating task thread 5/7 (pid)...
[test-schedule] creating task thread 6/7 (fast)...
[test-schedule] creating task thread 7/7 (modbus)...
[test-schedule] all 7 task threads created; entering watchdog
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.901 ms avg_latency=0.043 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.074 ms avg_latency=0.188 us/sample
--- PID Simulation Start ---
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.669 ms avg_latency=0.043 us/sample
[FAST] samples=3 total_time=19.915 ms avg_latency=6638.280 us/img
[PID] samples=1000000 total_time=76.052 ms avg_latency=0.076 us/op
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.067 ms avg_latency=0.188 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.758 ms avg_latency=0.043 us/sample

[ICP] Running ICP (point-to-plane)
--- PID Simulation Start ---
[EWMA] loops=100 stream_len_per_loop=5000 total_time=80.563 ms avg_latency=0.161 us/sample
[PID] samples=1000000 total_time=78.521 ms avg_latency=0.079 us/op
[FAST] samples=3 total_time=20.906 ms avg_latency=6968.807 us/img
[ICP] samples=1 total_time=357.968 ms avg_latency=357.968 ms/run
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.861 ms avg_latency=0.076 us/op
[FAST] samples=3 total_time=19.482 ms avg_latency=6493.973 us/img

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=350.709 ms avg_latency=350.709 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=352.006 ms avg_latency=352.006 ms/run
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
rtos_stress: info: [str-1] using 'all' methods
rtos_stress: info: [str-0] using 'all' methods
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
rtos_stress: info: [matrix-1] size=64, method=all
rtos_stress: info: [matrix-2] size=64, method=all
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
rtos_stress: info: [bitops-1] using 'all' methods
rtos_stress: info: [bitops-2] using 'all' methods
Running Job: [==============>               ] 31/65 (47%) -> bsearch
rtos_stress: debug: bsearch-size set to 16384 elements
rtos_stress: info: [bsearch-0] using 16384 elements (64 KB)
rtos_stress: info: [bsearch-0] using 'all' methods
rtos_stress: info: [bsearch-1] using 16384 elements (64 KB)
rtos_stress: info: [bsearch-1] using 'all' methods
rtos_stress: info: [bsearch-2] using 16384 elements (64 KB)
rtos_stress: info: [bsearch-2] using 'all' methods
Running Job: [==============>               ] 32/65 (49%) -> context
Running Job: [===============>              ] 33/65 (50%) -> fp
rtos_stress: info: [fp-0] float/double/long double math stressor
rtos_stress: info: [fp-1] float/double/long double math stressor
rtos_stress: info: [fp-0] using 'all' methods
rtos_stress: info: [fp-2] float/double/long double math stressor
Running Job: [===============>              ] 34/65 (52%) -> prime
rtos_stress: info: [prime-0] using 'all' methods
rtos_stress: info: [prime-1] using 'all' methods
rtos_stress: info: [prime-2] using 'all' methods
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
rtos_stress: info: [str-1] using 'all' methods
rtos_stress: info: [str-0] using 'all' methods
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
rtos_stress: info: [matrix-1] size=128, method=all
rtos_stress: info: [matrix-2] size=128, method=all
rtos_stress: info: [matrix-3] size=128, method=all
rtos_stress: info: [matrix-0] size=128, method=all
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
rtos_stress: info: [bsearch-1] using 32768 elements (128 KB)
rtos_stress: info: [bsearch-0] using 'all' methods
rtos_stress: info: [bsearch-1] using 'all' methods
rtos_stress: info: [bsearch-2] using 32768 elements (128 KB)
rtos_stress: info: [bsearch-2] using 'all' methods
rtos_stress: info: [bsearch-3] using 32768 elements (128 KB)
rtos_stress: info: [bsearch-3] using 'all' methods
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
rtos_stress: info: [prime-2] using 'all' methods
rtos_stress: info: [prime-3] using 'all' methods
Running Job: [======================>       ] 48/65 (73%) -> stack
rtos_stress: debug: stack-size set to 32768 bytes
rtos_stress: info: [stack-0] started (stack: 32 KB, depth: ~26)
rtos_stress: info: [stack-2] started (stack: 32 KB, depth: ~26)
rtos_stress: info: [stack-1] started (stack: 32 KB, depth: ~26)
rtos_stress: info: [stack-3] started (stack: 32 KB, depth: ~26)
Running Job: [======================>       ] 49/65 (75%) -> str
rtos_stress: debug: str-size set to 8192 bytes
rtos_stress: info: [str-0] string size 8192 bytes
rtos_stress: info: [str-1] string size 8192 bytes
rtos_stress: info: [str-2] string size 8192 bytes
rtos_stress: info: [str-3] string size 8192 bytes
rtos_stress: info: [str-0] using 'all' methods
rtos_stress: info: [str-1] using 'all' methods
rtos_stress: info: [str-2] using 'all' methods
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
rtos_stress: info: [matrix-2] size=128, method=all
rtos_stress: info: [matrix-0] size=128, method=all
rtos_stress: info: [matrix-1] size=128, method=all
rtos_stress: info: [matrix-4] size=128, method=all
rtos_stress: info: [matrix-3] size=128, method=all
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
rtos_stress: info: [bitops-0] using 'all' methods
rtos_stress: info: [bitops-1] using 'all' methods
rtos_stress: info: [bitops-2] using 'all' methods
rtos_stress: info: [bitops-3] using 'all' methods
rtos_stress: info: [bitops-4] using 'all' methods
Running Job: [==========================>   ] 57/65 (87%) -> bsearch
rtos_stress: debug: bsearch-size set to 65536 elements
rtos_stress: info: [bsearch-0] using 65536 elements (256 KB)
rtos_stress: info: [bsearch-1] using 65536 elements (256 KB)
rtos_stress: info: [bsearch-0] using 'all' methods
rtos_stress: info: [bsearch-1] using 'all' methods
rtos_stress: info: [bsearch-2] using 65536 elements (256 KB)
rtos_stress: info: [bsearch-3] using 65536 elements (256 KB)
rtos_stress: info: [bsearch-2] using 'all' methods
rtos_stress: info: [bsearch-3] using 'all' methods
rtos_stress: info: [bsearch-4] using 65536 elements (256 KB)
rtos_stress: info: [bsearch-4] using 'all' methods
Running Job: [==========================>   ] 58/65 (89%) -> context
Running Job: [===========================>  ] 59/65 (90%) -> fp
rtos_stress: info: [fp-0] float/double/long double math stressor
rtos_stress: info: [fp-0] using 'all' methods
rtos_stress: info: [fp-1] float/double/long double math stressor
rtos_stress: info: [fp-2] float/double/long double math stressor
rtos_stress: info: [fp-3] float/double/long double math stressor
rtos_stress: info: [fp-4] float/double/long double math stressor
Running Job: [===========================>  ] 60/65 (92%) -> prime
rtos_stress: info: [prime-0] using 'all' methods
rtos_stress: info: [prime-1] using 'all' methods
rtos_stress: info: [prime-2] using 'all' methods
rtos_stress: info: [prime-3] using 'all' methods
rtos_stress: info: [prime-4] using 'all' methods
Running Job: [============================> ] 61/65 (93%) -> stack
rtos_stress: debug: stack-size set to 65536 bytes
rtos_stress: info: [stack-0] started (stack: 64 KB, depth: ~56)
rtos_stress: info: [stack-1] started (stack: 64 KB, depth: ~56)
rtos_stress: info: [stack-2] started (stack: 64 KB, depth: ~56)
rtos_stress: info: [stack-3] started (stack: 64 KB, depth: ~56)
rtos_stress: info: [stack-4] started (stack: 64 KB, depth: ~56)
Running Job: [============================> ] 62/65 (95%) -> str
rtos_stress: debug: str-size set to 16384 bytes
rtos_stress: info: [str-0] string size 16384 bytes
rtos_stress: info: [str-1] string size 16384 bytes
rtos_stress: info: [str-2] string size 16384 bytes
rtos_stress: info: [str-2] using 'all' methods
rtos_stress: info: [str-1] using 'all' methods
rtos_stress: info: [str-0] using 'all' methods
rtos_stress: info: [str-3] string size 16384 bytes
rtos_stress: info: [str-3] using 'all' methods
rtos_stress: info: [str-4] string size 16384 bytes
rtos_stress: info: [str-4] using 'all' methods
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
 cpu         | 1     | 2800         | 1.06     | N/A
 matrix      | 1     | 6400         | 6.66     | N/A
 qsort       | 1     | 2500         | 3.59     | N/A
 atomic      | 1     | 3000000      | 1.34     | N/A
 bitops      | 1     | 24000        | 3.89     | N/A
 bsearch     | 1     | 3100         | 3.69     | N/A
 context     | 1     | 690000       | 0.20     | N/A
 fp          | 1     | 1050         | 13.76    | N/A
 prime       | 1     | 2700026      | 7.13     | N/A
 stack       | 1     | 6900         | 7.05     | N/A
 str         | 1     | 1300000      | 0.68     | N/A
 trig        | 1     | 210          | 2.48     | N/A
 vecmath     | 1     | 2700         | 2.77     | N/A
 cpu         | 2     | 9800         | 0.96     | N/A
 matrix      | 2     | 5600         | 3.37     | N/A
 qsort       | 2     | 1380         | 2.12     | N/A
 atomic      | 2     | 6600000      | 13.95    | N/A
 bitops      | 2     | 50000        | 4.05     | N/A
 bsearch     | 2     | 2400         | 2.21     | N/A
 context     | 2     | 880000       | 3.30     | N/A
 fp          | 2     | 1460         | 9.61     | N/A
 prime       | 2     | 4200548      | 5.60     | N/A
 stack       | 2     | 13400        | 6.90     | N/A
 str         | 2     | 750000       | 0.68     | N/A
 trig        | 2     | 320          | 1.90     | N/A
 vecmath     | 2     | 3800         | 1.96     | N/A
 cpu         | 3     | 13500        | 0.59     | N/A
 matrix      | 3     | 4800         | 1.95     | N/A
 qsort       | 3     | 300          | 2.12     | N/A
 atomic      | 3     | 5400000      | 10.47    | N/A
 bitops      | 3     | 42000        | 2.29     | N/A
 bsearch     | 3     | 420          | 0.71     | N/A
 context     | 3     | 630000       | 3.30     | N/A
 fp          | 3     | 1200         | 5.29     | N/A
 prime       | 3     | 3901236      | 3.49     | N/A
 stack       | 3     | 11100        | 3.94     | N/A
 str         | 3     | 465000       | 0.56     | N/A
 trig        | 3     | 360          | 1.44     | N/A
 vecmath     | 3     | 3900         | 1.36     | N/A
 cpu         | 4     | 17600        | 0.46     | N/A
 matrix      | 4     | 1120         | 1.30     | N/A
 qsort       | 4     | 180          | 1.94     | N/A
 atomic      | 4     | 5600000      | 17.02    | N/A
 bitops      | 4     | 42000        | 1.73     | N/A
 bsearch     | 4     | 220          | 0.55     | N/A
 context     | 4     | 600000       | 1.30     | N/A
 fp          | 4     | 1280         | 4.22     | N/A
 prime       | 4     | 4400608      | 3.00     | N/A
 stack       | 4     | 10800        | 3.04     | N/A
 str         | 4     | 660000       | 0.57     | N/A
 trig        | 4     | 500          | 1.50     | N/A
 vecmath     | 4     | 5400         | 1.41     | N/A
 cpu         | 5     | 20500        | 0.46     | N/A
 matrix      | 5     | 950          | 0.81     | N/A
 qsort       | 5     | 75           | 2.05     | N/A
 atomic      | 5     | 6000000      | 18.04    | N/A
 bitops      | 5     | 44000        | 1.46     | N/A
 bsearch     | 5     | 100          | 0.37     | N/A
 context     | 5     | 650000       | 1.50     | N/A
 fp          | 5     | 1300         | 3.14     | N/A
 prime       | 5     | 4004700      | 2.20     | N/A
 stack       | 5     | 6750         | 1.70     | N/A
 str         | 5     | 125000       | 0.29     | N/A
 trig        | 5     | 525          | 0.89     | N/A
 vecmath     | 5     | 5500         | 1.16     | N/A
------------------------------------------------------------------------
Total Run Time: 3m 40s
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
rtos_stress: info: [stream-0] bandwidth: 2970.93 MB/sec, compute: 155.76 Mflop/sec
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
rtos_stress: info: [stream-1] target 4096 elements (approx 96 KB total)
rtos_stress: info: [stream-1] ramp: 1024 of 4096 elements
rtos_stress: info: [stream-0] target 4096 elements (approx 96 KB total)
rtos_stress: info: [stream-0] ramp: 1024 of 4096 elements
rtos_stress: info: [stream-1] ramp: 2048 of 4096 elements
rtos_stress: info: [stream-1] ramp: 4096 of 4096 elements
rtos_stress: info: [stream-0] ramp: 2048 of 4096 elements
rtos_stress: info: [stream-0] ramp: 4096 of 4096 elements
rtos_stress: info: [stream-1] bandwidth: 11551.50 MB/sec, compute: 605.63 Mflop/sec
rtos_stress: info: [stream-0] bandwidth: 2869.74 MB/sec, compute: 150.46 Mflop/sec
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
rtos_stress: [ptr-chase-0] heartbeat: 30001152 ops
rtos_stress: [ptr-chase-0] heartbeat: 40001536 ops
rtos_stress: [ptr-chase-0] heartbeat: 50001920 ops
rtos_stress: [ptr-chase-0] heartbeat: 60002304 ops
rtos_stress: info: [ptr-chase-0] finished. visited 131072 of 131072 pointers (100.00%)
rtos_stress: [ptr-chase-1] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-1] heartbeat: 20000768 ops
rtos_stress: [ptr-chase-1] heartbeat: 30001152 ops
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
rtos_stress: info: [stream-0] target 8192 elements (approx 192 KB total)
rtos_stress: info: [stream-1] target 8192 elements (approx 192 KB total)
rtos_stress: info: [stream-0] ramp: 1024 of 8192 elements
rtos_stress: info: [stream-1] ramp: 1024 of 8192 elements
rtos_stress: info: [stream-0] ramp: 2048 of 8192 elements
rtos_stress: info: [stream-1] ramp: 2048 of 8192 elements
rtos_stress: info: [stream-0] ramp: 4096 of 8192 elements
rtos_stress: info: [stream-1] ramp: 4096 of 8192 elements
rtos_stress: info: [stream-2] target 8192 elements (approx 192 KB total)
rtos_stress: info: [stream-2] ramp: 1024 of 8192 elements
rtos_stress: info: [stream-0] ramp: 8192 of 8192 elements
rtos_stress: info: [stream-1] ramp: 8192 of 8192 elements
rtos_stress: info: [stream-2] ramp: 2048 of 8192 elements
rtos_stress: info: [stream-2] ramp: 4096 of 8192 elements
rtos_stress: info: [stream-2] ramp: 8192 of 8192 elements
rtos_stress: info: [stream-0] bandwidth: 11762.52 MB/sec, compute: 616.69 Mflop/sec
rtos_stress: info: [stream-1] bandwidth: 11733.76 MB/sec, compute: 615.19 Mflop/sec
rtos_stress: info: [stream-2] bandwidth: 2865.77 MB/sec, compute: 150.25 Mflop/sec
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
rtos_stress: info: [malloc-1] done
rtos_stress: info: [malloc-0] done
rtos_stress: info: [malloc-2] done
Running Job: [================>             ] 16/30 (53%) -> memthrash
rtos_stress: info: [memthrash-0] allocated 64 KB
rtos_stress: info: [memthrash-0] using 'all' methods
rtos_stress: info: [memthrash-1] allocated 64 KB
rtos_stress: info: [memthrash-1] using 'all' methods
rtos_stress: info: [memthrash-2] allocated 64 KB
rtos_stress: info: [memthrash-2] using 'all' methods
Running Job: [=================>            ] 17/30 (56%) -> ptr-chase
rtos_stress: debug: ptr-chase-pages set to 512
rtos_stress: info: [ptr-chase-0] chasing 512 pages (2048 KB)
rtos_stress: info: [ptr-chase-1] chasing 512 pages (2048 KB)
rtos_stress: info: [ptr-chase-2] chasing 512 pages (2048 KB)
rtos_stress: [ptr-chase-1] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-0] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-2] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-1] heartbeat: 20000768 ops
rtos_stress: [ptr-chase-0] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-1] finished. visited 262144 of 262144 pointers (100.00%)
rtos_stress: info: [ptr-chase-0] finished. visited 262144 of 262144 pointers (100.00%)
rtos_stress: [ptr-chase-2] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-2] finished. visited 262144 of 262144 pointers (100.00%)
Running Job: [==================>           ] 18/30 (60%) -> memcpy
rtos_stress: debug: memcpy-size set to 2048
rtos_stress: info: [memcpy-0] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-0] using 'all' methods
rtos_stress: info: [memcpy-1] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-1] using 'all' methods
rtos_stress: info: [memcpy-2] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-2] using 'all' methods
rtos_stress: info: [memcpy-3] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-3] using 'all' methods
Running Job: [===================>          ] 19/30 (63%) -> stream
rtos_stress: debug: stream-elem set to 16384 elements
rtos_stress: info: [stream-0] target 16384 elements (approx 384 KB total)
rtos_stress: info: [stream-0] ramp: 1024 of 16384 elements
rtos_stress: info: [stream-0] ramp: 2048 of 16384 elements
rtos_stress: info: [stream-0] ramp: 4096 of 16384 elements
rtos_stress: info: [stream-0] ramp: 8192 of 16384 elements
rtos_stress: info: [stream-0] ramp: 16384 of 16384 elements
rtos_stress: info: [stream-1] target 16384 elements (approx 384 KB total)
rtos_stress: info: [stream-1] ramp: 1024 of 16384 elements
rtos_stress: info: [stream-1] ramp: 2048 of 16384 elements
rtos_stress: info: [stream-1] ramp: 4096 of 16384 elements
rtos_stress: info: [stream-1] ramp: 8192 of 16384 elements
rtos_stress: info: [stream-1] ramp: 16384 of 16384 elements
rtos_stress: info: [stream-2] target 16384 elements (approx 384 KB total)
rtos_stress: info: [stream-2] ramp: 1024 of 16384 elements
rtos_stress: info: [stream-3] target 16384 elements (approx 384 KB total)
rtos_stress: info: [stream-3] ramp: 1024 of 16384 elements
rtos_stress: info: [stream-2] ramp: 2048 of 16384 elements
rtos_stress: info: [stream-3] ramp: 2048 of 16384 elements
rtos_stress: info: [stream-2] ramp: 4096 of 16384 elements
rtos_stress: info: [stream-3] ramp: 4096 of 16384 elements
rtos_stress: info: [stream-2] ramp: 8192 of 16384 elements
rtos_stress: info: [stream-3] ramp: 8192 of 16384 elements
rtos_stress: info: [stream-2] ramp: 16384 of 16384 elements
rtos_stress: info: [stream-3] ramp: 16384 of 16384 elements
rtos_stress: info: [stream-2] bandwidth: 11851.41 MB/sec, compute: 621.35 Mflop/sec
rtos_stress: info: [stream-3] bandwidth: 11260.74 MB/sec, compute: 590.39 Mflop/sec
rtos_stress: info: [stream-0] bandwidth: 1981.72 MB/sec, compute: 103.90 Mflop/sec
rtos_stress: info: [stream-1] bandwidth: 1995.61 MB/sec, compute: 104.63 Mflop/sec
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
rtos_stress: info: [malloc-2] max_slots=64 max_bytes=4096
rtos_stress: info: [malloc-3] max_slots=64 max_bytes=4096
rtos_stress: info: [malloc-0] done
rtos_stress: info: [malloc-3] done
rtos_stress: info: [malloc-1] done
rtos_stress: info: [malloc-2] done
Running Job: [======================>       ] 22/30 (73%) -> memthrash
rtos_stress: info: [memthrash-0] allocated 64 KB
rtos_stress: info: [memthrash-0] using 'all' methods
rtos_stress: info: [memthrash-1] allocated 64 KB
rtos_stress: info: [memthrash-1] using 'all' methods
rtos_stress: info: [memthrash-2] allocated 64 KB
rtos_stress: info: [memthrash-2] using 'all' methods
rtos_stress: info: [memthrash-3] allocated 64 KB
rtos_stress: info: [memthrash-3] using 'all' methods
Running Job: [=======================>      ] 23/30 (76%) -> ptr-chase
rtos_stress: debug: ptr-chase-pages set to 256
rtos_stress: info: [ptr-chase-0] chasing 256 pages (1024 KB)
rtos_stress: info: [ptr-chase-1] chasing 256 pages (1024 KB)
rtos_stress: info: [ptr-chase-2] chasing 256 pages (1024 KB)
rtos_stress: info: [ptr-chase-3] chasing 256 pages (1024 KB)
rtos_stress: [ptr-chase-1] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-2] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-1] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-1] finished. visited 131072 of 131072 pointers (100.00%)
rtos_stress: [ptr-chase-2] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-2] finished. visited 131072 of 131072 pointers (100.00%)
rtos_stress: [ptr-chase-3] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-0] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-3] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-3] finished. visited 131072 of 131072 pointers (100.00%)
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
rtos_stress: info: [stream-0] ramp: 2048 of 32768 elements
rtos_stress: info: [stream-0] ramp: 4096 of 32768 elements
rtos_stress: info: [stream-0] ramp: 8192 of 32768 elements
rtos_stress: info: [stream-0] ramp: 16384 of 32768 elements
rtos_stress: info: [stream-0] ramp: 32768 of 32768 elements
rtos_stress: info: [stream-1] target 32768 elements (approx 768 KB total)
rtos_stress: info: [stream-1] ramp: 2048 of 32768 elements
rtos_stress: info: [stream-1] ramp: 4096 of 32768 elements
rtos_stress: info: [stream-1] ramp: 8192 of 32768 elements
rtos_stress: info: [stream-1] ramp: 16384 of 32768 elements
rtos_stress: info: [stream-1] ramp: 32768 of 32768 elements
rtos_stress: info: [stream-2] target 32768 elements (approx 768 KB total)
rtos_stress: info: [stream-2] ramp: 2048 of 32768 elements
rtos_stress: info: [stream-2] ramp: 4096 of 32768 elements
rtos_stress: info: [stream-2] ramp: 8192 of 32768 elements
rtos_stress: info: [stream-3] target 32768 elements (approx 768 KB total)
rtos_stress: info: [stream-3] ramp: 2048 of 32768 elements
rtos_stress: info: [stream-3] ramp: 4096 of 32768 elements
rtos_stress: info: [stream-2] ramp: 16384 of 32768 elements
rtos_stress: info: [stream-3] ramp: 8192 of 32768 elements
rtos_stress: info: [stream-3] ramp: 16384 of 32768 elements
rtos_stress: info: [stream-2] ramp: 32768 of 32768 elements
rtos_stress: info: [stream-3] ramp: 32768 of 32768 elements
rtos_stress: info: [stream-2] bandwidth: 11764.25 MB/sec, compute: 616.79 Mflop/sec
rtos_stress: info: [stream-3] bandwidth: 11260.34 MB/sec, compute: 590.37 Mflop/sec
rtos_stress: info: [stream-0] bandwidth: 1920.71 MB/sec, compute: 100.70 Mflop/sec
rtos_stress: info: [stream-1] bandwidth: 1947.63 MB/sec, compute: 102.11 Mflop/sec
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
rtos_stress: info: [malloc-1] max_slots=64 max_bytes=8192
rtos_stress: info: [malloc-2] max_slots=64 max_bytes=8192
rtos_stress: info: [malloc-3] max_slots=64 max_bytes=8192
rtos_stress: info: [malloc-4] max_slots=64 max_bytes=8192
rtos_stress: info: [malloc-0] done
rtos_stress: info: [malloc-2] done
rtos_stress: info: [malloc-1] done
rtos_stress: info: [malloc-4] done
rtos_stress: info: [malloc-3] done
Running Job: [============================> ] 28/30 (93%) -> memthrash
rtos_stress: info: [memthrash-0] allocated 64 KB
rtos_stress: info: [memthrash-0] using 'all' methods
rtos_stress: info: [memthrash-1] allocated 64 KB
rtos_stress: info: [memthrash-1] using 'all' methods
rtos_stress: info: [memthrash-2] allocated 64 KB
rtos_stress: info: [memthrash-2] using 'all' methods
rtos_stress: info: [memthrash-3] allocated 64 KB
rtos_stress: info: [memthrash-3] using 'all' methods
rtos_stress: info: [memthrash-4] allocated 64 KB
rtos_stress: info: [memthrash-4] using 'all' methods
Running Job: [=============================>] 29/30 (96%) -> ptr-chase
rtos_stress: debug: ptr-chase-pages set to 128
rtos_stress: info: [ptr-chase-0] chasing 128 pages (512 KB)
rtos_stress: info: [ptr-chase-1] chasing 128 pages (512 KB)
rtos_stress: info: [ptr-chase-2] chasing 128 pages (512 KB)
rtos_stress: [ptr-chase-0] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-0] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-0] finished. visited 65536 of 65536 pointers (100.00%)
rtos_stress: info: [ptr-chase-3] chasing 128 pages (512 KB)
rtos_stress: info: [ptr-chase-4] chasing 128 pages (512 KB)
rtos_stress: [ptr-chase-3] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-4] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-3] heartbeat: 20000768 ops
rtos_stress: [ptr-chase-1] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-2] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-4] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-3] finished. visited 65536 of 65536 pointers (100.00%)
rtos_stress: info: [ptr-chase-4] finished. visited 65536 of 65536 pointers (100.00%)
rtos_stress: [ptr-chase-1] heartbeat: 20000768 ops
rtos_stress: [ptr-chase-2] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-1] finished. visited 65536 of 65536 pointers (100.00%)
rtos_stress: info: [ptr-chase-2] finished. visited 65536 of 65536 pointers (100.00%)
Running Job: [==============================] 30/30 (100%) -> Finished


========================================================================
 Job: stored_jobfile_memory.txt [Built-in]
========================================================================
 Stressor    | Stage | Bogo Ops     | Time(s)  | Metric
------------------------------------------------------------------------
 memcpy      | 1     | 10000        | 16.06    | N/A
 stream      | 1     | 52000        | 2.78     | 2970.93 MB/sec, 155.76 Mflop/sec
 vm          | 1     | 42000384     | 1.91     | N/A
 malloc      | 1     | 50000        | 0.45     | N/A
 memthrash   | 1     | 2300         | 10.17    | N/A
 ptr-chase   | 1     | 86000640     | 8.74     | N/A
 memcpy      | 2     | 20000        | 22.90    | N/A
 stream      | 2     | 41000        | 2.28     | 14421.23 MB/sec, 756.09 Mflop/sec
 vm          | 2     | 75000320     | 1.77     | N/A
 malloc      | 2     | 100000       | 0.45     | N/A
 memthrash   | 2     | 4400         | 10.11    | N/A
 ptr-chase   | 2     | 124000256    | 8.83     | N/A
 memcpy      | 3     | 30000        | 22.94    | N/A
 stream      | 3     | 27000        | 2.00     | 26362.05 MB/sec, 1382.13 Mflop/sec
 vm          | 3     | 78000128     | 1.74     | N/A
 malloc      | 3     | 150000       | 0.45     | N/A
 memthrash   | 3     | 5400         | 8.68     | N/A
 ptr-chase   | 3     | 82501632     | 5.24     | N/A
 memcpy      | 4     | 40000        | 22.93    | N/A
 stream      | 4     | 10800        | 1.80     | 27089.48 MB/sec, 1420.27 Mflop/sec
 vm          | 4     | 75000320     | 2.00     | N/A
 malloc      | 4     | 200000       | 0.45     | N/A
 memthrash   | 4     | 4400         | 5.57     | N/A
 ptr-chase   | 4     | 80003072     | 3.07     | N/A
 memcpy      | 5     | 50000        | 20.15    | N/A
 stream      | 5     | 4400         | 1.46     | 26892.93 MB/sec, 1409.96 Mflop/sec
 vm          | 5     | 87002112     | 1.37     | N/A
 malloc      | 5     | 250000       | 0.45     | N/A
 memthrash   | 5     | 4500         | 4.63     | N/A
 ptr-chase   | 5     | 130001920    | 3.42     | N/A
------------------------------------------------------------------------
Total Run Time: 3m 14s
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
rtos_stress: info: [pipe-0] throughput: 189581.63 KB/sec
Running Job: [======>                       ] 8/40 (20%) -> hdd
rtos_stress: info: [hdd-0] testing file '0.tmp', size 64 KB
Running Job: [======>                       ] 9/40 (22%) -> open
rtos_stress: debug: open-max set to 4
rtos_stress: info: [open-1] opening up to 4 files per round (base dir: "")
rtos_stress: info: [open-0] opening up to 4 files per round (base dir: "")
Running Job: [=======>                      ] 10/40 (25%) -> copy-file
rtos_stress: debug: copy-file-bytes set to 32768 bytes
rtos_stress: info: [copy-file-0] file size 32768 bytes, chunk 4096, verify every 4 ops
rtos_stress: info: [copy-file-1] file size 32768 bytes, chunk 4096, verify every 4 ops
rtos_stress: info: [copy-file-1] fsync available
rtos_stress: info: [copy-file-1] pre-allocating 32768 bytes...
rtos_stress: info: [copy-file-0] fsync available
rtos_stress: info: [copy-file-0] pre-allocating 32768 bytes...
rtos_stress: info: [copy-file-1] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-0] pre-alloc done, starting copy loop
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
rtos_stress: info: [pipe-0] throughput: 578670.05 KB/sec
rtos_stress: info: [pipe-1] throughput: 587969.85 KB/sec
Running Job: [============>                 ] 16/40 (40%) -> hdd
rtos_stress: info: [hdd-0] testing file '0.tmp', size 64 KB
Running Job: [============>                 ] 17/40 (42%) -> open
rtos_stress: debug: open-max set to 6
rtos_stress: info: [open-1] opening up to 6 files per round (base dir: "")
rtos_stress: info: [open-2] opening up to 6 files per round (base dir: "")
rtos_stress: info: [open-0] opening up to 6 files per round (base dir: "")
Running Job: [=============>                ] 18/40 (45%) -> copy-file
rtos_stress: debug: copy-file-bytes set to 65536 bytes
rtos_stress: info: [copy-file-0] file size 65536 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-1] file size 65536 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-2] file size 65536 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-1] fsync available
rtos_stress: info: [copy-file-2] fsync available
rtos_stress: info: [copy-file-2] pre-allocating 65536 bytes...
rtos_stress: info: [copy-file-1] pre-allocating 65536 bytes...
rtos_stress: info: [copy-file-0] fsync available
rtos_stress: info: [copy-file-0] pre-allocating 65536 bytes...
rtos_stress: info: [copy-file-1] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-2] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-0] pre-alloc done, starting copy loop
Running Job: [==============>               ] 19/40 (47%) -> unlink
rtos_stress: info: [unlink-2] starting with 32 files
rtos_stress: info: [unlink-1] starting with 32 files
rtos_stress: info: [unlink-0] starting with 32 files
rtos_stress: SUMMARY: [unlink-2] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-0] fd limit hit 0 times, final limit=32/32
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
rtos_stress: info: [rename-1] using dir 'R1', file size 16 bytes
rtos_stress: info: [rename-0] using dir 'R0', file size 16 bytes
Running Job: [=================>            ] 23/40 (57%) -> pipe
rtos_stress: info: [pipe-0] blocking mode chunk=32768 run_id=3
rtos_stress: info: [pipe-1] blocking mode chunk=32768 run_id=4
rtos_stress: info: [pipe-2] blocking mode chunk=32768 run_id=5
rtos_stress: info: [pipe-0] throughput: 374454.34 KB/sec
rtos_stress: info: [pipe-2] throughput: 571816.60 KB/sec
rtos_stress: info: [pipe-1] throughput: 178939.40 KB/sec
Running Job: [==================>           ] 24/40 (60%) -> hdd
rtos_stress: info: [hdd-0] testing file '0.tmp', size 128 KB
Running Job: [==================>           ] 25/40 (62%) -> open
rtos_stress: debug: open-max set to 8
rtos_stress: info: [open-1] opening up to 8 files per round (base dir: "")
rtos_stress: info: [open-2] opening up to 8 files per round (base dir: "")
rtos_stress: info: [open-3] opening up to 8 files per round (base dir: "")
rtos_stress: info: [open-0] opening up to 8 files per round (base dir: "")
Running Job: [===================>          ] 26/40 (65%) -> copy-file
rtos_stress: debug: copy-file-bytes set to 131072 bytes
rtos_stress: info: [copy-file-0] file size 131072 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-1] file size 131072 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-2] file size 131072 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-3] file size 131072 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-3] fsync available
rtos_stress: info: [copy-file-2] fsync available
rtos_stress: info: [copy-file-2] pre-allocating 131072 bytes...
rtos_stress: info: [copy-file-3] pre-allocating 131072 bytes...
rtos_stress: info: [copy-file-1] fsync available
rtos_stress: info: [copy-file-1] pre-allocating 131072 bytes...
rtos_stress: info: [copy-file-0] fsync available
rtos_stress: info: [copy-file-0] pre-allocating 131072 bytes...
rtos_stress: info: [copy-file-1] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-3] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-0] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-2] pre-alloc done, starting copy loop
Running Job: [====================>         ] 27/40 (67%) -> unlink
rtos_stress: info: [unlink-0] starting with 32 files
rtos_stress: info: [unlink-2] starting with 32 files
rtos_stress: info: [unlink-1] starting with 32 files
rtos_stress: info: [unlink-3] starting with 32 files
rtos_stress: SUMMARY: [unlink-0] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-2] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-3] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-1] fd limit hit 0 times, final limit=32/32
Running Job: [=====================>        ] 28/40 (70%) -> fstat
rtos_stress: debug: fstat-files set to 8
rtos_stress: info: [fstat-0] created 8/8 files
Running Job: [=====================>        ] 29/40 (72%) -> dentry
rtos_stress: debug: dentries set to 8
rtos_stress: info: [dentry-0] using 8 dentries in stress_dentry_0_dir
rtos_stress: info: [dentry-1] using 8 dentries in stress_dentry_1_dir
rtos_stress: info: [dentry-2] using 8 dentries in stress_dentry_2_dir
Running Job: [======================>       ] 30/40 (75%) -> rename
rtos_stress: info: [rename-0] using dir 'R0', file size 16 bytes
rtos_stress: info: [rename-1] using dir 'R1', file size 16 bytes
rtos_stress: info: [rename-2] using dir 'R2', file size 16 bytes
Running Job: [=======================>      ] 31/40 (77%) -> pipe
rtos_stress: info: [pipe-0] blocking mode chunk=32768 run_id=6
rtos_stress: info: [pipe-1] blocking mode chunk=32768 run_id=7
rtos_stress: info: [pipe-2] blocking mode chunk=32768 run_id=8
rtos_stress: info: [pipe-3] blocking mode chunk=32768 run_id=9
rtos_stress: info: [pipe-2] throughput: 497862.92 KB/sec
rtos_stress: info: [pipe-3] throughput: 498552.75 KB/sec
rtos_stress: info: [pipe-0] throughput: 201592.17 KB/sec
rtos_stress: info: [pipe-1] throughput: 200952.37 KB/sec
Running Job: [========================>     ] 32/40 (80%) -> hdd
rtos_stress: info: [hdd-0] testing file '0.tmp', size 128 KB
Running Job: [========================>     ] 33/40 (82%) -> open
rtos_stress: debug: open-max set to 10
rtos_stress: info: [open-0] opening up to 10 files per round (base dir: "")
rtos_stress: info: [open-1] opening up to 10 files per round (base dir: "")
rtos_stress: info: [open-2] opening up to 10 files per round (base dir: "")
rtos_stress: info: [open-3] opening up to 10 files per round (base dir: "")
rtos_stress: info: [open-4] opening up to 10 files per round (base dir: "")
Running Job: [=========================>    ] 34/40 (85%) -> copy-file
rtos_stress: debug: copy-file-bytes set to 262144 bytes
rtos_stress: info: [copy-file-0] file size 262144 bytes, chunk 4096, verify every 16 ops
rtos_stress: info: [copy-file-1] file size 262144 bytes, chunk 4096, verify every 16 ops
rtos_stress: info: [copy-file-2] file size 262144 bytes, chunk 4096, verify every 16 ops
rtos_stress: info: [copy-file-3] file size 262144 bytes, chunk 4096, verify every 16 ops
rtos_stress: info: [copy-file-4] file size 262144 bytes, chunk 4096, verify every 16 ops
rtos_stress: info: [copy-file-1] fsync available
rtos_stress: info: [copy-file-1] pre-allocating 262144 bytes...
rtos_stress: info: [copy-file-0] fsync available
rtos_stress: info: [copy-file-0] pre-allocating 262144 bytes...
rtos_stress: info: [copy-file-2] fsync available
rtos_stress: info: [copy-file-4] fsync available
rtos_stress: info: [copy-file-4] pre-allocating 262144 bytes...
rtos_stress: info: [copy-file-3] fsync available
rtos_stress: info: [copy-file-3] pre-allocating 262144 bytes...
rtos_stress: info: [copy-file-2] pre-allocating 262144 bytes...
rtos_stress: info: [copy-file-1] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-2] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-4] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-0] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-3] pre-alloc done, starting copy loop
Running Job: [==========================>   ] 35/40 (87%) -> unlink
rtos_stress: info: [unlink-0] starting with 32 files
rtos_stress: info: [unlink-4] starting with 32 files
rtos_stress: info: [unlink-1] starting with 32 files
rtos_stress: info: [unlink-3] starting with 32 files
rtos_stress: info: [unlink-2] starting with 32 files
rtos_stress: SUMMARY: [unlink-4] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-0] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-1] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-3] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-2] fd limit hit 0 times, final limit=32/32
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
rtos_stress: info: [rename-1] using dir 'R1', file size 16 bytes
rtos_stress: info: [rename-0] using dir 'R0', file size 16 bytes
rtos_stress: info: [rename-2] using dir 'R2', file size 16 bytes
Running Job: [=============================>] 39/40 (97%) -> pipe
rtos_stress: info: [pipe-0] blocking mode chunk=32768 run_id=10
rtos_stress: info: [pipe-1] blocking mode chunk=32768 run_id=11
rtos_stress: info: [pipe-2] blocking mode chunk=32768 run_id=12
rtos_stress: info: [pipe-0] throughput: 537695.53 KB/sec
rtos_stress: info: [pipe-1] throughput: 411294.59 KB/sec
rtos_stress: info: [pipe-3] blocking mode chunk=32768 run_id=13
rtos_stress: info: [pipe-4] blocking mode chunk=32768 run_id=14
rtos_stress: info: [pipe-2] throughput: 263314.16 KB/sec
rtos_stress: info: [pipe-4] throughput: 346034.65 KB/sec
rtos_stress: info: [pipe-3] throughput: 348682.83 KB/sec
Running Job: [==============================] 40/40 (100%) -> Finished


========================================================================
 Job: stored_jobfile_file.txt [Built-in]
========================================================================
 Stressor    | Stage | Bogo Ops     | Time(s)  | Metric
------------------------------------------------------------------------
 hdd         | 1     | 480          | 5.05     | N/A
 open        | 1     | 2400         | 0.71     | N/A
 copy-file   | 1     | 2400         | 9.11     | N/A
 unlink      | 1     | 2400         | 0.46     | N/A
 fstat       | 1     | 6000         | 0.15     | N/A
 dentry      | 1     | 3000         | 1.01     | N/A
 rename      | 1     | 6000         | 6.24     | N/A
 pipe        | 1     | 37939        | 0.20     | 189581.63 KB/sec
 hdd         | 2     | 240          | 2.53     | N/A
 open        | 2     | 2400         | 0.40     | N/A
 copy-file   | 2     | 2400         | 8.18     | N/A
 unlink      | 2     | 2400         | 0.34     | N/A
 fstat       | 2     | 6000         | 0.15     | N/A
 dentry      | 2     | 3000         | 0.30     | N/A
 rename      | 2     | 6000         | 3.15     | N/A
 pipe        | 2     | 30706        | 0.21     | 1166639.90 KB/sec
 hdd         | 3     | 160          | 1.71     | N/A
 open        | 3     | 2400         | 0.22     | N/A
 copy-file   | 3     | 2400         | 6.05     | N/A
 unlink      | 3     | 2400         | 0.29     | N/A
 fstat       | 3     | 6000         | 0.15     | N/A
 dentry      | 3     | 3000         | 0.22     | N/A
 rename      | 3     | 6000         | 3.15     | N/A
 pipe        | 3     | 4801         | 0.24     | 1125210.34 KB/sec
 hdd         | 4     | 160          | 1.04     | N/A
 open        | 4     | 2400         | 0.16     | N/A
 copy-file   | 4     | 2400         | 8.01     | N/A
 unlink      | 4     | 2400         | 0.29     | N/A
 fstat       | 4     | 6000         | 0.15     | N/A
 dentry      | 4     | 3000         | 0.16     | N/A
 rename      | 4     | 6000         | 2.12     | N/A
 pipe        | 4     | 4376         | 0.20     | 1398960.21 KB/sec
 hdd         | 5     | 160          | 1.06     | N/A
 open        | 5     | 2400         | 0.13     | N/A
 copy-file   | 5     | 2400         | 7.44     | N/A
 unlink      | 5     | 2400         | 0.27     | N/A
 fstat       | 5     | 6000         | 0.15     | N/A
 dentry      | 5     | 3000         | 0.13     | N/A
 rename      | 5     | 6000         | 2.12     | N/A
 pipe        | 5     | 7872         | 0.34     | 1907021.76 KB/sec
------------------------------------------------------------------------
Total Run Time: 1m 13s

[test-stress] Job 'all' completed: 135 stressor runs
=============================================================

>>> Running test-cmd...
Thu Jan  1 00:19:34 UTC 1970
    PID TTY          TIME CMD
    493 pts/0    00:00:00 bash
    621 pts/0    00:10:52 rtos-bench-open
   1869 pts/0    00:00:00 ps
/rtosbench
README  result_20261023.json                  rtos-bench-openeuler-feiteng-aarch64-vxworks-7wl-fix
logs    rtos-bench-openeuler-feiteng-aarch64  test_cmd_dir
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
    stub: 0.001 ms total, 0.000 ms avg
  Running workload: busywait
    busywait: 20.001 ms total, 2.000 ms avg
  Running workload: fast
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=19.523 ms avg_latency=6507.813 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=19.492 ms avg_latency=6497.313 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=19.538 ms avg_latency=6512.507 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=19.505 ms avg_latency=6501.680 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=19.478 ms avg_latency=6492.780 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=12.577 ms avg_latency=4192.247 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=19.417 ms avg_latency=6472.320 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=19.496 ms avg_latency=6498.667 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=19.428 ms avg_latency=6475.927 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=19.526 ms avg_latency=6508.547 us/img
[POSIX Thread][FAST] Benchmark thread finished.
    fast: 189.695 ms total, 18.969 ms avg
  Running workload: icp
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=354.731 ms avg_latency=354.731 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=354.228 ms avg_latency=354.228 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=147.738 ms avg_latency=147.738 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=356.187 ms avg_latency=356.187 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=147.808 ms avg_latency=147.808 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=353.540 ms avg_latency=353.540 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=353.514 ms avg_latency=353.514 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=147.512 ms avg_latency=147.512 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=147.809 ms avg_latency=147.809 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=147.756 ms avg_latency=147.756 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
    icp: 2530.001 ms total, 253.000 ms avg
  Running workload: modbus
[POSIX Thread][MODBUS][Server] Creating benchmark thread...
[POSIX Thread][MODBUS][Client] Creating benchmark thread...
[Client] Waiting for server...
[Client] Connecting to 127.0.0.1:5020...
[Client] Connected!
[Client] Modbus client created

=== R/W Benchmark Result ===
Time:       8.072 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.50
[MODBUS] samples=4 total_time=8071.958 ms avg_latency=2017.990 ms/request
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
Time:       8.055 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.50
[MODBUS] samples=4 total_time=8055.320 ms avg_latency=2013.830 ms/request
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
Time:       8.055 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.50
[MODBUS] samples=4 total_time=8055.428 ms avg_latency=2013.857 ms/request
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
Time:       8.055 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.50
[MODBUS] samples=4 total_time=8055.436 ms avg_latency=2013.859 ms/request
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
Time:       8.055 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.50
[MODBUS] samples=4 total_time=8055.383 ms avg_latency=2013.846 ms/request
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
Time:       8.055 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.50
[MODBUS] samples=4 total_time=8055.370 ms avg_latency=2013.842 ms/request
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
Time:       8.055 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.50
[MODBUS] samples=4 total_time=8055.454 ms avg_latency=2013.864 ms/request
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
Time:       8.055 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.50
[MODBUS] samples=4 total_time=8055.480 ms avg_latency=2013.870 ms/request
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
Time:       8.055 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.50
[MODBUS] samples=4 total_time=8055.492 ms avg_latency=2013.873 ms/request
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
Time:       8.055 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.50
[MODBUS] samples=4 total_time=8055.425 ms avg_latency=2013.856 ms/request
[Client] Finished
[POSIX Thread][MODBUS][Client] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Benchmark thread finished.
    modbus: 90576.659 ms total, 9057.666 ms avg
  Running workload: mqtt
[POSIX Thread][MQTT] Creating benchmark thread...
[MQTT] Thread Started...
[MQTT] Connecting to tcp://44.232.241.40:1883 (Raw TCP Mode)...
[MQTT] Socket Created
[MQTT] Error: connect: 101
[POSIX Thread][MQTT] Benchmark thread finished.
[POSIX Thread][MQTT] Creating benchmark thread...
[MQTT] Thread Started...
[MQTT] Connecting to tcp://44.232.241.40:1883 (Raw TCP Mode)...
[MQTT] Socket Created
[MQTT] Error: connect: 101
[POSIX Thread][MQTT] Benchmark thread finished.
[POSIX Thread][MQTT] Creating benchmark thread...
[MQTT] Thread Started...
[MQTT] Connecting to tcp://44.232.241.40:1883 (Raw TCP Mode)...
[MQTT] Socket Created
[MQTT] Error: connect: 101
[POSIX Thread][MQTT] Benchmark thread finished.
[POSIX Thread][MQTT] Creating benchmark thread...
[MQTT] Thread Started...
[MQTT] Connecting to tcp://44.232.241.40:1883 (Raw TCP Mode)...
[MQTT] Socket Created
[MQTT] Error: connect: 101
[POSIX Thread][MQTT] Benchmark thread finished.
[POSIX Thread][MQTT] Creating benchmark thread...
[MQTT] Thread Started...
[MQTT] Connecting to tcp://44.232.241.40:1883 (Raw TCP Mode)...
[MQTT] Socket Created
[MQTT] Error: connect: 101
[POSIX Thread][MQTT] Benchmark thread finished.
[POSIX Thread][MQTT] Creating benchmark thread...
[MQTT] Thread Started...
[MQTT] Connecting to tcp://44.232.241.40:1883 (Raw TCP Mode)...
[MQTT] Socket Created
[MQTT] Error: connect: 101
[POSIX Thread][MQTT] Benchmark thread finished.
[POSIX Thread][MQTT] Creating benchmark thread...
[MQTT] Thread Started...
[MQTT] Connecting to tcp://44.232.241.40:1883 (Raw TCP Mode)...
[MQTT] Socket Created
[MQTT] Error: connect: 101
[POSIX Thread][MQTT] Benchmark thread finished.
[POSIX Thread][MQTT] Creating benchmark thread...
[MQTT] Thread Started...
[MQTT] Connecting to tcp://44.232.241.40:1883 (Raw TCP Mode)...
[MQTT] Socket Created
[MQTT] Error: connect: 101
[POSIX Thread][MQTT] Benchmark thread finished.
[POSIX Thread][MQTT] Creating benchmark thread...
[MQTT] Thread Started...
[MQTT] Connecting to tcp://44.232.241.40:1883 (Raw TCP Mode)...
[MQTT] Socket Created
[MQTT] Error: connect: 101
[POSIX Thread][MQTT] Benchmark thread finished.
[POSIX Thread][MQTT] Creating benchmark thread...
[MQTT] Thread Started...
[MQTT] Connecting to tcp://44.232.241.40:1883 (Raw TCP Mode)...
[MQTT] Socket Created
[MQTT] Error: connect: 101
[POSIX Thread][MQTT] Benchmark thread finished.
    mqtt: 2.646 ms total, 0.265 ms avg
  Running workload: pid
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.919 ms avg_latency=0.076 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.843 ms avg_latency=0.076 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.849 ms avg_latency=0.076 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.794 ms avg_latency=0.076 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.840 ms avg_latency=0.076 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.797 ms avg_latency=0.076 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.849 ms avg_latency=0.076 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.845 ms avg_latency=0.076 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.798 ms avg_latency=0.076 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.809 ms avg_latency=0.076 us/op
[POSIX Thread][PID] Benchmark thread finished.
    pid: 760.801 ms total, 76.080 ms avg
  Running workload: cusum
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.687 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.674 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.688 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.676 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.683 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.678 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.682 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=69.678 ms avg_latency=0.116 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.685 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.677 ms avg_latency=0.043 us/sample
    cusum: 302.669 ms total, 30.267 ms avg
  Running workload: ewma
[EWMA] loops=100 stream_len_per_loop=5000 total_time=93.990 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=93.957 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=93.959 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=93.962 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=93.968 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=93.960 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=93.982 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=93.958 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=93.955 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.505 ms avg_latency=0.069 us/sample
    ewma: 882.162 ms total, 88.216 ms avg

=============================================================
[RTOS-Bench] Results saved to: result_20261023_7workloads.json
=============================================================