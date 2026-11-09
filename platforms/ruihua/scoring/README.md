# RTOS-Bench 7 负载算分工具（锐华版）

## 1. 用途

输入板端 `rtbench_test_all` 生成的 JSON，直接输出结果展示表对应的分数列。

固定 7 负载口径：

- 实时控制：PID
- 感知：FAST、ICP
- 通信：MODBUS、MQTT
- 运维：EWMA、CUSUM

EKF、EPnP 不参与评分。

## 2. 运行环境

- Python 3.7 或更高版本
- 不需要安装第三方库
- 不需要数据库
- 不需要联网

## 3. 快速开始

```bash
python score_rtbench.py result.json
```

运行后生成两个文件：

- `scores.json`：完整明细
- `scores.csv`：可直接用 Excel 打开

Windows 用法：

- 把 result.json 拖到 run_score.bat 上，直接运行。
- 或在命令行执行 `run_score.bat result.json`。

输出文件写在运行命令的当前目录。用 run_score.bat 时会写在工具目录。

## 4. 输入从哪里来

板端执行：

```text
rtbench_test_all
```

结果默认写到：

```text
/rtbench_result.json
```

把这个文件拷到电脑，作为本工具的输入。

## 5. 算分规则

基准固定为 SylixOS @ 龙芯 2K1000LA，已经写进 `baseline.json`，不需要另外提供。

延迟类基准比：

```text
R = 基准值 ÷ 实测值
```

单项得分：

```text
S = 100 × R ÷ (0.25 + R)
```

四类负载：

```text
实时控制 = PID 得分
感知     = (FAST 得分 + ICP 得分) ÷ 2
通信     = (MODBUS 得分 + MQTT 得分) ÷ 2
运维     = (EWMA 得分 + CUSUM 得分) ÷ 2
T        = 四类得分的平均
```

实时性能：

```text
调度 = 100 × (1 − 平均截止时间错失率)
平均错失率 = 总错失 job ÷ 总 job
基础实时 = 0.1×上下文切换 + 0.1×中断 + 0.4×调度时延 + 0.4×多核开销
（多核开销当前无正式基准，工具按前三项归一化，见第 7 节）
RT = 0.5 × 基础实时 + 0.5 × 调度
```

另外两项：

```text
F = 60（占位）
P = 静态、CPU、内存、文件四项功耗得分的平均
功耗 R = 基准功耗 ÷ 实测功耗
```

总分：

```text
总分 = 0.2×F + 0.3×RT + 0.4×T + 0.1×P
```

因为 F = 60，当前总分上限是 92。

## 6. 功耗怎么传

优先从 JSON 的 `modules.test-stress.power` 读取，格式与 stress-auto-test 的 `excel_exporter` 合并结果一致：

```json
{
  "modules": {
    "test-stress": {
      "power": [
        {"name": "standby", "power_data": 6579.6,    "power_unit": "J"},
        {"name": "cpu",     "power_data": 5131.69,   "power_unit": "J"},
        {"name": "memory",  "power_data": 21282.312, "power_unit": "J"},
        {"name": "file",    "power_data": 12282.114, "power_unit": "J"}
      ]
    }
  }
}
```

约定：

- `standby` 对应算分口径的 `static`（静态功耗），工具自动映射；也接受直接写 `static`。
- 同名项重复出现时取最后一条。stress-auto-test 是追加写入，最后一条最新。
- `placeholder=true` 或 `power_data <= 0` 的项视为缺失。
- `power_unit` 支持 `J`、`kJ`、`Wh`、`kWh`，统一换算成焦耳。

四项齐全，P 和总分才会计算。

命令行也可以直接传，并按项覆盖 JSON 里的值：

```bash
python score_rtbench.py result.json \
  --power-static 6579.6 \
  --power-cpu 5131.69 \
  --power-memory 21282.312 \
  --power-file 12282.114
```

## 7. 基础实时怎么处理

当前口径下，多核开销没有正式基准数据，工具只按单核三项复算：

- 上下文切换
- 中断
- 调度时延（20 项子项得分的平均）

三项权重按 0.1、0.1、0.4 归一化。

这个值是参考值，可能和正式结果展示表的「基础实时」相差 1 到 2 分。

要以正式表为准时，直接指定：

```bash
python score_rtbench.py result.json --base-realtime 89.98
```

输入里即使有多核数据，当前也不纳入基础实时计算，工具会给出提示。

## 8. 输出示例

下面示例使用 `--base-realtime 89.98` 对齐正式结果表。

```text
RTOS-Bench 7 负载算分
输入: result.json
----------------------------------------
实时控制            91.30
感知              91.73
通信              95.89
运维              88.28
T               91.80
----------------------------------------
调度              97.02
基础实时            89.98
RT              93.50
----------------------------------------
P               57.51
F               60.00
总分              82.52
----------------------------------------
```

## 9. baseline.json 里有什么

- 7 个负载的基准耗时，单位 ms
- 上下文切换、中断、20 项调度时延基准，单位 us
- 4 项功耗基准，单位 J

基准来源：

- 结果展示表的「基准/参考值」列
- SylixOS @ 龙芯 2K1000LA 的结果 JSON

## 10. 注意

1. 先用板端 `rtbench_test_all` 生成结果，再用本工具算分。
2. `rtbench_quick("pid")` 是单项调试记录，不是正式评分记录。
3. 平均错失率按总 job 计算，不是各梯度错失率的平均。
4. 缺 P 时总分不计算，这是预期行为。
5. 输出默认写在当前工作目录，文件名是 `scores.json` 和 `scores.csv`，重复运行会覆盖。
6. 退出码：0 表示总分已算出；2 表示缺少 P 或实时数据，或输入/参数错误；1 表示 run_score.bat 未收到输入文件。
7. 输入的 `avg_time_ms` 为空时，工具会用 `exec_time_ms ÷ rounds` 还原单轮耗时。
