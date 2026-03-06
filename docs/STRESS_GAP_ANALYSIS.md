# test-stress 数据流与能力差距分析

> 日期: 2026-03-06
> 分支: fix/stress03-06
> 状态: 待修复

---

## 1. 完整数据流

### 1.1 数据流总览

系统有 **两条入口路径**，最终都汇入同一个底层引擎 `stress_run_one_job()`：

```
路径 A: test-stress 独立子命令
路径 B: test-all 综合测试中的 stress 阶段

                     ┌──────────────────────────────────────────────┐
                     │           rtthread_entry.c (CLI 入口)         │
                     │  rtosbench_rtthread_entry(argc, argv)         │
                     └──────┬─────────────────────────┬─────────────┘
                            │                         │
                   argv[1]=="test-stress"    argv[1]=="test-all"
                            │                         │
                   ┌────────▼──────────┐    ┌────────▼──────────────┐
                   │ 路径A: 直接调用    │    │ 路径B: test_all_thread │
                   │ :367-406          │    │ :155-227               │
                   └────────┬──────────┘    └────────┬──────────────┘
                            │                         │
               ┌────────────▼──────────┐   ┌─────────▼────────────────┐
               │    test_stress.c      │   │  test_stress.c           │
               │  test_stress_run_     │   │  test_stress_run_        │
               │  stressor(name, dur)  │   │  stressor("cpu", dur)    │
               │  or run_config()      │   │  (硬编码 stressor="cpu") │
               └────────────┬──────────┘   └─────────┬────────────────┘
                            │                         │
                            └──────────┬──────────────┘
                                       │
                         构造 argv: {"rtos_stress", name,
                                     "-t", "Ns", "-c", "1"}
                                       │
                              ┌────────▼────────────────┐
                              │   stress-ng.c            │
                              │   stress_ng_main()       │
                              │   :640-670               │
                              └────────┬────────────────┘
                                       │
                            ┌──────────▼──────────────┐
                            │  stress_run_one_job()    │
                            │  :228-412                │
                            │                          │
                            │  解析: -c, -t, --ops,    │
                            │  --method, --<opt>        │
                            │  查找 stress_registry[]   │
                            │  创建 N 个 worker 线程    │
                            └──────────┬──────────────┘
                                       │
                            ┌──────────▼──────────────┐
                            │  stress_thread_trampoline│
                            │  :134-158                │
                            │  调用 stressor_info_t    │
                            │  .entry(args)            │
                            └──────────┬──────────────┘
                                       │
                         ┌─────────────▼─────────────────┐
                         │  stressor/ 目录下 27 个实现     │
                         │  stress_cpu(), stress_vm() ... │
                         └───────────────────────────────┘
```

### 1.2 路径 A 详解: `rtbench test-stress`

```
用户输入:  rtbench test-stress -s matrix -t 30
```

**Step 1 — CLI 解析** (`rtthread_entry.c:367-406`)

```c
// 仅解析 4 个参数
-s <name>   -> stressor = "matrix"    // 默认 "cpu"
-t <sec>    -> duration = 30          // 默认 10, 整数秒
-l/--list   -> list_stressors         // 列出后退出
-q          -> benchmark_verbosity    // 安静模式
```

**Step 2 — 分发**

- 若 `stressor == "all"` -> `test_stress_run_config()` 遍历 `stressor_names[]` (8个)
- 否则 -> `test_stress_run_stressor(stressor, duration)`

**Step 3 — argv 构造** (`test_stress.c:49-84`)

```c
argv[0] = "rtos_stress";
argv[1] = "matrix";      // 来自 -s 参数
argv[2] = "-t";
argv[3] = "30s";          // duration_sec 拼接 "s" 后缀
argv[4] = "-c";
argv[5] = "1";            // *** 硬编码，不可配 ***
// argc = 6
```

**Step 4 — 进入底层** -> `stress_ng_main(6, argv)` -> `stress_run_one_job()`

**Step 5 — 结果回收**

```c
s_last_bogo_ops = stress_ng_get_last_bogo_ops();  // 读取 g_last_bogo.current_ops
```

### 1.3 路径 B 详解: `rtbench test-all`

```
用户输入:  rtbench test-all --stress-duration 20
```

