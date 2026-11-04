# 赛迪 POSIX 冒烟测试（SylixOS 四架构）

本文记录赛迪 QEMU 仿真平台上 SylixOS 四种架构的 POSIX `open()` 冒烟结果。

## 结果

| 架构 | 实例 | 打开 /dev/null | 打开不存在路径 | close | 清理 | 结论 |
|---|---|---|---|---|---|---|
| aarch64 | sylixos-aarch64 | PASS（fd=6） | PASS（ENOENT） | PASS | 已确认 | PASS |
| loongarch64 | sylixos-loongarch64 | PASS（fd=6） | PASS（ENOENT） | PASS | 已确认 | PASS |
| riscv64 | sylixos-riscv64 | PASS（fd=6） | PASS（ENOENT） | PASS | 已确认 | PASS |
| x86_64 | sylixos-x86-64 | PASS（fd=7） | PASS（ENOENT） | PASS | 已确认 | PASS |

原始 runId 与机器可读结果见 `results.csv`。

## 覆盖范围

赛迪 v2 脚本只覆盖 `open()` 与 `close()`：

- 只读打开 `/dev/null`，应成功并返回描述符。
- 只读打开不存在的路径，应失败并返回 `ENOENT`。
- `close` 关闭描述符，平台确认清理。

**read、write、lseek、fsync、ftruncate、stat、fstat、mkdir 未覆盖。** 这是脚本本身的用例范围，不代表平台缺少这些能力。

## 文件

| 文件 | 说明 |
|---|---|
| `tech-003_POSIX接口测试大纲_SylixOS.md` | tech-003 大纲填充版（markdown） |
| `tech-003_POSIX接口测试大纲_SylixOS.docx` | 同上（Word） |
| `results.csv` | 四架构结果汇总 |

## 已知限制

- 未上传 ELF，不验证应用层链接行为。
- 未修改任何 OS 镜像。
- Token 只允许运行固定冒烟用例，不能执行任意命令。
- x86_64 openEuler 实例当前未运行，未计入。
