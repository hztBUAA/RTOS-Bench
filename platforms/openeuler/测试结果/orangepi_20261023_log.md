=============================================================
[RTOS-Bench] Comprehensive Test Suite
=============================================================
Output: /rtosbench/result_20261023.json
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
[  124.728650] sched: RT throttling activated
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
信号量释放 | 0.041      |            | 1.750      | 5.375
消息发送   | 0.708      | 5.833      | 2.166      | 5.625
消息接收   | 0.791      | 4.708      | 2.333      | 4.875
互斥锁获取 | 0.041      | 4.375      |            |
互斥锁释放 | 0.000      |            | 1.791      | 4.833
内存块申请 | 0.041      |            |            |
内存块释放 | 0.041      |            |            |
上下文切换延迟    AVG: 1.349
中断软件延迟    MIN: 0.291  MAX: 1.750  AVG: 0.402
系统调用延迟    MIN: 5.833  MAX: 21.000  AVG: 6.862

多核存取性能:
并发度     | 1          | 2          | 4          | 8
----------------------------------------------------------------------
rd         | 4.362      | 12.136     | 12.191     | 12.205
wr         | 1.748      | 1.776      | 1.745      | 1.771
cp         | 0.401      | 0.484      | 0.850      | 0.839
frd        | 17.156     | 32.792     | 23.520     | 32.646
fwr        | 16.225     | 40.903     | 37.669     | 38.824
fcp        | 7.613      | 15.987     | 18.135     | 16.573
memset     | 18.164     | 32.080     | 32.920     | 28.322
memcpy     | 11.653     | 22.322     | 22.728     | 21.362
多核系统服务:
并发度     | 1          | 2          | 4          | 8
任务间通信 | 0.708      | 1.343      | 1.134      | 1.284
创建与删除 | 57.672     | 36.944     | 37.556     | 40.236    （单位为us）
任务在同核与异核上通信比较:
同核通信: 0.696
异核通信: 0.579
（除特殊说明外，上述单位均为GB/s）

=============================================================
[test-realtime] Benchmark completed with code: 0
=============================================================

>>> Running test-schedule (cycles=3 util=30-100 step 10)...
[test-schedule] build Oct 23 2026 14:20:49
[test-schedule] Found 9 industrial workloads (excluded 2 utility workloads)
=============================================================
[Phase 1] Measuring WCET for 9 workloads...
=============================================================
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=28.082 ms avg_latency=9360.501 us/img
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=25.212 ms avg_latency=8403.959 us/img
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=14.386 ms avg_latency=4795.459 us/img
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=12.715 ms avg_latency=4238.334 us/img
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=10.639 ms avg_latency=3546.278 us/img
  [fast]: WCET = 31.587 ms (5 iters)
the random position is:
-1.75244
 -1.8501
 1.11887

the random rotation is:
  0.996927 -0.0491981  0.0609571
 0.0169308   0.895104   0.445536
-0.0764824  -0.443135   0.893187

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.739 ms avg_latency=738.500 us/iter
the random position is:
0.986131
-1.30323
-1.18148

the random rotation is:
 0.904519 -0.186093 -0.383687
 0.302203  0.914542  0.268862
 0.300864 -0.359142  0.883458

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.680 ms avg_latency=680.167 us/iter
the random position is:
  1.27337
-0.386118
 0.420356

the random rotation is:
    0.99876   0.0496347 -0.00386858
 -0.0451141    0.935176    0.351298
  0.0210544   -0.350688    0.936256

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.710 ms avg_latency=709.625 us/iter
the random position is:
  1.36967
-0.192257
 -1.74602

the random rotation is:
 0.989714 -0.126851 0.0661499
0.0973463  0.935966  0.338366
-0.104836 -0.328446  0.938687

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.719 ms avg_latency=718.959 us/iter
the random position is:
-1.02193
 1.26675
 1.81394

the random rotation is:
 0.851945  0.507222  -0.13006
 -0.44322  0.830779  0.336693
 0.278829 -0.229199  0.932589

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.669 ms avg_latency=669.084 us/iter
  [epnp]: WCET = 4.365 ms (5 iters)
Processing...
[EKF] samples=80 total_time=9.094 ms avg_latency=113.677 us/update
Processing...
[EKF] samples=80 total_time=6.114 ms avg_latency=76.428 us/update
Processing...
[EKF] samples=80 total_time=4.851 ms avg_latency=60.642 us/update
Processing...
[EKF] samples=80 total_time=4.358 ms avg_latency=54.472 us/update
Processing...
[EKF] samples=80 total_time=3.976 ms avg_latency=49.698 us/update
  [ekf]: WCET = 10.312 ms (5 iters)

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=123.552 ms avg_latency=123.552 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=62.947 ms avg_latency=62.947 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=62.468 ms avg_latency=62.468 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=62.485 ms avg_latency=62.485 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=62.567 ms avg_latency=62.567 ms/run
  [icp]: WCET = 123.984 ms (5 iters)
[SCHED-MODBUS] offline frames=6144 total_time=0.965 ms errors=0
[SCHED-MODBUS] offline frames=6144 total_time=0.970 ms errors=0
[SCHED-MODBUS] offline frames=6144 total_time=0.958 ms errors=0
[SCHED-MODBUS] offline frames=6144 total_time=0.958 ms errors=0
[SCHED-MODBUS] offline frames=6144 total_time=0.958 ms errors=0
  [modbus]: WCET = 0.982 ms (5 iters)
[SCHED-MQTT] offline packets=512 bytes=45115 checksum=3283016638 total_time=33.569 ms
[SCHED-MQTT] offline packets=512 bytes=45115 checksum=3283016638 total_time=32.370 ms
[SCHED-MQTT] offline packets=512 bytes=45115 checksum=3283016638 total_time=32.388 ms
[SCHED-MQTT] offline packets=512 bytes=45115 checksum=3283016638 total_time=32.388 ms
[SCHED-MQTT] offline packets=512 bytes=45115 checksum=3283016638 total_time=32.368 ms
  [mqtt]: WCET = 33.693 ms (5 iters)
--- PID Simulation Start ---
[PID] samples=1000000 total_time=59.245 ms avg_latency=0.059 us/op
--- PID Simulation Start ---
[PID] samples=1000000 total_time=28.609 ms avg_latency=0.029 us/op
--- PID Simulation Start ---
[PID] samples=1000000 total_time=14.685 ms avg_latency=0.015 us/op
--- PID Simulation Start ---
[PID] samples=1000000 total_time=13.345 ms avg_latency=0.013 us/op
--- PID Simulation Start ---
[PID] samples=1000000 total_time=12.428 ms avg_latency=0.012 us/op
  [pid]: WCET = 59.276 ms (5 iters)
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.612 ms avg_latency=0.031 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=16.893 ms avg_latency=0.028 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=6.217 ms avg_latency=0.010 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=6.226 ms avg_latency=0.010 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=6.303 ms avg_latency=0.011 us/sample
  [cusum]: WCET = 18.636 ms (5 iters)
[EWMA] loops=100 stream_len_per_loop=5000 total_time=54.002 ms avg_latency=0.108 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=24.572 ms avg_latency=0.049 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=19.295 ms avg_latency=0.039 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=17.117 ms avg_latency=0.034 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=16.015 ms avg_latency=0.032 us/sample
  [ewma]: WCET = 54.022 ms (5 iters)
[Phase 1] 9 workloads measured

=============================================================
[Phase 2] Running Task Sets (U: 30% - 100%, step 10%)
=============================================================

>>> Utilization Gradient: 30% <<<
[test-schedule] uunifast sum=0.3000 target=0.3000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 123.984    | 9.99     | 1241.472
pid          | 59.276     | 7.02     | 844.259
ewma         | 54.022     | 3.96     | 1363.424
mqtt         | 33.693     | 3.52     | 958.069
fast         | 31.587     | 1.87     | 1690.775
cusum        | 18.636     | 1.52     | 1222.468
ekf          | 10.312     | 1.13     | 911.795
epnp         | 4.365      | 0.80     | 545.867
modbus       | 0.982      | 0.19     | 517.531
Running task set for 3 cycles...
[test-schedule] creating task thread 1/9 (icp)...
[test-schedule] creating task thread 2/9 (pid)...
[test-schedule] creating task thread 3/9 (ewma)...
[test-schedule] creating task thread 4/9 (mqtt)...
[test-schedule] creating task thread 5/9 (fast)...
[test-schedule] creating task thread 6/9 (cusum)...
[test-schedule] creating task thread 7/9 (ekf)...
[test-schedule] creating task thread 8/9 (epnp)...
[test-schedule] creating task thread 9/9 (modbus)...
[test-schedule] all 9 task threads created; entering watchdog
the random position is:
1.52319
-1.0664
1.38136

the random rotation is:
  0.953243 -0.0876304   -0.28922
-0.0151702   0.941953  -0.335401
  0.301823   0.324106   0.896581

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=2.298 ms avg_latency=2298.333 us/iter
--- PID Simulation Start ---
[PID] samples=1000000 total_time=99.997 ms avg_latency=0.100 us/op
the random position is:
 0.974163
-0.707437
  1.30437

the random rotation is:
 0.946131 -0.320182 -0.048152
 0.309729  0.938339 -0.153582
0.0943573  0.130395  0.986962

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=2.070 ms avg_latency=2069.667 us/iter

[ICP] Running ICP (point-to-plane)
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=52.893 ms avg_latency=0.088 us/sample
[ICP] samples=1 total_time=179.466 ms avg_latency=179.466 ms/run
[EWMA] loops=100 stream_len_per_loop=5000 total_time=70.923 ms avg_latency=0.142 us/sample
the random position is:
 -1.03893
-0.589073
 -1.50357

the random rotation is:
  0.932527  -0.299217  -0.202147
  0.312137   0.949406   0.034616
  0.181562 -0.0953778   0.978743

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.275 ms avg_latency=274.750 us/iter
--- PID Simulation Start ---
[FAST] samples=3 total_time=27.584 ms avg_latency=9194.501 us/img
[PID] samples=1000000 total_time=80.067 ms avg_latency=0.080 us/op

[ICP] Running ICP (point-to-plane)
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=77.778 ms avg_latency=0.130 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=46.540 ms avg_latency=0.047 us/op
[ICP] samples=1 total_time=152.800 ms avg_latency=152.800 ms/run
[EWMA] loops=100 stream_len_per_loop=5000 total_time=77.283 ms avg_latency=0.155 us/sample
[FAST] samples=3 total_time=31.380 ms avg_latency=10459.945 us/img

[ICP] Running ICP (point-to-plane)
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=84.701 ms avg_latency=0.141 us/sample
[ICP] samples=1 total_time=151.465 ms avg_latency=151.465 ms/run
[EWMA] loops=100 stream_len_per_loop=5000 total_time=131.943 ms avg_latency=0.264 us/sample
[test-schedule] heartbeat (still running gradient):
    fast     jobs=2/3  since_progress=1s  stall_window=5s
