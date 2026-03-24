# RTOS-Bench 进度记录

## 2026-03-24: 分支整合 & 全流程审计

### 一、分支合并

将 `score-cal` 和 `feat/remote-deploy` 合入 main 并推送远端，清理已合并分支。

| 来源分支 | 操作 | 合入内容 |
|----------|------|----------|
| `score-cal` | cherry-pick 有效增量 | `docs/STRESS.md` 单 stressor 文档更新 |
| `feat/scoring-integration` | 已通过 PR#12 合入 | `score_single_json()` API、`--db` CLI |
| `feat/raw-log-capture` | 已通过 PR#11 合入 | raw log capture 模块 |
| `feat/remote-deploy` | `merge --no-ff` | deploy.py 编排 + config_schema + file_transfer + board_config |

> `score-cal` 的其余变更（删除主线程修复、删除 score_single_json API）是相对 main 的倒退，未合入。

### 二、端到端流程完成度审计

#### 总览

```
  被测板 (RTOS)                    宿主机 (Host)
 ┌───────────────┐        ┌──────────────────────────────────────────────┐
 │ rtbench        │        │  deploy.py (编排)                            │
 │ test-all       │        │    ↓                                        │
 │    ↓           │  SSH/  │  1. 上传固件到 DUT                           │
 │ result_export  │ Serial │  2. 连接 DUT (SSH/Serial/Telnet)            │
 │    ↓           │ Telnet │  3. 发送 test-all 命令                      │
 │ JSON 文件      │ ←────→ │  4. 等待完成 (end_regex)                    │
 │ + 终端日志     │        │  5. 取回 JSON (sftp / terminal_capture)     │
 └───────────────┘        │  6. 终端日志自动保存到文件                    │
                           │  7. flatten → 标准格式 (可选)               │
                           │  8. score_single_json → 评分 (可选)         │
                           └──────────────────────────────────────────────┘
```

#### 各模块完成度

| 模块 | 文件 | 状态 | 说明 |
|------|------|------|------|
| **C 端结构化 JSON 输出** | `generator/result_export.c/.h` | **完成** | 5 个模块全部序列化，无遗漏字段 |
| **C 端 test-all 调用链** | `generator/rtthread_entry.c` | **完成** | 5 模块全部 run + collect |
| **终端日志输出到文件** | `utils/remote-test/dut_connection.py` | **完成** | SSH/Serial/Telnet 三种连接均支持 `log_file` 参数 |
| **日志文件名生成** | `utils/remote-test/log_utils.py` | **完成** | 北京时间戳 `rtbench_raw_YYYYMMDD_HHMMSS_CST.log` |
| **远程编排** | `utils/remote-test/deploy.py` | **完成** | 12 步流水线，SSH happy-path 可用 |
| **配置加载/校验** | `utils/remote-test/config_schema.py` | **基本完成** | 有默认值填充，缺字段级/类型级校验 |
| **文件传输 (SFTP)** | `utils/remote-test/file_transfer.py` | **完成** | upload/download 正常 |
| **终端 JSON 提取** | `utils/remote-test/file_transfer.py` | **有缺陷** | 正则只支持 2 层嵌套，实际 JSON 3+ 层，会返回 None |
| **展平脚本** | `utils/flatten_rtbench_result.py` | **基本完成** | 4/5 模块有显式映射，test-cmd 走 fallback 也能工作 |
| **评分脚本** | `utils/score_caculate.py` | **完成** | `score_single_json()` API + CLI `import-json` 均可用 |
| **示例配置** | `utils/remote-test/board_config_example.yaml` | **有问题** | `end_regex` 不匹配实际 C 输出 |

### 三、rtbench_result.json 结构化输出完整性

`result_export.c` 的 `rtbench_result_to_json()` 对数据模型中的**所有字段**均有序列化，无遗漏。`docs/reference/rtbench_result_example.json` 展示了完整的 5 模块输出结构。

**JSON 序列化：完整。** 但 `collect_*_result()` 数据填充有以下空洞：

| 字段 | 问题 | 影响 |
|------|------|------|
| `test-realtime.duration_sec` | 未赋值，始终 0 | 不影响评分（评分不读此字段） |
| `test-schedule.duration_sec` | 未赋值，始终 0 | 同上 |
| `test-schedule.wcet_measurements` | 未填充，始终空数组 | WCET 数据有测量但未转入 result struct |
| `test-schedule.config` | 硬编码默认值 | quick 模式下不反映实际参数 |
| `test-schedule.gradients[].task_stats[].utilization/period_ms` | 未填充，始终 0 | 源数据在 `schedule_task_config` 而非 `schedule_task_stats` |
| `typical-workload.duration_sec` | 未赋值，始终 0 | 不影响评分 |

