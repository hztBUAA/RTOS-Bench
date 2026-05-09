# 飞腾派 SylixOS / RTOS-Bench test-all 崩溃反馈

日期：2026-04-30

## 一句话现象

同一个 RTOS-Bench 二进制，单独跑部分命令时更容易通过，但 `test-all` 串行跑完整框架能力时，飞腾派上会较高概率在 `pthread_create -> API_VmmStackAlloc` 相关路径崩溃并断开 telnet。其他 SylixOS 板卡没有复现同类问题。

## 环境和二进制

- 板卡：翼辉 SylixOS 飞腾派
- 程序：RTOS-Bench
- 最新测试二进制：`/apps/hzt/feiteng-rtos-bench_hzt_epnpaliasfix_20260430_0316`
- SHA256：`041D7B567FD82885319813720DD536B269648B39EEDCC85CD77B25CA0314877C`
- 二进制大小：`6406536` 字节
- `rtbench -L` 已能列出 workload：11 个条目，其中 9 个工业负载 + 2 个 utility workload。

## 主要复现命令

```sh
/apps/hzt/feiteng-rtos-bench_hzt_epnpaliasfix_20260430_0316 test-realtime
/apps/hzt/feiteng-rtos-bench_hzt_epnpaliasfix_20260430_0316 test-schedule
/apps/hzt/feiteng-rtos-bench_hzt_epnpaliasfix_20260430_0316 test-all -o /apps/hzt/feiteng_testall.json
```

目前同事反馈“裸跑 `test-realtime` 等命令能过，但 `test-all` 会出问题”。我们这边历史日志也支持这个方向：`test-all` 不是简单调用单个测试，而是会在一个总入口里连续执行 realtime、schedule、stress、cmd、workload collect、export 等模块，因此更容易暴露线程栈/VMM 分配累积问题。

## 典型崩溃栈 1：test-all 内 realtime 阶段

`test-all --no-stress` 曾在 realtime 的 `test6_4/test7_3` 附近崩溃，关键栈如下：

```text
API_BacktraceShow
__vmmVirtualPageAlloc
API_VmmStackAlloc
API_ThreadInit
pthread_create
test6_4
test7_3
```

对应现象：telnet 连接被板端关闭。

## 典型崩溃栈 2：test-all 内 stress 阶段

完整 `test-all` 曾经 realtime 和 schedule 都跑完，进入 stress 的 qsort job 时崩溃，关键栈如下：

```text
API_BacktraceShow
API_VmmStackAlloc
API_ThreadInit
pthread_create
stress_osal_thread_spawn
stress_jobfile_exec_ex
test_stress_run_job
```

这说明问题不固定在某一个业务 workload，而是集中出现在飞腾派 SylixOS 的线程创建/线程栈分配路径。

## 我们已经在 RTOS-Bench 侧处理/排除的点

1. 已修复 SylixOS 上 periodic benchmark 退出时 timer 清理不幂等的问题，避免重复 `timer_delete` 导致 ptmalloc abort。
2. 已修复 `test-schedule` 在 SylixOS 上运行多 workload 后状态未完全复位的问题。
3. 已将 `test-schedule` 默认配置调整为已验证过的 `cycles=3`，WCET 默认 5 次。
4. 已对 `test-all` 的 schedule 结果导出做边界保护，避免 JSON 导出阶段因为字符串/数组边界触发异常。
5. 已让 SylixOS 入口命令的 parser 对 help/非法参数做规范 fallback，避免误触发全量运行。
6. 已定位并修复 ePnP 在飞腾派上 Eigen 自引用表达式导致的独立崩溃风险，修复方式是等价临时变量计算，没有跳过或削弱 workload。

## 当前怀疑点

`test-all` 在 SylixOS 入口中会额外创建一个总控 worker 线程运行整套测试，栈大小当前为 4MB；schedule 内部的 WCET 测量线程和周期任务线程也会分别创建 pthread。飞腾派上，当 realtime/stress 再创建自己的内部线程时，容易进入：

```text
pthread_create -> API_ThreadInit -> API_VmmStackAlloc -> __vmmVirtualPageAlloc
```

所以我们希望厂家重点协助确认：

- 飞腾派 SylixOS 当前 BSP/内核对 pthread 栈分配、VMM stack alloc、最大线程数、单进程/单模块栈虚拟内存是否有已知限制。
- 多轮创建/销毁 pthread 后，`API_VmmStackAlloc` 是否存在碎片化、重复释放、链表损坏或资源未及时回收的已知问题。
- 对这种 benchmark 场景，推荐的 pthread stack size、shell task stack size、VMM 配置或系统参数应该如何设置。
- 当一次 6MB 板端 `cp` 长时间未返回后，telnet 出现 `server is full of links`、FTP listing timeout，是否有 shell/telnet 连接资源无法回收的问题。

## 我们希望厂家给的结论

1. 这个 `API_VmmStackAlloc` 链路的崩溃是否属于飞腾派 SylixOS/BSP 层需要修复或配置调整的问题。
2. 如果需要应用侧规避，请给出明确建议：最大线程数、最大/推荐 pthread 栈大小、是否必须避免在 shell/test-all 外层再包 worker 线程。
3. 如果可以，请提供能打开更多内核诊断日志或定位 `API_VmmStackAlloc` 失败原因的方法。