[FAST] samples=3 total_time=55.399 ms avg_latency=18466.488 us/img
Gradient 30% complete: MR = 0.0000 (0/27)

>>> Utilization Gradient: 40% <<<
[test-schedule] uunifast sum=0.4000 target=0.4000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 123.984    | 14.01    | 884.996
pid          | 59.276     | 8.64     | 686.196
ewma         | 54.022     | 6.00     | 900.785
mqtt         | 33.693     | 3.68     | 916.025
fast         | 31.587     | 2.82     | 1118.536
cusum        | 18.636     | 2.50     | 745.490
ekf          | 10.312     | 1.09     | 948.337
epnp         | 4.365      | 0.71     | 611.594
modbus       | 0.982      | 0.55     | 177.998
Running task set for 3 cycles...
[test-schedule] creating task thread 1/9 (icp)...
[test-schedule] creating task thread 2/9 (pid)...
[test-schedule] creating task thread 3/9 (ewma)...
[test-schedule] creating task thread 4/9 (mqtt)...
[test-schedule] creating task thread 5/9 (fast)...
[test-schedule] creating task thread 6/9 (cusum)...
[test-schedule] creating task thread 7/9 (ekf)...
[test-schedule] creating task thread 8/9 (epnp)...
[test-schedule] creating task thread 9/9 (modbus)...
[test-schedule] all 9 task threads created; entering watchdog
the random position is:
-0.0976866
 -0.231044
   1.56272

the random rotation is:
   0.969731   -0.177513   -0.167661
   0.179531    0.983747 -0.00316551
   0.165498  -0.0270308     0.98584

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=2.267 ms avg_latency=2267.417 us/iter
--- PID Simulation Start ---
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=29.320 ms avg_latency=0.049 us/sample
[PID] samples=1000000 total_time=101.470 ms avg_latency=0.101 us/op

[ICP] Running ICP (point-to-plane)
[EWMA] loops=100 stream_len_per_loop=5000 total_time=104.027 ms avg_latency=0.208 us/sample
[ICP] samples=1 total_time=130.840 ms avg_latency=130.840 ms/run
[FAST] samples=3 total_time=16.443 ms avg_latency=5481.153 us/img
the random position is:
 1.99594
 -1.0562
0.976708

the random rotation is:
 0.977769 -0.149602  0.146926
0.0748451  0.903541  0.421914
-0.195873 -0.401538  0.894651

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.536 ms avg_latency=536.375 us/iter
--- PID Simulation Start ---
[PID] samples=1000000 total_time=77.052 ms avg_latency=0.077 us/op
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=44.751 ms avg_latency=0.075 us/sample

[ICP] Running ICP (point-to-plane)
the random position is:
0.535712
0.192661
-1.43263

the random rotation is:
   0.865767   -0.451701    0.215438
   0.463609    0.886023 -0.00538582
   -0.18845    0.104542    0.976503

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.745 ms avg_latency=744.626 us/iter
[EWMA] loops=100 stream_len_per_loop=5000 total_time=79.154 ms avg_latency=0.158 us/sample
[ICP] samples=1 total_time=136.135 ms avg_latency=136.135 ms/run
--- PID Simulation Start ---
[PID] samples=1000000 total_time=74.363 ms avg_latency=0.074 us/op
[FAST] samples=3 total_time=38.426 ms avg_latency=12808.737 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=52.742 ms avg_latency=0.088 us/sample

[ICP] Running ICP (point-to-plane)
[EWMA] loops=100 stream_len_per_loop=5000 total_time=65.453 ms avg_latency=0.131 us/sample
[ICP] samples=1 total_time=138.555 ms avg_latency=138.555 ms/run
[FAST] samples=3 total_time=54.111 ms avg_latency=18037.155 us/img
Gradient 40% complete: MR = 0.0000 (0/27)

>>> Utilization Gradient: 50% <<<
[test-schedule] uunifast sum=0.5000 target=0.5000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 123.984    | 23.49    | 527.920
pid          | 59.276     | 7.77     | 762.959
ewma         | 54.022     | 5.77     | 935.816
mqtt         | 33.693     | 5.02     | 671.654
fast         | 31.587     | 3.11     | 1016.978
cusum        | 18.636     | 2.58     | 721.050
ekf          | 10.312     | 1.38     | 748.985
epnp         | 4.365      | 0.61     | 716.044
modbus       | 0.982      | 0.28     | 351.599
Running task set for 3 cycles...
[test-schedule] creating task thread 1/9 (icp)...
[test-schedule] creating task thread 2/9 (pid)...
[test-schedule] creating task thread 3/9 (ewma)...
[test-schedule] creating task thread 4/9 (mqtt)...
[test-schedule] creating task thread 5/9 (fast)...
[test-schedule] creating task thread 6/9 (cusum)...
[test-schedule] creating task thread 7/9 (ekf)...
[test-schedule] creating task thread 8/9 (epnp)...
[test-schedule] creating task thread 9/9 (modbus)...
[test-schedule] all 9 task threads created; entering watchdog

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=152.292 ms avg_latency=152.292 ms/run
the random position is:
-0.544733
 0.421679
 0.519667

the random rotation is:
 0.843224 -0.416859  0.339413
 0.402518  0.908118   0.11533
-0.356303 0.0393713   0.93354

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.918 ms avg_latency=917.875 us/iter
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=35.864 ms avg_latency=0.060 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=66.578 ms avg_latency=0.067 us/op
[EWMA] loops=100 stream_len_per_loop=5000 total_time=87.425 ms avg_latency=0.175 us/sample
[FAST] samples=3 total_time=25.290 ms avg_latency=8429.848 us/img

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=73.273 ms avg_latency=73.273 ms/run
the random position is:
  1.51465
-0.351668
 0.976724

the random rotation is:
  0.865512   0.249077  -0.434569
 -0.284111   0.958651 -0.0163912
  0.412517   0.137653   0.900489

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=2.413 ms avg_latency=2412.501 us/iter
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=39.213 ms avg_latency=0.065 us/sample
--- PID Simulation Start ---

[ICP] Running ICP (point-to-plane)
[PID] samples=1000000 total_time=70.078 ms avg_latency=0.070 us/op
[ICP] samples=1 total_time=134.631 ms avg_latency=134.631 ms/run
[EWMA] loops=100 stream_len_per_loop=5000 total_time=100.232 ms avg_latency=0.200 us/sample
[FAST] samples=3 total_time=27.572 ms avg_latency=9190.682 us/img
the random position is:
0.189478
 1.03825
-1.16809

the random rotation is:
 0.860478 -0.249704 -0.444101
 0.197865  0.967025  -0.16035
 0.469497 0.0501056  0.881511

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=2.235 ms avg_latency=2234.751 us/iter
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=36.343 ms avg_latency=0.061 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=73.783 ms avg_latency=0.074 us/op
[EWMA] loops=100 stream_len_per_loop=5000 total_time=113.786 ms avg_latency=0.228 us/sample
[FAST] samples=3 total_time=53.164 ms avg_latency=17721.474 us/img
Gradient 50% complete: MR = 0.0000 (0/27)

>>> Utilization Gradient: 60% <<<
[test-schedule] uunifast sum=0.6000 target=0.6000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 123.984    | 27.55    | 450.049
pid          | 59.276     | 15.75    | 376.426
ewma         | 54.022     | 4.50     | 1200.696
mqtt         | 33.693     | 2.99     | 1127.693
fast         | 31.587     | 2.55     | 1240.232
cusum        | 18.636     | 2.50     | 745.763
ekf          | 10.312     | 2.03     | 508.191
epnp         | 4.365      | 1.35     | 324.437
modbus       | 0.982      | 0.80     | 123.316
Running task set for 3 cycles...
[test-schedule] creating task thread 1/9 (icp)...
[test-schedule] creating task thread 2/9 (pid)...
[test-schedule] creating task thread 3/9 (ewma)...
[test-schedule] creating task thread 4/9 (mqtt)...
[test-schedule] creating task thread 5/9 (fast)...
[test-schedule] creating task thread 6/9 (cusum)...
[test-schedule] creating task thread 7/9 (ekf)...
[test-schedule] creating task thread 8/9 (epnp)...
[test-schedule] creating task thread 9/9 (modbus)...
[test-schedule] all 9 task threads created; entering watchdog
the random position is:
-1.0703
1.09822
-0.4199

the random rotation is:
  0.907737    0.40616  -0.105105
 -0.412774   0.819811  -0.396898
-0.0750378   0.403663   0.911825

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.749 ms avg_latency=748.708 us/iter
--- PID Simulation Start ---
[PID] samples=1000000 total_time=57.881 ms avg_latency=0.058 us/op

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=169.075 ms avg_latency=169.075 ms/run
the random position is:
0.710111
0.655956
-1.81463

the random rotation is:
 0.896988 -0.438949 0.0522992
 0.422848  0.817498  -0.39102
 0.128883  0.372855  0.918895

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=1.450 ms avg_latency=1449.875 us/iter
--- PID Simulation Start ---
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=19.172 ms avg_latency=0.032 us/sample
[PID] samples=1000000 total_time=74.340 ms avg_latency=0.074 us/op

[ICP] Running ICP (point-to-plane)
the random position is:
 1.44017
-1.16726
 1.86753

the random rotation is:
  0.987534  -0.141907 -0.0681178
  0.156839   0.923833   0.349191
 0.0133769  -0.355521   0.934572

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.587 ms avg_latency=587.125 us/iter
[ICP] samples=1 total_time=83.067 ms avg_latency=83.067 ms/run
--- PID Simulation Start ---
[PID] samples=1000000 total_time=63.416 ms avg_latency=0.063 us/op
[EWMA] loops=100 stream_len_per_loop=5000 total_time=52.441 ms avg_latency=0.105 us/sample
[FAST] samples=3 total_time=25.166 ms avg_latency=8388.515 us/img

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=100.267 ms avg_latency=100.267 ms/run
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=69.409 ms avg_latency=0.116 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=71.290 ms avg_latency=0.119 us/sample
[FAST] samples=3 total_time=15.450 ms avg_latency=5150.153 us/img
[EWMA] loops=100 stream_len_per_loop=5000 total_time=124.630 ms avg_latency=0.249 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=125.042 ms avg_latency=0.250 us/sample
[FAST] samples=3 total_time=23.251 ms avg_latency=7750.306 us/img
Gradient 60% complete: MR = 0.0000 (0/27)

