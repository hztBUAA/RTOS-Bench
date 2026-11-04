# TECH-003 POSIX 接口测试大纲（SylixOS 仿真环境）

更新时间：2026-11-01

## 1. 测试目的

在赛迪 QEMU 仿真平台上，验证 SylixOS 四种架构实例的 POSIX `open()` 调用兼容性，覆盖成功打开、失败打开与描述符清理。

## 2. 被测对象与环境

| 项 | 内容 |
|---|---|
| 仿真平台 | https://saidi.kniter.xyz |
| 被测系统 | SylixOS |
| 架构 | aarch64、loongarch64、riscv64、x86_64 |
| 实例 | sylixos-aarch64、sylixos-loongarch64、sylixos-riscv64、sylixos-x86-64 |
| 测试时间 | 2026-10-29 |
| 镜像改动 | 无，四种实例使用平台现有镜像 |

## 3. 测试工具

- 脚本：`saidi_posix_smoke.py`（v2）
- 运行环境：Python 3.8+，无需第三方依赖
- 认证：平台签发的 180 天专用 Token（不登录、不使用 Cookie）
- 执行位置：QEMU Guest 内部，不以宿主机结果代替

## 4. 测试方法与判定

统一用例为 POSIX `open()`：

1. 只读打开 `/dev/null`，应成功并返回文件描述符。
2. 只读打开带本次唯一标识的不存在路径，应失败并返回 `ENOENT`。
3. 使用 `close` 关闭成功打开的描述符。

SylixOS 由原生 Shell 的 `open` / `close` 命令调用系统接口。判定依据为平台记录的本次运行起止标记与断言响应；串口日志按本次标记截取，旧 PASS 命令回显不计入。

## 5. 测试结果（四架构）

| 架构 | 实例 | runId | 打开 /dev/null | 打开不存在路径 | close | 清理 | 结论 |
|---|---|---|---|---|---|---|---|
| aarch64 | sylixos-aarch64 | `635a6197d3abdc3e67b47a79` | PASS（fd=6） | PASS（ENOENT） | PASS（fd=6） | 已确认 | PASS |
| loongarch64 | sylixos-loongarch64 | `7d7532ebb60fcb4321e74ea9` | PASS（fd=6） | PASS（ENOENT） | PASS（fd=6） | 已确认 | PASS |
| riscv64 | sylixos-riscv64 | `fec6ef093d0a6f4f52a45ac0` | PASS（fd=6） | PASS（ENOENT） | PASS（fd=6） | 已确认 | PASS |
| x86_64 | sylixos-x86-64 | `fc5b4780256f0f4191d6d93c` | PASS（fd=7） | PASS（ENOENT） | PASS（fd=7） | 已确认 | PASS |

四种 SylixOS 架构全部通过。

## 6. 分项结果（tech-003 大纲）

| 序号 | 测试项目 | 覆盖情况 | 测试结果 | 说明 |
|---|---|---|---|---|
| 1 | open | 已覆盖 | PASS（4/4） | 只读打开 /dev/null 成功，返回 fd 6/6/6/7 |
| 2 | close | 已覆盖 | PASS（4/4） | 关闭上述 fd，平台确认清理 |
| 3 | read | 未覆盖 | 待测 | 本轮脚本仅测 open/close |
| 4 | write | 未覆盖 | 待测 | 同上 |
| 5 | lseek | 未覆盖 | 待测 | 同上 |
| 6 | fsync | 未覆盖 | 待测 | 同上 |
| 7 | ftruncate | 未覆盖 | 待测 | 同上 |
| 8 | stat | 未覆盖 | 待测 | 同上 |
| 9 | fstat | 未覆盖 | 待测 | 同上 |
| 10 | mkdir | 未覆盖 | 待测 | 同上 |

说明：本轮赛迪脚本 v2 的统一用例只覆盖 `open`，并用 `close` 清理描述符。其余接口不能据此判定为通过。

## 7. 结论

- SylixOS 四种架构的 POSIX `open()` 调用兼容性通过。
- `open` 失败路径返回 `ENOENT`，行为符合预期。
- `close` 清理完成，无描述符泄漏。
- 本轮为单接口冒烟测试，不构成完整 POSIX 兼容性验收。

## 8. 证据

| 架构 | 串口输出（节选） |
|---|---|
| aarch64 | `open file return: 6 dev 0 inode 0 size 0`；`open file return: -1 error: No such file or directory`；`close 6` |
| loongarch64 | `open file return: 6 dev 0 inode 0 size 0`；`open file return: -1 error: No such file or directory`；`close 6` |
| riscv64 | `open file return: 6 dev 0 inode 0 size 0`；`open file return: -1 error: No such file or directory`；`close 6` |
| x86_64 | `open file return: 7 dev 0 inode 0 size 0`；`open file return: -1 error: No such file or directory`；`close 7` |

完整日志见交付包的 `examples/<架构>-sylixos/` 目录：`result.json`、`console.log`、`serial.log`、`test.log`。

## 9. 局限与后续

- 仅覆盖 `open` / `close`，其余 8 个接口需补充用例。
- x86_64 openEuler 实例当前未运行，脚本返回 `VM_NOT_RUNNING`，未计入通过。
- 未上传 ELF，不验证应用层链接行为。
- 未修改任何 OS 镜像。