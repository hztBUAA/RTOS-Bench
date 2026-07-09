# Intewell E2000Q test-schedule "SMP 死锁" — 已定位并修复（我方缺陷）

**板卡**：Phytium E2000Q（aarch64），Intewell（TTOS + POSIX），`CONFIG_TTOS_SMP=1`
**状态**：**已解决（RESOLVED）** — 根因为 RTOS-Bench 侧一处 `long double` 堆内存**未 16 字节对齐**，
并非 Intewell 堆分配器的 SMP 安全性问题。原先归因于 OS 侧 SMP 堆锁的判断**予以撤回**。

> 本文档原为"请 OS 侧排查 SMP 堆锁"的缺陷反馈；经东土工程师实测 + 我方复查，已确认是我方代码缺陷，
> 现更正为解决记录。历史结论（第 2、3 节的"SMP 证据/根因"）**作废**，保留仅供追溯。

---

## 1. 最终根因（更正）

`test-schedule` 看门狗 `wait_all_tasks_deadline()` 中：

```c
long double *last_progress_ts = (long double *)calloc((size_t)n, sizeof(long double));
```

- aarch64 上 `long double` 为 128 位，要求 **16 字节对齐**；
- Intewell/TTOS 的 `calloc` 只保证 8 字节对齐，且内核**开启了非对齐访问异常**；
- 因此看门狗对 `last_progress_ts[i]` 的首次写入触发**非对齐数据异常**，异常静默、无 fault 打印，
  整机表现为"死锁"。

## 2. 为何此前的"证据"全部指向（错误的）SMP 堆锁

| 原判断 | 实际解释 |
|---|---|
| 主线程一同冻住 | 看门狗**就是主线程**，非对齐写发生在主线程 → 主线程被陷阱冻住 |
| 无 fault 打印 | 非对齐数据异常静默（未接管打印） |
| 概率性、利用率越高越频繁 | `calloc` 返回指针的对齐随堆状态变化；利用率越高分配越多，越易返回非 16 对齐地址 |
| 两种调度实现都复现 | 两者共用**同一个**看门狗、**同一行** `calloc` |

## 3. 修复

在共享、平台无关的 `generator/test_schedule.c` 中，将看门狗进度时间戳数组由 `long double`
改为 **`double`**（8 字节，任何 `calloc` 都天然满足其对齐要求），从根上消除非对齐隐患：

```c
double *last_progress_ts = (double *)calloc((size_t)n, sizeof(double));
```

- `double` 精度对 ≥5 秒的停滞判定窗口绰绰有余，**无功能回归**；
- 不引入任何对齐分配器 / `memalign`，对**所有板卡 / 所有 RTOS**（rt-thread、oneos、sylixos、
  ruihua、linux …）都零风险——这是全框架唯一一处此类堆分配；
- 东土工程师已在板上用等价的 `memalign(16, …)` 对齐方案验证过：修复后 `test-schedule`
  全量 30%–100% 梯度**顺利跑完、无死锁**，佐证不存在独立的 SMP 死锁。（我方最终选用更简洁的
  `double` 方案，等价地消除该异常。）

## 4. 复现日志（历史，供追溯）

启动板卡、telnet 后运行 `rtbench test-schedule`。

**正常档（低利用率，calloc 恰好 16 对齐，可完成）**：
```
>>> Utilization Gradient: 30% <<<
Running task set for 3 cycles...
[test-schedule] all 9 task threads created; entering watchdog
Gradient 30% complete: MR = 0.0000 (0/27)
```

**"死锁"档（calloc 返回非 16 对齐，随机中高档）**：
```
>>> Utilization Gradient: 50% <<<
Running task set for 3 cycles...
[test-schedule] all 9 task threads created; entering watchdog
        <── 此后无任何输出（含每 5 秒主线程心跳），永不返回：主线程在看门狗首写 last_progress_ts 时被非对齐异常陷住
```

## 5. 结论

无需 Intewell OS 侧改动。此为 RTOS-Bench 我方缺陷，已在 `test_schedule.c` 修复并提交。
原"请 OS 侧排查 SMP 堆锁"的诉求作废。
