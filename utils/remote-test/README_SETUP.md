# 翼辉(SylixOS)板卡自动化测试 - 配置说明

## 更新日期
2025-04-27

## 重要变更

### 废弃的连接方式
**10.134.151.3 FRP端口转发方式已废弃**，请勿继续使用。

旧方式（已废弃）：
```
直接连接: 10.134.151.3:3020 -> 板卡
```

### 正确的连接方式
**通过SSH跳板机连接到内网板卡**

新方式（推荐）：
```
本地 -> SSH跳板机(10.134.151.45:1026) -> Telnet到内网板卡(192.168.31.x:23)
```

## 板卡配置

所有板卡配置文件已更新，包含以下信息：

### 1. 跳板机配置
```yaml
jumphost:
  host: "10.134.151.45"
  port: 1026
  username: "rtbench"
  password: "rtbench"  # 实际使用SSH密钥认证
```

### 2. 板卡内网地址映射

| 板卡名称 | 内网地址 | 架构 | 状态 |
|---------|---------|------|------|
| 龙芯 LS2K1000 | 192.168.31.200:23 | loongarch | 在线 |
| 香橙派 RK3588 | 192.168.31.201:23 | aarch64 | 在线 |
| 哪吒 D1 | 192.168.31.202:23 | riscv64 | 在线 |
| 工控机 MH7700 | 192.168.31.203:23 | x86_64 | 在线 |
| 飞腾派 E2000Q | 192.168.31.204:23 | aarch64 | 离线(2025-04-27) |

### 3. 板卡上的二进制文件路径

根据实际检查，飞腾派上的文件名为：
- 实际文件: `/apps/feiteng_rtos_bench`
- ~~配置错误: `/apps/hzt_feiteng_rtos_bench`~~ (已修正)

其他板卡需要验证实际文件名。

## 使用方法

### 前提条件
1. SSH密钥已配置到跳板机 (rtbench@10.134.151.45:1026)
2. Python 3.x 已安装
3. 已安装依赖: `pip install pyyaml`

### 测试单个板卡
```bash
cd utils/remote-test
python telnet_test_remote.py -c boards/board_feiteng.yaml -t test_schedule_quick
```

### 测试所有板卡
```bash
cd utils/remote-test
python test_all_yihui_boards.py
```

## 已知问题

### 1. 飞腾派板卡离线
- 时间: 2025-04-27 02:22
- 症状: No route to host (Errno 113)
- 需要: 检查板卡电源和网络连接

### 2. 编码问题
- Windows环境下需要在subprocess调用中指定 `encoding='utf-8', errors='ignore'`
- 已在所有脚本中修复

### 3. Telnet协议协商
- SylixOS的telnet服务器需要正确处理IAC协议协商
- 已在远程脚本中实现negotiate_telnet函数

### 4. 命令执行时序
- 登录后需要适当的延时等待shell提示符
- 参考时序: login等待1.5s, username后0.5s, password后1s

## 测试脚本说明

### telnet_test_remote.py
主测试脚本，通过SSH跳板机执行telnet测试：
1. 读取板卡配置文件
2. 创建Python测试脚本
3. 通过SCP上传到跳板机
4. 通过SSH在跳板机上执行
5. 捕获输出并解析结果

### test_all_yihui_boards.py
批量测试脚本，自动测试所有5个翼辉板卡：
1. 逐个调用telnet_test_remote.py
2. 解析测试输出中的"Final Score"
3. 生成测试报告

### test_single_board.py
简化的调试脚本，用于快速验证连接。

## 下一步工作

1. 等待飞腾派板卡恢复在线
2. 验证其他4个板卡上的二进制文件路径
3. 更新对应的配置文件
4. 执行完整的自动化测试
5. 生成最终测试报告

## 技术细节

### Telnet协议协商
```python
IAC = 255  # Interpret As Command
DONT = 254
DO = 253
WONT = 252
WILL = 251

# 响应策略: 拒绝所有选项协商
if cmd == DO:
    response += bytes([IAC, WONT, opt])
elif cmd == WILL:
    response += bytes([IAC, DONT, opt])
```

### SSH跳板机认证
- 使用SSH密钥认证（无需密码）
- 测试命令: `ssh -p 1026 rtbench@10.134.151.45 "echo test"`

### SylixOS Shell特性
- 不支持标准的`find`命令
- 使用`ll`代替`ls -l`
- 命令提示符: `[root@sylixos:/root]#`
