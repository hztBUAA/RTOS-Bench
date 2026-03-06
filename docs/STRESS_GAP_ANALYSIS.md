# test-stress 能力差距分析

> 日期: 2026-03-06
> 分支: fix/stress03-06
> 状态: 待修复

## 背景

`test-stress` 命令是 `rtos_stress`（stress-ng RTOS 移植版）的框架封装层。
当前封装层仅透传了底层引擎约 15%-20% 的能力，存在严重的能力降级。

本文档记录完整的差距清单，供团队讨论修复方案。

---

## 1. 全局参数对比

| 参数 | stress_orig (底层引擎) | test-stress (框架 CLI) | 状态 |
|------|----------------------|----------------------|------|
| stressor 名称 | 位置参数, 27 种 | `-s <name>`, 仅 8 种 | ❌ 降级 |
| 持续时间 | `-t <time>`, 支持 `s/m/h` 后缀 | `-t <sec>`, 仅整数秒 | ⚠️ 降级 |
| 并发 worker 数 | `-c <N>`, 可配置 | 硬编码 `1` | ❌ 未暴露 |
| 最大操作数 | `--ops <N>` | — | ❌ 未暴露 |
| 子方法选择 | `--method <name>` | — | ❌ 未暴露 |
| 内置 jobfile | `--job <name>` (cpu/memory/file/all) | — | ❌ 未暴露 |
| 外部 jobfile | `--jobfile <path>` | — | ❌ 未暴露 |
| 列出 stressor | — | `-l` / `--list` | ✅ 框架增强 |
| 安静模式 | 内部 silent flag | `-q` | ✅ 已暴露 |

### 框架调用方式（硬编码）

```c
// test_stress.c:49-84
argv = {"rtos_stress", name, "-t", "10s", "-c", "1"};
//                                         ^^^^^^^^ 永远只有 1 个 worker
```

---

## 2. Stressor 注册差距

### 框架已注册（8 个）

```c
// test_stress.h 枚举
STRESS_TYPE_CPU, STRESS_TYPE_MATRIX, STRESS_TYPE_VM,
STRESS_TYPE_MALLOC, STRESS_TYPE_MEMCPY, STRESS_TYPE_PRIME,
STRESS_TYPE_TRIG, STRESS_TYPE_FP
```

### 底层支持但框架未注册（19 个）

| 类别 | 缺失的 stressor | 说明 |
|------|-----------------|------|
| **计算密集型** | `vecmath`, `bitops`, `atomic`, `context`, `ptr-chase` | 含向量运算、原子操作、上下文切换等关键场景 |
| **递归/重逻辑** | `qsort`, `bsearch`, `str` | 排序、查找、字符串操作 |
| **内存操作** | `memthrash`, `stream`, `stack` | 含 STREAM 带宽测试、栈深度压力 |
| **文件系统** | `hdd`, `open`, `copy-file`, `unlink`, `fstat`, `dentry`, `rename`, `pipe` | **整个 I/O 类别全部缺失** |

> **影响**: `test-stress -s all` 只能跑 8 个 stressor，而非文档描述的 27 个。
> `docs/STRESS.md` 中列出了 27 个 stressor 但实际 CLI 只能执行 8 个，**文档与实现不一致**。

---

## 3. Per-Stressor 参数（完全未暴露）

底层引擎每个 stressor 都有 1-2 个专属调参选项，框架全部未透传：

### 计算密集型

| Stressor | 参数 | 默认值 | 说明 |
|----------|------|--------|------|
| `cpu` | `--cpu-load` | 100 (%) | CPU 负载百分比 0-100 |
| `vecmath` | `--vecmath-loops` | 100 | 循环次数 |
| `bitops` | `--bitops-loops` | 1000 | 循环次数 |
| `prime` | `--prime-start` | 10000000 | 素数起始值 |
| `fp` | `--fp-loops` | 2048 | 循环次数 |
| `trig` | `--trig-loops` | 5000 | 循环次数 |
| `atomic` | `--atomic-threads` | 4 | 原子操作竞争线程数 |
| `context` | `--context-threads` | 2 | 上下文切换线程数 |
| `ptr-chase` | `--ptr-chase-pages` | 4096 | 追踪页数 (min: 4) |

### 递归/重逻辑

| Stressor | 参数 | 默认值 | 说明 |
|----------|------|--------|------|
| `matrix` | `--matrix-size` | 64 | 矩阵维度 16-128 |
| `qsort` | `--qsort-size` | 1024 | 数组元素数 128-1048576 |
| `bsearch` | `--bsearch-size` | 262144 | 数组大小 128-2097152 |
| `str` | `--str-size` | 256 | 字符串缓冲区 32-65536 |

### 内存操作

| Stressor | 参数 | 默认值 | 说明 |
|----------|------|--------|------|
| `vm` | `--vm-bytes` | 262144 (256KB) | 4KB-16MB, 支持 k/M/G 后缀 |
| `malloc` | `--malloc-bytes`, `--malloc-max` | 4096, 32 | 分配大小 / 最大分配槽位 |
| `memcpy` | `--memcpy-loops`, `--memcpy-size` | 64, 32768 | 迭代次数 / 缓冲区大小 |
| `memthrash` | `--memthrash-size` | 65536 (64KB) | 1KB-4MB |
| `stream` | `--stream-elem` | 1024 | 128-262144 |

### 文件系统

