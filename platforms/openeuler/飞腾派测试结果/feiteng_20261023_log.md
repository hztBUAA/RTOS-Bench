[root@openEuler-phytium rtosbench]# ./rtos-bench-openeuler-feiteng-aarch64 test-all --multicore -o /rtosbench/result_20261022_2.json

=============================================================
[RTOS-Bench] Comprehensive Test Suite
=============================================================
Output: /rtosbench/result_20261022_2.json
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
信号量获取 | 0.020      | 4.480      |            |
信号量释放 | 0.040      |            | 1.460      | 4.940
消息发送   | 1.620      | 6.460      | 2.480      | 6.040
消息接收   | 1.660      | 4.980      | 2.720      | 5.580
互斥锁获取 | 0.040      | 4.100      |            |
互斥锁释放 | 0.040      |            | 1.560      | 4.680
内存块申请 | 0.040      |            |            |
内存块释放 | 0.060      |            |            |
上下文切换延迟    AVG: 1.854
中断软件延迟    MIN: 0.160  MAX: 0.640  AVG: 0.179
系统调用延迟    MIN: 8.960  MAX: 29.480  AVG: 9.739

多核存取性能:
并发度     | 1          | 2          | 4          | 8
----------------------------------------------------------------------
rd         | 1.164      | 2.324      | 2.341      | 2.335
wr         | 0.271      | 0.335      | 0.332      | 0.336
cp         | 0.275      | 0.436      | 0.431      | 0.414
frd        | 8.910      | 17.814     | 17.840     | 16.241
fwr        | 12.557     | 11.323     | 11.290     | 11.542
fcp        | 3.814      | 6.513      | 6.320      | 6.316
memset     | 11.483     | 10.895     | 11.543     | 11.120
memcpy     | 4.033      | 6.516      | 6.756      | 6.716
多核系统服务:
并发度     | 1          | 2          | 4          | 8
任务间通信 | 0.356      | 0.584      | 0.404      | 0.351
创建与删除 | 147.598    | 113.069    | 112.814    | 108.118   （单位为us）
任务在同核与异核上通信比较:
同核通信: 0.343
异核通信: 0.287
（除特殊说明外，上述单位均为GB/s）

=============================================================
[test-realtime] Benchmark completed with code: 0
=============================================================

>>> Running test-schedule (cycles=3 util=30-100 step 10)...
[test-schedule] build Oct 13 2026 12:07:50
[test-schedule] Found 9 industrial workloads (excluded 2 utility workloads)
=============================================================
[Phase 1] Measuring WCET for 9 workloads...
=============================================================
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=20.325 ms avg_latency=6775.100 us/img
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=19.913 ms avg_latency=6637.527 us/img
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=19.596 ms avg_latency=6532.007 us/img
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=19.539 ms avg_latency=6513.067 us/img
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=19.510 ms avg_latency=6503.400 us/img
  [fast]: WCET = 20.419 ms (5 iters)
the random position is:
   1.2556
-0.239033
 -1.51599

the random rotation is:
 0.949976 -0.258196  0.175727
 0.259257  0.965653 0.0172987
-0.174157 0.0291251  0.984287

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.722 ms avg_latency=722.140 us/iter
the random position is:
-1.05219
0.617048
0.255706

the random rotation is:
  0.880698 0.00172864   0.473675
 -0.199884   0.907955   0.368327
 -0.429439  -0.419065   0.799979

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.657 ms avg_latency=657.160 us/iter
the random position is:
0.736404
0.314588
-1.99024

the random rotation is:
 0.933914  0.238857  0.265991
-0.268974  0.959592 0.0826852
-0.235492 -0.148766  0.960423

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.794 ms avg_latency=793.740 us/iter
the random position is:
0.338789
-1.97761
-1.17958

the random rotation is:
  0.988164  -0.121412 -0.0937581
   0.13393   0.980849   0.141411
 0.0747937  -0.152294   0.985501

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.673 ms avg_latency=672.660 us/iter
the random position is:
 -0.138045
-0.0788203
 -0.405317

the random rotation is:
  0.985091  -0.165609 -0.0465758
    0.1684   0.983621  0.0642743
 0.0351685 -0.0711594   0.996845

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.763 ms avg_latency=762.800 us/iter
  [epnp]: WCET = 1.163 ms (5 iters)
Processing...
[EKF] samples=80 total_time=2.971 ms avg_latency=37.142 us/update
Processing...
[EKF] samples=80 total_time=2.789 ms avg_latency=34.864 us/update
Processing...
[EKF] samples=80 total_time=2.790 ms avg_latency=34.875 us/update
Processing...
[EKF] samples=80 total_time=2.813 ms avg_latency=35.158 us/update
Processing...
[EKF] samples=80 total_time=2.799 ms avg_latency=34.986 us/update
  [ekf]: WCET = 3.091 ms (5 iters)

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=154.983 ms avg_latency=154.983 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=154.471 ms avg_latency=154.471 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=155.422 ms avg_latency=155.422 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=153.772 ms avg_latency=153.772 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=153.072 ms avg_latency=153.072 ms/run
  [icp]: WCET = 156.673 ms (5 iters)
[SCHED-MODBUS] offline frames=6144 total_time=1.346 ms errors=0
[SCHED-MODBUS] offline frames=6144 total_time=1.259 ms errors=0
[SCHED-MODBUS] offline frames=6144 total_time=1.286 ms errors=0
[SCHED-MODBUS] offline frames=6144 total_time=1.257 ms errors=0
[SCHED-MODBUS] offline frames=6144 total_time=1.292 ms errors=0
  [modbus]: WCET = 1.375 ms (5 iters)
[SCHED-MQTT] offline packets=512 bytes=45115 checksum=3283016638 total_time=39.707 ms
[SCHED-MQTT] offline packets=512 bytes=45115 checksum=3283016638 total_time=39.606 ms
[SCHED-MQTT] offline packets=512 bytes=45115 checksum=3283016638 total_time=39.721 ms
[SCHED-MQTT] offline packets=512 bytes=45115 checksum=3283016638 total_time=39.648 ms
[SCHED-MQTT] offline packets=512 bytes=45115 checksum=3283016638 total_time=39.695 ms
  [mqtt]: WCET = 39.867 ms (5 iters)
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.948 ms avg_latency=0.076 us/op
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.783 ms avg_latency=0.076 us/op
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.815 ms avg_latency=0.076 us/op
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.742 ms avg_latency=0.076 us/op
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.801 ms avg_latency=0.076 us/op
  [pid]: WCET = 75.993 ms (5 iters)
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.730 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.675 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.705 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.662 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.684 ms avg_latency=0.043 us/sample
  [cusum]: WCET = 25.763 ms (5 iters)
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.055 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.090 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.006 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.004 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=93.989 ms avg_latency=0.188 us/sample
  [ewma]: WCET = 94.124 ms (5 iters)
[Phase 1] 9 workloads measured

=============================================================
[Phase 2] Running Task Sets (U: 30% - 100%, step 10%)
=============================================================

>>> Utilization Gradient: 30% <<<
[test-schedule] uunifast sum=0.3000 target=0.3000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 156.673    | 6.52     | 2402.744
ewma         | 94.124     | 5.71     | 1648.163
pid          | 75.993     | 4.14     | 1834.923
mqtt         | 39.867     | 3.87     | 1031.178
cusum        | 25.763     | 3.17     | 812.345
fast         | 20.419     | 2.54     | 804.833
ekf          | 3.091      | 2.36     | 130.793
modbus       | 1.375      | 1.22     | 112.445
epnp         | 1.163      | 0.47     | 249.720
Running task set for 3 cycles...
[test-schedule] creating task thread 1/9 (icp)...
[test-schedule] creating task thread 2/9 (ewma)...
[test-schedule] creating task thread 3/9 (pid)...
[test-schedule] creating task thread 4/9 (mqtt)...
[test-schedule] creating task thread 5/9 (cusum)...
[test-schedule] creating task thread 6/9 (fast)...
[test-schedule] creating task thread 7/9 (ekf)...
[test-schedule] creating task thread 8/9 (modbus)...
[test-schedule] creating task thread 9/9 (epnp)...
[test-schedule] all 9 task threads created; entering watchdog
the random position is:
 -1.69756
-0.570466
 -0.24701

the random rotation is:
   0.966568    0.252845  -0.0426056
  -0.256324    0.948503   -0.186118
-0.00664761    0.190817    0.981603

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.744 ms avg_latency=744.140 us/iter
the random position is:
  0.338845
-0.0413274
 -0.217952

the random rotation is:
 0.869304  0.144441  0.472702
-0.278397  0.933307  0.226788
-0.408419 -0.328746   0.85154

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.752 ms avg_latency=752.300 us/iter
the random position is:
-1.88809
0.699015
 1.97033

the random rotation is:
  0.968303  -0.249775 0.00135101
  0.247012   0.958366   0.143246
 -0.037074  -0.138372   0.989686

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.683 ms avg_latency=682.800 us/iter
[FAST] samples=3 total_time=19.475 ms avg_latency=6491.740 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.937 ms avg_latency=0.043 us/sample
[FAST] samples=3 total_time=19.487 ms avg_latency=6495.513 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.784 ms avg_latency=0.043 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.091 ms avg_latency=0.188 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.804 ms avg_latency=0.076 us/op