**Step 1 — CLI 解析** (`rtthread_entry.c:234-267`)

```c
// test-all 中 stress 相关参数仅 2 个:
--no-stress          -> params.run_stress = 0    // 跳过 stress
--stress-duration N  -> params.stress_duration = 20  // 默认 10
```

**Step 2 — Worker 线程** (`test_all_thread_entry`, :155-227)

```c
// 在独立线程中运行(32KB 栈，避免 tshell 溢出)
if (p->run_stress) {
    test_stress_run_stressor("cpu", p->stress_duration);  // *** stressor 硬编码 "cpu" ***
    collect_stress_result("cpu", p->stress_duration);
}
```

**Step 3 — 结果持久化** (`collect_stress_result`, :662-674)

```c
uint64_t bogo_ops = test_stress_get_last_bogo_ops();
double bogo_per_sec = (double)bogo_ops / duration;
rtbench_stress_add_stressor(stress, "cpu", "cpu", bogo_ops, duration, bogo_per_sec, "ops/s");
```

-> 写入 `rtbench_result` -> `rtbench_result_export_json()` 导出到文件

### 1.4 第三条路径: `--job` / `--jobfile` (Jobfile 模式)

这条路径**完全独立于框架**，只能通过底层直接调用触达：

```
stress_ng_main() 入口判断:
  argv[1] == "--job"     -> handle_job_command(argv[2])
  argv[1] == "--jobfile" -> stress_jobfile_exec(argv[2])
```

**Jobfile 执行流程** (`stress-ng.c:463-621`)

```
stress_jobfile_exec(filepath)
  │
  ├─ 匹配内置 BUILTIN_JOBS[]?
  │   ├─ "stored_jobfile_cpu.txt"    -> JOB_DATA_CPU    (13 stressor x 5 阶段)
  │   ├─ "stored_jobfile_memory.txt" -> JOB_DATA_MEMORY (6 stressor x 5 阶段)
  │   └─ "stored_jobfile_file.txt"   -> JOB_DATA_FILE   (8 stressor x 5 阶段)
  │
  ├─ 否则 fopen(filepath) 读取外部 jobfile
  │
  ├─ 第一轮: 扫描计数 total_tasks
  │
  ├─ 第二轮: 逐行解析执行
  │   每行格式: "stressor --ops N -c M --param V"
  │   │
  │   ├─ stress_tokenize() -> argv[]
  │   └─ stress_run_one_job(argc, argv, SILENT, &result.bogo)
  │
  └─ 打印汇总表: Stressor | Bogo Ops | Time(s) | Metric
```

**Jobfile 数据设计** (`stress_stored_job.c`)

每个 Job 按 5 级递增负载设计（20%/40%/60%/80%/100%），每级调整：
- `-c N`: worker 并发数递增 (1->2->3->4->5)
- `--ops N`: 操作数按级递减（维持总时间约 10 分钟）
- `per-stressor 参数`: 按级递增难度（如 matrix-size 32->64->64->128->128）

| Job 名称 | 包含 stressor | 总任务行数 |
|----------|--------------|-----------|
| `cpu` | cpu, matrix, qsort + atomic, bitops, bsearch, context, fp, prime, stack, str, trig, vecmath | 65 (13x5) |
| `memory` | memcpy, stream, vm, malloc, memthrash, ptr-chase | 30 (6x5) |
| `file` | hdd, open, copy-file, unlink, fstat, dentry, rename, pipe | 40 (8x5) |
| `all` | 以上三个顺序执行 | 135 |

---

## 2. 底层引擎 `stress_run_one_job()` 完整参数接口

这是所有路径的最终汇聚点 (`stress-ng.c:228-412`)。

### 2.1 argv 解析能力

```c
for (i = 1; i < argc; i++) {
    "-c" <N>          -> num_instances  (并发 worker 数)
    "-t" <time>       -> timeout_ticks  (stress_parse_time: 支持 s/m/h)
    "--ops" <N>       -> max_ops        (最大 bogo 操作数)
    "--method" <name> -> target_method  (子方法选择)
    "--<opt>" <val>   -> stress_handle_stressor_opt()  (per-stressor 参数)
}
```

### 2.2 Stressor 注册表 (`stress_registry[]`)

