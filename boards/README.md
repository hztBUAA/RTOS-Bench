# boards/ —— 每块板一份配置(per-board config)

把"板子差异"集中到这里,通用编排代码(`utils/remote-test/`)不硬编码任何板子 IP。
新增一块板 = 复制 `utils/remote-test/board_config_example.yaml` 或本目录已有 yaml,改 `connection.ip`、
`board.arch`、`deploy.mode`、`test.workload_allowlist` 等即可。

## 现场验收(on-site acceptance)在 ccid 上完成
- 测试主控 **ccid = rtos-test-master01 `192.168.137.101`**(`ssh ccid`)。
- TFTP 启动文件:`/data/tftpboot/<arch>/`;结果取回:anonymous FTP(二进制,勿用 cat 抓串口)。
- 编排脚本:`/data/rtbench/scripts/<board>/`;上传器:`/data/rtbench/bin/upload_flow_result.py`。
- **凭证**:真实 `.env` 已放 ccid 的 `/data/.env` 与 `/data/rtbench/.env`(600,root),`load_dotenv` 自动读取;
  本仓库 `.env` 仍 gitignore,**勿提交真实密钥**(模板见 `bin/flow.env.example` / `board` yaml 注释)。
- 权威 SOP:ccid `/data/rtbench/docs/NETWORK-TOPOLOGY-AND-BOARD-DEBUG.md`(= 本仓库 0627 同名文档)。

## 已有配置
- `ruihua-feiteng.yaml` —— 锐华 ReWorks 飞腾派 FTE2000 / AArch64 / 192.168.137.210 / TFTP 静态镜像网启动 / 全 9 负载。

## 字段说明
基础段(`board/connection/deploy/test/result`)对齐 `utils/remote-test/config_schema.py`;
扩展段(`deploy.mode/tftp_*/uboot_env`、`result.retrieval_method: ftp`、`flow_upload`、`firmware_notes`)
为板级特化,供闭环脚本(`scripts/<board>/*_flow.py`)与 `bin/upload_flow_result.py` 使用。
