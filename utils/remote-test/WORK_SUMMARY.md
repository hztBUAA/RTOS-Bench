# 翼辉(SylixOS)板卡自动化测试 - 工作总结

## 完成时间
2025-04-27

## 已完成的工作

### 1. 配置文件更新 ✓
- [x] 标记 10.134.151.3 为废弃连接方式
- [x] 添加 SSH 跳板机配置到所有板卡配置文件
- [x] 更新内网地址映射 (192.168.31.x)
- [x] 修正飞腾派二进制文件路径 (feiteng_rtos_bench)

更新的配置文件：
- `boards/board_feiteng.yaml` - 飞腾派 E2000Q
- `boards/board_nezha.yaml` - 哪吒 D1 RISC-V
- `boards/board_gongkong.yaml` - 工控机 MH7700
- `boards/board_loongson.yaml` - 龙芯 LS2K1000
- `boards/board_orangepi.yaml` - 香橙派 RK3588

### 2. 测试脚本修复 ✓
- [x] 修复 Windows 环境下的编码问题 (UTF-8)
- [x] 实现 Telnet 协议协商 (IAC/DO/DONT/WILL/WONT)
- [x] 优化登录时序 (参考 PowerShell 脚本)
- [x] 更新 `telnet_test_remote.py` 读取新配置格式
- [x] 添加 subprocess 编码参数

### 3. 连接验证 ✓
- [x] 验证 SSH 跳板机连接 (10.134.151.45:1026)
- [x] 验证 SSH 密钥认证工作正常
- [x] 测试从跳板机到板卡的 Telnet 连接
- [x] 确认 4/5 板卡在线 (除飞腾派外)

### 4. 板卡状态检查 ✓
- [x] 龙芯 LS2K1000 (192.168.31.200) - 在线
- [x] 香橙派 RK3588 (192.168.31.201) - 在线
- [x] 哪吒 D1 (192.168.31.202) - 在线
- [x] 工控机 MH7700 (192.168.31.203) - 在线
- [ ] 飞腾派 E2000Q (192.168.31.204) - **离线**

### 5. 文档创建 ✓
- [x] 创建 `README_SETUP.md` - 配置说明文档
- [x] 创建 `WORK_SUMMARY.md` - 工作总结文档

## 当前状态

### 可用的测试脚本
1. **telnet_test_remote.py** - 主测试脚本
   - 通过 SSH 跳板机执行 telnet 测试
   - 支持读取新的配置格式
   - 已修复编码和协议问题

2. **test_all_yihui_boards.py** - 批量测试脚本
   - 自动测试所有 5 个板卡
   - 解析测试结果并生成报告
   - 保存结果到时间戳文件

3. **test_single_board.py** - 调试脚本
   - 简化的测试流程
   - 用于快速验证连接

### 技术问题已解决
- ✓ Windows GBK 编码问题
- ✓ Telnet IAC 协议协商
- ✓ SSH 跳板机连接
- ✓ 配置文件格式更新
- ✓ 二进制文件路径错误

## 待完成的工作

### 1. 飞腾派板卡恢复 ⏳
**优先级: 高**

当前状态: 离线 (No route to host)

需要操作:
- 检查板卡电源状态
- 检查网络连接
- 重启板卡服务

### 2. 验证其他板卡的二进制文件路径 ⏳
**优先级: 高**

需要检查:
- [ ] 龙芯: `/apps/` 目录下的实际文件名
- [ ] 香橙派: `/apps/` 目录下的实际文件名
- [ ] 哪吒: `/apps/` 目录下的实际文件名
- [ ] 工控机: `/apps/` 目录下的实际文件名

已知:
- ✓ 飞腾派: `/apps/feiteng_rtos_bench` (已验证并修正)

### 3. 执行完整自动化测试 ⏳
**优先级: 中**

等待飞腾派恢复后:
```bash
cd utils/remote-test
python test_all_yihui_boards.py
```

预期输出:
- 5 个板卡的测试结果
- Final Score 分数
- 测试耗时
- 结果报告文件

### 4. 更新 SOP 文档 ⏳
**优先级: 低**

需要更新:
- 标记废弃的连接方式
- 添加新的 SSH 跳板机方式
- 更新测试流程说明

## 测试命令参考

### 检查板卡在线状态
```bash
ssh -p 1026 rtbench@10.134.151.45 'for ip in 192.168.31.200 192.168.31.201 192.168.31.202 192.168.31.203 192.168.31.204; do echo -n "Testing $ip:23 ... "; timeout 2 python3 -c "import socket; s=socket.socket(); s.settimeout(2); s.connect((\"$ip\", 23)); print(\"OK\")" 2>&1 || echo "FAIL"; done'
```

### 检查板卡上的文件
```bash
ssh -p 1026 rtbench@10.134.151.45 "python3 << 'EOF'
import socket, time

IAC, DONT, DO, WONT, WILL = 255, 254, 253, 252, 251

def negotiate(sock, data):
    resp, text = b'', b''
    i = 0
    while i < len(data):
        if data[i] == IAC and i + 2 < len(data):
            cmd, opt = data[i+1], data[i+2]
            if cmd == DO: resp += bytes([IAC, WONT, opt])
            elif cmd == WILL: resp += bytes([IAC, DONT, opt])
            i += 3
        else:
            text += bytes([data[i]])
            i += 1
    if resp: sock.sendall(resp)
    return text

sock = socket.socket()
sock.settimeout(30)
sock.connect(('192.168.31.200', 23))  # 修改IP测试不同板卡

time.sleep(1.5)
negotiate(sock, sock.recv(4096))
sock.sendall(b'root\r\n')
time.sleep(0.5)
negotiate(sock, sock.recv(4096))
sock.sendall(b'root\r\n')
time.sleep(1)
negotiate(sock, sock.recv(4096))
time.sleep(0.5)
try:
    sock.settimeout(0.1)
    while True: negotiate(sock, sock.recv(4096))
except: pass

sock.settimeout(30)
sock.sendall(b'll /apps/\r\n')
time.sleep(1)
out = b''
try:
    sock.settimeout(1)
    for _ in range(5):
        data = sock.recv(4096)
        if not data: break
        out += negotiate(sock, data)
except: pass

print(out.decode('utf-8', errors='ignore'))
sock.close()
EOF
"
```

### 测试单个板卡
```bash
cd utils/remote-test
python telnet_test_remote.py -c boards/board_loongson.yaml -t test_schedule_quick
```

### 测试所有板卡
```bash
cd utils/remote-test
python test_all_yihui_boards.py
```

## 技术要点

### 1. SSH 跳板机认证
- 使用 SSH 密钥认证（已配置）
- 无需 sshpass
- 端口: 1026

### 2. Telnet 协议
- 必须处理 IAC 协议协商
- 拒绝所有选项: DO->WONT, WILL->DONT
- 登录时序: 1.5s -> username -> 0.5s -> password -> 1s

### 3. SylixOS Shell
- 命令提示符: `[root@sylixos:/root]#`
- 使用 `ll` 代替 `ls -l`
- 不支持标准 `find` 命令

### 4. 编码处理
- Windows 环境需要 UTF-8 编码
- subprocess 调用: `encoding='utf-8', errors='ignore'`

## 联系方式

如有问题，请参考:
- `README_SETUP.md` - 详细配置说明
- `boards/TESTING_SOP.md` - 测试 SOP 文档
- `../../docs/SOP.md` - 项目 SOP 文档
