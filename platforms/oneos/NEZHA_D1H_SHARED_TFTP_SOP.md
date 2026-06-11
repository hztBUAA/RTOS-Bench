# OneOS 哪吒派 D1H 共享 TFTP 验收流程

本文档说明如何把 OneOS 哪吒派的部署入口从个人 Windows 电脑迁到共享主机 `rtbench`，让多位同事可以在自己的电脑或厂家 IDE 中改代码、编译 `.out`，再统一通过 `rtbench:/tftp` 给板子拉取运行。

## 1. 网络角色

- 板子：OneOS 哪吒派 D1H，静态 IP `192.168.31.211`
- 共享 TFTP 主机：`rtbench`
  - 内网 IP：`192.168.31.110`
  - TFTP 根目录：`/tftp`
  - TFTP 服务：`tftpd-hpa`，当前以 `/tftp` 为 secure root
- 开发电脑：同事自己的电脑，可运行厂家 IDE 或 WSL/Linux 工具链

板子不需要知道某位同事的电脑地址，只需要能访问 `192.168.31.110`。

## 2. 共享主机状态检查

在任意可 SSH 到 `rtbench` 的电脑上执行：

```bash
ssh rtbench "hostname; ip -4 addr show eno1; ls -ld /tftp; systemctl is-active tftpd-hpa"
```

期望看到：

```text
192.168.31.110/24
/tftp
active
```

## 3. 语义化产物命名

同事可以在自己的电脑上完成代码修改和 OneOS `.out` 编译。最终目标是得到一个 OneOS 动态模块，例如厂家工程生成的：

```text
d1h-nezha_out.out
```

共享 TFTP 上不要长期使用 `ctfix.out`、`rtrt.out` 这类通用短名作为唯一产物名，否则多人并行测试时容易互相覆盖。推荐在 `/tftp` 下按板卡和批次建目录：

```text
/tftp/oneos/nezha-d1h/<YYYYMMDD_HHMMSS>/
```

每个批次目录内使用语义化文件名：

```text
oneos-nezha-d1h-rtbench-<YYYYMMDD_HHMMSS>.out
oneos-nezha-d1h-runner-schedule-<YYYYMMDD_HHMMSS>.out
oneos-nezha-d1h-runner-realtime-<YYYYMMDD_HHMMSS>.out
oneos-nezha-d1h-runner-workloads-<YYYYMMDD_HHMMSS>.out
oneos-nezha-d1h-runner-stress-<YYYYMMDD_HHMMSS>.out
oneos-nezha-d1h-runner-testall-<YYYYMMDD_HHMMSS>.out
```

板端 FAT 目标路径仍建议使用短名，例如 `/user/ctest.out`、`/user/rtrt.out`。这样可以同时满足两点：

- 共享主机上语义化、可追溯、避免多人覆盖。
- 板端 `/user` 文件名短、稳定，避免 FAT/模块名路径问题。

## 4. 上传到 rtbench:/tftp

从开发电脑执行：

```bash
TS="$(date +%Y%m%d_%H%M%S)"
REMOTE_DIR="/tftp/oneos/nezha-d1h/${TS}"
ssh rtbench "mkdir -p ${REMOTE_DIR}"
scp d1h-nezha_out.out "rtbench:${REMOTE_DIR}/oneos-nezha-d1h-rtbench-${TS}.out"
```

如果是辅助 runner，也上传到同一个批次目录：

```bash
scp schedrun.out "rtbench:${REMOTE_DIR}/oneos-nezha-d1h-runner-schedule-${TS}.out"
scp rtrt.out     "rtbench:${REMOTE_DIR}/oneos-nezha-d1h-runner-realtime-${TS}.out"
scp wlrun.out    "rtbench:${REMOTE_DIR}/oneos-nezha-d1h-runner-workloads-${TS}.out"
scp strun.out    "rtbench:${REMOTE_DIR}/oneos-nezha-d1h-runner-stress-${TS}.out"
scp allrun.out   "rtbench:${REMOTE_DIR}/oneos-nezha-d1h-runner-testall-${TS}.out"
```

