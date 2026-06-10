# OneOS 哪吒派 D1H 共享 TFTP 验收流程

本文档说明如何把 OneOS 哪吒派的部署入口从个人 Windows 电脑迁到共享主机 `rtbench`，让多位同事可以在自己的电脑或厂家 IDE 中改代码、编译 `.out`，再统一通过 `rtbench:/tftp` 给板子拉取运行。

## 1. 网络角色

- 板子：OneOS 哪吒派 D1H，静态 IP `192.168.31.211`
- 共享 TFTP 主机：`rtbench`
  - 内网 IP：`192.168.31.110`
  - TFTP 根目录：`/tftp`
  - TFTP 服务：`tftpd-hpa`，以 `/tftp` 为 secure root
- 开发电脑：同事自己的电脑，可运行厂家 IDE、WSL 或 Linux 工具链

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

## 3. 目录与命名规则

不要把所有人的产物都直接覆盖到 `/tftp/ctfix.out`。共享 TFTP 侧使用语义化目录和时间戳，板端 `/user` 侧使用稳定短名。

推荐目录：

```text
/tftp/oneos/nezha-d1h/
├── 20260610_132750/
│   ├── ctest.out
│   ├── schedrun.out
│   ├── rtrt.out
│   ├── wlrun.out
│   ├── strun.out
│   ├── allrun.out
│   └── manifest.md
└── current/
    ├── ctest.out
    ├── schedrun.out
    ├── rtrt.out
    ├── wlrun.out
    ├── strun.out
    ├── allrun.out
    └── manifest.md
```

命名建议：

- TFTP 归档目录：`/tftp/oneos/nezha-d1h/<YYYYMMDD_HHMMSS>/`
- TFTP 当前稳定目录：`/tftp/oneos/nezha-d1h/current/`
- 主模块文件名：`ctest.out`
- runner 文件名：`schedrun.out`、`rtrt.out`、`wlrun.out`、`strun.out`、`allrun.out`
- 板端加载路径：`/user/ctest.out`、`/user/<runner>.out`

为什么板端仍使用短名：

- 当前 runner 通过 `os_module_find("/user/ctest.out")` 查找主模块，板端模块路径必须稳定。
- OneOS/FAT 文件系统和动态模块加载器对短文件名更稳。
- 多人隔离应该放在 TFTP 源目录，而不是放在板端 `/user` 目录。

## 4. 同事改代码后的编译流程

同事可以在自己的电脑上完成代码修改和 OneOS `.out` 编译。最终目标是得到 OneOS 动态模块，例如：

```text
d1h-nezha_out.out
```

把主模块发布为 TFTP 侧的 `ctest.out`：

```bash
TS="$(date +%Y%m%d_%H%M%S)"
DEST="/tftp/oneos/nezha-d1h/${TS}"

ssh rtbench "mkdir -p '${DEST}'"
scp d1h-nezha_out.out "rtbench:${DEST}/ctest.out"
```

如果同事同时生成了 runner：

```bash
scp schedrun.out rtrt.out wlrun.out strun.out allrun.out "rtbench:${DEST}/"
```

写入 manifest：

```bash
ssh rtbench "cat > '${DEST}/manifest.md'" <<EOF
# OneOS Nezha D1H artifact set

- Timestamp: ${TS}
- Board: oneos-nezha-d1h
- Main module: ctest.out
- Runners: schedrun.out, rtrt.out, wlrun.out, strun.out, allrun.out
- Source branch: <branch>
- Source commit: <commit>
- Builder: <name>
EOF
```

## 5. 发布 current 稳定入口

某个时间戳目录通过编译和基础检查后，再发布为 `current`：

```bash
TS="<YYYYMMDD_HHMMSS>"
ssh rtbench "rm -rf /tftp/oneos/nezha-d1h/current && \
  mkdir -p /tftp/oneos/nezha-d1h/current && \
  cp /tftp/oneos/nezha-d1h/${TS}/* /tftp/oneos/nezha-d1h/current/"
```

正式验收只从 `current` 拉取，避免误拉某位同事的临时产物。

## 6. 板端部署

在 OneOS shell 中执行：

```sh
tftp_client 192.168.31.110 get oneos/nezha-d1h/current/ctest.out /user/ctest.out
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

## 7. 验收命令顺序

按以下顺序从 `current` 拉取 runner 并加载：

```sh
tftp_client 192.168.31.110 get oneos/nezha-d1h/current/schedrun.out /user/schedrun.out
ld /user/schedrun.out

tftp_client 192.168.31.110 get oneos/nezha-d1h/current/rtrt.out /user/rtrt.out
ld /user/rtrt.out

tftp_client 192.168.31.110 get oneos/nezha-d1h/current/wlrun.out /user/wlrun.out
ld /user/wlrun.out

tftp_client 192.168.31.110 get oneos/nezha-d1h/current/strun.out /user/strun.out
ld /user/strun.out

tftp_client 192.168.31.110 get oneos/nezha-d1h/current/allrun.out /user/allrun.out
ld /user/allrun.out
```

当前 runner 语义：

- `schedrun.out`：quick `test-schedule`
- `rtrt.out`：`test-realtime`
- `wlrun.out`：workloads 串行 smoke
- `strun.out`：`test-stress -s cpu -t 1`
- `allrun.out`：quick `test-all --quick --no-export`

## 8. 多人协作建议

- 不要把板子的 TFTP 源指向个人电脑，统一使用 `192.168.31.110`。
- 同事只需要能 SSH/SCP 到 `rtbench`，即可发布自己的 `.out`。
- 每个人先上传到自己的时间戳目录，不直接覆盖 `current`。
- 只有通过基础检查的目录才复制到 `current`。
- PR 或验收日志里必须写清楚使用的 TFTP 目录，例如：

```text
TFTP artifact set: /tftp/oneos/nezha-d1h/20260610_132750
Published as: /tftp/oneos/nezha-d1h/current
```

## 9. 常见问题

### ping 通但 Telnet 没输出

`ping` 只能证明网络栈活着，不能证明 shell 可用。Telnet 端口能连也只表示 `telnetd` 监听还在。真正可用的标准是 `help`、`ifconfig` 有回显。

### Telnet 无回显但串口可用

优先用串口执行部署和 `ld`。关闭其他 Telnet/MobaXterm 会话，避免 OneOS shell 被第一个客户端绑定。

### TFTP file not found

确认文件是否真的在 `rtbench:/tftp` 根目录：

```bash
ssh rtbench "ls -l /tftp/oneos/nezha-d1h/current/<file>"
```

板端 `tftp_client` 的文件名是相对 `/tftp` 的路径，例如：

```sh
tftp_client 192.168.31.110 get oneos/nezha-d1h/current/ctest.out /user/ctest.out
```