[ICP] Running ICP (point-to-plane)
[FAST] samples=3 total_time=20.320 ms avg_latency=6773.473 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=26.386 ms avg_latency=0.044 us/sample
[ICP] samples=1 total_time=353.248 ms avg_latency=353.248 ms/run
[EWMA] loops=100 stream_len_per_loop=5000 total_time=93.994 ms avg_latency=0.188 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.831 ms avg_latency=0.076 us/op

[ICP] Running ICP (point-to-plane)
[test-schedule] heartbeat (still running gradient):
    icp      jobs=1/3  since_progress=2s  stall_window=7s
    ewma     jobs=2/3  since_progress=1s  stall_window=5s
    pid      jobs=2/3  since_progress=1s  stall_window=5s
[EWMA] loops=100 stream_len_per_loop=5000 total_time=97.173 ms avg_latency=0.194 us/sample
[ICP] samples=1 total_time=347.005 ms avg_latency=347.005 ms/run
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.936 ms avg_latency=0.076 us/op

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=343.175 ms avg_latency=343.175 ms/run
Gradient 30% complete: MR = 0.0000 (0/27)

>>> Utilization Gradient: 40% <<<
[test-schedule] uunifast sum=0.4000 target=0.4000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 156.673    | 11.86    | 1321.580
ewma         | 94.124     | 9.82     | 958.851
pid          | 75.993     | 7.25     | 1048.271
mqtt         | 39.867     | 4.27     | 932.697
cusum        | 25.763     | 3.07     | 838.446
fast         | 20.419     | 2.53     | 805.507
ekf          | 3.091      | 0.48     | 643.606
modbus       | 1.375      | 0.40     | 344.007
epnp         | 1.163      | 0.32     | 366.808
Running task set for 3 cycles...
[test-schedule] creating task thread 1/9 (icp)...
[test-schedule] creating task thread 2/9 (ewma)...
[test-schedule] creating task thread 3/9 (pid)...
[test-schedule] creating task thread 4/9 (mqtt)...
[test-schedule] creating task thread 5/9 (cusum)...
[test-schedule] creating task thread 6/9 (fast)...
[test-schedule] creating task thread 7/9 (ekf)...
[test-schedule] creating task thread 8/9 (modbus)...
[test-schedule] creating task thread 9/9 (epnp)...
[test-schedule] all 9 task threads created; entering watchdog
the random position is:
0.0221521
  1.67726
  1.54733

the random rotation is:
  0.86878  0.387795 -0.307956
-0.472804  0.834483  -0.28301
 0.147234  0.391476  0.908333

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.680 ms avg_latency=680.280 us/iter
the random position is:
 0.820355
-0.477602
 -0.44487

the random rotation is:
 0.875964   0.38697 -0.287995
-0.386368  0.920299 0.0614043
 0.288803 0.0574841  0.955661

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.661 ms avg_latency=660.740 us/iter
[FAST] samples=3 total_time=19.433 ms avg_latency=6477.753 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.674 ms avg_latency=0.043 us/sample
--- PID Simulation Start ---
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.065 ms avg_latency=0.188 us/sample
the random position is:
-0.129258
  1.60927
 0.950427

the random rotation is:
   0.87263   0.427002  -0.237037
 -0.457097   0.885003 -0.0885009
  0.171989   0.185578   0.967461

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.788 ms avg_latency=787.860 us/iter
[PID] samples=1000000 total_time=75.869 ms avg_latency=0.076 us/op

[ICP] Running ICP (point-to-plane)
[FAST] samples=3 total_time=19.677 ms avg_latency=6559.160 us/img
[ICP] samples=1 total_time=346.093 ms avg_latency=346.093 ms/run
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.689 ms avg_latency=0.043 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.057 ms avg_latency=0.188 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.832 ms avg_latency=0.076 us/op
[FAST] samples=3 total_time=19.422 ms avg_latency=6473.940 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.661 ms avg_latency=0.043 us/sample

[ICP] Running ICP (point-to-plane)
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.164 ms avg_latency=0.188 us/sample
[ICP] samples=1 total_time=348.990 ms avg_latency=348.990 ms/run
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.884 ms avg_latency=0.076 us/op

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=343.375 ms avg_latency=343.375 ms/run
Gradient 40% complete: MR = 0.0000 (0/27)

>>> Utilization Gradient: 50% <<<
[test-schedule] uunifast sum=0.5000 target=0.5000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 156.673    | 20.60    | 760.615
ewma         | 94.124     | 10.97    | 857.864
pid          | 75.993     | 5.57     | 1363.400
mqtt         | 39.867     | 4.58     | 871.228
cusum        | 25.763     | 2.87     | 896.356
fast         | 20.419     | 2.70     | 756.163
ekf          | 3.091      | 1.70     | 182.282
modbus       | 1.375      | 0.57     | 241.489
epnp         | 1.163      | 0.44     | 264.274
Running task set for 3 cycles...
[test-schedule] creating task thread 1/9 (icp)...
[test-schedule] creating task thread 2/9 (ewma)...
[test-schedule] creating task thread 3/9 (pid)...
[test-schedule] creating task thread 4/9 (mqtt)...
[test-schedule] creating task thread 5/9 (cusum)...
[test-schedule] creating task thread 6/9 (fast)...
[test-schedule] creating task thread 7/9 (ekf)...
[test-schedule] creating task thread 8/9 (modbus)...
[test-schedule] creating task thread 9/9 (epnp)...
[test-schedule] all 9 task threads created; entering watchdog
the random position is:
-1.21906
0.749749
0.750531

the random rotation is:
  0.917577   0.396693 -0.0261946
 -0.382952   0.864254   -0.32621
 -0.106766   0.309354   0.944934

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.692 ms avg_latency=692.320 us/iter
the random position is:
 -0.58803
-0.851494
  1.78054

the random rotation is:
  0.894712    0.41474   0.165772
 -0.444443   0.863503   0.238397
-0.0442714  -0.286973   0.956915

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.723 ms avg_latency=723.060 us/iter

[ICP] Running ICP (point-to-plane)
[FAST] samples=3 total_time=19.579 ms avg_latency=6526.260 us/img
the random position is:
0.711256
 -1.8071
-1.66858

the random rotation is:
  0.925364  -0.118402  -0.360115
  0.106571   0.992912 -0.0526101
  0.363792  0.0103057   0.931423

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.692 ms avg_latency=691.760 us/iter
[EWMA] loops=100 stream_len_per_loop=5000 total_time=76.573 ms avg_latency=0.153 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=45.778 ms avg_latency=0.076 us/sample
[ICP] samples=1 total_time=348.828 ms avg_latency=348.828 ms/run
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.824 ms avg_latency=0.076 us/op

[ICP] Running ICP (point-to-plane)
[FAST] samples=3 total_time=19.575 ms avg_latency=6525.033 us/img
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.448 ms avg_latency=0.069 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=7.532 ms avg_latency=0.013 us/sample
[ICP] samples=1 total_time=349.034 ms avg_latency=349.034 ms/run

[ICP] Running ICP (point-to-plane)
[FAST] samples=3 total_time=19.621 ms avg_latency=6540.240 us/img
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.827 ms avg_latency=0.070 us/sample
[ICP] samples=1 total_time=344.093 ms avg_latency=344.093 ms/run
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=7.522 ms avg_latency=0.013 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.743 ms avg_latency=0.076 us/op
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.800 ms avg_latency=0.076 us/op
Gradient 50% complete: MR = 0.0000 (0/27)

>>> Utilization Gradient: 60% <<<
[test-schedule] uunifast sum=0.6000 target=0.6000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 156.673    | 33.86    | 462.715
ewma         | 94.124     | 6.72     | 1401.349
pid          | 75.993     | 5.96     | 1275.032
mqtt         | 39.867     | 5.27     | 756.314
cusum        | 25.763     | 4.27     | 603.017
fast         | 20.419     | 1.83     | 1115.438
ekf          | 3.091      | 1.04     | 298.662
modbus       | 1.375      | 0.93     | 147.632
epnp         | 1.163      | 0.12     | 947.484
Running task set for 3 cycles...
[test-schedule] creating task thread 1/9 (icp)...
[test-schedule] creating task thread 2/9 (ewma)...
[test-schedule] creating task thread 3/9 (pid)...
[test-schedule] creating task thread 4/9 (mqtt)...
[test-schedule] creating task thread 5/9 (cusum)...
[test-schedule] creating task thread 6/9 (fast)...
[test-schedule] creating task thread 7/9 (ekf)...
[test-schedule] creating task thread 8/9 (modbus)...
[test-schedule] creating task thread 9/9 (epnp)...
[test-schedule] all 9 task threads created; entering watchdog

[ICP] Running ICP (point-to-plane)
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=7.610 ms avg_latency=0.013 us/sample
[ICP] samples=1 total_time=150.969 ms avg_latency=150.969 ms/run

[ICP] Running ICP (point-to-plane)
the random position is:
 1.21859
-1.97532
-1.38247

the random rotation is:
  0.89661 0.0976621  0.431918
