+ 复现流程（尽量还原）： 以前pull的一个旧的分支-执行install.sh-修改了rtthread配置（pthreads最大数量改成20000）-发现可调度性测试报错，每次报错不一样-pull新分支-仍然有报错（其中至少有一次是报错信息1）-回退rtthread配置（pthreads最大数量改成了8）-能够执行更多测试轮次了，但仍然报错，与报错信息1相似（报错信息2）。

+ 修改 scons --menuconfig 中的配置是
```
[*] Enable pthreads APIs
(8)     Maximum number of pthreads    *这一条
```



报错信息1：
执行中输出：
```
rtbench test-schedule
[test-schedule] Starting schedulability test
  Cycles: 10000, Utilization: 30% - 100% (step 10%)
=============================================================
[Phase 1] Measuring WCET for 10 workloads...
=============================================================
  [stub]: WCET = 10.000 ms
  [busywait]: WCET = 10.000 ms
[POSIX] Starting FAST Benchmark ...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)

| Image           | Size      | Corners  | Time(us)  | FPS     |
|-----------------|-----------|----------|-----------|---------|
| leuven          | 640 x480  | 5631     | Function[_rt_mutex_take]: scheduler is not available
(0) assertion failed at function:_rt_mutex_take, line number:1334 
please use: addr2line -e rtthread.elf -a -f
 0x40158334 0x4015387c 0x40153db8 0x4011b27c 0x4011b618 0x4011b690 0x4011ef3c 0x40082c28 0x4024ad40 0x402497cc 0x402483cc 0x40260210 0x4024a164 0x400ea944 0x401d8828 0x4018d674 0x4018e1b4 0x4016e8dc 0x400d268c 0x4014d974 0x4014da10 0x40150b50 0x400caa78
```

addr2line检查报错输出：
```
addr2line -e rtthread.elf -a -f 0x40158334 0x4015387c 0x40153db8 0x4011b27c 0x4011b618 0x4011b690 0x4011ef3c 0x40082c28 0x4024ad40 0x402497cc 0x402483cc 0x40260210 0x4024a164 0x400ea7a0 0x401d8828 0x4018d674 0x4018e1b4 0x4016e8dc 0x400d268c 0x4014d974 0x4014da10 0x40150b50 0x400caa78
0x0000000040158334
rt_assert_handler
/home/orangepuzzle/rtos/rtos-bench/RTOS-Bench/extern/rt-thread/src/kservice.c:1233
0x000000004015387c
_rt_mutex_take
/home/orangepuzzle/rtos/rtos-bench/RTOS-Bench/extern/rt-thread/src/ipc.c:1334 (discriminator 4)
0x0000000040153db8
rt_mutex_take
/home/orangepuzzle/rtos/rtos-bench/RTOS-Bench/extern/rt-thread/src/ipc.c:1541
0x000000004011b27c
dfs_file_lock
/home/orangepuzzle/rtos/rtos-bench/RTOS-Bench/extern/rt-thread/components/dfs/dfs_v1/src/dfs.c:144
0x000000004011b618
fdt_fd_get
/home/orangepuzzle/rtos/rtos-bench/RTOS-Bench/extern/rt-thread/components/dfs/dfs_v1/src/dfs.c:354
0x000000004011b690
fd_get
/home/orangepuzzle/rtos/rtos-bench/RTOS-Bench/extern/rt-thread/components/dfs/dfs_v1/src/dfs.c:374
0x000000004011ef3c
write
/home/orangepuzzle/rtos/rtos-bench/RTOS-Bench/extern/rt-thread/components/dfs/dfs_v1/src/dfs_posix.c:249
0x0000000040082c28
_write_r
/home/orangepuzzle/rtos/rtos-bench/RTOS-Bench/extern/rt-thread/components/libc/compilers/newlib/syscalls.c:318
0x000000004024ad40
__swrite
??:?
0x00000000402497cc
__sfvwrite_r
??:?
0x00000000402483cc
__sprint_r
??:?
0x0000000040260210
_vfprintf_r
??:?
0x000000004024a164
printf
??:?
0x00000000400ea7a0
fast_bench_run_once
/home/orangepuzzle/rtos/rtos-bench/RTOS-Bench/extern/rt-thread/bsp/qemu-virt64-aarch64/rtos-bench/workloads/FAST/fast_bench.c:50
0x00000000401d8828
_ZL9fast_execiPPv
/home/orangepuzzle/rtos/rtos-bench/RTOS-Bench/extern/rt-thread/bsp/qemu-virt64-aarch64/rtos-bench/workloads/rtbench_workloads.cpp:37
0x000000004018d674
measure_wcet_ns
/home/orangepuzzle/rtos/rtos-bench/RTOS-Bench/extern/rt-thread/bsp/qemu-virt64-aarch64/rtos-bench/generator/test_schedule.c:88
0x000000004018e1b4
test_schedule_run_custom
/home/orangepuzzle/rtos/rtos-bench/RTOS-Bench/extern/rt-thread/bsp/qemu-virt64-aarch64/rtos-bench/generator/test_schedule.c:406
0x000000004016e8dc
rtosbench_rtthread_entry
/home/orangepuzzle/rtos/rtos-bench/RTOS-Bench/extern/rt-thread/bsp/qemu-virt64-aarch64/rtos-bench/generator/rtthread_entry.c:290
0x00000000400d268c
cmd_rtbench
/home/orangepuzzle/rtos/rtos-bench/RTOS-Bench/extern/rt-thread/bsp/qemu-virt64-aarch64/applications/rtbench_cmd.c:14
0x000000004014d974
_msh_exec_cmd
/home/orangepuzzle/rtos/rtos-bench/RTOS-Bench/extern/rt-thread/components/finsh/msh.c:351
0x000000004014da10
msh_exec
/home/orangepuzzle/rtos/rtos-bench/RTOS-Bench/extern/rt-thread/components/finsh/msh.c:559
0x0000000040150b50
finsh_thread_entry
/home/orangepuzzle/rtos/rtos-bench/RTOS-Bench/extern/rt-thread/components/finsh/shell.c:843
0x00000000400caa78
_thread_start
/home/orangepuzzle/rtos/rtos-bench/RTOS-Bench/extern/rt-thread/libcpu/aarch64/common/stack_gcc.S:25
```





