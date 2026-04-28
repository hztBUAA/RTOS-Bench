# RTOS-Bench Entry 参数解析 SOP

本文档约束各 RTOS 平台入口文件的命令行行为，避免不同平台的
`*_entry.c` 在参数、help、fallback 上漂移。

## 目标

所有平台 entry 必须满足三条底线：

1. `-h`、`--help`、`help` 只打印帮助并退出，不能启动 benchmark。
2. 未知命令或未知参数必须打印 usage 并返回错误，不能 fallback 到默认 workload 或全量测试。
3. 同一条命令在 Linux/RT-Thread/OneOS/SylixOS 等平台上应尽量使用相同参数名、默认值和错误语义。

## 标准顶层命令

平台 entry 至少应识别以下命令：

| 命令 | 语义 |
| --- | --- |
| `rtbench [OPTIONS]` | 执行 workload benchmark |
| `rtbench test-all [OPTIONS]` | 执行端到端综合测试并导出结果 |
| `rtbench test-schedule [OPTIONS]` | 执行调度可行性测试 |
| `rtbench test-realtime [OPTIONS]` | 执行实时性能测试 |
| `rtbench test-stress [OPTIONS]` | 执行压力测试 |
| `rtbench test-cmd [OPTIONS]` | 执行 shell 命令支持测试 |
| `rtbench export-result [OPTIONS]` | 导出当前结果为 JSON |
| `rtbench -L` / `rtbench --list` | 列出 workload |
| `rtbench -h` / `rtbench --help` | 打印帮助 |

不支持的命令，例如 `rtbench test -schedule --help`、`rtbench foo`、
`rtbench test-schedule --bad-option`，必须返回错误并打印帮助。

## 标准 workload 参数

| 参数 | 语义 |
| --- | --- |
| `-b <name>` / `-w <name>` / `--workload <name>` | 选择 workload |
| `-p <sec>` / `--period <sec>` | 周期，单位秒 |
| `-d <sec>` / `--deadline <sec>` | deadline，单位秒 |
| `-t <count>` / `--tasks <count>` | 执行次数或任务激活次数 |
| `-f <prio>` / `--fifo <prio>` | FIFO 优先级 |
| `-c <cpu>` / `--core-affinity <cpu>` | CPU 亲和性 |
| `-A` / `--all-workloads` | 顺序运行所有 workload |
| `-G <cat[,cat2]>` / `--category <cat[,cat2]>` | 按类别过滤 workload |
| `-s` / `--run-workload-suite` | 运行简单 workload suite |
| `-q` | 降低输出详细程度 |

`-b/-w/--workload` 不能与 `-A/--all-workloads` 或 `-G/--category` 混用。

## `test-schedule` 参数

| 参数 | 语义 |
| --- | --- |
| `--cycles <n>` | 每个 task 的执行 cycles，默认 `TEST_SCHEDULE_CYCLES` |
| `--util-start <pct>` | 起始 utilization |
| `--util-end <pct>` | 结束 utilization |
| `--util-step <pct>` | utilization 步长 |
| `--quick` | 使用 smoke-test 默认值 |
| `-q` | 降低输出详细程度 |
| `-h` / `--help` / `help` | 打印 `test-schedule` 帮助并退出 |

解析要求：

1. 支持 `--cycles 1` 和 `--cycles=1` 两种形式。
2. `cycles >= 1`。
3. `1 <= util-start <= util-end <= 100`。
4. `1 <= util-step <= 100`。
5. 缺少参数值或数值非法时必须打印错误和 usage，不能继续运行。

## `test-all` 参数

`test-all` 是平台入口的端到端闭环命令，应顺序运行 realtime、
schedule、stress、cmd、workload 模块，并把结构化结果导出为 JSON。

| 参数 | 语义 |
| --- | --- |
| `-o <path>` / `--output <path>` | JSON 导出路径 |
| `--no-realtime` | 跳过 realtime 模块 |
| `--no-schedule` | 跳过 schedule 模块 |
| `--no-stress` | 跳过 stress 模块 |
| `--no-cmd` | 跳过 cmd 模块 |
| `--no-workload` | 跳过 workload 模块 |
| `-m` / `--multicore` | 启用 realtime multicore 子项 |
| `--quick` | 使用有界 smoke profile |
| `--schedule-cycles <n>` | 覆盖 schedule cycles |
| `--util-start/end/step <pct>` | 覆盖 schedule utilization 梯度 |
| `--stress-job <name>` | 覆盖 stress job，例如 `all-quick` |
| `-q` | 降低输出详细程度 |
| `-h` / `--help` / `help` | 打印 `test-all` 帮助并退出 |

解析要求：

1. `test-all --quick` 仍然运行所有模块，只把 schedule/stress/workload
   切到有界配置，适合作为板卡端到端连通性验证。
2. 不带 `--quick` 时，平台应使用正式默认值；如果时间过长，应通过
   `--schedule-cycles` 或 `--stress-job` 显式收窄，而不是在代码中静默降级。
3. `test-all` 中的 `--schedule-cycles`、`--util-start/end/step` 必须与
   `test-schedule` 的数值校验规则一致。
4. 缺少参数值、非法数值、未知参数必须打印错误和 usage，不能继续运行。

## `export-result` 参数

| 参数 | 语义 |
| --- | --- |
| `-o <path>` / `--output <path>` | JSON 导出路径 |
| `-h` / `--help` / `help` | 打印 `export-result` 帮助并退出 |

## 未知参数处理

每个 subcommand 的 parser 都应有明确的 final `else`：

```c
printf("[subcommand] Unknown option: %s\n", argv[i]);
print_usage();
return -1;
```

不能静默忽略未知参数。需要透传给子模块的参数必须放在明确的
`--opts <string>` 或等价白名单参数里，不能把任意未知 `--xxx`
当作合法参数吞掉。

## Help 行为

以下命令都必须只打印帮助，不能启动测试：

```bash
rtbench --help
rtbench help
rtbench test-all --help
rtbench test-schedule --help
rtbench test-realtime --help
rtbench test-stress --help
rtbench test-cmd --help
rtbench export-result --help
```

以下命令都必须报错并打印帮助，不能执行默认 workload：

```bash
rtbench test -schedule --help
rtbench test-schedule --cycles
rtbench test-schedule --cycles abc
rtbench test-schedule --unknown
rtbench test-all --bad-option
rtbench export-result --bad-option
rtbench --unknown
```

## 平台 entry 更新检查清单

修改任意平台 entry 时，至少检查：

1. 顶层 help 是否先于默认 workload 分支处理。
2. subcommand help 是否先于真正运行测试处理。
3. 参数缺值、非法数字、未知参数是否返回错误。
4. 默认无参是否打印帮助，还是按平台约定运行默认 workload；若平台选择默认运行，必须在文档中说明。
5. `test-schedule` 是否与本文档的参数和默认值保持一致。
6. `test-all` 是否能运行所有模块并导出 JSON，`--quick` 是否仍保留端到端语义。
7. 不支持某个参数时，是否明确报错，而不是静默忽略。
8. 新增参数是否同步到 usage 文本和本文档。