>>> Utilization Gradient: 70% <<<
[test-schedule] uunifast sum=0.7000 target=0.7000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 123.984    | 21.66    | 572.467
pid          | 59.276     | 15.11    | 392.377
ewma         | 54.022     | 12.42    | 434.845
mqtt         | 33.693     | 8.21     | 410.247
fast         | 31.587     | 7.80     | 404.729
cusum        | 18.636     | 2.48     | 751.886
ekf          | 10.312     | 1.84     | 559.469
epnp         | 4.365      | 0.30     | 1445.353
modbus       | 0.982      | 0.17     | 574.367
Running task set for 3 cycles...
[test-schedule] creating task thread 1/9 (icp)...
[test-schedule] creating task thread 2/9 (pid)...
[test-schedule] creating task thread 3/9 (ewma)...
[test-schedule] creating task thread 4/9 (mqtt)...
[test-schedule] creating task thread 5/9 (fast)...
[test-schedule] creating task thread 6/9 (cusum)...
[test-schedule] creating task thread 7/9 (ekf)...
[test-schedule] creating task thread 8/9 (epnp)...
[test-schedule] creating task thread 9/9 (modbus)...
[test-schedule] all 9 task threads created; entering watchdog
--- PID Simulation Start ---
[FAST] samples=3 total_time=27.492 ms avg_latency=9164.070 us/img
[PID] samples=1000000 total_time=83.798 ms avg_latency=0.084 us/op
[EWMA] loops=100 stream_len_per_loop=5000 total_time=74.799 ms avg_latency=0.150 us/sample

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=158.380 ms avg_latency=158.380 ms/run
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=5.095 ms avg_latency=0.008 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=30.093 ms avg_latency=0.030 us/op
[FAST] samples=3 total_time=23.621 ms avg_latency=7873.709 us/img
[EWMA] loops=100 stream_len_per_loop=5000 total_time=36.861 ms avg_latency=0.074 us/sample

[ICP] Running ICP (point-to-plane)
--- PID Simulation Start ---
[FAST] samples=3 total_time=16.676 ms avg_latency=5558.625 us/img
[ICP] samples=1 total_time=92.221 ms avg_latency=92.221 ms/run
[PID] samples=1000000 total_time=70.182 ms avg_latency=0.070 us/op
[EWMA] loops=100 stream_len_per_loop=5000 total_time=57.312 ms avg_latency=0.115 us/sample
the random position is:
0.349305
 1.86106
0.862654

the random rotation is:
  0.921442 -0.0174003   0.388127
-0.0520266   0.984473    0.16765
 -0.385017  -0.174673   0.906229

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.181 ms avg_latency=180.542 us/iter
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=51.831 ms avg_latency=0.086 us/sample

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=91.452 ms avg_latency=91.452 ms/run
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=71.687 ms avg_latency=0.119 us/sample
the random position is:
0.720004
-1.59341
-0.89102

the random rotation is:
  0.898276  -0.184858   0.398658
   0.22314    0.97343 -0.0514097
 -0.378563   0.135137   0.915657

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=2.190 ms avg_latency=2189.542 us/iter
the random position is:
 1.02706
-1.03729
0.171614

the random rotation is:
 0.916919    0.3396  0.209599
-0.266641  0.912111 -0.311377
-0.296921   0.22962  0.926883

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=2.278 ms avg_latency=2277.625 us/iter
Gradient 70% complete: MR = 0.0000 (0/27)

>>> Utilization Gradient: 80% <<<
[test-schedule] uunifast sum=0.8000 target=0.8000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 123.984    | 35.62    | 348.107
pid          | 59.276     | 21.21    | 279.449
ewma         | 54.022     | 7.50     | 720.584
mqtt         | 33.693     | 6.34     | 531.682
fast         | 31.587     | 4.05     | 779.388
cusum        | 18.636     | 3.46     | 538.246
ekf          | 10.312     | 0.67     | 1528.384
epnp         | 4.365      | 0.58     | 757.491
modbus       | 0.982      | 0.57     | 171.837
Running task set for 3 cycles...
[test-schedule] creating task thread 1/9 (icp)...
[test-schedule] creating task thread 2/9 (pid)...
[test-schedule] creating task thread 3/9 (ewma)...
[test-schedule] creating task thread 4/9 (mqtt)...
[test-schedule] creating task thread 5/9 (fast)...
[test-schedule] creating task thread 6/9 (cusum)...
[test-schedule] creating task thread 7/9 (ekf)...
[test-schedule] creating task thread 8/9 (epnp)...
[test-schedule] creating task thread 9/9 (modbus)...
[test-schedule] all 9 task threads created; entering watchdog
--- PID Simulation Start ---

[ICP] Running ICP (point-to-plane)
[PID] samples=1000000 total_time=77.229 ms avg_latency=0.077 us/op
[ICP] samples=1 total_time=139.973 ms avg_latency=139.973 ms/run
--- PID Simulation Start ---
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=38.393 ms avg_latency=0.064 us/sample
[PID] samples=1000000 total_time=34.608 ms avg_latency=0.035 us/op

[ICP] Running ICP (point-to-plane)
the random position is:
-1.90767
-0.31138
-1.01903

the random rotation is:
  0.869165  0.0985333  -0.484606
 -0.157594   0.984046 -0.0825701
  0.468739   0.148138   0.870827

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.538 ms avg_latency=538.125 us/iter
[ICP] samples=1 total_time=66.138 ms avg_latency=66.138 ms/run
[EWMA] loops=100 stream_len_per_loop=5000 total_time=53.995 ms avg_latency=0.108 us/sample
[FAST] samples=3 total_time=16.679 ms avg_latency=5559.612 us/img
--- PID Simulation Start ---
[PID] samples=1000000 total_time=26.395 ms avg_latency=0.026 us/op

[ICP] Running ICP (point-to-plane)
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.584 ms avg_latency=0.031 us/sample
[ICP] samples=1 total_time=73.174 ms avg_latency=73.174 ms/run
the random position is:
-0.594132
-0.733605
 -1.46492

the random rotation is:
 0.896876 -0.225599  -0.38042
 0.309283  0.934762  0.174825
 0.316162 -0.274454  0.908139

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.621 ms avg_latency=620.666 us/iter
[EWMA] loops=100 stream_len_per_loop=5000 total_time=95.526 ms avg_latency=0.191 us/sample
[FAST] samples=3 total_time=25.345 ms avg_latency=8448.418 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=39.714 ms avg_latency=0.066 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.672 ms avg_latency=0.189 us/sample
the random position is:
-1.01793
0.579389
-0.62007

the random rotation is:
 0.930517  0.359765  0.068614
-0.342896  0.921585 -0.181941
 -0.12869  0.145772  0.980913

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=2.143 ms avg_latency=2143.167 us/iter
[FAST] samples=3 total_time=54.752 ms avg_latency=18250.655 us/img
Gradient 80% complete: MR = 0.0000 (0/27)

>>> Utilization Gradient: 90% <<<
[test-schedule] uunifast sum=0.9000 target=0.9000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 123.984    | 23.46    | 528.518
pid          | 59.276     | 21.90    | 270.613
ewma         | 54.022     | 13.43    | 402.390
mqtt         | 33.693     | 12.02    | 280.257
fast         | 31.587     | 10.11    | 312.575
cusum        | 18.636     | 3.03     | 614.912
ekf          | 10.312     | 2.97     | 346.672
epnp         | 4.365      | 2.97     | 147.090
modbus       | 0.982      | 0.11     | 884.139
Running task set for 3 cycles...
[test-schedule] creating task thread 1/9 (icp)...
[test-schedule] creating task thread 2/9 (pid)...
[test-schedule] creating task thread 3/9 (ewma)...
[test-schedule] creating task thread 4/9 (mqtt)...
[test-schedule] creating task thread 5/9 (fast)...
[test-schedule] creating task thread 6/9 (cusum)...
[test-schedule] creating task thread 7/9 (ekf)...
[test-schedule] creating task thread 8/9 (epnp)...
[test-schedule] creating task thread 9/9 (modbus)...
[test-schedule] all 9 task threads created; entering watchdog
the random position is:
 -0.345871
   -1.0817
-0.0994865

the random rotation is:
 0.904699 -0.423749 0.0442348
 0.385318  0.858084  0.339445
-0.181796 -0.290051  0.939585

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=2.151 ms avg_latency=2151.333 us/iter
--- PID Simulation Start ---
the random position is:
  1.92434
  1.72415
0.0098685

the random rotation is:
 0.958016  0.285744 0.0235495
 -0.28643   0.95019  0.122866
0.0127317 -0.124453  0.992144

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.817 ms avg_latency=816.958 us/iter
[FAST] samples=3 total_time=18.402 ms avg_latency=6134.014 us/img
[PID] samples=1000000 total_time=76.745 ms avg_latency=0.077 us/op
the random position is:
 -1.52568
 0.343307
-0.900536

the random rotation is:
  0.995893  0.0867708  0.0258299
-0.0751361   0.951329  -0.298878
-0.0505066    0.29571   0.953942

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=1.416 ms avg_latency=1415.750 us/iter
[EWMA] loops=100 stream_len_per_loop=5000 total_time=106.832 ms avg_latency=0.214 us/sample

[ICP] Running ICP (point-to-plane)
--- PID Simulation Start ---
[PID] samples=1000000 total_time=30.465 ms avg_latency=0.030 us/op
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=7.421 ms avg_latency=0.012 us/sample
[FAST] samples=3 total_time=13.357 ms avg_latency=4452.486 us/img
[ICP] samples=1 total_time=160.471 ms avg_latency=160.471 ms/run
--- PID Simulation Start ---
[EWMA] loops=100 stream_len_per_loop=5000 total_time=33.153 ms avg_latency=0.066 us/sample
[PID] samples=1000000 total_time=68.749 ms avg_latency=0.069 us/op
[FAST] samples=3 total_time=27.627 ms avg_latency=9208.890 us/img

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=112.512 ms avg_latency=112.512 ms/run
[EWMA] loops=100 stream_len_per_loop=5000 total_time=38.314 ms avg_latency=0.077 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=36.942 ms avg_latency=0.062 us/sample

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=148.776 ms avg_latency=148.776 ms/run
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=69.830 ms avg_latency=0.116 us/sample
Gradient 90% complete: MR = 0.0000 (0/27)

>>> Utilization Gradient: 100% <<<
[test-schedule] uunifast sum=1.0000 target=1.0000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 123.984    | 28.15    | 440.462
pid          | 59.276     | 20.53    | 288.771
ewma         | 54.022     | 14.34    | 376.836
mqtt         | 33.693     | 11.21    | 300.517
fast         | 31.587     | 9.70     | 325.640
cusum        | 18.636     | 7.75     | 240.479
ekf          | 10.312     | 5.18     | 199.198
epnp         | 4.365      | 1.64     | 265.760
modbus       | 0.982      | 1.51     | 65.128
Running task set for 3 cycles...
[test-schedule] creating task thread 1/9 (icp)...
[test-schedule] creating task thread 2/9 (pid)...
[test-schedule] creating task thread 3/9 (ewma)...
[test-schedule] creating task thread 4/9 (mqtt)...
[test-schedule] creating task thread 5/9 (fast)...
[test-schedule] creating task thread 6/9 (cusum)...
[test-schedule] creating task thread 7/9 (ekf)...
[test-schedule] creating task thread 8/9 (epnp)...
[test-schedule] creating task thread 9/9 (modbus)...
[test-schedule] all 9 task threads created; entering watchdog
the random position is:
0.373551
 1.36834
 1.43824

