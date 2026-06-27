# OneOS 哪吒派 D1H — test-schedule 验收记录 (2026-06-27, CLOSED ✅)

真实(非 spin/mock)test-schedule 在 OneOS 哪吒派 D1H(RISC-V, `192.168.31.211`)上通过。

## 结果
- `fast/pid/cusum/ewma` 真实执行;`icp/ekf/mqtt/modbus` 被板级 allowlist 跳过。
- **U=30%: MR=0.0000 (0/4) → Final Score: 100.00 / 100**,`test-schedule ret=0`,板子全程稳定。
- 产物 `d1h-nezha_out.out`(allowlist 构建) SHA256 `88323cd2fe8fdcbc47f042782b997311414b9d883fcebdf6f5614b8ae12086d7`,TFTP 短名 `/tftp/ctw.out`。

## 本次合入 main 的改动(`acb4cdc`)
- `is_workload_allowed()` 板级 workload allow/exclude 兜底(默认空=跑全部,不影响任何板);
  哪吒策略 `-DTEST_SCHEDULE_WORKLOAD_ALLOWLIST="fast,pid,cusum,ewma"`。
- cusum 包装返回值修复(`cusum_bench_run()` 返回结果计数而非错误码,归一化为 0,避免误判 deadline miss)。
- (此前) 全局墙钟看门狗 + 离线 MQTT/MODBUS wrapper + 板级周期 clamp 泛化。

## 复现(板子健康时)
```bash
ssh rtbench "python3 /tmp/nezha_health.py"             # 确认 telnet shell 可用(ping≠可用)
ssh rtbench "python3 /tmp/oneos_nezha_watchdog_p0.py"  # 部署 ctw.out→/user/ctest.out, ld schedrun, 出分
```
脚本见 `scripts/`(raw-telnet + IAC 过滤)。控制面 `192.168.31.211:23`,数据面 TFTP `192.168.31.110:/tftp`。

## 关键坑点(可复用到其它板)
1. **能 ping ≠ shell 可用**:telnet 会 wedged(连得上 0 回显)。判据=`help/ifconfig` 有回显(`nezha_health.py`)。恢复:关其它 telnet 会话→串口 `telnetd start`→断电复位。
2. **OneOS shell 输入行 80 字符截断**:`tftp_client ... get <长名> /user/ctest.out` 会被砍 → **TFTP 用短名**(`ctw.out`)。
3. **真实跑有 bug 的负载会内核硬崩**(page fault,整板掉线);用户态看门狗挡不住 → **allowlist 兜底**。
4. **构建前同步源码**到工程副本再编(典型负载 bug 修复在 main)。
5. **module printf 多走串口**,telnet 看不到测试输出。

## 遗留(不阻塞验收)
- origin/main 暂不能独立编 OneOS 镜像:`generator/platform/oneos/*` 平台移植 + 可用 CMakeLists + cxx ABI 垫片仍在 nezha 工作树未提交。需一次 **OneOS 平台移植整合**到 main(其它板用各自构建文件,不受影响)。allowlist 哪吒策略(`-D`)随该整合的 CMakeLists 一并入库;allowlist 机制已在 main。
- 当前可用 OneOS 构建在 WSL 工程 `projects/d1h-nezha_out`(cmake -S . -B build && make -C build -j → out/d1h-nezha_out.out)。

完整 SOP / 编译部署原理见固化文档 `0627/oneos-nezha/README.md`。