| Stressor | 参数 | 默认值 | 说明 |
|----------|------|--------|------|
| `hdd` | `--hdd-bytes` | 65536 (64KB) | 4KB-8MB |
| `open` | `--open-max` | 32 | 4-256 |
| `copy-file` | `--copy-file-bytes` | 65536 (64KB) | 4KB-4MB |
| `unlink` | `--unlink-files` | 32 | max: 256 |
| `fstat` | `--fstat-files` | 32 | — |
| `dentry` | `--dentries` | 32 | 1-512 |
| `rename` | `--rename-file-size` | 16 | 0-256KB |
| `pipe` | `--pipe-data-size` | 4096 | 64-65536 |

### 其他

| Stressor | 参数 | 默认值 | 说明 |
|----------|------|--------|------|
| `stack` | `--stack-size` | 16384 (16KB) | 2KB-64KB |

---

## 4. --method 子方法（完全未暴露）

8 个 stressor 支持通过 `--method` 选择特定算法，框架未提供此能力：

| Stressor | 可选 method |
|----------|-------------|
| `cpu` | `sqrt`, `bitops`, `matrixprod`, `ackermann`, `fibonacci`, `prime`, `all` |
| `matrix` | `prod`, `add`, `sub`, `trans`, `mean`, `identity`, `all` |
| `vm` | `write64`, `read64`, `rand-set`, `toggle`, `walk-1`, `galpat-1`, `gray`, `rowhammer`, `modulo-x`, `all` |
| `memcpy` | `libc`, `builtin`, `naive`, `naive_o0`, `naive_o1`, `naive_o2`, `naive_o3`, `all` |
| `prime` | `inc`, `sieve`, `factorial`, `all` |
| `trig` | `cos`, `cosf`, `cosl`, `sin`, `sinf`, `sinl`, `sincos`, `tan`, `tanf`, `tanl`, `all` |
| `fp` | `floatadd`, `floatsub`, `floatmul`, `floatdiv`, `doubleadd`, `doublesub`, `doublemul`, `doublediv`, `ldoubleadd`, `ldoublesub`, `ldoublemul`, `ldoublediv`, `all` |
| `memthrash` | `chunk1`, `chunk8`, `chunk64`, `chunk256`, `memset`, `memset64`, `memmove`, `matrix`, `prefetch`, `swap`, `lock`, `spinread`, `tlb`, `all` |

---

## 5. 已发现的 Bug

### 5.1 内置 Jobfile 参数名不匹配

`generator/stress_orig/common/stress_stored_job.c` 中的 jobfile 数据存在命名错误：

| Jobfile 中使用 | Stressor 注册名 | 后果 |
|---------------|----------------|------|
| `--atomic-thread` (单数) | `atomic-threads` (复数) | 参数被静默忽略，使用默认值 |
| `--mem-size` | `memthrash-size` | 参数被静默忽略，使用默认值 |

### 5.2 文档与实现不一致

`docs/STRESS.md` 列出了 27 个可用 stressor（含 I/O、atomic、context 等），但 `test-stress` CLI 实际只能执行其中 8 个。用户按文档操作会发现大部分 stressor 无法使用。

---

## 6. 影响评估

| 影响维度 | 说明 |
|---------|------|
| **测试覆盖度** | 仅覆盖 8/27 = 30% 的 stressor，I/O 类 0% 覆盖 |
| **并发测试** | 无法测试多 worker 场景，无法暴露并发相关问题 |
| **精细调参** | 无法调整任何 per-stressor 参数，无法针对特定场景深入测试 |
| **方法选择** | 无法选择子算法，只能跑 `all` 默认组合 |
| **Jobfile 分级** | 无法使用内置的 cpu/memory/file/all 分级压力方案 |
| **可复现性** | 缺少 `--ops` 支持，无法按固定操作数执行确定性测试 |

---

## 7. 关键源文件定位

| 文件 | 行号 | 关注点 |
|------|------|--------|
| `generator/rtthread_entry.c` | 453-487 | CLI 参数解析，仅 4 个选项 |
| `generator/test_stress.h` | 18-30 | stressor 枚举，仅 8 种 |
| `generator/test_stress.c` | 49-84 | argv 构造，`-c 1` 硬编码 |
| `generator/stress_orig/common/stress-ng.c` | 68-106 | 底层 stressor 注册表，27 种 |
| `generator/stress_orig/common/stress-ng.c` | 261-283 | 底层参数解析，支持完整选项 |
| `generator/stress_orig/common/stress_stored_job.c` | — | 内置 jobfile，含命名 bug |

---

## 8. 修复建议（待讨论）

### 方案 A: 最小修复 — 补齐 stressor 注册

- 将枚举扩展到 27 种
- `-s all` 覆盖全部 stressor
- 工作量小，但仍无法调参

### 方案 B: 中等修复 — 透传核心参数

- 补齐 stressor 注册
- 暴露 `-c`（worker 数）、`--ops`、`--method`
- 不透传 per-stressor 参数（保持 CLI 简洁）

### 方案 C: 完整修复 — 全能力透传

- 补齐 stressor 注册
- 暴露所有全局参数
- 支持 `--<opt-name> <value>` 任意 per-stressor 参数透传
- 支持 `--job` / `--jobfile`
- 修复内置 jobfile 命名 bug
- 同步更新 `docs/STRESS.md`

### 推荐

**方案 B** 作为第一步：覆盖最关键的能力缺口（stressor 数量 + 并发 + method），同时保持 CLI 易用性。per-stressor 参数可在后续按需添加。