> 这些空洞不影响评分流程（评分读取的是 single_core 指标、miss_rate、workload exec_time 等实际填充的字段），但会影响展平输出的完整性。

### 四、评分集成状态

`score_caculate.py` 的 `score_single_json()` 可直接消费 `rtbench_result.json`，路径硬编码提取各指标：

| 评分维度 | 权重 | 数据来源 | 是否有数据 |
|----------|------|----------|-----------|
| RT_Score (实时性能) | 30% | test-realtime: context_switch, interrupt, syscall, service_cost | 有 |
| T_Score (典型负载) | 40% | typical-workload: exec_time_ms / avg_time_ms | 有 |
| Sched_Score (可调度性) | RT 内 50% | test-schedule: summary.average_miss_rate | 有 |
| P_Score (功耗) | 10% | 外部功耗仪数据 | 无（占位 0） |
| F_Score (功能) | 20% | test-cmd: pass_count/cmd_count | 占位 60 |

**关键限制**: 评分使用跨 OS min-max 归一化。单 OS 数据库中 R_max == R_min，所有 `item_score` 为 null。需至少 2 个不同 OS 在同一板子上的结果才能产生有效分数。

### 五、已知缺陷 & 待修项

| # | 严重度 | 模块 | 问题 | 状态 |
|---|--------|------|------|------|
| 1 | HIGH | `file_transfer.py` | `extract_json_from_buffer` 正则只支持 2 层 JSON 嵌套，实际 3+ 层 → terminal_capture 模式失效 | 待修 |
| 2 | MEDIUM | `board_config_example.yaml` | `end_regex` 写 `\[result-export\] JSON result saved`，C 代码实际输出 `[RTOS-Bench] Results saved to:` | 待修 |
| 3 | MEDIUM | `deploy.py` | SFTP upload/download 硬编码 SSH 字段，Serial 连接会 KeyError | 待修 |
| 4 | MEDIUM | `config_schema.py` | 无字段级校验，缺少必填字段检查 | 待改善 |
| 5 | MEDIUM | `flatten_rtbench_result.py` | `bogo_ops` 被无条件跳过，stress 主指标丢失 | 待修 |
| 6 | LOW | `collect_schedule_result()` | wcet、task utilization/period_ms 未填充 | 待修 |
| 7 | LOW | `collect_*_result()` | realtime/schedule/workload 模块级 duration_sec 未赋值 | 待修 |
| 8 | LOW | `deploy.py` | `run_score` 默认 false，需 YAML 显式开启 | 设计如此 |
| 9 | LOW | 项目整体 | 无 `requirements.txt`（paramiko, pyyaml, pandas, openpyxl） | 待补 |

### 六、结论

**能跑通的路径**: SSH 连接 → test-all → SFTP 取回 JSON → 终端日志落盘。这条 happy path 是完整的。

**展平**: `flatten_rtbench_result.py` 能将 JSON 转为标准格式记录，但 bogo_ops 被跳过、test-cmd 映射不一致。输出面向外部平台（阿里云 schema），与评分脚本是**并行的两条管线**。

**评分**: `score_single_json()` 能消费 JSON 并计算得分，但需 ≥2 个 OS 数据才有意义。deploy.py 已集成调用入口，默认关闭需配置开启。

---

## 2026-03-04: E2E 测试修复 (feat/e2e-test-rtt)

### 修复的 Bug

#### 1. [HIGH] test-cmd 无文件系统崩溃 → FIXED

- **文件**: `generator/test_cmd.c`
- **问题**: `mv` 命令在 QEMU virt (无挂载 FS) 触发 `dfs_file_rename` 空指针解引用
- **修复**: 新增 `test_cmd_has_filesystem()` 使用 `dfs_filesystem_lookup("/")` 检测 FS 挂载状态。无 FS 时跳过文件操作命令 (mkdir/cp/mv/cat/rm/echo)，标记为 "skipped (no FS)"。非文件命令 (date/ps/pwd/ls/cd) 正常执行
- **QEMU 验证**: PASS — 5/11 commands supported, 无崩溃

#### 2. [HIGH] test-all tshell 栈溢出 → FIXED

- **文件**: `generator/rtthread_entry.c`
- **问题**: tshell 线程栈仅 4KB，test-all → test-realtime 调用链过深导致栈溢出
- **修复**: 将 test-all 逻辑提取到 `test_all_thread_entry()`，通过 `rt_thread_create("rtbench", ..., 32*1024, 20, 10)` 在独立 32KB 线程中运行。tshell 线程通过信号量等待完成
- **编译验证**: PASS

#### 3. [MEDIUM] collect_realtime_result() placeholder 值 → FIXED