the random rotation is:
 0.922811 0.0978267 -0.372626
-0.240746  0.901546 -0.359523
 0.300769  0.421479  0.855508

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=1.644 ms avg_latency=1643.834 us/iter
--- PID Simulation Start ---
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=51.512 ms avg_latency=0.086 us/sample
[FAST] samples=3 total_time=18.894 ms avg_latency=6298.084 us/img
[PID] samples=1000000 total_time=78.950 ms avg_latency=0.079 us/op

[ICP] Running ICP (point-to-plane)
[EWMA] loops=100 stream_len_per_loop=5000 total_time=74.135 ms avg_latency=0.148 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.779 ms avg_latency=0.043 us/sample
the random position is:
0.704144
-1.66384
-1.75834

the random rotation is:
   0.993093   -0.116944 -0.00954587
   0.112559    0.926555    0.358924
 -0.0331293   -0.357519    0.933318

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.837 ms avg_latency=837.375 us/iter
--- PID Simulation Start ---
[ICP] samples=1 total_time=147.540 ms avg_latency=147.540 ms/run
[PID] samples=1000000 total_time=51.736 ms avg_latency=0.052 us/op
[FAST] samples=3 total_time=19.919 ms avg_latency=6639.751 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=38.127 ms avg_latency=0.064 us/sample
the random position is:
-0.118613
-0.531913
 0.823618

the random rotation is:
 0.966273 -0.119175  0.228284
 0.167005  0.964783 -0.203232
-0.196024  0.234502  0.952147

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.664 ms avg_latency=664.125 us/iter
[EWMA] loops=100 stream_len_per_loop=5000 total_time=77.172 ms avg_latency=0.154 us/sample
--- PID Simulation Start ---

[ICP] Running ICP (point-to-plane)
[PID] samples=1000000 total_time=30.836 ms avg_latency=0.031 us/op
[ICP] samples=1 total_time=84.989 ms avg_latency=84.989 ms/run
[FAST] samples=3 total_time=12.599 ms avg_latency=4199.709 us/img
[EWMA] loops=100 stream_len_per_loop=5000 total_time=36.296 ms avg_latency=0.073 us/sample

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=93.398 ms avg_latency=93.398 ms/run
Gradient 100% complete: MR = 0.0000 (0/27)

=============================================================
[Phase 3] Final Results
=============================================================
U= 30%: MR=0.0000 (0 misses / 27 jobs)
U= 40%: MR=0.0000 (0 misses / 27 jobs)
U= 50%: MR=0.0000 (0 misses / 27 jobs)
U= 60%: MR=0.0000 (0 misses / 27 jobs)
U= 70%: MR=0.0000 (0 misses / 27 jobs)
U= 80%: MR=0.0000 (0 misses / 27 jobs)
U= 90%: MR=0.0000 (0 misses / 27 jobs)
U=100%: MR=0.0000 (0 misses / 27 jobs)

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
rtos_stress: info: [matrix-0] size=64, method=all
rtos_stress: info: [matrix-1] size=64, method=all
Running Job: [======>                       ] 15/65 (23%) -> qsort
rtos_stress: debug: qsort-size set to 4096 elements
rtos_stress: info: [qsort-1] sorting 4096 elements (16 KB)
rtos_stress: info: [qsort-1] using 'all' methods
rtos_stress: info: [qsort-0] sorting 4096 elements (16 KB)
rtos_stress: info: [qsort-0] using 'all' methods
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
rtos_stress: info: [matrix-0] size=64, method=all
rtos_stress: info: [matrix-1] size=64, method=all
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
rtos_stress: info: [bsearch-1] using 16384 elements (64 KB)
rtos_stress: info: [bsearch-1] using 'all' methods
rtos_stress: info: [bsearch-2] using 16384 elements (64 KB)
rtos_stress: info: [bsearch-2] using 'all' methods
rtos_stress: info: [bsearch-0] using 'all' methods
Running Job: [==============>               ] 32/65 (49%) -> context
Running Job: [===============>              ] 33/65 (50%) -> fp
rtos_stress: info: [fp-1] float/double/long double math stressor
rtos_stress: info: [fp-2] float/double/long double math stressor
rtos_stress: info: [fp-0] float/double/long double math stressor
rtos_stress: info: [fp-0] using 'all' methods
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
rtos_stress: info: [str-0] using 'all' methods
rtos_stress: info: [str-2] using 'all' methods
rtos_stress: info: [str-1] using 'all' methods
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
rtos_stress: info: [matrix-2] size=128, method=all
rtos_stress: info: [matrix-1] size=128, method=all
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
rtos_stress: info: [bsearch-0] using 'all' methods
rtos_stress: info: [bsearch-1] using 32768 elements (128 KB)
rtos_stress: info: [bsearch-1] using 'all' methods
rtos_stress: info: [bsearch-2] using 32768 elements (128 KB)
rtos_stress: info: [bsearch-3] using 32768 elements (128 KB)
rtos_stress: info: [bsearch-2] using 'all' methods
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
rtos_stress: info: [prime-2] using 'all' methods
rtos_stress: info: [prime-1] using 'all' methods
rtos_stress: info: [prime-3] using 'all' methods
Running Job: [======================>       ] 48/65 (73%) -> stack
rtos_stress: debug: stack-size set to 32768 bytes
rtos_stress: info: [stack-0] started (stack: 32 KB, depth: ~26)
rtos_stress: info: [stack-1] started (stack: 32 KB, depth: ~26)
rtos_stress: info: [stack-2] started (stack: 32 KB, depth: ~26)
rtos_stress: info: [stack-3] started (stack: 32 KB, depth: ~26)
Running Job: [======================>       ] 49/65 (75%) -> str
rtos_stress: debug: str-size set to 8192 bytes
rtos_stress: info: [str-0] string size 8192 bytes
rtos_stress: info: [str-1] string size 8192 bytes
rtos_stress: info: [str-2] string size 8192 bytes
rtos_stress: info: [str-3] string size 8192 bytes
rtos_stress: info: [str-1] using 'all' methods
rtos_stress: info: [str-2] using 'all' methods
rtos_stress: info: [str-0] using 'all' methods
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
rtos_stress: info: [matrix-1] size=128, method=all
rtos_stress: info: [matrix-0] size=128, method=all
rtos_stress: info: [matrix-4] size=128, method=all
rtos_stress: info: [matrix-3] size=128, method=all
rtos_stress: info: [matrix-2] size=128, method=all
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
rtos_stress: info: [bitops-3] using 'all' methods
rtos_stress: info: [bitops-4] using 'all' methods
rtos_stress: info: [bitops-2] using 'all' methods
Running Job: [==========================>   ] 57/65 (87%) -> bsearch
rtos_stress: debug: bsearch-size set to 65536 elements
rtos_stress: info: [bsearch-0] using 65536 elements (256 KB)
rtos_stress: info: [bsearch-1] using 65536 elements (256 KB)
rtos_stress: info: [bsearch-2] using 65536 elements (256 KB)
rtos_stress: info: [bsearch-1] using 'all' methods
rtos_stress: info: [bsearch-3] using 65536 elements (256 KB)
rtos_stress: info: [bsearch-4] using 65536 elements (256 KB)
rtos_stress: info: [bsearch-2] using 'all' methods
rtos_stress: info: [bsearch-0] using 'all' methods
rtos_stress: info: [bsearch-4] using 'all' methods
rtos_stress: info: [bsearch-3] using 'all' methods
Running Job: [==========================>   ] 58/65 (89%) -> context
Running Job: [===========================>  ] 59/65 (90%) -> fp
rtos_stress: info: [fp-1] float/double/long double math stressor
rtos_stress: info: [fp-2] float/double/long double math stressor
rtos_stress: info: [fp-0] float/double/long double math stressor
rtos_stress: info: [fp-0] using 'all' methods
rtos_stress: info: [fp-4] float/double/long double math stressor
rtos_stress: info: [fp-3] float/double/long double math stressor
Running Job: [===========================>  ] 60/65 (92%) -> prime
rtos_stress: info: [prime-0] using 'all' methods
rtos_stress: info: [prime-1] using 'all' methods
rtos_stress: info: [prime-2] using 'all' methods
rtos_stress: info: [prime-3] using 'all' methods
rtos_stress: info: [prime-4] using 'all' methods
Running Job: [============================> ] 61/65 (93%) -> stack
rtos_stress: debug: stack-size set to 65536 bytes
rtos_stress: info: [stack-1] started (stack: 64 KB, depth: ~56)
rtos_stress: info: [stack-2] started (stack: 64 KB, depth: ~56)
rtos_stress: info: [stack-0] started (stack: 64 KB, depth: ~56)
rtos_stress: info: [stack-4] started (stack: 64 KB, depth: ~56)
rtos_stress: info: [stack-3] started (stack: 64 KB, depth: ~56)
Running Job: [============================> ] 62/65 (95%) -> str
rtos_stress: debug: str-size set to 16384 bytes
rtos_stress: info: [str-0] string size 16384 bytes
rtos_stress: info: [str-1] string size 16384 bytes
rtos_stress: info: [str-3] string size 16384 bytes
rtos_stress: info: [str-2] string size 16384 bytes
rtos_stress: info: [str-4] string size 16384 bytes
rtos_stress: info: [str-1] using 'all' methods
rtos_stress: info: [str-4] using 'all' methods
rtos_stress: info: [str-2] using 'all' methods
rtos_stress: info: [str-0] using 'all' methods
rtos_stress: info: [str-3] using 'all' methods
Running Job: [=============================>] 63/65 (96%) -> trig
rtos_stress: info: [trig-0] starting trigonometric stressor
rtos_stress: info: [trig-0] using 'all' methods
rtos_stress: info: [trig-2] starting trigonometric stressor
rtos_stress: info: [trig-2] using 'all' methods
rtos_stress: info: [trig-4] starting trigonometric stressor
rtos_stress: info: [trig-4] using 'all' methods
rtos_stress: info: [trig-1] starting trigonometric stressor
rtos_stress: info: [trig-1] using 'all' methods
rtos_stress: info: [trig-3] starting trigonometric stressor
rtos_stress: info: [trig-3] using 'all' methods
Running Job: [=============================>] 64/65 (98%) -> vecmath
Running Job: [==============================] 65/65 (100%) -> Finished