-0.212112  0.950919  0.225303
-0.388715 -0.293624  0.873319

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.690 ms avg_latency=690.400 us/iter
[ICP] samples=1 total_time=149.525 ms avg_latency=149.525 ms/run
[FAST] samples=3 total_time=12.815 ms avg_latency=4271.627 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=7.621 ms avg_latency=0.013 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=24.017 ms avg_latency=0.024 us/op

[ICP] Running ICP (point-to-plane)
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.492 ms avg_latency=0.069 us/sample
[ICP] samples=1 total_time=155.039 ms avg_latency=155.039 ms/run
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=7.605 ms avg_latency=0.013 us/sample
the random position is:
  0.54007
  1.68364
-0.807516

the random rotation is:
  0.90208 -0.418213 -0.106536
 0.427422  0.899903  0.086515
0.0596904 -0.123579  0.990538

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.725 ms avg_latency=725.460 us/iter
[FAST] samples=3 total_time=12.845 ms avg_latency=4281.633 us/img
--- PID Simulation Start ---
[PID] samples=1000000 total_time=23.990 ms avg_latency=0.024 us/op
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.507 ms avg_latency=0.069 us/sample
the random position is:
-1.57717
 1.68874
0.538305

the random rotation is:
 0.831246 -0.470684 -0.295781
 0.342783  0.852866 -0.393851
 0.437642  0.225998  0.870284

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.690 ms avg_latency=690.400 us/iter
[FAST] samples=3 total_time=12.840 ms avg_latency=4279.953 us/img
--- PID Simulation Start ---
[PID] samples=1000000 total_time=24.026 ms avg_latency=0.024 us/op
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.507 ms avg_latency=0.069 us/sample
Gradient 60% complete: MR = 0.0000 (0/27)

>>> Utilization Gradient: 70% <<<
[test-schedule] uunifast sum=0.7000 target=0.7000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 156.673    | 17.26    | 907.719
ewma         | 94.124     | 16.17    | 581.913
pid          | 75.993     | 15.65    | 485.553
mqtt         | 39.867     | 6.38     | 625.286
cusum        | 25.763     | 4.57     | 564.009
fast         | 20.419     | 3.92     | 520.598
ekf          | 3.091      | 3.17     | 97.470
modbus       | 1.375      | 2.17     | 63.354
epnp         | 1.163      | 0.71     | 164.789
Running task set for 3 cycles...
[test-schedule] creating task thread 1/9 (icp)...
[test-schedule] creating task thread 2/9 (ewma)...
[test-schedule] creating task thread 3/9 (pid)...
[test-schedule] creating task thread 4/9 (mqtt)...
[test-schedule] creating task thread 5/9 (cusum)...
[test-schedule] creating task thread 6/9 (fast)...
[test-schedule] creating task thread 7/9 (ekf)...
[test-schedule] creating task thread 8/9 (modbus)...
[test-schedule] creating task thread 9/9 (epnp)...
[test-schedule] all 9 task threads created; entering watchdog
the random position is:
-1.98567
-1.37414
0.550702

the random rotation is:
 0.866408 -0.407647 -0.288377
 0.313983   0.89382 -0.320158
 0.388269  0.186842  0.902407

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.716 ms avg_latency=716.280 us/iter
the random position is:
 1.8979
-1.4157
1.60378

the random rotation is:
  0.958067   0.276426  0.0754674
 -0.286061   0.937958   0.195969
-0.0166142   -0.20934   0.977702

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.668 ms avg_latency=668.300 us/iter
--- PID Simulation Start ---
the random position is:
0.0348539
  1.93237
  1.80237

the random rotation is:
 0.932831  0.210042  0.292761
-0.303517  0.895935  0.324312
-0.194176 -0.391386  0.899507

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.665 ms avg_latency=664.580 us/iter
[FAST] samples=3 total_time=19.383 ms avg_latency=6460.847 us/img
[PID] samples=1000000 total_time=76.179 ms avg_latency=0.076 us/op
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.677 ms avg_latency=0.043 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.111 ms avg_latency=0.188 us/sample

[ICP] Running ICP (point-to-plane)
--- PID Simulation Start ---
[PID] samples=1000000 total_time=73.885 ms avg_latency=0.074 us/op
[FAST] samples=3 total_time=19.694 ms avg_latency=6564.667 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=26.493 ms avg_latency=0.044 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=91.454 ms avg_latency=0.183 us/sample
[ICP] samples=1 total_time=347.988 ms avg_latency=347.988 ms/run
--- PID Simulation Start ---
[PID] samples=1000000 total_time=24.254 ms avg_latency=0.024 us/op
[FAST] samples=3 total_time=19.403 ms avg_latency=6467.533 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.678 ms avg_latency=0.043 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.859 ms avg_latency=0.070 us/sample

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=345.342 ms avg_latency=345.342 ms/run

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=344.352 ms avg_latency=344.352 ms/run
Gradient 70% complete: MR = 0.0000 (0/27)

>>> Utilization Gradient: 80% <<<
[test-schedule] uunifast sum=0.8000 target=0.8000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 156.673    | 38.59    | 406.031
ewma         | 94.124     | 11.07    | 850.087
pid          | 75.993     | 6.91     | 1099.983
mqtt         | 39.867     | 6.30     | 632.827
cusum        | 25.763     | 5.76     | 447.337
fast         | 20.419     | 5.06     | 403.674
ekf          | 3.091      | 4.59     | 67.366
modbus       | 1.375      | 1.25     | 110.414
epnp         | 1.163      | 0.48     | 241.913
Running task set for 3 cycles...
[test-schedule] creating task thread 1/9 (icp)...
[test-schedule] creating task thread 2/9 (ewma)...
[test-schedule] creating task thread 3/9 (pid)...
[test-schedule] creating task thread 4/9 (mqtt)...
[test-schedule] creating task thread 5/9 (cusum)...
[test-schedule] creating task thread 6/9 (fast)...
[test-schedule] creating task thread 7/9 (ekf)...
[test-schedule] creating task thread 8/9 (modbus)...
[test-schedule] creating task thread 9/9 (epnp)...
[test-schedule] all 9 task threads created; entering watchdog
the random position is:
   1.60774
   1.48788
-0.0637771

the random rotation is:
  0.932325   0.101414  -0.347109
-0.0169116   0.971048   0.238285
  0.361225  -0.216289   0.907048

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.637 ms avg_latency=637.200 us/iter

[ICP] Running ICP (point-to-plane)
[FAST] samples=3 total_time=12.587 ms avg_latency=4195.527 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=7.528 ms avg_latency=0.013 us/sample
the random position is:
-1.70872
0.342516
-1.91793

the random rotation is:
 0.945132  0.244902 -0.216214
 -0.13216  0.891886  0.432519
 0.298763 -0.380213  0.875316

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.756 ms avg_latency=756.140 us/iter
[ICP] samples=1 total_time=258.701 ms avg_latency=258.701 ms/run
the random position is:
-0.132383
 -1.06589
 0.470141

the random rotation is:
 0.829563 -0.482647 -0.280852
 0.424707  0.871873 -0.243848
  0.36256 0.0830069  0.928257

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.823 ms avg_latency=822.560 us/iter

[ICP] Running ICP (point-to-plane)
[FAST] samples=3 total_time=12.599 ms avg_latency=4199.507 us/img
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.392 ms avg_latency=0.069 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=7.533 ms avg_latency=0.013 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=23.997 ms avg_latency=0.024 us/op
[ICP] samples=1 total_time=352.527 ms avg_latency=352.527 ms/run

[ICP] Running ICP (point-to-plane)
[FAST] samples=3 total_time=12.620 ms avg_latency=4206.793 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=7.526 ms avg_latency=0.013 us/sample
[ICP] samples=1 total_time=352.995 ms avg_latency=352.995 ms/run
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.493 ms avg_latency=0.069 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=24.004 ms avg_latency=0.024 us/op
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.428 ms avg_latency=0.069 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=24.003 ms avg_latency=0.024 us/op
Gradient 80% complete: MR = 0.0000 (0/27)

>>> Utilization Gradient: 90% <<<
[test-schedule] uunifast sum=0.9000 target=0.9000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 156.673    | 34.76    | 450.786
ewma         | 94.124     | 15.39    | 611.671
pid          | 75.993     | 14.81    | 512.990
mqtt         | 39.867     | 10.19    | 391.387
cusum        | 25.763     | 5.31     | 485.339
fast         | 20.419     | 5.09     | 401.059
ekf          | 3.091      | 3.17     | 97.483
modbus       | 1.375      | 1.24     | 110.921
epnp         | 1.163      | 0.05     | 10000.000
Running task set for 3 cycles...
[test-schedule] creating task thread 1/9 (icp)...
[test-schedule] creating task thread 2/9 (ewma)...
[test-schedule] creating task thread 3/9 (pid)...
[test-schedule] creating task thread 4/9 (mqtt)...
[test-schedule] creating task thread 5/9 (cusum)...
[test-schedule] creating task thread 6/9 (fast)...
[test-schedule] creating task thread 7/9 (ekf)...
[test-schedule] creating task thread 8/9 (modbus)...
[test-schedule] creating task thread 9/9 (epnp)...
[test-schedule] all 9 task threads created; entering watchdog
[FAST] samples=3 total_time=12.562 ms avg_latency=4187.347 us/img

