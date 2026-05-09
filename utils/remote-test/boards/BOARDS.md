# RTOS-Bench 测试板卡端口配置

服务器 IP: `10.134.151.3`

## 翼辉 SylixOS 板卡

| 板卡名称 | 项目目录 | FTP 端口 | Telnet 端口 | 架构 | 备注 |
|----------|----------|----------|-------------|------|------|
| 飞腾派 (Phytium E2000Q) | `feiteng_rtos_bench` | 3018 | 3020 | ARM64 | FTC664×2 + FTC310×2 |
| 哪吒 (Nezha D1) | `nezha-rtos-bench` | 3012 | 3014 | RISC-V 64 | Allwinner D1 |
| 工控机-东土 | `gongkong_rtos_bench` | 3024 | 3026 | - | 东土科技工控机 |

## OneOS 板卡

| 板卡名称 | 项目目录 | FTP 端口 | Telnet 端口 | 架构 | 备注 |
|----------|----------|----------|-------------|------|------|
| 飞腾派 OneOS | - | 3021 | 3023 | ARM64 | - |

## 东土科技板卡

| 板卡名称 | 项目目录 | FTP 端口 | Telnet/SSH 端口 | 架构 | 备注 |
|----------|----------|----------|-----------------|------|------|
| 工控机-东土 | `gongkong_rtos_bench` | 3024 | 3026 (telnet) | - | - |
| 香橙派-东土 | - | 3027 | 3029 (telnet) / 3028 (ssh) | - | - |

## 其他板卡

| 板卡名称 | FTP 端口 | Telnet 端口 | 备注 |
|----------|----------|-------------|------|
| OriginPi | 3009 | 3011 | - |
| MH7700 | 3015 | 3017 | - |

## 连接方式

### Telnet 登录
```bash
telnet 10.134.151.3 <telnet_port>
# 用户名: root
# 密码: root
```

### FTP 上传
```bash
ftp 10.134.151.3 <ftp_port>
# 用户名: root
# 密码: root
# 上传目录: /apps/
```

### 使用 curl 上传 (推荐)
```bash
curl -T <local_file> ftp://10.134.151.3:<ftp_port>/apps/<remote_name> --user root:root
```

## test-schedule 验证状态

| 板卡 | Quick Mode (--cycles 3) | Full Mode (--cycles 5) | 备注 |
|------|-------------------------|------------------------|------|
| 飞腾派 SylixOS | ✅ 100.00/100 | ❌ MQTT 崩溃 | 2026-04-16 |
| 哪吒 SylixOS | ❌ EPNP 卡死 | - | RISC-V 性能较慢 |
| 工控机-东土 | ❌ 连接超时 | - | FTP 端口 3024 无响应 |

### 已知问题

1. **MQTT Workload Bug (飞腾派)**: Full Mode 下 MQTT 第 2 次迭代崩溃
   - 崩溃位置: `memcpy` ← `mg_iobuf_add` ← `mg_send` ← `mg_mqtt_pub`
   - 原因: Mongoose 库内存管理问题
   - 影响: Full Mode 无法完成，Quick Mode 正常（会跳过 MQTT）

2. **EPNP Workload Bug (哪吒)**: Phase 1 WCET 测量卡在 EPNP
   - 现象: EPNP 第 1 次测量后无输出，进程僵死 (Z 状态)
   - 可能原因: RISC-V 架构下 OpenGV/Eigen 计算问题
   - 影响: test-schedule 无法完成

3. **MODBUS/MQTT WCET > 2s**: 在 Quick Mode 下会被自动跳过