========================================================================
 Job: stored_jobfile_cpu.txt [Built-in]
========================================================================
 Stressor    | Stage | Bogo Ops     | Time(s)  | Metric
------------------------------------------------------------------------
 cpu         | 1     | 2800         | 0.47     | N/A
 matrix      | 1     | 6400         | 6.70     | N/A
 qsort       | 1     | 2500         | 3.65     | N/A
 atomic      | 1     | 3000000      | 1.32     | N/A
 bitops      | 1     | 24000        | 4.28     | N/A
 bsearch     | 1     | 3100         | 4.14     | N/A
 context     | 1     | 690000       | 0.20     | N/A
 fp          | 1     | 1050         | 5.80     | N/A
 prime       | 1     | 2700026      | 7.32     | N/A
 stack       | 1     | 6900         | 7.09     | N/A
 str         | 1     | 1300000      | 0.29     | N/A
 trig        | 1     | 210          | 0.81     | N/A
 vecmath     | 1     | 2700         | 2.78     | N/A
 cpu         | 2     | 9800         | 0.47     | N/A
 matrix      | 2     | 5600         | 3.52     | N/A
 qsort       | 2     | 1380         | 1.47     | N/A
 atomic      | 2     | 6600000      | 5.89     | N/A
 bitops      | 2     | 50000        | 4.53     | N/A
 bsearch     | 2     | 2400         | 1.99     | N/A
 context     | 2     | 880000       | 1.80     | N/A
 fp          | 2     | 1460         | 4.10     | N/A
 prime       | 2     | 4200548      | 6.02     | N/A
 stack       | 2     | 13400        | 6.97     | N/A
 str         | 2     | 750000       | 0.37     | N/A
 trig        | 2     | 320          | 0.59     | N/A
 vecmath     | 2     | 3800         | 1.98     | N/A
 cpu         | 3     | 13500        | 0.30     | N/A
 matrix      | 3     | 4800         | 2.06     | N/A
 qsort       | 3     | 300          | 0.83     | N/A
 atomic      | 3     | 5400000      | 5.51     | N/A
 bitops      | 3     | 42000        | 2.44     | N/A
 bsearch     | 3     | 420          | 0.43     | N/A
 context     | 3     | 630000       | 1.80     | N/A
 fp          | 3     | 1200         | 2.31     | N/A
 prime       | 3     | 3901236      | 3.76     | N/A
 stack       | 3     | 11100        | 4.02     | N/A
 str         | 3     | 465000       | 0.66     | N/A
 trig        | 3     | 360          | 0.50     | N/A
 vecmath     | 3     | 3900         | 1.37     | N/A
 cpu         | 4     | 17600        | 0.21     | N/A
 matrix      | 4     | 1120         | 0.55     | N/A
 qsort       | 4     | 180          | 0.89     | N/A
 atomic      | 4     | 5600000      | 9.92     | N/A
 bitops      | 4     | 42000        | 1.92     | N/A
 bsearch     | 4     | 220          | 0.28     | N/A
 context     | 4     | 600000       | 1.70     | N/A
 fp          | 4     | 1280         | 1.80     | N/A
 prime       | 4     | 4400608      | 3.20     | N/A
 stack       | 4     | 10800        | 3.16     | N/A
 str         | 4     | 660000       | 0.21     | N/A
 trig        | 4     | 500          | 0.49     | N/A
 vecmath     | 4     | 5400         | 1.42     | N/A
 cpu         | 5     | 20500        | 0.29     | N/A
 matrix      | 5     | 950          | 0.44     | N/A
 qsort       | 5     | 75           | 0.81     | N/A
 atomic      | 5     | 6000000      | 10.69    | N/A
 bitops      | 5     | 44000        | 1.62     | N/A
 bsearch     | 5     | 100          | 0.24     | N/A
 context     | 5     | 650000       | 1.60     | N/A
 fp          | 5     | 1300         | 1.66     | N/A
 prime       | 5     | 4004700      | 2.34     | N/A
 stack       | 5     | 6750         | 1.75     | N/A
 str         | 5     | 125000       | 0.27     | N/A
 trig        | 5     | 525          | 0.47     | N/A
 vecmath     | 5     | 5500         | 1.17     | N/A
------------------------------------------------------------------------
Total Run Time: 2m 39s
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
rtos_stress: info: [stream-0] bandwidth: 24901.81 MB/sec, compute: 1305.57 Mflop/sec
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
rtos_stress: info: [stream-0] bandwidth: 22521.62 MB/sec, compute: 1180.78 Mflop/sec
rtos_stress: info: [stream-1] bandwidth: 22521.07 MB/sec, compute: 1180.75 Mflop/sec
Running Job: [========>                     ] 8/30 (26%) -> vm
rtos_stress: debug: vm-bytes set to 4194304 bytes
rtos_stress: info: [vm-0] allocated 4096 KB buffer
rtos_stress: info: [vm-0] using 'all' methods
rtos_stress: info: [vm-1] allocated 4096 KB buffer
rtos_stress: info: [vm-1] using 'all' methods
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
rtos_stress: [ptr-chase-1] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-0] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-1] heartbeat: 20000768 ops
rtos_stress: [ptr-chase-0] heartbeat: 20000768 ops
rtos_stress: [ptr-chase-1] heartbeat: 30001152 ops
rtos_stress: [ptr-chase-0] heartbeat: 30001152 ops
rtos_stress: [ptr-chase-1] heartbeat: 40001536 ops
rtos_stress: [ptr-chase-0] heartbeat: 40001536 ops
rtos_stress: [ptr-chase-1] heartbeat: 50001920 ops
rtos_stress: [ptr-chase-0] heartbeat: 50001920 ops
rtos_stress: [ptr-chase-1] heartbeat: 60002304 ops
rtos_stress: [ptr-chase-0] heartbeat: 60002304 ops
rtos_stress: info: [ptr-chase-1] finished. visited 131072 of 131072 pointers (100.00%)
rtos_stress: info: [ptr-chase-0] finished. visited 131072 of 131072 pointers (100.00%)
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
rtos_stress: info: [stream-0] ramp: 1024 of 8192 elements
rtos_stress: info: [stream-1] target 8192 elements (approx 192 KB total)
rtos_stress: info: [stream-1] ramp: 1024 of 8192 elements
rtos_stress: info: [stream-2] target 8192 elements (approx 192 KB total)
rtos_stress: info: [stream-2] ramp: 1024 of 8192 elements
rtos_stress: info: [stream-0] ramp: 2048 of 8192 elements
rtos_stress: info: [stream-1] ramp: 2048 of 8192 elements
rtos_stress: info: [stream-2] ramp: 2048 of 8192 elements
rtos_stress: info: [stream-0] ramp: 4096 of 8192 elements
rtos_stress: info: [stream-1] ramp: 4096 of 8192 elements
rtos_stress: info: [stream-2] ramp: 4096 of 8192 elements
rtos_stress: info: [stream-0] ramp: 8192 of 8192 elements
rtos_stress: info: [stream-1] ramp: 8192 of 8192 elements
rtos_stress: info: [stream-2] ramp: 8192 of 8192 elements
rtos_stress: info: [stream-0] bandwidth: 22301.16 MB/sec, compute: 1169.22 Mflop/sec
rtos_stress: info: [stream-1] bandwidth: 22293.22 MB/sec, compute: 1168.81 Mflop/sec
rtos_stress: info: [stream-2] bandwidth: 22153.25 MB/sec, compute: 1161.47 Mflop/sec
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
rtos_stress: info: [malloc-2] done
rtos_stress: info: [malloc-0] done
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
rtos_stress: info: [memcpy-1] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-1] using 'all' methods
rtos_stress: info: [memcpy-0] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-0] using 'all' methods
rtos_stress: info: [memcpy-2] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-2] using 'all' methods
rtos_stress: info: [memcpy-3] buffer size: 2 x 2048 bytes
rtos_stress: info: [memcpy-3] using 'all' methods
Running Job: [===================>          ] 19/30 (63%) -> stream
rtos_stress: debug: stream-elem set to 16384 elements
rtos_stress: info: [stream-0] target 16384 elements (approx 384 KB total)
rtos_stress: info: [stream-0] ramp: 1024 of 16384 elements
rtos_stress: info: [stream-1] target 16384 elements (approx 384 KB total)
rtos_stress: info: [stream-1] ramp: 1024 of 16384 elements
rtos_stress: info: [stream-2] target 16384 elements (approx 384 KB total)
rtos_stress: info: [stream-2] ramp: 1024 of 16384 elements
rtos_stress: info: [stream-3] target 16384 elements (approx 384 KB total)
rtos_stress: info: [stream-3] ramp: 1024 of 16384 elements
rtos_stress: info: [stream-0] ramp: 2048 of 16384 elements
rtos_stress: info: [stream-1] ramp: 2048 of 16384 elements
rtos_stress: info: [stream-2] ramp: 2048 of 16384 elements
rtos_stress: info: [stream-3] ramp: 2048 of 16384 elements
rtos_stress: info: [stream-0] ramp: 4096 of 16384 elements
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
rtos_stress: info: [stream-2] bandwidth: 22542.52 MB/sec, compute: 1181.88 Mflop/sec
rtos_stress: info: [stream-3] bandwidth: 22310.73 MB/sec, compute: 1169.72 Mflop/sec
rtos_stress: info: [stream-0] bandwidth: 19511.85 MB/sec, compute: 1022.98 Mflop/sec
rtos_stress: info: [stream-1] bandwidth: 19430.35 MB/sec, compute: 1018.71 Mflop/sec
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
rtos_stress: info: [malloc-1] done
rtos_stress: info: [malloc-3] done
rtos_stress: info: [malloc-2] done
rtos_stress: info: [malloc-0] done
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
rtos_stress: [ptr-chase-3] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-2] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-3] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-3] finished. visited 131072 of 131072 pointers (100.00%)
rtos_stress: [ptr-chase-2] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-2] finished. visited 131072 of 131072 pointers (100.00%)
rtos_stress: [ptr-chase-1] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-0] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-1] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-1] finished. visited 131072 of 131072 pointers (100.00%)
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
rtos_stress: info: [stream-1] target 32768 elements (approx 768 KB total)
rtos_stress: info: [stream-1] ramp: 2048 of 32768 elements
rtos_stress: info: [stream-2] target 32768 elements (approx 768 KB total)
rtos_stress: info: [stream-2] ramp: 2048 of 32768 elements
rtos_stress: info: [stream-3] target 32768 elements (approx 768 KB total)
rtos_stress: info: [stream-3] ramp: 2048 of 32768 elements
rtos_stress: info: [stream-0] ramp: 4096 of 32768 elements
rtos_stress: info: [stream-1] ramp: 4096 of 32768 elements
rtos_stress: info: [stream-2] ramp: 4096 of 32768 elements
rtos_stress: info: [stream-3] ramp: 4096 of 32768 elements
rtos_stress: info: [stream-0] ramp: 8192 of 32768 elements
rtos_stress: info: [stream-1] ramp: 8192 of 32768 elements
rtos_stress: info: [stream-2] ramp: 8192 of 32768 elements
rtos_stress: info: [stream-3] ramp: 8192 of 32768 elements
rtos_stress: info: [stream-1] ramp: 16384 of 32768 elements
rtos_stress: info: [stream-0] ramp: 16384 of 32768 elements
rtos_stress: info: [stream-2] ramp: 16384 of 32768 elements
rtos_stress: info: [stream-3] ramp: 16384 of 32768 elements
rtos_stress: info: [stream-1] ramp: 32768 of 32768 elements
rtos_stress: info: [stream-0] ramp: 32768 of 32768 elements
rtos_stress: info: [stream-2] ramp: 32768 of 32768 elements
rtos_stress: info: [stream-3] ramp: 32768 of 32768 elements
rtos_stress: info: [stream-0] bandwidth: 4455.51 MB/sec, compute: 233.60 Mflop/sec
rtos_stress: info: [stream-2] bandwidth: 4416.11 MB/sec, compute: 231.53 Mflop/sec
rtos_stress: info: [stream-3] bandwidth: 4380.39 MB/sec, compute: 229.66 Mflop/sec
rtos_stress: info: [stream-1] bandwidth: 4387.07 MB/sec, compute: 230.01 Mflop/sec
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
rtos_stress: info: [malloc-3] max_slots=64 max_bytes=8192
rtos_stress: info: [malloc-2] max_slots=64 max_bytes=8192
rtos_stress: info: [malloc-4] max_slots=64 max_bytes=8192
rtos_stress: info: [malloc-2] done
rtos_stress: info: [malloc-0] done
rtos_stress: info: [malloc-3] done
rtos_stress: info: [malloc-1] done
rtos_stress: info: [malloc-4] done
Running Job: [============================> ] 28/30 (93%) -> memthrash
rtos_stress: info: [memthrash-0] allocated 64 KB
rtos_stress: info: [memthrash-0] using 'all' methods
rtos_stress: info: [memthrash-1] allocated 64 KB
rtos_stress: info: [memthrash-1] using 'all' methods
rtos_stress: info: [memthrash-4] allocated 64 KB
rtos_stress: info: [memthrash-4] using 'all' methods
rtos_stress: info: [memthrash-2] allocated 64 KB
rtos_stress: info: [memthrash-2] using 'all' methods
rtos_stress: info: [memthrash-3] allocated 64 KB
rtos_stress: info: [memthrash-3] using 'all' methods
Running Job: [=============================>] 29/30 (96%) -> ptr-chase
rtos_stress: debug: ptr-chase-pages set to 128
rtos_stress: info: [ptr-chase-0] chasing 128 pages (512 KB)
rtos_stress: info: [ptr-chase-1] chasing 128 pages (512 KB)
rtos_stress: info: [ptr-chase-2] chasing 128 pages (512 KB)
rtos_stress: info: [ptr-chase-3] chasing 128 pages (512 KB)
rtos_stress: info: [ptr-chase-4] chasing 128 pages (512 KB)
rtos_stress: [ptr-chase-3] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-2] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-3] heartbeat: 20000768 ops
rtos_stress: [ptr-chase-2] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-3] finished. visited 65536 of 65536 pointers (100.00%)
rtos_stress: info: [ptr-chase-2] finished. visited 65536 of 65536 pointers (100.00%)
rtos_stress: [ptr-chase-1] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-4] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-0] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-1] heartbeat: 20000768 ops
rtos_stress: [ptr-chase-4] heartbeat: 20000768 ops
rtos_stress: [ptr-chase-0] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-1] finished. visited 65536 of 65536 pointers (100.00%)
rtos_stress: info: [ptr-chase-4] finished. visited 65536 of 65536 pointers (100.00%)
rtos_stress: info: [ptr-chase-0] finished. visited 65536 of 65536 pointers (100.00%)
Running Job: [==============================] 30/30 (100%) -> Finished