报错信息2：
执行中输出：
```
rtbench test-schedule  
[test-schedule] Starting schedulability test
  Cycles: 10000, Utilization: 30% - 100% (step 10%)
=============================================================
[Phase 1] Measuring WCET for 10 workloads...
=============================================================
  [stub]: WCET = 0.000 ms
  [busywait]: WCET = 10.000 ms
[POSIX] Starting FAST Benchmark ...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)

| Image           | Size      | Corners  | Time(us)  | FPS     |
|-----------------|-----------|----------|-----------|---------|
| leuven          | 640 x480  | 5631     |   20708.8 |    48.3 |
| graf            | 640 x480  | 5820     |   25358.6 |    39.4 |
| bikes           | 640 x480  | 3719     |   21852.2 |    45.8 |
|-----------------|-----------|----------|-----------|---------|

[Result] Total Time: 67.920 s
[POSIX] Benchmark Finished.
[POSIX] Starting FAST Benchmark ...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)

| Image           | Size      | Corners  | Time(us)  | FPS     |
|-----------------|-----------|----------|-----------|---------|
| leuven          | 640 x480  | 5631     |   22724.7 |    44.0 |
| graf            | 640 x480  | 5820     |   24836.2 |    40.3 |
| bikes           | 640 x480  | 3719     |   21139.8 |    47.3 |
|-----------------|-----------|----------|-----------|---------|

[Result] Total Time: 68.701 s
[POSIX] Benchmark Finished.
[POSIX] Starting FAST Benchmark ...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)

| Image           | Size      | Corners  | Time(us)  | FPS     |
|-----------------|-----------|----------|-----------|---------|
| leuven          | 640 x480  | 5631     |   26823.2 |    37.3 |
| graf            | 640 x480  | 5820     |   30772.6 |    32.5 |
| bikes           | 640 x480  | 3719     |   23931.7 |    41.8 |
|-----------------|-----------|----------|-----------|---------|

[Result] Total Time: 81.527 s
[POSIX] Benchmark Finished.
[POSIX] Starting FAST Benchmark ...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)

| Image           | Size      | Corners  | Time(us)  | FPS     |
|-----------------|-----------|----------|-----------|---------|
| leuven          | 640 x480  | 5631     |   21995.8 |    45.5 |
| graf            | 640 x480  | 5820     |   24203.4 |    41.3 |
| bikes           | 640 x480  | 3719     |   25474.1 |    39.3 |
|-----------------|-----------|----------|-----------|---------|

[Result] Total Time: 71.673 s
[POSIX] Benchmark Finished.
[POSIX] Starting FAST Benchmark ...
Total Images: 3
Allocating RAM buffer: 307200 bytes (KB: 300)

| Image           | Size      | Corners  | Time(us)  | FPS     |
|-----------------|-----------|----------|-----------|---------|
| leuven          | 640 x480  | 5631     |   24093.7 |    41.5 |
| graf            | 640 x480  | 5820     |   27996.6 |    35.7 |
| bikes           | 640 x480  | 3719     | Function[_rt_mutex_take]: scheduler is not available
(0) assertion failed at function:_rt_mutex_take, line number:1334 
please use: addr2line -e rtthread.elf -a -f
 0x40157b34 0x4015307c 0x401535b8 0x4011aa7c 0x4011ae18 0x4011ae90 0x4011e73c 0x40082c28 0x4024a540 0x40248fcc 0x40247bcc 0x4025fa10 0x40249964 0x400ea144 0x401d8028 0x4018ce74 0x4018d9b4 0x4016e0dc 0x400d1e8c 0x4014d174 0x4014d210 0x40150350 0x400caa60
```
