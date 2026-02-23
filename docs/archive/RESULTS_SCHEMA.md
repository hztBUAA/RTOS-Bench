# rt-bench 结果入库参考（草案）

面向下游消费者，描述从 rt-bench 运行输出解析到数据库的字段设计。兼容现有输出（CSV/日志）与未来扩展。

## rt-bench 当前输出要点
- 主 CSV（当日志级别为 `LOG_LEVEL_FILE` 或指定 `-l 2` 并设置 `-o <file>`）：每个周期一行，字段顺序固定：
  1) `period_start_cycles`
  2) `period_end_cycles`
  3) `job_end_cycles`
  4) `job_deadline_cycles`
  5) `job_elapsed_cycles`
  6) `period_start_sec`
  7) `period_end_sec`
  8) `job_end_sec`
  9) `job_deadline_sec`
  10) `job_elapsed_sec`
  11) `deadline_met` (1=满足，0=miss)
  12) `job_utilization`
  13) `job_density`
  14+) 若开启 PMCs（AARCH64 CORTEX_A53 或 X86_64 CORE_I7 且 `memory_profiling_enable=1`）追加：
      - `l1_refs`,`l1_misses`,`l1_miss_ratio_pct`,`l2_refs`,`l2_misses`,`l2_miss_ratio_pct`,`instructions_retired`,`cpu_clock_count`
  15+) 若编译 `EXTENDED_REPORT` 且当前 workload 提供 `log_header()/log_data()`，会再追加自定义列。
- 可选性能采样 CSV（文件名在主输出加 `_perf_profile` 后缀），由 `performance_sampler.c` 生成，含时间戳 + 采样值（具体列见编译目标 CPU）。
- 运行期 stdout/stderr 日志：包含参数解析、定时器/亲和力设置、错误信息等。

## 推荐数据表
### 1) `runs`
| 字段 | 类型 | 说明 |
| --- | --- | --- |
| id | bigint PK | 自增 |
| created_at | timestamptz | 入库时间 |
| bench_version | text | git commit / tag |
| platform | text | linux / rt-thread / sylixos / oneos / dongtu / ruihua |
| workload | text | busywait/fast/... |
| category | text | detection/network/...（来自 workload 定义，可空） |
| period_sec | numeric | 命令行 `-p` |
| deadline_sec | numeric | `-d`，无则 =period |
| tasks | bigint | `-t` |
| prio | int | `-f` |
| cpu_mask | int | `-c` 转换的掩码 |
| mem_prealloc_bytes | bigint | `-m` |
| memory_profiling | bool | `-M` 开关 |
| log_level | int | 0=ERR,1=INFO,2=FILE,3=TRACE |
| output_path | text | 原始 CSV 路径（可为空） |
| notes | jsonb | 运行备注/环境（CPU 型号、板卡、编译选项等） |

### 2) `jobs`
逐周期记录，主 CSV 一行对应一条。
| 字段 | 类型 | 说明 |
| --- | --- | --- |
| id | bigint PK |
| run_id | bigint FK -> runs.id |
| seq | bigint | 周期序号，从 0 开始 |
| period_start_cycles | numeric |
| period_end_cycles | numeric |
| job_end_cycles | numeric |
| job_deadline_cycles | numeric |
| job_elapsed_cycles | numeric |
| period_start_sec | double precision |
| period_end_sec | double precision |
| job_end_sec | double precision |
| job_deadline_sec | double precision |
| job_elapsed_sec | double precision |
| deadline_met | boolean |
| job_utilization | double precision |
| job_density | double precision |
| l1_refs | bigint | 可空 |
| l1_misses | bigint | 可空 |
| l1_miss_ratio_pct | double precision | 可空 |
| l2_refs | bigint | 可空 |
| l2_misses | bigint | 可空 |
| l2_miss_ratio_pct | double precision | 可空 |
| instructions_retired | bigint | 可空 |
| cpu_clock_count | bigint | 可空 |
| workload_custom | jsonb | 由 EXTENDED_REPORT 自定义列转存为 JSON（列名->值） |

