# RTOS-Bench 进度记录

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

---

### 二、框架与板子的关系（架构边界）

#### 框架负责什么

RTOS-Bench 是一个**运行在被测 RTOS 上**的基准测试框架，其职责边界：

```
┌─────────────────────────────────────────────────────────────────┐
│ 被测板子 (Target)                                                │
│                                                                  │
│   RTOS-Bench 框架 (编译链接到 RTOS 固件中)                       │
│   ┌──────────────────────────────────────────────────────┐      │
│   │  test-realtime  test-schedule  test-stress  test-cmd │      │
│   │  typical-workload  periodic-benchmark                │      │
│   ├──────────────────────────────────────────────────────┤      │
│   │  result_export.c → 生成 /rtbench_result.json         │      │
│   │  (存储在板子本地文件系统)                              │      │
│   └──────────────────────────────────────────────────────┘      │
│                                                                  │
│   输出: /rtbench_result.json (中间格式 JSON)                     │
└──────────────────────────┬──────────────────────────────────────┘
                           │ SCP / 串口传输 / SD卡拷贝
                           ▼
┌─────────────────────────────────────────────────────────────────┐
│ 宿主机 (Host)                                                    │
│                                                                  │
│   1. 收集 rtbench_result.json                                   │
│   2. 展平转换 → standard_results_example.json (阿里云 schema)    │
│   3. 上传到阿里云数据库                                          │
└─────────────────────────────────────────────────────────────────┘
```

**框架边界说明**:

| 层次 | 负责方 | 说明 |
|------|--------|------|
| 测试执行 | 框架 (Target 上) | 运行 5 个测试模块，收集性能数据 |
| 中间结果 JSON 生成 | 框架 (Target 上) | `result_export.c` 序列化为 JSON |
| 中间结果持久化 | 框架 (Target 上) | 写入 Target 本地文件系统 |
| 结果传输 (Target→Host) | **SOP / 外部工具** | SCP、串口文件传输、SD 卡 |
| 展平转换 (中间→标准格式) | **Host 端脚本** (TODO) | Python 脚本，将嵌套 JSON 展平为一条一条记录 |
| 上传阿里云 | **Host 端脚本** (TODO) | 待阿里云 API 对接后实现 |

---

### 三、板级测试 SOP 工作流

#### 3.1 现有能力

- [x] 框架在 Target 上运行测试并生成 JSON (`test-all -o /rtbench_result.json`)
- [x] JSON Schema 定义 (`docs/reference/rtbench_result_schema.json`)
- [x] 标准格式示例 (`docs/reference/standard_results_example.json`)
- [x] 多平台入口 (RT-Thread / SylixOS / OneOS / Dongtu / Ruihua)
- [x] Host 端展平转换脚本 (`utils/flatten_rtbench_result.py`，已验证: 252 行 → 270 条标准记录)
- [ ] Host 端结果收集脚本 (SCP 拷贝)
- [ ] 阿里云 API 上传脚本

#### 3.2 完整 SOP 流程

```
Phase 1: 准备 (宿主机)
  1. 交叉编译 RTOS 固件 (含 RTOS-Bench)
  2. 烧录到目标板 / 通过 JTAG/TFTP 加载

Phase 2: 执行 (被测板子)
  3. 上电启动，进入 Shell
  4. 运行: rtbench test-all -o /rtbench_result.json
  5. 结果保存在板子文件系统中

Phase 3: 收集 (宿主机←被测板子)
  6. SCP 拷贝:
     scp user@target:/rtbench_result.json ./results/<board>_<date>.json
     或: 串口 YMODEM 传输
     或: 拔 SD 卡读取

Phase 4: 转换 (宿主机)
  7. 展平转换:
     python3 utils/flatten_rtbench_result.py \
       results/<board>_<date>.json \
       -o results/<board>_<date>_flat.json -p

Phase 5: 上传 (宿主机→阿里云)   [TODO]
  8. python3 utils/upload_results.py \
       --input results/<board>_<date>_flat.json \
       --api-endpoint <阿里云API>
```

#### 3.3 龙芯 + SylixOS 适配示例

