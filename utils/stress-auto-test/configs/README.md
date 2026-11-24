# 参考配置

这两份是已验证过连接参数的参考 `config.py`，来自功耗测试联调。

| 文件 | 板卡 | 系统 | 说明 |
|---|---|---|---|
| `config-oneos-phytium.py` | Phytium-Pi | oneos | 电源 `/dev/ttyUSB0`，DUT 串口 `/dev/ttyUSB1` |
| `config-sylix-loong.py` | LS2K1000LA | SylixOS | 电源 `/dev/ttyUSB0`，DUT 串口 `/dev/ttyUSB1` |

使用方法：

```bash
cp configs/config-oneos-phytium.py config.py
python test.py profile
python test.py power
python test.py dut --connection COM --read-seconds 5
python main.py
```

注意：

- 串口名按测试主机实际设备改，例如 `/dev/ttyUSB0` / `/dev/ttyUSB1`。
- 电源波特率 9600，DUT 波特率 115200。
- `boot_wait: 30.0` 用于等待 OneOS 固化系统启动完成。
- `EXCEL_EXPORTER.merge_result_json` 是相对 `config.py` 的路径，默认 `data/result.json`。