为了方便板端手工输入，可以给“当前验收批次”创建短别名。短别名只作为当前批次入口，真实归档仍以批次目录为准：

```bash
ssh rtbench "
  cp ${REMOTE_DIR}/oneos-nezha-d1h-rtbench-${TS}.out /tftp/oneos-nezha-d1h-current.out
  cp ${REMOTE_DIR}/oneos-nezha-d1h-runner-schedule-${TS}.out /tftp/oneos-nezha-d1h-schedule-current.out
  cp ${REMOTE_DIR}/oneos-nezha-d1h-runner-realtime-${TS}.out /tftp/oneos-nezha-d1h-realtime-current.out
  cp ${REMOTE_DIR}/oneos-nezha-d1h-runner-workloads-${TS}.out /tftp/oneos-nezha-d1h-workloads-current.out
  cp ${REMOTE_DIR}/oneos-nezha-d1h-runner-stress-${TS}.out /tftp/oneos-nezha-d1h-stress-current.out
  cp ${REMOTE_DIR}/oneos-nezha-d1h-runner-testall-${TS}.out /tftp/oneos-nezha-d1h-testall-current.out
"
```

上传后确认：

```bash
ssh rtbench "ls -l ${REMOTE_DIR}; ls -l /tftp/oneos-nezha-d1h-*-current.out"
```

## 5. 板端部署

在 OneOS shell 中执行：

```sh
tftp_client 192.168.31.110 get oneos-nezha-d1h-current.out /user/ctest.out
ld /user/ctest.out
list_lmodule
```

成功标志：

```text
TFTP client get file end, err=0
module[/user/ctest.out] loaded
modlue[/user/ctest.out] stat[1]
```

注意：当前哪吒派 image 下，动态 `.out` 内的 `SH_CMD_EXPORT` 不一定会把 `rtbench` 注册到主 shell，因此验收时使用 runner 模块间接调用 `cmd_rtbench_stub`。

## 6. 验收命令顺序

按以下顺序从 `rtbench:/tftp` 拉取 runner 并加载：

```sh
tftp_client 192.168.31.110 get oneos-nezha-d1h-schedule-current.out /user/schedrun.out
ld /user/schedrun.out

tftp_client 192.168.31.110 get oneos-nezha-d1h-realtime-current.out /user/rtrt.out
ld /user/rtrt.out

tftp_client 192.168.31.110 get oneos-nezha-d1h-workloads-current.out /user/wlrun.out
ld /user/wlrun.out

tftp_client 192.168.31.110 get oneos-nezha-d1h-stress-current.out /user/strun.out
ld /user/strun.out

tftp_client 192.168.31.110 get oneos-nezha-d1h-testall-current.out /user/allrun.out
ld /user/allrun.out
```

当前 runner 语义：

- `schedrun.out`：quick `test-schedule`
- `rtrt.out`：`test-realtime`
- `wlrun.out`：workloads 串行 smoke
- `strun.out`：`test-stress -s cpu -t 1`
- `allrun.out`：quick `test-all --quick --no-export`

## 7. 多人协作建议

- 不要把板子的 TFTP 源指向个人电脑，统一使用 `192.168.31.110`。
- 同事只需要能 SSH/SCP 到 `rtbench`，即可发布自己的 `.out`。
- 多人同时测试时，不要直接覆盖 `/tftp/*current.out`。先上传到自己的批次目录，例如：

```bash
TS="$(date +%Y%m%d_%H%M%S)"
REMOTE_DIR="/tftp/oneos/nezha-d1h/${USER}-${TS}"
ssh rtbench "mkdir -p ${REMOTE_DIR}"
scp d1h-nezha_out.out "rtbench:${REMOTE_DIR}/oneos-nezha-d1h-rtbench-${USER}-${TS}.out"
```

板端临时验证可以直接拉取批次目录中的文件：

```sh
tftp_client 192.168.31.110 get oneos/nezha-d1h/zhangsan-20260610_153000/oneos-nezha-d1h-rtbench-zhangsan-20260610_153000.out /user/ctest.out
ld /user/ctest.out
```