========================================================================
 Job: stored_jobfile_memory.txt [Built-in]
========================================================================
 Stressor    | Stage | Bogo Ops     | Time(s)  | Metric
------------------------------------------------------------------------
 memcpy      | 1     | 10000        | 17.02    | N/A
 stream      | 1     | 52000        | 0.38     | 24901.81 MB/sec, 1305.57 Mflop/sec
 vm          | 1     | 42000384     | 1.11     | N/A
 malloc      | 1     | 50000        | 0.45     | N/A
 memthrash   | 1     | 2300         | 10.68    | N/A
 ptr-chase   | 1     | 86000640     | 0.84     | N/A
 memcpy      | 2     | 20000        | 16.27    | N/A
 stream      | 2     | 41000        | 0.33     | 45042.69 MB/sec, 2361.53 Mflop/sec
 vm          | 2     | 75000320     | 1.06     | N/A
 malloc      | 2     | 100000       | 0.45     | N/A
 memthrash   | 2     | 4400         | 10.81    | N/A
 ptr-chase   | 2     | 124000256    | 1.62     | N/A
 memcpy      | 3     | 30000        | 16.51    | N/A
 stream      | 3     | 27000        | 0.30     | 66747.63 MB/sec, 3499.50 Mflop/sec
 vm          | 3     | 78000128     | 0.99     | N/A
 malloc      | 3     | 150000       | 0.46     | N/A
 memthrash   | 3     | 5400         | 9.79     | N/A
 ptr-chase   | 3     | 82501632     | 1.91     | N/A
 memcpy      | 4     | 40000        | 15.91    | N/A
 stream      | 4     | 10800        | 0.22     | 83795.45 MB/sec, 4393.29 Mflop/sec
 vm          | 4     | 75000320     | 1.14     | N/A
 malloc      | 4     | 200000       | 0.45     | N/A
 memthrash   | 4     | 4400         | 6.53     | N/A
 ptr-chase   | 4     | 80003072     | 0.79     | N/A
 memcpy      | 5     | 50000        | 15.01    | N/A
 stream      | 5     | 4400         | 0.65     | 17639.08 MB/sec, 924.80 Mflop/sec
 vm          | 5     | 87002112     | 0.73     | N/A
 malloc      | 5     | 250000       | 0.46     | N/A
 memthrash   | 5     | 4500         | 5.39     | N/A
 ptr-chase   | 5     | 130001920    | 0.67     | N/A