[ICP] Running ICP (point-to-plane)
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=7.526 ms avg_latency=0.013 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=24.014 ms avg_latency=0.024 us/op
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.482 ms avg_latency=0.069 us/sample
[ICP] samples=1 total_time=244.687 ms avg_latency=244.687 ms/run
[FAST] samples=3 total_time=12.564 ms avg_latency=4188.047 us/img

[ICP] Running ICP (point-to-plane)
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=7.524 ms avg_latency=0.013 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=23.990 ms avg_latency=0.024 us/op
[FAST] samples=3 total_time=12.899 ms avg_latency=4299.520 us/img
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.489 ms avg_latency=0.069 us/sample
[ICP] samples=1 total_time=363.909 ms avg_latency=363.909 ms/run

[ICP] Running ICP (point-to-plane)
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=7.534 ms avg_latency=0.013 us/sample
--- PID Simulation Start ---
[PID] samples=1000000 total_time=24.053 ms avg_latency=0.024 us/op
[ICP] samples=1 total_time=358.126 ms avg_latency=358.126 ms/run
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.428 ms avg_latency=0.069 us/sample
[test-schedule] heartbeat (still running gradient):
    epnp     jobs=0/3  since_progress=5s  stall_window=30s
the random position is:
-0.742759
-0.229771
   0.1134

the random rotation is:
 0.839309  0.534077  0.101597
-0.425878  0.762056 -0.487747
-0.337918  0.366103  0.867053

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.712 ms avg_latency=711.540 us/iter
[test-schedule] heartbeat (still running gradient):
    epnp     jobs=1/3  since_progress=0s  stall_window=30s
[test-schedule] heartbeat (still running gradient):
    epnp     jobs=1/3  since_progress=5s  stall_window=30s
the random position is:
  1.51042
-0.840662
  1.26681

the random rotation is:
 0.866829  0.455737 -0.202264
-0.320208  0.819774  0.474803
 0.382196 -0.346807  0.856535

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.685 ms avg_latency=684.860 us/iter
[test-schedule] heartbeat (still running gradient):
    epnp     jobs=2/3  since_progress=0s  stall_window=30s
[test-schedule] heartbeat (still running gradient):
    epnp     jobs=2/3  since_progress=5s  stall_window=30s
the random position is:
  1.51042
-0.840662
  1.26681

the random rotation is:
 0.866829  0.455737 -0.202264
-0.320208  0.819774  0.474803
 0.382196 -0.346807  0.856535

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.681 ms avg_latency=680.600 us/iter
[test-schedule] heartbeat (still running gradient):
Gradient 90% complete: MR = 0.0000 (0/27)

>>> Utilization Gradient: 100% <<<
[test-schedule] uunifast sum=1.0000 target=1.0000
Name         | WCET(ms)   | Util(%)  | Period(ms)
------------------------------------------------------
icp          | 156.673    | 33.94    | 461.615
ewma         | 94.124     | 22.90    | 410.931
pid          | 75.993     | 16.85    | 450.936
mqtt         | 39.867     | 11.60    | 343.768
cusum        | 25.763     | 5.86     | 439.888
fast         | 20.419     | 4.45     | 459.120
ekf          | 3.091      | 2.49     | 123.909
modbus       | 1.375      | 1.08     | 127.356
epnp         | 1.163      | 0.83     | 140.763
Running task set for 3 cycles...
[test-schedule] creating task thread 1/9 (icp)...
[test-schedule] creating task thread 2/9 (ewma)...
[test-schedule] creating task thread 3/9 (pid)...
[test-schedule] creating task thread 4/9 (mqtt)...
[test-schedule] creating task thread 5/9 (cusum)...
[test-schedule] creating task thread 6/9 (fast)...
[test-schedule] creating task thread 7/9 (ekf)...
[test-schedule] creating task thread 8/9 (modbus)...
[test-schedule] creating task thread 9/9 (epnp)...
[test-schedule] all 9 task threads created; entering watchdog
the random position is:
 1.28776
 1.61247
-1.33554

the random rotation is:
 0.941328   0.31262 -0.127163
 -0.28574  0.938747  0.192633
 0.179595 -0.144995  0.972996

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.652 ms avg_latency=652.000 us/iter
the random position is:
  1.68725
 0.717361
-0.823308

the random rotation is:
 0.937364  0.331081  0.108327
-0.347922  0.905245  0.243891
-0.017315 -0.266304  0.963734

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.763 ms avg_latency=762.520 us/iter
the random position is:
-1.01866
 1.14495
0.133292

the random rotation is:
   0.88904    0.41866   0.185291
 -0.449146   0.876007   0.175726
-0.0887467   -0.23945   0.966844

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.722 ms avg_latency=722.340 us/iter
--- PID Simulation Start ---

[ICP] Running ICP (point-to-plane)
[FAST] samples=3 total_time=24.626 ms avg_latency=8208.673 us/img
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=49.199 ms avg_latency=0.082 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=105.853 ms avg_latency=0.212 us/sample
[PID] samples=1000000 total_time=97.150 ms avg_latency=0.097 us/op
[ICP] samples=1 total_time=169.749 ms avg_latency=169.749 ms/run
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=7.526 ms avg_latency=0.013 us/sample
--- PID Simulation Start ---
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.025 ms avg_latency=0.188 us/sample

[ICP] Running ICP (point-to-plane)
[FAST] samples=3 total_time=12.577 ms avg_latency=4192.360 us/img
[PID] samples=1000000 total_time=76.159 ms avg_latency=0.076 us/op
[ICP] samples=1 total_time=146.988 ms avg_latency=146.988 ms/run
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.006 ms avg_latency=0.188 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=7.543 ms avg_latency=0.013 us/sample
--- PID Simulation Start ---

[ICP] Running ICP (point-to-plane)
[FAST] samples=3 total_time=12.585 ms avg_latency=4195.060 us/img
[PID] samples=1000000 total_time=75.899 ms avg_latency=0.076 us/op
[ICP] samples=1 total_time=148.730 ms avg_latency=148.730 ms/run
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
rtos_stress: info: [stack-1] started (stack: 8 KB, depth: ~3)
rtos_stress: info: [stack-0] started (stack: 8 KB, depth: ~3)
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
rtos_stress: info: [matrix-0] size=64, method=all
rtos_stress: info: [matrix-1] size=64, method=all
rtos_stress: info: [matrix-2] size=64, method=all
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
rtos_stress: info: [fp-0] using 'all' methods
rtos_stress: info: [fp-1] float/double/long double math stressor
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
rtos_stress: info: [matrix-2] size=128, method=all
rtos_stress: info: [matrix-0] size=128, method=all
rtos_stress: info: [matrix-1] size=128, method=all
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
rtos_stress: info: [bsearch-1] using 32768 elements (128 KB)
rtos_stress: info: [bsearch-0] using 'all' methods
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
rtos_stress: info: [str-0] using 'all' methods
rtos_stress: info: [str-2] using 'all' methods
rtos_stress: info: [str-1] using 'all' methods
rtos_stress: info: [str-3] string size 8192 bytes
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
rtos_stress: info: [bsearch-2] using 65536 elements (256 KB)
rtos_stress: info: [bsearch-3] using 65536 elements (256 KB)
rtos_stress: info: [bsearch-1] using 'all' methods
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
rtos_stress: info: [stack-1] started (stack: 64 KB, depth: ~56)
rtos_stress: info: [stack-0] started (stack: 64 KB, depth: ~56)
rtos_stress: info: [stack-3] started (stack: 64 KB, depth: ~56)
rtos_stress: info: [stack-2] started (stack: 64 KB, depth: ~56)
rtos_stress: info: [stack-4] started (stack: 64 KB, depth: ~56)
Running Job: [============================> ] 62/65 (95%) -> str
rtos_stress: debug: str-size set to 16384 bytes
rtos_stress: info: [str-0] string size 16384 bytes
rtos_stress: info: [str-1] string size 16384 bytes
rtos_stress: info: [str-2] string size 16384 bytes
rtos_stress: info: [str-3] string size 16384 bytes
rtos_stress: info: [str-1] using 'all' methods
rtos_stress: info: [str-3] using 'all' methods
rtos_stress: info: [str-0] using 'all' methods
rtos_stress: info: [str-2] using 'all' methods
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
 matrix      | 1     | 6400         | 6.56     | N/A
 qsort       | 1     | 2500         | 3.22     | N/A
 atomic      | 1     | 3000000      | 1.37     | N/A
 bitops      | 1     | 24000        | 3.77     | N/A
 bsearch     | 1     | 3100         | 3.47     | N/A
 context     | 1     | 690000       | 0.20     | N/A
 fp          | 1     | 1050         | 8.75     | N/A
 prime       | 1     | 2700026      | 7.09     | N/A
 stack       | 1     | 6900         | 7.05     | N/A
 str         | 1     | 1300000      | 0.32     | N/A
 trig        | 1     | 210          | 1.18     | N/A
 vecmath     | 1     | 2700         | 2.76     | N/A
 cpu         | 2     | 9800         | 0.47     | N/A
 matrix      | 2     | 5600         | 3.37     | N/A
 qsort       | 2     | 1380         | 1.57     | N/A
 atomic      | 2     | 6600000      | 14.08    | N/A
 bitops      | 2     | 50000        | 4.05     | N/A
 bsearch     | 2     | 2400         | 1.78     | N/A
 context     | 2     | 880000       | 3.30     | N/A
 fp          | 2     | 1460         | 6.15     | N/A
 prime       | 2     | 4200548      | 5.60     | N/A
 stack       | 2     | 13400        | 6.90     | N/A
 str         | 2     | 750000       | 0.68     | N/A
 trig        | 2     | 320          | 1.91     | N/A
 vecmath     | 2     | 3800         | 1.96     | N/A
 cpu         | 3     | 13500        | 0.59     | N/A
 matrix      | 3     | 4800         | 1.95     | N/A
 qsort       | 3     | 300          | 1.59     | N/A
 atomic      | 3     | 5400000      | 10.38    | N/A
 bitops      | 3     | 42000        | 2.29     | N/A
 bsearch     | 3     | 420          | 0.71     | N/A
 context     | 3     | 630000       | 3.20     | N/A
 fp          | 3     | 1200         | 5.29     | N/A
 prime       | 3     | 3901236      | 3.49     | N/A
 stack       | 3     | 11100        | 3.94     | N/A
 str         | 3     | 465000       | 0.56     | N/A
 trig        | 3     | 360          | 1.45     | N/A
 vecmath     | 3     | 3900         | 1.36     | N/A
 cpu         | 4     | 17600        | 0.46     | N/A
 matrix      | 4     | 1120         | 1.30     | N/A
 qsort       | 4     | 180          | 1.90     | N/A
 atomic      | 4     | 5600000      | 17.11    | N/A
 bitops      | 4     | 42000        | 1.73     | N/A
 bsearch     | 4     | 220          | 0.55     | N/A
 context     | 4     | 600000       | 1.40     | N/A
 fp          | 4     | 1280         | 4.23     | N/A
 prime       | 4     | 4400608      | 3.00     | N/A
 stack       | 4     | 10800        | 3.04     | N/A
 str         | 4     | 660000       | 0.63     | N/A
 trig        | 4     | 500          | 1.51     | N/A
 vecmath     | 4     | 5400         | 1.41     | N/A
 cpu         | 5     | 20500        | 0.36     | N/A
 matrix      | 5     | 950          | 0.80     | N/A
 qsort       | 5     | 75           | 1.88     | N/A
 atomic      | 5     | 6000000      | 17.79    | N/A
 bitops      | 5     | 44000        | 1.46     | N/A
 bsearch     | 5     | 100          | 0.38     | N/A
 context     | 5     | 650000       | 1.40     | N/A
 fp          | 5     | 1300         | 3.13     | N/A
 prime       | 5     | 4004700      | 2.20     | N/A
 stack       | 5     | 6750         | 1.70     | N/A
 str         | 5     | 125000       | 0.27     | N/A
 trig        | 5     | 525          | 0.90     | N/A
 vecmath     | 5     | 5500         | 1.16     | N/A
