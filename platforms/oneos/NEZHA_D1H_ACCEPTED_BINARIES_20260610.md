# OneOS 哪吒派 D1H 已验收二进制归档

本文记录 2026-06-10 通过完整板端验收的 OneOS 哪吒派 D1H `.out` 二进制版本。二进制文件本体保留在 Windows 本机固定归档目录中，仓库只记录路径、校验和和验收证据，避免把大体积或板端临时产物直接提交进 Git。

## 1. 归档位置

- Windows 归档目录：`C:\Users\hzt\yihui-workspace\oneos-nezha-artifacts\accepted\oneos-nezha-d1h-acceptance-20260610_132750`
- 验收日志目录：`C:\Users\hzt\yihui-workspace\rtos-bench\RTOS-Bench\utils\remote-test\logs\oneos-nezha-d1h-acceptance-20260610_132750`
- 总验收摘要：`C:\Users\hzt\yihui-workspace\rtos-bench\RTOS-Bench\utils\remote-test\logs\oneos-nezha-d1h-acceptance-20260610_132750\ACCEPTANCE_SUMMARY.md`
- 共享 TFTP 源：`rtbench:/tftp`
- 板卡：OneOS 哪吒派 D1H，`192.168.31.211`

## 2. 已归档二进制

| Windows 文件 | 大小 | SHA256 | TFTP 源 | 板端路径 | 用途 |
|---|---:|---|---|---|---|
| `ctest.out` | 1544096 | `275ed7e09eb13f06c8816255d19582091775ecc12be50b8a5803ab4954c53fe5` | `/tftp/oneos-nezha-d1h-current.out` | `/user/ctest.out` | RTOS-Bench 主模块，包含 OneOS 入口、realtime、schedule、stress、cmd、workloads |
| `schedrun.out` | 13048 | `866e53cc01adf0a21a43dff73f0033641d7383286dcc2516a70acb36d3af4562` | `/tftp/oneos-nezha-d1h-schedule-current.out` | `/user/schedrun.out` | 调用 `test-schedule` |
| `rtrt.out` | 12344 | `864632319df85e1f4b8ea493564f908d17a4e6c491e77707f39b8c99417bc8d0` | `/tftp/oneos-nezha-d1h-realtime-current.out` | `/user/rtrt.out` | 调用 `test-realtime` |
| `wlrun.out` | 12992 | `c0c39f848b70d96267129076c94928ee44b86f9ea825be631077a6fae52ed9de` | `/tftp/wlrun.out` | `/user/wlrun.out` | workloads 串行 quick 验收，导出 `/user/wlrun.json` |
| `strun.out` | 12776 | `2a115ad11576a821ef5a41dff787b2cf94f18d6f9175201cd4619f8f1b3cdd0f` | `/tftp/strun.out` | `/user/strun.out` | 调用 `test-stress -s cpu -t 1` |
| `allrun.out` | 12816 | `f952a14383982cbb8ce68137619d42eaed0c6d6f64479910bebbf55be2cd381b` | `/tftp/allrun.out` | `/user/allrun.out` | quick `test-all`，导出 `/user/nezha_testall.json` |
| `wlfull.out` | 12968 | `38d2ce65889d9eb858a7446f14a8dfaf06c7c01117318ea2007fd4cfae8422b7` | `/tftp/wlfull.out` | `/user/wlfull.out` | 非 quick workloads-only 全量轮次验收，导出 `/user/wlfull.json` |

## 3. 验收范围

本批次已完成：

- P0 `test-schedule`：`Final Score: 100.00 / 100`
- P1 `test-realtime`：`Benchmark completed with code: 0`
- P2 workloads 串行 quick：`workloads ret=0`
- P3 `test-stress`：`test-stress ret=0`
- P4 quick `test-all` + export/report：`test-all ret=0`
- P5 非 quick workloads-only：当前 OneOS D1H 镜像实际注册的 `stub`、`busywait`、`cusum`、`ewma` 均以 `rounds=10` 通过

## 4. 手工复现命令

板端先加载主模块：

```sh
tftp_client 192.168.31.110 get oneos-nezha-d1h-current.out /user/ctest.out
ld /user/ctest.out
```

然后按需加载 runner。加载 runner 后会自动执行对应测试：

```sh
tftp_client 192.168.31.110 get wlfull.out /user/wlfull.out
ld /user/wlfull.out
```

`wlfull.out` 等价于调用：

```sh
rtbench test-all -o /user/wlfull.json --no-realtime --no-schedule --no-stress --no-cmd
```

## 5. 校验方法

在 Windows 上核对归档文件：

```powershell
Get-FileHash -Algorithm SHA256 "C:\Users\hzt\yihui-workspace\oneos-nezha-artifacts\accepted\oneos-nezha-d1h-acceptance-20260610_132750\*.out"
```

在共享主机上核对 TFTP 当前入口：

```bash
ssh rtbench "sha256sum /tftp/oneos-nezha-d1h-current.out /tftp/oneos-nezha-d1h-schedule-current.out /tftp/oneos-nezha-d1h-realtime-current.out /tftp/wlrun.out /tftp/strun.out /tftp/allrun.out /tftp/wlfull.out"
```
