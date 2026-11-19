# OneOS 哪吒派 D1H 已验收二进制归档

本文记录 2026-06-10 通过完整板端验收的 OneOS 哪吒派 D1H `.out` 二进制版本。二进制文件本体保留在 Windows 本机固定归档目录中，仓库只记录路径、校验和和验收证据，避免把大体积或板端临时产物直接提交进 Git。

2026-06-11 已使用 Windows 直连 TFTP + `COM9` 串口重新完成正式复验，验收脚本返回 `ACCEPTANCE_RC=0`。复验日志和摘要见本文第 6 节。

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

## 6. 2026-06-11 Windows 直连复验

本次复验不依赖 `rtbench:/tftp`，而是使用 Windows 本机 TFTP server：

```text
TFTP root:
C:\Users\hzt\yihui-workspace\oneos-nezha-artifacts\accepted\oneos-nezha-d1h-9workload-20260611_144550

TFTP bind:
192.168.31.100:69
```

正式复验前，已将该 TFTP root 内的 `ctest.out` 替换回 2026-06-10 已验收主模块：

```text
ctest.out size   = 1544096
ctest.out SHA256 = 275ED7E09EB13F06C8816255D19582091775ECC12BE50B8A5803AB4954C53FE5
```

正式复验日志：

```text
C:\Users\hzt\yihui-workspace\rtos-bench-oneos-nezha-pr\utils\remote-test\logs\oneos-nezha-d1h-win-direct-20260611_152709\oneos_nezha_serial_tftp_acceptance_20260611_152711.log
C:\Users\hzt\yihui-workspace\rtos-bench-oneos-nezha-pr\utils\remote-test\logs\oneos-nezha-d1h-win-direct-20260611_152709\oneos_nezha_serial_tftp_acceptance_20260611_152711.rc
C:\Users\hzt\yihui-workspace\rtos-bench-oneos-nezha-pr\utils\remote-test\logs\oneos-nezha-d1h-win-direct-20260611_152709\oneos_nezha_serial_tftp_acceptance_20260611_152711.md
```

复验结果：

```text
P-main-ctest       PASS
P0 test-schedule   PASS
P1 test-realtime   PASS
P2 workloads quick PASS
P3 test-stress     PASS
P4 test-all quick  PASS
P5 workloads full  PASS
ACCEPTANCE_RC=0
```

复验过程中确认 Windows 直连 TFTP 会偶发 `timeout`。`oneos_nezha_serial_tftp_acceptance.ps1` 已增强为短文件名部署、TFTP 重试、下载失败不执行 `ld`，避免把传输残缺文件误判为 RTOS-Bench 模块问题。

注意：`oneos-nezha-d1h-9workload-20260611_144550` 目录内曾生成过 9-workload 方向的新 `ctest.out`，其 SHA256 为 `54A1EB21749228CBCAB47F99E9458007F4732686E4A28B3F0AC77E54B6559F97`。该模块在当前 OneOS D1H image 下 `ld /user/ctest.out` 报 `ELF64_R_TYPE(rela->r_info)=7 unreloced`，未纳入验收二进制。当前正式通过范围仍以已验收主模块实际注册的 workloads 为准。