```c
// stress-ng.c:68-106
stressor_info_t = {name, entry, stack_size, priority, opts[]}
```

| # | 名称 | 栈大小 | 优先级 | 类别 |
|---|------|--------|--------|------|
| 1 | vecmath | 16KB | 20 | 计算 |
| 2 | cpu | 16KB | 20 | 计算 |
| 3 | bitops | 16KB | 20 | 计算 |
| 4 | prime | 16KB | 19 | 计算 |
| 5 | fp | 16KB | 20 | 计算 |
| 6 | trig | 16KB | 20 | 计算 |
| 7 | atomic | 16KB | 20 | 计算 |
| 8 | context | 16KB | 20 | 计算 |
| 9 | ptr-chase | 16KB | 20 | 计算 |
| 10 | matrix | 32KB | 21 | 递归 |
| 11 | qsort | 32KB | 20 | 递归 |
| 12 | bsearch | 32KB | 20 | 递归 |
| 13 | str | 32KB | 20 | 递归 |
| 14 | vm | 64KB | 20 | 内存 |
| 15 | malloc | 64KB | 20 | 内存 |
| 16 | memcpy | 64KB | 20 | 内存 |
| 17 | memthrash | 64KB | 20 | 内存 |
| 18 | stream | 64KB | 20 | 内存 |
| 19 | hdd | 16KB | 25 | 文件 |
| 20 | open | 16KB | 25 | 文件 |
| 21 | copy-file | 16KB | 25 | 文件 |
| 22 | unlink | 16KB | 25 | 文件 |
| 23 | fstat | 16KB | 25 | 文件 |
| 24 | dentry | 16KB | 20 | 文件 |
| 25 | rename | 16KB | 20 | 文件 |
| 26 | pipe | 16KB | 20 | 文件 |
| 27 | stack | 64KB | 20 | 其他 |

### 2.3 执行模型

```
stress_run_one_job()
  │
  ├─ 创建 job_sem (信号量, 初始值 0)
  │
  ├─ for i in 0..num_instances:
  │     分配 stress_args_t
  │     设置 name, method, instance, time_end, max_ops
  │     创建线程 -> stress_thread_trampoline()
  │       └─ 调用 stressor_info_t.entry(args)
  │       └─ 完成后 sem_release(job_sem)
  │
  ├─ stress_wait_for_workers()
  │     循环 sem_take(job_sem, 1s_slice)
  │     直到 spawned_count 个信号或超时
  │
  ├─ 汇总 bogo_ops (所有 worker 累加)
  │     -> output_result (调用方传入)
  │     -> g_last_bogo   (全局缓存, 供 stress_ng_get_last_bogo_ops() 读取)
  │
  └─ 清理 (args, sem, tid_list; 若有 orphan 则故意泄漏防 crash)
```

### 2.4 关键数据结构流转

```
stress_args_t                          stress_bogo_t
┌─────────────────────┐               ┌──────────────────────┐
│ name: "cpu"         │               │ current_ops: uint64  │ <- 主要输出
│ method_name: "sqrt" │               │ max_ops:     uint64  │ <- 终止条件
│ instance: 0         │               │ metric_val[2]: double│ <- 附加指标
│ num_instances: 3    │  每个 worker   │ metric_name[2][32]   │
│ bogo: stress_bogo_t ├──累积写入──>  └──────────────────────┘
│ time_start / end    │                         │
│ user_data: entry()  │                  完成后汇总到
│ complete_sem        │                         │
│ stop_request        │               ┌─────────▼──────────┐
└─────────────────────┘               │ g_last_bogo        │
                                      │ (全局, 单次缓存)    │
                                      └─────────┬──────────┘
                                                │
                                      stress_ng_get_last_bogo_ops()
                                                │
                                      ┌─────────▼──────────┐
                                      │ test_stress.c      │
                                      │ s_last_bogo_ops    │
                                      └─────────┬──────────┘
                                                │
                                      test_stress_get_last_bogo_ops()
                                                │
                                ┌───────────────▼──────────────────┐
                                │ rtthread_entry.c                 │
                                │ collect_stress_result()          │
                                │ -> rtbench_stress_add_stressor() │
                                │ -> JSON 导出                     │
                                └──────────────────────────────────┘
```

---

## 3. 框架封装层的瓶颈点

