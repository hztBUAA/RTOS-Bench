# 板卡配置（赛迪内网）

更新时间：2026-11-18 10:55

每块板卡一份 `board_*.yaml`，直接用于 `deploy.py`。

## 板卡与地址

| 板卡 | 系统 | 架构 | 赛迪内网 IP | 原内网 IP | 配置文件 |
|---|---|---|---|---|---|
| 翼辉-龙芯 | SylixOS | loongarch64 | 192.168.137.200 | 192.168.31.200 | `board_sylixos_loongson.yaml` |
| 翼辉-香橙派 | SylixOS | aarch64 | 192.168.31.201 | - | `board_sylixos_orangepi.yaml` |
| 翼辉-哪吒 | SylixOS | riscv64 | 192.168.137.202 | 192.168.31.202 | `board_sylixos_nezha.yaml` |
| 翼辉工控机 | SylixOS | x86_64 | 192.168.31.203 | - | `board_sylixos_gongkong.yaml` |
| 翼辉-飞腾 | SylixOS | aarch64 | 192.168.31.204 | - | `board_sylixos_feiteng.yaml` |
| OneOS-飞腾 | OneOS | aarch64 | 192.168.137.214 | 192.168.31.205 | `board_oneos_feiteng.yaml` |
| 东土-工控机 | Intewell | x86_64 | 192.168.137.215 | 192.168.31.206 | `board_intewell_gongkong.yaml` |
| 东土-香橙派 | Intewell | aarch64 | 192.168.137.216 | 192.168.31.207 | `board_intewell_orangepi.yaml` |
| OneOS-香橙派 | OneOS | aarch64 | 192.168.137.217 | 192.168.31.208 | `board_oneos_orangepi.yaml` |
| 锐华-龙芯派 | ReWorks | loongarch64 | 192.168.137.218 | 192.168.31.209 | `board_reworks_loongson.yaml` |
| 锐华-飞腾派 | ReWorks | aarch64 | 192.168.137.210 | 192.168.31.210 | `board_reworks_feiteng.yaml` |
| OneOS-哪吒 | OneOS | riscv64 | 192.168.137.211 | 192.168.31.211 | `board_oneos_nezha.yaml` |
| 锐华-香橙派 | ReWorks | aarch64 | 192.168.137.212 | 192.168.31.212 | `board_reworks_orangepi.yaml` |
| OneOS-龙芯 | OneOS | loongarch64 | 192.168.137.213 | 192.168.31.213 | `board_oneos_loongson.yaml` |
| 东土-飞腾 | Intewell | aarch64 | 192.168.137.219 | 192.168.137.242（备用） | `board_intewell_feiteng.yaml` |

## 使用

```bash
python utils/remote-test/deploy.py -c utils/remote-test/boards/board_sylixos_loongson.yaml
```

默认连接方式为 SSH（22 端口），结果用 SFTP 取回。

板卡只开放 telnet 时，改两处：

```yaml
connection:
  type: "telnet"
  port: 23

result:
  retrieval_method: "terminal_capture"
```

## 上传与执行

`deploy.local_binary` 留空时跳过上传，直接运行板上已有程序。需要上传时取消注释，并指向该板卡的实际构建产物。

上传后板上二进制名取 `local_binary` 的文件名。若文件名不是 `rtbench`，同步修改 `test.command` 与 `result.remote_path` 不变。

`test.command` 默认执行 7 负载全量 `test-all`。快速冒烟在命令末尾加 `--quick`。

## 自检

执行前确认板卡 `env` 字段可用：

```bash
rtbench test-all --quick -o /tmp/rtbench_result.json
python3 -c "import json;e=json.load(open('/tmp/rtbench_result.json'))['env'];print(e['board'],e['os_name'],e['cpu_type'])"
```

`env.cpu_type` 应有值，不能是 `Unknown` 或 `0`。该字段由二进制按编译宏填写，与 board.yaml 无关。