------------------------------------------------------------------------
Total Run Time: 2m 18s
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
rtos_stress: info: [pipe-0] throughput: 222682.50 KB/sec
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
rtos_stress: SUMMARY: [unlink-1] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-0] fd limit hit 0 times, final limit=32/32
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
rtos_stress: info: [pipe-0] throughput: 520406.60 KB/sec
rtos_stress: info: [pipe-1] throughput: 501627.35 KB/sec
Running Job: [============>                 ] 16/40 (40%) -> hdd
rtos_stress: info: [hdd-0] testing file '0.tmp', size 64 KB
Running Job: [============>                 ] 17/40 (42%) -> open
rtos_stress: debug: open-max set to 6
rtos_stress: info: [open-0] opening up to 6 files per round (base dir: "")
rtos_stress: info: [open-1] opening up to 6 files per round (base dir: "")
rtos_stress: info: [open-2] opening up to 6 files per round (base dir: "")
Running Job: [=============>                ] 18/40 (45%) -> copy-file
rtos_stress: debug: copy-file-bytes set to 65536 bytes
rtos_stress: info: [copy-file-0] file size 65536 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-1] file size 65536 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-2] file size 65536 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-2] fsync available
rtos_stress: info: [copy-file-2] pre-allocating 65536 bytes...
rtos_stress: info: [copy-file-1] fsync available
rtos_stress: info: [copy-file-1] pre-allocating 65536 bytes...
rtos_stress: info: [copy-file-0] fsync available
rtos_stress: info: [copy-file-0] pre-allocating 65536 bytes...
rtos_stress: info: [copy-file-2] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-0] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-1] pre-alloc done, starting copy loop
Running Job: [==============>               ] 19/40 (47%) -> unlink
rtos_stress: info: [unlink-0] starting with 32 files
rtos_stress: info: [unlink-2] starting with 32 files
rtos_stress: info: [unlink-1] starting with 32 files
rtos_stress: SUMMARY: [unlink-0] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-1] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-2] fd limit hit 0 times, final limit=32/32
Running Job: [===============>              ] 20/40 (50%) -> fstat
rtos_stress: debug: fstat-files set to 6
rtos_stress: info: [fstat-0] created 6/6 files
Running Job: [===============>              ] 21/40 (52%) -> dentry
rtos_stress: debug: dentries set to 4
rtos_stress: info: [dentry-1] using 4 dentries in stress_dentry_1_dir
rtos_stress: info: [dentry-2] using 4 dentries in stress_dentry_2_dir
rtos_stress: info: [dentry-0] using 4 dentries in stress_dentry_0_dir
Running Job: [================>             ] 22/40 (55%) -> rename
rtos_stress: info: [rename-0] using dir 'R0', file size 16 bytes
rtos_stress: info: [rename-1] using dir 'R1', file size 16 bytes
Running Job: [=================>            ] 23/40 (57%) -> pipe
rtos_stress: info: [pipe-0] blocking mode chunk=32768 run_id=3
rtos_stress: info: [pipe-1] blocking mode chunk=32768 run_id=4
rtos_stress: info: [pipe-2] blocking mode chunk=32768 run_id=5
rtos_stress: info: [pipe-0] throughput: 571746.27 KB/sec
rtos_stress: info: [pipe-2] throughput: 431598.42 KB/sec
rtos_stress: info: [pipe-1] throughput: 570652.15 KB/sec
Running Job: [==================>           ] 24/40 (60%) -> hdd
rtos_stress: info: [hdd-0] testing file '0.tmp', size 128 KB
Running Job: [==================>           ] 25/40 (62%) -> open
rtos_stress: debug: open-max set to 8
rtos_stress: info: [open-0] opening up to 8 files per round (base dir: "")
rtos_stress: info: [open-1] opening up to 8 files per round (base dir: "")
rtos_stress: info: [open-3] opening up to 8 files per round (base dir: "")
rtos_stress: info: [open-2] opening up to 8 files per round (base dir: "")
Running Job: [===================>          ] 26/40 (65%) -> copy-file
rtos_stress: debug: copy-file-bytes set to 131072 bytes
rtos_stress: info: [copy-file-1] file size 131072 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-2] file size 131072 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-0] file size 131072 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-3] file size 131072 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-0] fsync available
rtos_stress: info: [copy-file-0] pre-allocating 131072 bytes...
rtos_stress: info: [copy-file-1] fsync available
rtos_stress: info: [copy-file-1] pre-allocating 131072 bytes...
rtos_stress: info: [copy-file-2] fsync available
rtos_stress: info: [copy-file-2] pre-allocating 131072 bytes...
rtos_stress: info: [copy-file-3] fsync available
rtos_stress: info: [copy-file-3] pre-allocating 131072 bytes...
rtos_stress: info: [copy-file-1] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-0] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-2] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-3] pre-alloc done, starting copy loop
Running Job: [====================>         ] 27/40 (67%) -> unlink
rtos_stress: info: [unlink-0] starting with 32 files
rtos_stress: info: [unlink-2] starting with 32 files
rtos_stress: info: [unlink-1] starting with 32 files
rtos_stress: info: [unlink-3] starting with 32 files
rtos_stress: SUMMARY: [unlink-0] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-1] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-3] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-2] fd limit hit 0 times, final limit=32/32
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
rtos_stress: info: [pipe-0] throughput: 576384.26 KB/sec
rtos_stress: info: [pipe-1] throughput: 478614.63 KB/sec
rtos_stress: info: [pipe-2] throughput: 409995.36 KB/sec
rtos_stress: info: [pipe-3] throughput: 554560.55 KB/sec
Running Job: [========================>     ] 32/40 (80%) -> hdd
rtos_stress: info: [hdd-0] testing file '0.tmp', size 128 KB
Running Job: [========================>     ] 33/40 (82%) -> open
rtos_stress: debug: open-max set to 10
rtos_stress: info: [open-0] opening up to 10 files per round (base dir: "")
rtos_stress: info: [open-1] opening up to 10 files per round (base dir: "")
rtos_stress: info: [open-3] opening up to 10 files per round (base dir: "")
rtos_stress: info: [open-4] opening up to 10 files per round (base dir: "")
rtos_stress: info: [open-2] opening up to 10 files per round (base dir: "")
Running Job: [=========================>    ] 34/40 (85%) -> copy-file
rtos_stress: debug: copy-file-bytes set to 262144 bytes
rtos_stress: info: [copy-file-1] file size 262144 bytes, chunk 4096, verify every 16 ops
rtos_stress: info: [copy-file-2] file size 262144 bytes, chunk 4096, verify every 16 ops
rtos_stress: info: [copy-file-0] file size 262144 bytes, chunk 4096, verify every 16 ops
rtos_stress: info: [copy-file-3] file size 262144 bytes, chunk 4096, verify every 16 ops
rtos_stress: info: [copy-file-4] file size 262144 bytes, chunk 4096, verify every 16 ops
rtos_stress: info: [copy-file-1] fsync available
rtos_stress: info: [copy-file-1] pre-allocating 262144 bytes...
rtos_stress: info: [copy-file-2] fsync available
rtos_stress: info: [copy-file-2] pre-allocating 262144 bytes...
rtos_stress: info: [copy-file-3] fsync available
rtos_stress: info: [copy-file-3] pre-allocating 262144 bytes...
rtos_stress: info: [copy-file-0] fsync available
rtos_stress: info: [copy-file-0] pre-allocating 262144 bytes...
rtos_stress: info: [copy-file-4] fsync available
rtos_stress: info: [copy-file-4] pre-allocating 262144 bytes...
rtos_stress: info: [copy-file-1] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-4] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-2] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-0] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-3] pre-alloc done, starting copy loop
Running Job: [==========================>   ] 35/40 (87%) -> unlink
rtos_stress: info: [unlink-1] starting with 32 files
rtos_stress: info: [unlink-0] starting with 32 files
rtos_stress: info: [unlink-3] starting with 32 files
rtos_stress: info: [unlink-2] starting with 32 files
rtos_stress: info: [unlink-4] starting with 32 files
rtos_stress: SUMMARY: [unlink-0] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-4] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-1] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-3] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-2] fd limit hit 0 times, final limit=32/32
Running Job: [===========================>  ] 36/40 (90%) -> fstat
rtos_stress: debug: fstat-files set to 10
rtos_stress: info: [fstat-0] created 10/10 files
Running Job: [===========================>  ] 37/40 (92%) -> dentry
rtos_stress: debug: dentries set to 8
rtos_stress: info: [dentry-1] using 8 dentries in stress_dentry_1_dir
rtos_stress: info: [dentry-2] using 8 dentries in stress_dentry_2_dir
rtos_stress: info: [dentry-0] using 8 dentries in stress_dentry_0_dir
rtos_stress: info: [dentry-3] using 8 dentries in stress_dentry_3_dir
Running Job: [============================> ] 38/40 (95%) -> rename
rtos_stress: info: [rename-0] using dir 'R0', file size 16 bytes
rtos_stress: info: [rename-1] using dir 'R1', file size 16 bytes
rtos_stress: info: [rename-2] using dir 'R2', file size 16 bytes
Running Job: [=============================>] 39/40 (97%) -> pipe
rtos_stress: info: [pipe-0] blocking mode chunk=32768 run_id=10
rtos_stress: info: [pipe-1] blocking mode chunk=32768 run_id=11
rtos_stress: info: [pipe-2] blocking mode chunk=32768 run_id=12
rtos_stress: info: [pipe-4] blocking mode chunk=32768 run_id=14
rtos_stress: info: [pipe-3] blocking mode chunk=32768 run_id=13
rtos_stress: info: [pipe-1] throughput: 458243.37 KB/sec
rtos_stress: info: [pipe-0] throughput: 614044.72 KB/sec
rtos_stress: info: [pipe-4] throughput: 612779.24 KB/sec
rtos_stress: info: [pipe-2] throughput: 485166.22 KB/sec
rtos_stress: info: [pipe-3] throughput: 508978.85 KB/sec
Running Job: [==============================] 40/40 (100%) -> Finished


========================================================================
 Job: stored_jobfile_file.txt [Built-in]
========================================================================
 Stressor    | Stage | Bogo Ops     | Time(s)  | Metric
------------------------------------------------------------------------
 hdd         | 1     | 480          | 6.40     | N/A
 open        | 1     | 2400         | 0.72     | N/A
 copy-file   | 1     | 2400         | 7.39     | N/A
 unlink      | 1     | 2400         | 0.73     | N/A
 fstat       | 1     | 6000         | 0.15     | N/A
 dentry      | 1     | 3000         | 0.91     | N/A
 rename      | 1     | 6000         | 6.33     | N/A
 pipe        | 1     | 44687        | 0.20     | 222682.50 KB/sec
 hdd         | 2     | 240          | 4.45     | N/A
 open        | 2     | 2400         | 0.39     | N/A
 copy-file   | 2     | 2400         | 6.34     | N/A
 unlink      | 2     | 2400         | 0.24     | N/A
 fstat       | 2     | 6000         | 0.15     | N/A
 dentry      | 2     | 3000         | 0.27     | N/A
 rename      | 2     | 6000         | 3.15     | N/A
 pipe        | 2     | 25636        | 0.20     | 1022033.95 KB/sec
 hdd         | 3     | 160          | 1.40     | N/A
 open        | 3     | 2400         | 0.22     | N/A
 copy-file   | 3     | 2400         | 4.23     | N/A
 unlink      | 3     | 2400         | 1.06     | N/A
 fstat       | 3     | 6000         | 0.15     | N/A
 dentry      | 3     | 3000         | 0.20     | N/A
 rename      | 3     | 6000         | 3.15     | N/A
 pipe        | 3     | 4949         | 0.20     | 1573996.83 KB/sec
 hdd         | 4     | 160          | 1.26     | N/A
 open        | 4     | 2400         | 0.15     | N/A
 copy-file   | 4     | 2400         | 5.69     | N/A
 unlink      | 4     | 2400         | 0.94     | N/A
 fstat       | 4     | 6000         | 0.15     | N/A
 dentry      | 4     | 3000         | 0.15     | N/A
 rename      | 4     | 6000         | 2.12     | N/A
 pipe        | 4     | 6323         | 0.20     | 2019554.81 KB/sec
 hdd         | 5     | 160          | 1.50     | N/A
 open        | 5     | 2400         | 0.13     | N/A
 copy-file   | 5     | 2400         | 5.69     | N/A
 unlink      | 5     | 2400         | 0.21     | N/A
 fstat       | 5     | 6000         | 0.15     | N/A
 dentry      | 5     | 3000         | 0.12     | N/A
 rename      | 5     | 6000         | 2.12     | N/A
 pipe        | 5     | 8391         | 0.20     | 2679212.40 KB/sec
------------------------------------------------------------------------
Total Run Time: 1m 9s

[test-stress] Job 'all' completed: 135 stressor runs
=============================================================

>>> Running test-cmd...
Fri Jan  1 16:32:53 UTC 2021
    PID TTY          TIME CMD
    593 ttyFIQ0  00:00:00 bash
    727 ttyFIQ0  00:07:45 rtos-bench-open
   1994 ttyFIQ0  00:00:00 ps
/rtosbench
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
    stub: 0.001 ms total, 0.000 ms avg
  Running workload: busywait
    busywait: 41.670 ms total, 4.167 ms avg
  Running workload: fast
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=19.483 ms avg_latency=6494.376 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=14.270 ms avg_latency=4756.667 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=15.811 ms avg_latency=5270.223 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=14.141 ms avg_latency=4713.625 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=15.753 ms avg_latency=5250.973 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=15.199 ms avg_latency=5066.250 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=15.517 ms avg_latency=5172.223 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=15.353 ms avg_latency=5117.778 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=14.119 ms avg_latency=4706.431 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=15.453 ms avg_latency=5151.028 us/img
[POSIX Thread][FAST] Benchmark thread finished.
    fast: 157.580 ms total, 15.758 ms avg
  Running workload: epnp
[POSIX Thread][ePnP] Creating benchmark thread...
the random position is:
0.756637
-0.65598
-1.70802

the random rotation is:
  0.999748  0.0220291 0.00437866
-0.0192703   0.941452  -0.336597
-0.0115372   0.336428   0.941639

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.630 ms avg_latency=630.291 us/iter
[POSIX Thread][ePnP] Benchmark thread finished.
[POSIX Thread][ePnP] Creating benchmark thread...
the random position is:
 -1.45096
0.0271841
 0.109044

the random rotation is:
 0.928712 -0.369355 0.0327352
 0.364847  0.894467 -0.258489
0.0661935  0.252005  0.965459

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.587 ms avg_latency=586.834 us/iter
[POSIX Thread][ePnP] Benchmark thread finished.
[POSIX Thread][ePnP] Creating benchmark thread...
the random position is:
-1.28576
-1.92417
 1.70154