------------------------------------------------------------------------
Total Run Time: 3m 27s
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
rtos_stress: info: [stream-0] bandwidth: 3054.45 MB/sec, compute: 160.14 Mflop/sec
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
rtos_stress: info: [stream-1] ramp: 2048 of 4096 elements
rtos_stress: info: [stream-1] ramp: 4096 of 4096 elements
rtos_stress: info: [stream-0] ramp: 2048 of 4096 elements
rtos_stress: info: [stream-0] ramp: 4096 of 4096 elements
rtos_stress: info: [stream-1] bandwidth: 11565.85 MB/sec, compute: 606.38 Mflop/sec
rtos_stress: info: [stream-0] bandwidth: 2870.35 MB/sec, compute: 150.49 Mflop/sec
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
rtos_stress: info: [malloc-0] done
rtos_stress: info: [malloc-1] done
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
rtos_stress: info: [stream-0] ramp: 2048 of 8192 elements
rtos_stress: info: [stream-0] ramp: 4096 of 8192 elements
rtos_stress: info: [stream-0] ramp: 8192 of 8192 elements
rtos_stress: info: [stream-1] target 8192 elements (approx 192 KB total)
rtos_stress: info: [stream-1] ramp: 1024 of 8192 elements
rtos_stress: info: [stream-1] ramp: 2048 of 8192 elements
rtos_stress: info: [stream-1] ramp: 4096 of 8192 elements
rtos_stress: info: [stream-1] ramp: 8192 of 8192 elements
rtos_stress: info: [stream-2] target 8192 elements (approx 192 KB total)
rtos_stress: info: [stream-2] ramp: 1024 of 8192 elements
rtos_stress: info: [stream-2] ramp: 2048 of 8192 elements
rtos_stress: info: [stream-2] ramp: 4096 of 8192 elements
rtos_stress: info: [stream-2] ramp: 8192 of 8192 elements
rtos_stress: info: [stream-2] bandwidth: 11792.40 MB/sec, compute: 618.26 Mflop/sec
rtos_stress: info: [stream-0] bandwidth: 2234.58 MB/sec, compute: 117.16 Mflop/sec
rtos_stress: info: [stream-1] bandwidth: 2232.22 MB/sec, compute: 117.03 Mflop/sec
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
rtos_stress: info: [malloc-0] done
rtos_stress: info: [malloc-2] done
rtos_stress: info: [malloc-1] done
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
rtos_stress: [ptr-chase-2] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-0] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-1] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-1] finished. visited 262144 of 262144 pointers (100.00%)
rtos_stress: [ptr-chase-0] heartbeat: 20000768 ops
rtos_stress: [ptr-chase-2] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-0] finished. visited 262144 of 262144 pointers (100.00%)
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
rtos_stress: info: [stream-1] target 16384 elements (approx 384 KB total)
rtos_stress: info: [stream-1] ramp: 1024 of 16384 elements
rtos_stress: info: [stream-1] ramp: 2048 of 16384 elements
rtos_stress: info: [stream-0] ramp: 4096 of 16384 elements
rtos_stress: info: [stream-1] ramp: 4096 of 16384 elements
rtos_stress: info: [stream-0] ramp: 8192 of 16384 elements
rtos_stress: info: [stream-1] ramp: 8192 of 16384 elements
rtos_stress: info: [stream-0] ramp: 16384 of 16384 elements
rtos_stress: info: [stream-1] ramp: 16384 of 16384 elements
rtos_stress: info: [stream-2] target 16384 elements (approx 384 KB total)
rtos_stress: info: [stream-2] ramp: 1024 of 16384 elements
rtos_stress: info: [stream-2] ramp: 2048 of 16384 elements
rtos_stress: info: [stream-2] ramp: 4096 of 16384 elements
rtos_stress: info: [stream-2] ramp: 8192 of 16384 elements
rtos_stress: info: [stream-2] ramp: 16384 of 16384 elements
rtos_stress: info: [stream-3] target 16384 elements (approx 384 KB total)
rtos_stress: info: [stream-3] ramp: 1024 of 16384 elements
rtos_stress: info: [stream-3] ramp: 2048 of 16384 elements
rtos_stress: info: [stream-3] ramp: 4096 of 16384 elements
rtos_stress: info: [stream-3] ramp: 8192 of 16384 elements
rtos_stress: info: [stream-3] ramp: 16384 of 16384 elements
rtos_stress: info: [stream-2] bandwidth: 11828.47 MB/sec, compute: 620.15 Mflop/sec
rtos_stress: info: [stream-3] bandwidth: 11395.48 MB/sec, compute: 597.45 Mflop/sec
rtos_stress: info: [stream-0] bandwidth: 1939.41 MB/sec, compute: 101.68 Mflop/sec
rtos_stress: info: [stream-1] bandwidth: 1914.24 MB/sec, compute: 100.36 Mflop/sec
Running Job: [====================>         ] 20/30 (66%) -> vm
rtos_stress: debug: vm-bytes set to 4194304 bytes
rtos_stress: info: [vm-1] allocated 4096 KB buffer
rtos_stress: info: [vm-1] using 'all' methods
rtos_stress: info: [vm-0] allocated 4096 KB buffer
rtos_stress: info: [vm-0] using 'all' methods
Running Job: [=====================>        ] 21/30 (70%) -> malloc
rtos_stress: debug: malloc-bytes set to 4096
rtos_stress: debug: malloc-max set to 64 slots
rtos_stress: info: [malloc-0] max_slots=64 max_bytes=4096
rtos_stress: info: [malloc-1] max_slots=64 max_bytes=4096
rtos_stress: info: [malloc-2] max_slots=64 max_bytes=4096
rtos_stress: info: [malloc-3] max_slots=64 max_bytes=4096
rtos_stress: info: [malloc-1] done
rtos_stress: info: [malloc-3] done
rtos_stress: info: [malloc-0] done
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
rtos_stress: [ptr-chase-2] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-1] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-2] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-2] finished. visited 131072 of 131072 pointers (100.00%)
rtos_stress: [ptr-chase-1] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-1] finished. visited 131072 of 131072 pointers (100.00%)
rtos_stress: [ptr-chase-0] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-3] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-0] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-0] finished. visited 131072 of 131072 pointers (100.00%)
rtos_stress: [ptr-chase-3] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-3] finished. visited 131072 of 131072 pointers (100.00%)
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
rtos_stress: info: [stream-3] target 32768 elements (approx 768 KB total)
rtos_stress: info: [stream-3] ramp: 2048 of 32768 elements
rtos_stress: info: [stream-2] ramp: 4096 of 32768 elements
rtos_stress: info: [stream-3] ramp: 4096 of 32768 elements
rtos_stress: info: [stream-2] ramp: 8192 of 32768 elements
rtos_stress: info: [stream-3] ramp: 8192 of 32768 elements
rtos_stress: info: [stream-2] ramp: 16384 of 32768 elements
rtos_stress: info: [stream-3] ramp: 16384 of 32768 elements
rtos_stress: info: [stream-2] ramp: 32768 of 32768 elements
rtos_stress: info: [stream-3] ramp: 32768 of 32768 elements
rtos_stress: info: [stream-2] bandwidth: 11801.57 MB/sec, compute: 618.74 Mflop/sec
rtos_stress: info: [stream-3] bandwidth: 11286.94 MB/sec, compute: 591.76 Mflop/sec
rtos_stress: info: [stream-0] bandwidth: 1945.77 MB/sec, compute: 102.01 Mflop/sec
rtos_stress: info: [stream-1] bandwidth: 1930.05 MB/sec, compute: 101.19 Mflop/sec
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
rtos_stress: info: [malloc-3] done
rtos_stress: info: [malloc-4] done
rtos_stress: info: [malloc-1] done
rtos_stress: info: [malloc-2] done
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
rtos_stress: info: [ptr-chase-3] chasing 128 pages (512 KB)
rtos_stress: [ptr-chase-0] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-1] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-0] heartbeat: 20000768 ops
rtos_stress: [ptr-chase-1] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-0] finished. visited 65536 of 65536 pointers (100.00%)
rtos_stress: info: [ptr-chase-1] finished. visited 65536 of 65536 pointers (100.00%)
rtos_stress: info: [ptr-chase-4] chasing 128 pages (512 KB)
rtos_stress: [ptr-chase-4] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-4] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-4] finished. visited 65536 of 65536 pointers (100.00%)
rtos_stress: [ptr-chase-2] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-3] heartbeat: 10000384 ops
rtos_stress: [ptr-chase-2] heartbeat: 20000768 ops
rtos_stress: [ptr-chase-3] heartbeat: 20000768 ops
rtos_stress: info: [ptr-chase-2] finished. visited 65536 of 65536 pointers (100.00%)
rtos_stress: info: [ptr-chase-3] finished. visited 65536 of 65536 pointers (100.00%)
Running Job: [==============================] 30/30 (100%) -> Finished