### 3) `perf_samples`（可选）
| 字段 | 类型 | 说明 |
| --- | --- | --- |
| id | bigint PK |
| run_id | bigint FK |
| ts_sec | double precision | 采样时间 |
| counters | jsonb | 采样器导出的各项（列名->值） |

## 落库流程示例
1) 运行：`rtbench -b fast -p 1 -t 100 -q -l 2 -o fast_run.csv`  
2) 解析：按逗号读 `fast_run.csv`，第一行是表头；逐行写入 `jobs`，生成 `run_id` 关联。  
3) 元数据：将运行命令、平台、git 提交、板卡信息填入 `runs`。  
4) 如存在 `<output>_perf_profile.csv`，同样解析到 `perf_samples`。  
5) 如编译了 EXTENDED_REPORT：表头末尾的新列转成 `workload_custom` 的键值对（名称取列名）。  
6) 失败/异常：若日志含 deadline miss，可在 `runs.notes` 里追加 `"deadline_miss": true`。

## 下游最小参考（仅关心及时性）
- 关注字段：`deadline_met`、`job_elapsed_sec`、`job_utilization`、`job_density`。  
- 典型查询：按 workload + platform 聚合 deadline miss 率；或统计 99.9% job_elapsed_sec。

## 示例：FAST 工作负载完整流程（非平凡 workload）

### 运行命令
```
rtbench -b fast -p 1 -t 5 -q -l 2 -o fast_run.csv
```
- `-b fast`：FAST 角点提取 workload  
- `-p 1`：周期 1s  
- `-t 5`：运行 5 个周期  
- `-l 2`：LOG_LEVEL_FILE，输出 CSV  
- `-o fast_run.csv`：主 CSV 文件名  
- 未指定 deadline，则 deadline=period

### 主 CSV 样例（含表头 + 前 2 行）
```
period_start(clock_cycles),period_end(clock_cycles),job_end(clock_cycles),job_deadline(clock_cycles),job_elapsed(clock_cycles),period_start(seconds),period_end(seconds),job_end(seconds),job_deadline(seconds),job_elapsed(seconds),deadline_status(1=met),job_utilization,job_density
102340000,102680000,102670000,103000000,330000,0.000000,1.000120,1.000110,1.000000,0.000110,1,0.00011,0.00011
203400000,203740000,203725000,204000000,325000,1.000000,2.000105,2.000090,2.000000,0.000090,1,0.00009,0.00009
```
- 每行对应一个周期；`deadline_status=1` 表示未 miss。  
- 如启用 PMCs 或 EXTENDED_REPORT，会在末尾追加对应列。

### 落库映射示例
- `runs` 表（示例值）：  
  - platform=`linux`（按实际填写）  
  - workload=`fast`  
  - period_sec=1, deadline_sec=1, tasks=5, prio=100, cpu_mask=0  
  - log_level=2, output_path=`fast_run.csv`  
  - notes：CPU 型号 / git commit / 设备信息
- `jobs` 表（seq=0,1 对应上面两行）：  
  - period_start_cycles=102340000 / 203400000  
  - period_end_cycles=102680000 / 203740000  
  - job_end_cycles=102670000 / 203725000  
  - job_deadline_cycles=103000000 / 204000000  
  - job_elapsed_cycles=330000 / 325000  
  - period_start_sec=0.0 / 1.0  
  - period_end_sec=1.000120 / 2.000105  
  - job_end_sec=1.000110 / 2.000090  
  - job_deadline_sec=1.000000 / 2.000000  
  - job_elapsed_sec=0.000110 / 0.000090  
  - deadline_met=true / true  
  - job_utilization=0.00011 / 0.00009  
  - job_density=0.00011 / 0.00009  
  - PMCs / workload_custom：本例未启用，填 NULL
- 若存在 `_perf_profile.csv`，同一 run_id 写入 `perf_samples`。

### 快速复用步骤
1) 运行命令得到 CSV（如上）。  
2) 读表头，按字段映射写入 `jobs`；生成 `run_id` 写入 `runs`。  
3) 若有性能采样 CSV，同步写入 `perf_samples`。  
4) deadline miss 率可在 `runs.notes` 标记（可选）。  