数据流中有 **3 个瓶颈**，导致底层能力无法到达用户：

### 瓶颈 1: CLI 解析层 (`rtthread_entry.c:367-406`)

```
用户能输入的:  -s <name>  -t <sec>  -l  -q    (4 个参数)
底层能接受的:  name  -c  -t  --ops  --method  --<per-stressor-opt>  (6+ 类参数)
```

**丢失**: `-c`, `--ops`, `--method`, 所有 per-stressor 参数, `--job`/`--jobfile`

### 瓶颈 2: test_stress.c 中间层 (`test_stress.c:49-84`)

```c
// 构造 argv 时写死了:
argv[5] = "1";   // -c 永远为 1
// 没有传递 --ops, --method, --<per-stressor-opt> 的通道
```

**附加问题**: `stressor_names[]` 只有 8 个名字，`-s all` 只遍历这 8 个。

### 瓶颈 3: test-all 路径 (`rtthread_entry.c:195`)

```c
test_stress_run_stressor("cpu", p->stress_duration);
//                        ^^^^ stressor 名称硬编码
```

用户只能控制 `--stress-duration` 和 `--no-stress`，无法选择 stressor 种类。

---

## 4. Jobfile 数据中已暴露的 stressor 与参数

`stress_stored_job.c` 定义了 3 个内置 Job，每个 Job 精确规定了每个 stressor 在每个阶段应使用的参数：

### JOB_DATA_CPU (13 个 stressor)

| Stressor | 调控参数 | Stage1 (20%) | Stage5 (100%) |
|----------|---------|--------------|---------------|
| cpu | `--cpu-load`, `-c` | load=20, c=1 | load=100, c=5 |
| matrix | `--matrix-size`, `-c` | size=32, c=1 | size=128, c=5 |
| qsort | `--qsort-size`, `-c` | size=1024, c=1 | size=65536, c=5 |
| atomic | `--atomic-thread`, `-c` | thread=1, c=1 | thread=4, c=5 |
| bitops | `-c` | c=1 | c=5 |
| bsearch | `--bsearch-size`, `-c` | size=1024, c=1 | size=65536, c=5 |
| context | `--context-threads`, `-c` | threads=1, c=1 | threads=3, c=5 |
| fp | `-c` | c=1 | c=5 |
| prime | `-c` | c=1 | c=5 |
| stack | `--stack-size`, `-c` | size=4096, c=1 | size=65536, c=5 |
| str | `--str-size`, `-c` | size=1024, c=1 | size=16384, c=5 |
| trig | `-c` | c=1 | c=5 |
| vecmath | `-c` | c=1 | c=5 |

### JOB_DATA_MEMORY (6 个 stressor)

| Stressor | 调控参数 | Stage1 (20%) | Stage5 (100%) |
|----------|---------|--------------|---------------|
| memcpy | `--memcpy-size`, `-c` | size=32768, c=1 | size=524288, c=5 |
| stream | `--stream-elem`, `-c` | elem=2048, c=1 | elem=32768, c=5 |
| vm | `--vm-bytes`, `-c` | bytes=1MB, c=1 | bytes=8MB, c=3 |
| malloc | `--malloc-bytes`, `-c` | bytes=256, c=1 | bytes=16384, c=5 |
| memthrash | `--mem-size`, `-c` | size=64KB, c=1 | size=512KB, c=5 |
| ptr-chase | `--ptr-chase-pages`, `-c` | pages=128, c=1 | pages=128, c=5 |

### JOB_DATA_FILE (8 个 stressor)

| Stressor | 调控参数 | Stage1 (20%) | Stage5 (100%) |
|----------|---------|--------------|---------------|
| hdd | `--hdd-bytes`, `-c` | bytes=128KB, c=1 | bytes=4MB, c=5 |
| open | `--open-max`, `-c` | max=32, c=1 | max=40, c=5 |
| copy-file | `--copy-file-bytes`, `-c` | bytes=128KB, c=1 | bytes=4MB, c=5 |
| unlink | `-c` | c=1 | c=5 |
| fstat | `--fstat-files`, `-c` | files=10, c=1 | files=5, c=5 |
| dentry | `--dentries`, `-c` | dentries=32, c=1 | dentries=128, c=2 |
| rename | `-c` | c=1 | c=5 |
| pipe | `--pipe-data-size`, `-c` | size=512, c=1 | size=32768, c=5 |