========================================================================
 Job: stored_jobfile_memory.txt [Built-in]
========================================================================
 Stressor    | Stage | Bogo Ops     | Time(s)  | Metric
------------------------------------------------------------------------
 memcpy      | 1     | 10000        | 16.12    | N/A
 stream      | 1     | 52000        | 2.71     | 3054.45 MB/sec, 160.14 Mflop/sec
 vm          | 1     | 42000384     | 1.91     | N/A
 malloc      | 1     | 50000        | 0.45     | N/A
 memthrash   | 1     | 2300         | 10.16    | N/A
 ptr-chase   | 1     | 86000640     | 8.76     | N/A
 memcpy      | 2     | 20000        | 22.89    | N/A
 stream      | 2     | 41000        | 2.28     | 14436.20 MB/sec, 756.87 Mflop/sec
 vm          | 2     | 75000320     | 1.99     | N/A
 malloc      | 2     | 100000       | 0.45     | N/A
 memthrash   | 2     | 4400         | 10.06    | N/A
 ptr-chase   | 2     | 124000256    | 1.22     | N/A
 memcpy      | 3     | 30000        | 22.90    | N/A
 stream      | 3     | 27000        | 2.60     | 16259.20 MB/sec, 852.45 Mflop/sec
 vm          | 3     | 78000128     | 1.94     | N/A
 malloc      | 3     | 150000       | 0.45     | N/A
 memthrash   | 3     | 5400         | 8.62     | N/A
 ptr-chase   | 3     | 82501632     | 4.78     | N/A
 memcpy      | 4     | 40000        | 22.94    | N/A
 stream      | 4     | 10800        | 1.78     | 27077.60 MB/sec, 1419.65 Mflop/sec
 vm          | 4     | 75000320     | 1.99     | N/A
 malloc      | 4     | 200000       | 0.45     | N/A
 memthrash   | 4     | 4400         | 5.59     | N/A
 ptr-chase   | 4     | 80003072     | 3.07     | N/A
 memcpy      | 5     | 50000        | 21.14    | N/A
 stream      | 5     | 4400         | 1.49     | 26964.34 MB/sec, 1413.71 Mflop/sec
 vm          | 5     | 87002112     | 1.44     | N/A
 malloc      | 5     | 250000       | 0.45     | N/A
 memthrash   | 5     | 4500         | 4.62     | N/A
 ptr-chase   | 5     | 130001920    | 3.41     | N/A
------------------------------------------------------------------------
Total Run Time: 3m 8s
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
rtos_stress: info: [pipe-0] throughput: 192131.51 KB/sec
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
rtos_stress: info: [copy-file-0] fsync available
rtos_stress: info: [copy-file-0] pre-allocating 32768 bytes...
rtos_stress: info: [copy-file-1] fsync available
rtos_stress: info: [copy-file-1] pre-allocating 32768 bytes...
rtos_stress: info: [copy-file-0] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-1] pre-alloc done, starting copy loop
Running Job: [========>                     ] 11/40 (27%) -> unlink
rtos_stress: info: [unlink-1] starting with 32 files
rtos_stress: info: [unlink-0] starting with 32 files
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
rtos_stress: info: [pipe-1] blocking mode chunk=4096 run_id=2
rtos_stress: info: [pipe-0] blocking mode chunk=4096 run_id=1
rtos_stress: info: [pipe-1] throughput: 536624.06 KB/sec
rtos_stress: info: [pipe-0] throughput: 573052.77 KB/sec
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
rtos_stress: info: [copy-file-0] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-2] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-1] pre-alloc done, starting copy loop
Running Job: [==============>               ] 19/40 (47%) -> unlink
rtos_stress: info: [unlink-0] starting with 32 files
rtos_stress: info: [unlink-1] starting with 32 files
rtos_stress: info: [unlink-2] starting with 32 files
rtos_stress: SUMMARY: [unlink-0] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-1] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-2] fd limit hit 0 times, final limit=32/32
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
rtos_stress: info: [pipe-2] throughput: 205980.29 KB/sec
rtos_stress: info: [pipe-0] throughput: 205993.82 KB/sec
rtos_stress: info: [pipe-1] throughput: 428220.44 KB/sec
Running Job: [==================>           ] 24/40 (60%) -> hdd
rtos_stress: info: [hdd-0] testing file '0.tmp', size 128 KB
Running Job: [==================>           ] 25/40 (62%) -> open
rtos_stress: debug: open-max set to 8
rtos_stress: info: [open-1] opening up to 8 files per round (base dir: "")
rtos_stress: info: [open-0] opening up to 8 files per round (base dir: "")
rtos_stress: info: [open-2] opening up to 8 files per round (base dir: "")
rtos_stress: info: [open-3] opening up to 8 files per round (base dir: "")
Running Job: [===================>          ] 26/40 (65%) -> copy-file
rtos_stress: debug: copy-file-bytes set to 131072 bytes
rtos_stress: info: [copy-file-1] file size 131072 bytes, chunk 8192, verify every 8 ops
rtos_stress: info: [copy-file-0] file size 131072 bytes, chunk 8192, verify every 8 ops
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
rtos_stress: info: [copy-file-3] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-1] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-2] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-0] pre-alloc done, starting copy loop
Running Job: [====================>         ] 27/40 (67%) -> unlink
rtos_stress: info: [unlink-0] starting with 32 files
rtos_stress: info: [unlink-1] starting with 32 files
rtos_stress: info: [unlink-2] starting with 32 files
rtos_stress: info: [unlink-3] starting with 32 files
rtos_stress: SUMMARY: [unlink-1] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-0] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-2] fd limit hit 0 times, final limit=32/32
rtos_stress: SUMMARY: [unlink-3] fd limit hit 0 times, final limit=32/32
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
rtos_stress: info: [pipe-0] throughput: 334406.79 KB/sec
rtos_stress: info: [pipe-2] throughput: 211704.26 KB/sec
rtos_stress: info: [pipe-1] throughput: 378156.51 KB/sec
rtos_stress: info: [pipe-3] throughput: 467004.70 KB/sec
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
rtos_stress: info: [copy-file-2] fsync available
rtos_stress: info: [copy-file-2] pre-allocating 262144 bytes...
rtos_stress: info: [copy-file-1] fsync available
rtos_stress: info: [copy-file-1] pre-allocating 262144 bytes...
rtos_stress: info: [copy-file-0] fsync available
rtos_stress: info: [copy-file-0] pre-allocating 262144 bytes...
rtos_stress: info: [copy-file-3] fsync available
rtos_stress: info: [copy-file-3] pre-allocating 262144 bytes...
rtos_stress: info: [copy-file-4] fsync available
rtos_stress: info: [copy-file-4] pre-allocating 262144 bytes...
rtos_stress: info: [copy-file-1] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-2] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-0] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-4] pre-alloc done, starting copy loop
rtos_stress: info: [copy-file-3] pre-alloc done, starting copy loop
Running Job: [==========================>   ] 35/40 (87%) -> unlink
rtos_stress: info: [unlink-0] starting with 32 files
rtos_stress: info: [unlink-4] starting with 32 files
rtos_stress: info: [unlink-1] starting with 32 files
rtos_stress: info: [unlink-2] starting with 32 files
rtos_stress: info: [unlink-3] starting with 32 files
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
rtos_stress: info: [dentry-0] using 8 dentries in stress_dentry_0_dir
rtos_stress: info: [dentry-1] using 8 dentries in stress_dentry_1_dir
rtos_stress: info: [dentry-2] using 8 dentries in stress_dentry_2_dir
rtos_stress: info: [dentry-3] using 8 dentries in stress_dentry_3_dir
Running Job: [============================> ] 38/40 (95%) -> rename
rtos_stress: info: [rename-0] using dir 'R0', file size 16 bytes
rtos_stress: info: [rename-1] using dir 'R1', file size 16 bytes
rtos_stress: info: [rename-2] using dir 'R2', file size 16 bytes
Running Job: [=============================>] 39/40 (97%) -> pipe
rtos_stress: info: [pipe-0] blocking mode chunk=32768 run_id=10
rtos_stress: info: [pipe-1] blocking mode chunk=32768 run_id=11
rtos_stress: info: [pipe-2] blocking mode chunk=32768 run_id=12
rtos_stress: info: [pipe-3] blocking mode chunk=32768 run_id=13
rtos_stress: info: [pipe-0] throughput: 381852.13 KB/sec
rtos_stress: info: [pipe-1] throughput: 381910.61 KB/sec
rtos_stress: info: [pipe-4] blocking mode chunk=32768 run_id=14
rtos_stress: info: [pipe-3] throughput: 386468.08 KB/sec
rtos_stress: info: [pipe-2] throughput: 390950.72 KB/sec
rtos_stress: info: [pipe-4] throughput: 461322.36 KB/sec
Running Job: [==============================] 40/40 (100%) -> Finished