the random rotation is:
  0.891914   0.408362   0.194241
 -0.413024   0.910547 -0.0177641
  -0.18412 -0.0643823   0.980793

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.548 ms avg_latency=548.334 us/iter
[POSIX Thread][ePnP] Benchmark thread finished.
[POSIX Thread][ePnP] Creating benchmark thread...
the random position is:
 -1.39639
0.0610511
  1.05558

the random rotation is:
 0.984296  0.174456 0.0269502
-0.143869  0.881266 -0.450191
-0.102289  0.439244  0.892526

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.493 ms avg_latency=492.625 us/iter
[POSIX Thread][ePnP] Benchmark thread finished.
[POSIX Thread][ePnP] Creating benchmark thread...
the random position is:
 -1.64647
-0.689715
 -1.04275

the random rotation is:
 0.811716 -0.545977   0.20743
 0.440376  0.805426  0.396684
 -0.38365 -0.230648  0.894212

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.530 ms avg_latency=529.666 us/iter
[POSIX Thread][ePnP] Benchmark thread finished.
[POSIX Thread][ePnP] Creating benchmark thread...
the random position is:
  1.56329
 -1.75092
0.0914641

the random rotation is:
 0.870324 -0.455663 -0.186837
 0.334572  0.825452 -0.454632
 0.361384  0.333167  0.870862

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.473 ms avg_latency=473.375 us/iter
[POSIX Thread][ePnP] Benchmark thread finished.
[POSIX Thread][ePnP] Creating benchmark thread...
the random position is:
0.385959
 1.47938
0.442675

the random rotation is:
 0.859809 -0.235131  0.453257
 0.234854  0.970308 0.0578482
-0.453401 0.0567108  0.889501

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.463 ms avg_latency=463.458 us/iter
[POSIX Thread][ePnP] Benchmark thread finished.
[POSIX Thread][ePnP] Creating benchmark thread...
the random position is:
   1.3459
 0.319486
-0.365864

the random rotation is:
 0.837768  0.500489  0.218301
-0.384147  0.824365 -0.415758
-0.388042  0.264449  0.882888

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.450 ms avg_latency=449.750 us/iter
[POSIX Thread][ePnP] Benchmark thread finished.
[POSIX Thread][ePnP] Creating benchmark thread...
the random position is:
 1.94657
0.608491
0.684684

the random rotation is:
  0.987802  -0.147291 -0.0505282
  0.154146   0.970902   0.183269
  0.022064  -0.188822   0.981763

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.467 ms avg_latency=467.250 us/iter
[POSIX Thread][ePnP] Benchmark thread finished.
[POSIX Thread][ePnP] Creating benchmark thread...
the random position is:
-1.54571
 1.48564
 1.52095

the random rotation is:
 0.983409 -0.142808  -0.11186
 0.149049  0.987589 0.0495272
 0.103398 -0.065378  0.992489

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.454 ms avg_latency=454.125 us/iter
[POSIX Thread][ePnP] Benchmark thread finished.
    epnp: 9.975 ms total, 0.998 ms avg
  Running workload: ekf
[POSIX Thread][EKF] Creating benchmark thread...
Processing...
5644000: EKF aligned, (baro hgt, IMU buf: 12, OBS buf: 9)
GPS checks passed
reset position to GPS
reset velocity to GPS
starting GPS fusion
[EKF] samples=3407 total_time=41.702 ms avg_latency=12.240 us/update
[POSIX Thread][EKF] Benchmark thread finished.
[POSIX Thread][EKF] Creating benchmark thread...
Processing...
5644000: EKF aligned, (baro hgt, IMU buf: 12, OBS buf: 9)
GPS checks passed
reset position to GPS
reset velocity to GPS
starting GPS fusion
[EKF] samples=3407 total_time=74.380 ms avg_latency=21.832 us/update
[POSIX Thread][EKF] Benchmark thread finished.
[POSIX Thread][EKF] Creating benchmark thread...
Processing...
5644000: EKF aligned, (baro hgt, IMU buf: 12, OBS buf: 9)
GPS checks passed
reset position to GPS
reset velocity to GPS
starting GPS fusion
[EKF] samples=3407 total_time=74.257 ms avg_latency=21.795 us/update
[POSIX Thread][EKF] Benchmark thread finished.
[POSIX Thread][EKF] Creating benchmark thread...
Processing...
5644000: EKF aligned, (baro hgt, IMU buf: 12, OBS buf: 9)
GPS checks passed
reset position to GPS
reset velocity to GPS
starting GPS fusion
[EKF] samples=3407 total_time=74.260 ms avg_latency=21.796 us/update
[POSIX Thread][EKF] Benchmark thread finished.
[POSIX Thread][EKF] Creating benchmark thread...
Processing...
5644000: EKF aligned, (baro hgt, IMU buf: 12, OBS buf: 9)
GPS checks passed
reset position to GPS
reset velocity to GPS
starting GPS fusion
[EKF] samples=3407 total_time=21.755 ms avg_latency=6.385 us/update
[POSIX Thread][EKF] Benchmark thread finished.
[POSIX Thread][EKF] Creating benchmark thread...
Processing...
5644000: EKF aligned, (baro hgt, IMU buf: 12, OBS buf: 9)
GPS checks passed
reset position to GPS
reset velocity to GPS
starting GPS fusion
[EKF] samples=3407 total_time=77.619 ms avg_latency=22.782 us/update
[POSIX Thread][EKF] Benchmark thread finished.
[POSIX Thread][EKF] Creating benchmark thread...
Processing...
5644000: EKF aligned, (baro hgt, IMU buf: 12, OBS buf: 9)
GPS checks passed
reset position to GPS
reset velocity to GPS
starting GPS fusion
[EKF] samples=3407 total_time=25.542 ms avg_latency=7.497 us/update
[POSIX Thread][EKF] Benchmark thread finished.
[POSIX Thread][EKF] Creating benchmark thread...
Processing...
5644000: EKF aligned, (baro hgt, IMU buf: 12, OBS buf: 9)
GPS checks passed
reset position to GPS
reset velocity to GPS
starting GPS fusion
[EKF] samples=3407 total_time=74.331 ms avg_latency=21.817 us/update
[POSIX Thread][EKF] Benchmark thread finished.
[POSIX Thread][EKF] Creating benchmark thread...
Processing...
5644000: EKF aligned, (baro hgt, IMU buf: 12, OBS buf: 9)
GPS checks passed
reset position to GPS
reset velocity to GPS
starting GPS fusion
[EKF] samples=3407 total_time=74.255 ms avg_latency=21.795 us/update
[POSIX Thread][EKF] Benchmark thread finished.
[POSIX Thread][EKF] Creating benchmark thread...
Processing...
5644000: EKF aligned, (baro hgt, IMU buf: 12, OBS buf: 9)
GPS checks passed
reset position to GPS
reset velocity to GPS
starting GPS fusion
[EKF] samples=3407 total_time=74.253 ms avg_latency=21.794 us/update
[POSIX Thread][EKF] Benchmark thread finished.
    ekf: 614.753 ms total, 61.475 ms avg
  Running workload: icp
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=197.656 ms avg_latency=197.656 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=192.632 ms avg_latency=192.632 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=192.454 ms avg_latency=192.454 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=192.436 ms avg_latency=192.436 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=192.528 ms avg_latency=192.528 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=192.678 ms avg_latency=192.678 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=91.350 ms avg_latency=91.350 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=68.256 ms avg_latency=68.256 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=68.492 ms avg_latency=68.492 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=72.605 ms avg_latency=72.605 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
    icp: 1474.607 ms total, 147.461 ms avg
  Running workload: modbus
[POSIX Thread][MODBUS][Server] Creating benchmark thread...
[POSIX Thread][MODBUS][Client] Creating benchmark thread...
[Client] Waiting for server...
[Client] Connecting to 127.0.0.1:5020...
[Client] Connected!
[Client] Modbus client created

=== R/W Benchmark Result ===
Time:       8.535 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.47
[MODBUS] samples=4 total_time=8535.031 ms avg_latency=2133.758 ms/request
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
[MODBUS] samples=4 total_time=8598.240 ms avg_latency=2149.560 ms/request
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
[MODBUS] samples=4 total_time=8598.508 ms avg_latency=2149.627 ms/request
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
[MODBUS] samples=4 total_time=8598.509 ms avg_latency=2149.627 ms/request
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
[MODBUS] samples=4 total_time=8598.542 ms avg_latency=2149.636 ms/request
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
[MODBUS] samples=4 total_time=8598.526 ms avg_latency=2149.631 ms/request
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
[MODBUS] samples=4 total_time=8598.531 ms avg_latency=2149.633 ms/request
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
[MODBUS] samples=4 total_time=8598.530 ms avg_latency=2149.633 ms/request
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
[MODBUS] samples=4 total_time=8598.536 ms avg_latency=2149.634 ms/request
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
[MODBUS] samples=4 total_time=8598.533 ms avg_latency=2149.633 ms/request
[Client] Finished
[POSIX Thread][MODBUS][Client] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Benchmark thread finished.
    modbus: 95940.120 ms total, 9594.012 ms avg
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
    mqtt: 16315.992 ms total, 1631.599 ms avg
  Running workload: pid
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=22.886 ms avg_latency=0.023 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=54.690 ms avg_latency=0.055 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=54.682 ms avg_latency=0.055 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=54.660 ms avg_latency=0.055 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=54.695 ms avg_latency=0.055 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=54.715 ms avg_latency=0.055 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=54.689 ms avg_latency=0.055 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=54.749 ms avg_latency=0.055 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=54.716 ms avg_latency=0.055 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=54.690 ms avg_latency=0.055 us/op
[POSIX Thread][PID] Benchmark thread finished.
    pid: 517.489 ms total, 51.749 ms avg
  Running workload: cusum
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.539 ms avg_latency=0.031 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.543 ms avg_latency=0.031 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.544 ms avg_latency=0.031 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.540 ms avg_latency=0.031 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.538 ms avg_latency=0.031 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.531 ms avg_latency=0.031 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.535 ms avg_latency=0.031 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.536 ms avg_latency=0.031 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.533 ms avg_latency=0.031 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=18.536 ms avg_latency=0.031 us/sample
    cusum: 186.766 ms total, 18.677 ms avg
  Running workload: ewma
[EWMA] loops=100 stream_len_per_loop=5000 total_time=66.593 ms avg_latency=0.133 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=66.592 ms avg_latency=0.133 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=66.592 ms avg_latency=0.133 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=66.591 ms avg_latency=0.133 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=66.591 ms avg_latency=0.133 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=66.622 ms avg_latency=0.133 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=66.591 ms avg_latency=0.133 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=66.605 ms avg_latency=0.133 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=66.592 ms avg_latency=0.133 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=66.592 ms avg_latency=0.133 us/sample
    ewma: 667.225 ms total, 66.723 ms avg

=============================================================
[RTOS-Bench] Results saved to: /rtosbench/result_20261023.json
=============================================================