- **文件**: `generator/realtime_orig/les/bench_init.c`, `generator/rtthread_entry.c`
- **问题**: `collect_realtime_result()` 使用 -1 占位值，未读取实际测量数据
- **修复**: bench_init.c 新增 8 个 getter 函数暴露 static 数组。`collect_realtime_result()` 调用 getter 读取实际值，ns→us 转换后写入 result 结构体。覆盖: service cost (8 ops x 4 scenarios)、context switch、interrupt、syscall、multicore (mem_bw/ipc_bw/task_lat/intra_inter)

#### 4. [MEDIUM] collect_stress_result() bogo_ops 为 0 → FIXED

- **文件**: `generator/stress_orig/common/stress-ng.c`, `generator/test_stress.c`, `generator/test_stress.h`, `generator/rtthread_entry.c`
- **问题**: `stress_ng_main()` 调用 `stress_run_one_job()` 时传 `output_result=NULL`，bogo_ops 被丢弃
- **修复**: stress-ng.c 新增 `g_last_bogo` 静态变量，`stress_run_one_job()` 完成后始终填充。链式暴露: `stress_ng_get_last_bogo_ops()` → `test_stress_get_last_bogo_ops()` → `collect_stress_result()` 读取实际 bogo_ops 和 ops/s

### 编译/验证状态

| 测试模块 | 编译 | QEMU 运行 | 备注 |
|----------|------|-----------|------|
| test-cmd | PASS | PASS | 5/11 支持 (无 FS 环境正常降级) |
| test-stress | PASS | 未重测 | 此前已 PASS，本次只改数据采集 |
| test-realtime | PASS | 未重测 | 此前已 PASS，本次只改数据采集 |
| test-all | PASS | 待验证 | 32KB 栈线程，需完整运行验证 |
| test-schedule | PASS | 未测 | QEMU 下耗时过长 |

### 剩余已知问题

- test-realtime 中断延迟: QEMU 环境下数据无效 (需内核插桩，真实板子上正常)
- test-schedule: QEMU 下 FAST workload 单次约 575 秒，不适合在模拟器中运行

---

## 2026-03-03: 端到端验证 & 板级测试工作流梳理

### 一、QEMU 端到端验证结果

在 QEMU virt aarch64 (Cortex-A53 x4, 128MB) 上对 RT-Thread 5.3.0 进行了真实运行验证。

#### 1. test-stress: PASS

```
rtbench test-stress -s cpu -t 5
rtos_stress: info: [cpu-0] completed, 0x000000000000184b ops (6219 bogo-ops)
[test-stress] Stressor cpu completed with code: 0
```

- CPU stressor 正常运行 5 秒
- bogo_ops 输出正确

#### 2. test-realtime: PASS

```
rtbench test-realtime
```

完整输出结果:

| 指标 | 立即执行 | 挂起睡眠 | 低优就绪 | 高优恢复 |
|------|----------|----------|----------|----------|
| 信号量获取 | 2.080 us | 4.336 us | - | - |
| 信号量释放 | 3.264 us | - | 103.920 us | 1.824 us |
| 消息发送 | 16.288 us | 68.400 us | 191.680 us | 72.144 us |
| 消息接收 | 15.056 us | 48.480 us | 203.520 us | 45.920 us |
| 互斥锁获取 | 2.512 us | 165.008 us | - | - |
| 互斥锁释放 | 3.328 us | - | 69.376 us | 121.728 us |
| 内存块申请 | 9.952 us | - | - | - |
| 内存块释放 | 9.216 us | - | - | - |

- 上下文切换延迟 AVG: 1.863 us
- 中断软件延迟: 需要内核插桩，QEMU 数据无效
- 系统调用延迟 MIN: 0.176 us  MAX: 0.256 us  AVG: 0.191 us

#### 3. test-cmd: CRASH (已知问题)

`mv` 命令在无挂载文件系统时触发 `dfs_file_rename` 空指针解引用崩溃。

**根因**: QEMU virt 板未挂载可写文件系统到 `/`，`mkdir` 失败后后续 `cp`/`mv` 操作导致 DFS 层 null dereference。

**修复方向**: `test_cmd.c` 的 `cmd_examine()` 需要增加对 `msh_exec` 返回值的 crash-safe 处理，或在无文件系统时跳过文件操作类命令。

#### 4. test-all: 栈溢出

`tshell` 线程栈 4KB (0x1000) 不足以支撑 test-all 调用 test-realtime，后者内部有较深的调用栈。

**修复方向**: 增大 `tshell` 线程栈（建议 ≥ 8KB），或将 test-all 拆分到独立大栈线程执行。

#### 5. test-schedule: 未本次验证

此前已验证框架可用（见 docs/TEST_REPORT.md），WCET 测量阶段正常，但 FAST workload 单次约 575 秒，完整测试在 QEMU 下不现实。