========================================================================
 Job: stored_jobfile_file.txt [Built-in]
========================================================================
 Stressor    | Stage | Bogo Ops     | Time(s)  | Metric
------------------------------------------------------------------------
 hdd         | 1     | 480          | 5.07     | N/A
 open        | 1     | 2400         | 0.75     | N/A
 copy-file   | 1     | 2400         | 9.83     | N/A
 unlink      | 1     | 2400         | 0.49     | N/A
 fstat       | 1     | 6000         | 0.15     | N/A
 dentry      | 1     | 3000         | 0.91     | N/A
 rename      | 1     | 6000         | 6.24     | N/A
 pipe        | 1     | 38445        | 0.20     | 192131.51 KB/sec
 hdd         | 2     | 240          | 2.44     | N/A
 open        | 2     | 2400         | 0.41     | N/A
 copy-file   | 2     | 2400         | 8.55     | N/A
 unlink      | 2     | 2400         | 0.35     | N/A
 fstat       | 2     | 6000         | 0.15     | N/A
 dentry      | 2     | 3000         | 0.30     | N/A
 rename      | 2     | 6000         | 3.15     | N/A
 pipe        | 2     | 29286        | 0.21     | 1109676.83 KB/sec
 hdd         | 3     | 160          | 3.61     | N/A
 open        | 3     | 2400         | 0.22     | N/A
 copy-file   | 3     | 2400         | 15.54    | N/A
 unlink      | 3     | 2400         | 0.43     | N/A
 fstat       | 3     | 6000         | 0.15     | N/A
 dentry      | 3     | 3000         | 0.22     | N/A
 rename      | 3     | 6000         | 3.15     | N/A
 pipe        | 3     | 2635         | 0.20     | 840194.56 KB/sec
 hdd         | 4     | 160          | 1.06     | N/A
 open        | 4     | 2400         | 0.16     | N/A
 copy-file   | 4     | 2400         | 7.95     | N/A
 unlink      | 4     | 2400         | 0.29     | N/A
 fstat       | 4     | 6000         | 0.15     | N/A
 dentry      | 4     | 3000         | 0.16     | N/A
 rename      | 4     | 6000         | 2.12     | N/A
 pipe        | 4     | 4351         | 0.25     | 1391272.26 KB/sec
 hdd         | 5     | 160          | 1.05     | N/A
 open        | 5     | 2400         | 0.13     | N/A
 copy-file   | 5     | 2400         | 7.31     | N/A
 unlink      | 5     | 2400         | 0.27     | N/A
 fstat       | 5     | 6000         | 0.15     | N/A
 dentry      | 5     | 3000         | 0.14     | N/A
 rename      | 5     | 6000         | 2.12     | N/A
 pipe        | 5     | 6292         | 0.30     | 2002503.91 KB/sec
------------------------------------------------------------------------
Total Run Time: 1m 26s

[test-stress] Job 'all' completed: 135 stressor runs
=============================================================

>>> Running test-cmd...
Thu Jan  1 00:43:12 UTC 1970
    PID TTY          TIME CMD
    429 pts/0    00:00:00 bash
   1317 pts/0    00:10:24 rtos-bench-open
   2578 pts/0    00:00:00 ps
/rtosbench
logs                  rtos-bench-openeuler-feiteng-aarch64       test_cmd_dir
result_20261022.json  rtos-bench-openeuler-feiteng-aarch64.prev
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
[FAST] samples=3 total_time=12.602 ms avg_latency=4200.647 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=19.553 ms avg_latency=6517.807 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=12.605 ms avg_latency=4201.727 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=19.429 ms avg_latency=6476.433 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=12.725 ms avg_latency=4241.740 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=19.506 ms avg_latency=6501.880 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=12.814 ms avg_latency=4271.420 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=19.414 ms avg_latency=6471.220 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=12.881 ms avg_latency=4293.753 us/img
[POSIX Thread][FAST] Benchmark thread finished.
[POSIX Thread][FAST] Creating benchmark thread...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)
[FAST] samples=3 total_time=19.499 ms avg_latency=6499.573 us/img
[POSIX Thread][FAST] Benchmark thread finished.
    fast: 162.524 ms total, 16.252 ms avg
  Running workload: epnp
[POSIX Thread][ePnP] Creating benchmark thread...
the random position is:
-1.35929
  -1.625
 1.83503

the random rotation is:
  0.972627  -0.163639  -0.164982
  0.172139    0.98432  0.0385125
  0.156093 -0.0658582   0.985544

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.277 ms avg_latency=277.040 us/iter
[POSIX Thread][ePnP] Benchmark thread finished.
[POSIX Thread][ePnP] Creating benchmark thread...
the random position is:
   0.2504
-0.595506
-0.280524

the random rotation is:
  0.947073  -0.210215   0.242614
  0.226261   0.973247 -0.0399588
 -0.227724  0.0927381     0.9693

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.241 ms avg_latency=240.800 us/iter
[POSIX Thread][ePnP] Benchmark thread finished.
[POSIX Thread][ePnP] Creating benchmark thread...
the random position is:
  -1.9245
-0.866927
-0.347798

the random rotation is:
  0.947834  -0.107169   0.300208
   0.11029   0.993878 0.00658195
 -0.299075  0.0268715   0.953851

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.237 ms avg_latency=236.920 us/iter
[POSIX Thread][ePnP] Benchmark thread finished.
[POSIX Thread][ePnP] Creating benchmark thread...
the random position is:
  1.63717
-0.546496
 0.417157

the random rotation is:
 0.917537  0.215081 -0.334463
-0.113454  0.947733  0.298211
 0.381121 -0.235674  0.893982

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.655 ms avg_latency=655.120 us/iter
[POSIX Thread][ePnP] Benchmark thread finished.
[POSIX Thread][ePnP] Creating benchmark thread...
the random position is:
  1.01972
 -1.37834
-0.256502

the random rotation is:
 0.864814   0.18956 -0.464933
-0.205474  0.978519 0.0167589
 0.458123 0.0810382  0.885187

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.254 ms avg_latency=253.520 us/iter
[POSIX Thread][ePnP] Benchmark thread finished.
[POSIX Thread][ePnP] Creating benchmark thread...
the random position is:
 1.10275
 1.72034
-1.17736

the random rotation is:
 0.961529  0.196426  -0.19204
-0.234196  0.951534 -0.199337
 0.143578  0.236643  0.960929

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.247 ms avg_latency=246.880 us/iter
[POSIX Thread][ePnP] Benchmark thread finished.
[POSIX Thread][ePnP] Creating benchmark thread...
the random position is:
-0.234142
  1.18452
 -1.29996

the random rotation is:
 0.865026  0.312166  0.392787
-0.431372  0.862527   0.26451
-0.256218 -0.398246  0.880768

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.221 ms avg_latency=220.980 us/iter
[POSIX Thread][ePnP] Benchmark thread finished.
[POSIX Thread][ePnP] Creating benchmark thread...
the random position is:
  0.85519
 0.719147
-0.527722

the random rotation is:
  0.914743  -0.387269   0.115189
  0.397646   0.913419 -0.0868552