---

## 5. 已发现的 Bug

### 5.1 内置 Jobfile 参数名不匹配

`stress_stored_job.c` 中有两处参数名与 stressor `opts[]` 注册名不一致：

| 行号 | Jobfile 中写的 | Stressor opts 注册名 | 后果 |
|------|---------------|---------------------|------|
| 多处 | `--atomic-thread` (单数) | `atomic-threads` (复数) | `stress_handle_stressor_opt()` 匹配失败, 静默忽略, 使用 config.h 默认值 |
| 多处 | `--mem-size` | `memthrash-size` | 同上 |

匹配逻辑在 `stress-ng.c:160-174`:
```c
if (strcmp(opt + 2, opt_ptr->opt_name) == 0) {
    // "atomic-thread" != "atomic-threads"  -> 不匹配, 跳过
}
```

### 5.2 文档与实现不一致

`docs/STRESS.md` 列出 27 个可用 stressor（含 I/O、atomic、context 等），
但 `test-stress` CLI (`stressor_names[]`) 实际只注册了 8 个。

---

## 6. 能力差距汇总

| 维度 | 框架 test-stress | 底层 stress_orig | 差距 |
|------|-----------------|-----------------|------|
| **Stressor 数** | 8 + all(遍历8个) | 27 + jobfile | 缺 19 个, 覆盖率 30% |
| **并发 worker** | 硬编码 1 | `-c <N>` 可配 | 完全未暴露 |
| **持续时间** | 整数秒 | 支持 s/m/h 后缀 | 轻微降级 |
| **操作数上限** | 无 | `--ops <N>` | 未暴露 |
| **子方法** | 无 | `--method <name>` (8个stressor支持) | 未暴露 |
| **Per-stressor 参数** | 无 | 30+ 个专属参数 | 完全未暴露 |
| **Jobfile 5级递增** | 无 | `--job cpu/memory/file/all` | 未暴露 |
| **外部 Jobfile** | 无 | `--jobfile <path>` | 未暴露 |
| **test-all 中 stressor 选择** | 仅 "cpu" | — | 硬编码 |
| **结果回收** | 仅 bogo_ops (uint64) | bogo_ops + metric_val[2] + metric_name[2] | 附加指标丢失 |

---

## 7. 关键源文件索引

| 文件 | 行号 | 角色 |
|------|------|------|
| `generator/rtthread_entry.c` | :367-406 | test-stress CLI 解析 (瓶颈 1) |
| `generator/rtthread_entry.c` | :234-267 | test-all CLI 解析 |
| `generator/rtthread_entry.c` | :192-197 | test-all 中调用 stress (瓶颈 3) |
| `generator/rtthread_entry.c` | :662-674 | stress 结果收集进 JSON |
| `generator/test_stress.h` | :27-37 | stressor 枚举定义 (仅 8 种) |
| `generator/test_stress.c` | :29-39 | stressor_names[] 字符串表 (仅 8 个) |
| `generator/test_stress.c` | :49-84 | argv 构造, `-c 1` 硬编码 (瓶颈 2) |
| `generator/test_stress.c` | :86-115 | run_config(), all 模式遍历 |
| `generator/stress_orig/stress_bench.c` | :23-62 | workload registry 适配器 (也是 `-c 1`) |
| `generator/stress_orig/common/stress-ng.c` | :68-106 | 底层 stressor 注册表 (27 种) |
| `generator/stress_orig/common/stress-ng.c` | :228-412 | `stress_run_one_job()` 核心执行 |
| `generator/stress_orig/common/stress-ng.c` | :261-284 | 底层参数解析 (完整能力) |
| `generator/stress_orig/common/stress-ng.c` | :463-621 | jobfile 执行器 |
| `generator/stress_orig/common/stress-ng.c` | :640-670 | `stress_ng_main()` 入口分发 |
| `generator/stress_orig/common/stress-ng.h` | :24-66 | 核心数据结构定义 |
| `generator/stress_orig/common/stress_stored_job.c` | 全文 | 内置 jobfile 数据 (含命名 bug) |
| `generator/stress_orig/common/config.h` | — | per-stressor 参数默认值 |