对正式验收，再由验收负责人把该批次发布成 `current` 别名：

```bash
ssh rtbench "cp /tftp/oneos/nezha-d1h/zhangsan-20260610_153000/oneos-nezha-d1h-rtbench-zhangsan-20260610_153000.out /tftp/oneos-nezha-d1h-current.out"
```

## 8. 常见问题

### 串口兜底恢复流程

如果 Telnet 无法连接、TFTP 不通，或者运行较长时间后怀疑网络控制面异常，先不要判断为板端验收失败。哪吒派应优先使用直连串口作为兜底控制面：

```powershell
powershell -ExecutionPolicy Bypass -File utils\remote-test\oneos_nezha_serial_recover.ps1
```

脚本会通过 `COM9` 执行：

```sh
ifconfig
set_if e00 192.168.31.211 192.168.31.1 255.255.255.0
default_netif e00
telnetd start
ping 192.168.31.1
ping 192.168.31.110
ping 192.168.31.107
list_lmodule
```

默认模式只要求串口 shell 和板端 IP 恢复，适合先确认 OneOS 没有死机。若要把网络作为继续 TFTP/验收的硬门槛，增加：

```powershell
powershell -ExecutionPolicy Bypass -File utils\remote-test\oneos_nezha_serial_recover.ps1 -RequireNetwork
```

如果确实需要软重启，可增加 `-Reboot`：

```powershell
powershell -ExecutionPolicy Bypass -File utils\remote-test\oneos_nezha_serial_recover.ps1 -Reboot -RequireNetwork
```

若 `-Reboot` 后串口没有重新出现 shell，或网口仍 `LINK_UP` 但 ping 网关失败，需要人工断电重启或检查网线/交换机/VLAN；串口日志仍会保留在 acceptance 日志目录下。

### 串口 + TFTP 一键完整验收

网络正常时，可以直接通过串口控制、TFTP 拉取、`ld` 加载 runner 的方式完整验收：

```powershell
powershell -ExecutionPolicy Bypass -File utils\remote-test\oneos_nezha_serial_tftp_acceptance.ps1
```

该脚本会执行：

1. 串口恢复控制面：`default_netif e00`、`telnetd start`、`ifconfig`、`ping 192.168.31.110`
2. 从 `rtbench:/tftp` 拉取并加载 `/user/ctest.out`
3. 按顺序加载 `schedrun.out`、`rtrt.out`、`wlrun.out`、`strun.out`、`allrun.out`、`wlfull.out`
4. 分别检查 `test-schedule`、`test-realtime`、workloads quick、`test-stress`、quick `test-all`、非 quick workloads full 的通过标志
5. 生成 `.log`、`.rc` 和 Markdown summary

如果网络/TFTP 当前不可达，但板端 `/user` 已经保留了通过验收的 `.out`，可以先用串口-only 方式复验完整运行链路：

```powershell
powershell -ExecutionPolicy Bypass -File utils\remote-test\oneos_nezha_serial_tftp_acceptance.ps1 -UseExistingBoardFiles
```

注意：`-UseExistingBoardFiles` 不重新从 TFTP 拉取二进制，适合确认 OneOS shell、模块加载和测试入口仍可运行；正式发布验收仍应在网络恢复后去掉该参数，强制从 `rtbench:/tftp` 拉取当前归档版本。

### ping 通但 Telnet 没输出

`ping` 只能证明网络栈活着，不能证明 shell 可用。Telnet 端口能连也只表示 `telnetd` 监听还在。真正可用的标准是 `help`、`ifconfig` 有回显。

### Telnet 无回显但串口可用

优先用串口执行部署和 `ld`。关闭其他 Telnet/MobaXterm 会话，避免 OneOS shell 被第一个客户端绑定。

### TFTP file not found

确认文件是否真的在 `rtbench:/tftp` 根目录：

```bash
ssh rtbench "ls -l /tftp/<file>"
```

板端 `tftp_client` 的文件名是相对 `/tftp` 的路径。