-0.0715798   0.125255   0.989539

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.242 ms avg_latency=241.940 us/iter
[POSIX Thread][ePnP] Benchmark thread finished.
[POSIX Thread][ePnP] Creating benchmark thread...
the random position is:
 1.65716
-1.55236
 1.25362

the random rotation is:
  0.983724   0.117979   0.135527
 -0.127003   0.990088  0.0599614
  -0.12711 -0.0761978   0.988958

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.232 ms avg_latency=232.360 us/iter
[POSIX Thread][ePnP] Benchmark thread finished.
[POSIX Thread][ePnP] Creating benchmark thread...
the random position is:
-0.639654
-0.548805
  1.73981

the random rotation is:
  0.921357   0.388009  0.0234711
 -0.380626   0.912785  -0.148148
-0.0789067   0.127563   0.988687

the noise in the data is:
0
the outlier fraction is:
0
Running ePnP (using all 100 correspondences)...
[ePnP] samples=1 total_time=0.232 ms avg_latency=232.480 us/iter
[POSIX Thread][ePnP] Benchmark thread finished.
    epnp: 5.407 ms total, 0.541 ms avg
  Running workload: ekf
[POSIX Thread][EKF] Creating benchmark thread...
Processing...
5644000: EKF aligned, (baro hgt, IMU buf: 12, OBS buf: 9)
GPS checks passed
reset position to GPS
reset velocity to GPS
starting GPS fusion
[EKF] samples=3407 total_time=35.132 ms avg_latency=10.312 us/update
[POSIX Thread][EKF] Benchmark thread finished.
[POSIX Thread][EKF] Creating benchmark thread...
Processing...
5644000: EKF aligned, (baro hgt, IMU buf: 12, OBS buf: 9)
GPS checks passed
reset position to GPS
reset velocity to GPS
starting GPS fusion
[EKF] samples=3407 total_time=35.106 ms avg_latency=10.304 us/update
[POSIX Thread][EKF] Benchmark thread finished.
[POSIX Thread][EKF] Creating benchmark thread...
Processing...
5644000: EKF aligned, (baro hgt, IMU buf: 12, OBS buf: 9)
GPS checks passed
reset position to GPS
reset velocity to GPS
starting GPS fusion
[EKF] samples=3407 total_time=35.179 ms avg_latency=10.326 us/update
[POSIX Thread][EKF] Benchmark thread finished.
[POSIX Thread][EKF] Creating benchmark thread...
Processing...
5644000: EKF aligned, (baro hgt, IMU buf: 12, OBS buf: 9)
GPS checks passed
reset position to GPS
reset velocity to GPS
starting GPS fusion
[EKF] samples=3407 total_time=35.060 ms avg_latency=10.291 us/update
[POSIX Thread][EKF] Benchmark thread finished.
[POSIX Thread][EKF] Creating benchmark thread...
Processing...
5644000: EKF aligned, (baro hgt, IMU buf: 12, OBS buf: 9)
GPS checks passed
reset position to GPS
reset velocity to GPS
starting GPS fusion
[EKF] samples=3407 total_time=35.079 ms avg_latency=10.296 us/update
[POSIX Thread][EKF] Benchmark thread finished.
[POSIX Thread][EKF] Creating benchmark thread...
Processing...
5644000: EKF aligned, (baro hgt, IMU buf: 12, OBS buf: 9)
GPS checks passed
reset position to GPS
reset velocity to GPS
starting GPS fusion
[EKF] samples=3407 total_time=35.119 ms avg_latency=10.308 us/update
[POSIX Thread][EKF] Benchmark thread finished.
[POSIX Thread][EKF] Creating benchmark thread...
Processing...
5644000: EKF aligned, (baro hgt, IMU buf: 12, OBS buf: 9)
GPS checks passed
reset position to GPS
reset velocity to GPS
starting GPS fusion
[EKF] samples=3407 total_time=35.173 ms avg_latency=10.324 us/update
[POSIX Thread][EKF] Benchmark thread finished.
[POSIX Thread][EKF] Creating benchmark thread...
Processing...
5644000: EKF aligned, (baro hgt, IMU buf: 12, OBS buf: 9)
GPS checks passed
reset position to GPS
reset velocity to GPS
starting GPS fusion
[EKF] samples=3407 total_time=35.077 ms avg_latency=10.296 us/update
[POSIX Thread][EKF] Benchmark thread finished.
[POSIX Thread][EKF] Creating benchmark thread...
Processing...
5644000: EKF aligned, (baro hgt, IMU buf: 12, OBS buf: 9)
GPS checks passed
reset position to GPS
reset velocity to GPS
starting GPS fusion
[EKF] samples=3407 total_time=35.131 ms avg_latency=10.311 us/update
[POSIX Thread][EKF] Benchmark thread finished.
[POSIX Thread][EKF] Creating benchmark thread...
Processing...
5644000: EKF aligned, (baro hgt, IMU buf: 12, OBS buf: 9)
GPS checks passed
reset position to GPS
reset velocity to GPS
starting GPS fusion
[EKF] samples=3407 total_time=35.138 ms avg_latency=10.314 us/update
[POSIX Thread][EKF] Benchmark thread finished.
    ekf: 352.220 ms total, 35.222 ms avg
  Running workload: icp
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=148.222 ms avg_latency=148.222 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=355.297 ms avg_latency=355.297 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=147.089 ms avg_latency=147.089 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=147.432 ms avg_latency=147.432 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=147.454 ms avg_latency=147.454 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=356.644 ms avg_latency=356.644 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=146.978 ms avg_latency=146.978 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=354.891 ms avg_latency=354.891 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=355.902 ms avg_latency=355.902 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
[POSIX Thread][ICP] Creating benchmark thread...

[ICP] Running ICP (point-to-plane)
[ICP] samples=1 total_time=355.010 ms avg_latency=355.010 ms/run
[POSIX Thread][ICP] Benchmark thread finished.
    icp: 2533.287 ms total, 253.329 ms avg
  Running workload: modbus
[POSIX Thread][MODBUS][Server] Creating benchmark thread...
[POSIX Thread][MODBUS][Client] Creating benchmark thread...
[Client] Waiting for server...
[Client] Connecting to 127.0.0.1:5020...
[Client] Connected!
[Client] Modbus client created

=== R/W Benchmark Result ===
Time:       8.049 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.50
[MODBUS] samples=4 total_time=8048.940 ms avg_latency=2012.235 ms/request
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
Time:       8.056 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.50
[MODBUS] samples=4 total_time=8055.636 ms avg_latency=2013.909 ms/request
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
[MODBUS] samples=4 total_time=8055.487 ms avg_latency=2013.872 ms/request
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
[MODBUS] samples=4 total_time=8055.484 ms avg_latency=2013.871 ms/request
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
[MODBUS] samples=4 total_time=8055.445 ms avg_latency=2013.861 ms/request
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
Time:       8.056 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.50
[MODBUS] samples=4 total_time=8055.522 ms avg_latency=2013.881 ms/request
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
[MODBUS] samples=4 total_time=8055.483 ms avg_latency=2013.871 ms/request
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
[MODBUS] samples=4 total_time=8055.385 ms avg_latency=2013.846 ms/request
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
[MODBUS] samples=4 total_time=8055.358 ms avg_latency=2013.839 ms/request
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
Time:       8.056 s
Requests:   4 (Write+Read)
Errors:     0
TPS:        0.50
[MODBUS] samples=4 total_time=8055.517 ms avg_latency=2013.879 ms/request
[Client] Finished
[POSIX Thread][MODBUS][Client] Benchmark thread finished.
[POSIX Thread][MODBUS][Server] Benchmark thread finished.
    modbus: 90553.487 ms total, 9055.349 ms avg
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
    mqtt: 2.299 ms total, 0.230 ms avg
  Running workload: pid
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.973 ms avg_latency=0.076 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=23.991 ms avg_latency=0.024 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.721 ms avg_latency=0.076 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.778 ms avg_latency=0.076 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.800 ms avg_latency=0.076 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.814 ms avg_latency=0.076 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=23.987 ms avg_latency=0.024 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.794 ms avg_latency=0.076 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.819 ms avg_latency=0.076 us/op
[POSIX Thread][PID] Benchmark thread finished.
[POSIX Thread][PID] Creating benchmark thread...
--- PID Simulation Start ---
[PID] samples=1000000 total_time=75.810 ms avg_latency=0.076 us/op
[POSIX Thread][PID] Benchmark thread finished.
    pid: 655.935 ms total, 65.593 ms avg
  Running workload: cusum
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.732 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.685 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.731 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.672 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.727 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.665 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.723 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.679 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.720 ms avg_latency=0.043 us/sample
[CUSUM] loops=100 stream_len_per_loop=6000 total_time=25.686 ms avg_latency=0.043 us/sample
    cusum: 258.444 ms total, 25.844 ms avg
  Running workload: ewma
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.001 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.023 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.002 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.008 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=93.995 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=93.983 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=34.323 ms avg_latency=0.069 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=93.987 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.005 ms avg_latency=0.188 us/sample
[EWMA] loops=100 stream_len_per_loop=5000 total_time=94.008 ms avg_latency=0.188 us/sample
    ewma: 881.847 ms total, 88.185 ms avg

=============================================================
[RTOS-Bench] Results saved to: /rtosbench/result_20261022_2.json
=============================================================