```
# 1. 在宿主机交叉编译 (SylixOS IDE 或 Makefile)
make PLATFORM=sylixos ARCH=mips64 BOARD=loongson-2k1000

# 2. 通过 TFTP/NFS 部署到板子
tftp -g -r rtbench <host_ip>

# 3. 在板子上运行
./rtbench test-all -o /tmp/rtbench_result.json

# 4. SCP 回宿主机
scp root@<board_ip>:/tmp/rtbench_result.json ./results/

# 5. 展平 + 上传
python3 utils/flatten_rtbench_result.py results/rtbench_result.json -o results/standard.json -p
```

---

### 四、当前阻塞问题与 TODO

#### 4.1 需修复的 Bug

| # | 问题 | 严重度 | 状态 | 修复说明 |
|---|------|--------|------|----------|
| 1 | test-cmd 在无文件系统时崩溃 (mv → dfs_file_rename null deref) | HIGH | **FIXED** | 新增 `test_cmd_has_filesystem()` 通过 `dfs_filesystem_lookup("/")` 检测 FS，无 FS 时跳过文件命令 |
| 2 | test-all 在 tshell 线程栈溢出 | HIGH | **FIXED** | test-all 逻辑抽取到 `test_all_thread_entry()`，在独立 32KB 栈线程中运行，tshell 线程仅等待信号量 |
| 3 | test-realtime 中断延迟数据无效 (需内核插桩) | MEDIUM | **已知限制** | QEMU 环境下中断延迟数据无意义，真实板子上正常。文档已说明 |
| 4 | collect_realtime_result() 使用 placeholder 值 | MEDIUM | **FIXED** | bench_init.c 新增 getter 函数暴露 static 数组，collect_realtime_result() 读取实际测量值并转换 ns→us |
| 5 | collect_stress_result() bogo_ops 为 0 | MEDIUM | **FIXED** | stress-ng.c 新增 `g_last_bogo` 累加器，通过 `stress_ng_get_last_bogo_ops()` → `test_stress_get_last_bogo_ops()` 链式暴露 |

#### 4.2 Host 端工具 (TODO)

| # | 工具 | 说明 | 状态 |
|---|------|------|------|
| 1 | `utils/flatten_rtbench_result.py` | 中间格式 JSON → 阿里云标准格式 (一条记录/指标) | DONE (已验证) |
| 2 | `utils/upload_results.py` | 标准格式 → 阿里云 API | TODO (待对方 API ready) |
| 3 | `utils/collect_from_board.sh` | SCP 自动收集脚本 | TODO |

#### 4.3 阿里云对接 (TODO)

**数据格式已确认**: `docs/reference/standard_results_example.json` 中的展平格式，每个指标一条记录，包含:
- `flow_job_history_id` (UUID)
- `sw_info` (sdk_type, version, kernel_version)
- `hw_info` (platform_type, cpu_type, soc_info)
- `test_case_info` (test_suite, test_dir, test_case, test_data_source)
- `test_config_info` (test_mcpu, test_cpu_core_num, test_option_alias)
- `test_result_info` (test_result, test_unit, test_optimal_type, test_result_valid)

**待对方提供**: API endpoint、认证方式、batch upload 是否支持。

---

### 五、数据流总览

```
   被测板 (RTOS)                    宿主机 (Linux)                 阿里云
  ┌───────────┐               ┌─────────────────────┐        ┌──────────┐
  │ test-all  │               │                     │        │          │
  │    ↓      │   SCP/串口    │  rtbench_result.json│  API   │ 数据库   │
  │ JSON 文件 │ ──────────→ │         ↓            │ ────→ │          │
  │ (中间格式) │              │  flatten_results.py │        │          │
  └───────────┘               │         ↓            │        │          │
                              │  标准格式 JSON       │        │          │
                              │  (展平, 一条/指标)    │        │          │
                              │         ↓            │        │          │
                              │  upload_results.py   │        │          │
                              └─────────────────────┘        └──────────┘

  框架边界 ←──────────────→  SOP + Host 工具 ←──────→ 阿里云联调
  (已完成)                    (本次梳理)               (TODO)
```

### 六、验证用原始日志

原始 QEMU 测试日志保存在 `/tmp/qemu_test_output.log` (test-cmd 崩溃) 和 `/tmp/qemu_test_output2.log` (test-stress + test-realtime 通过)。
