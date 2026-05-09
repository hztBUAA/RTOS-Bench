# SylixOS 板卡远程测试 SOP

本文档记录在 SylixOS 板卡上进行远程测试的标准操作流程和常见问题解决方案。

## 板卡信息

| 板卡 | 架构 | FTP 端口 | Telnet 端口 | 项目目录 | 二进制路径 |
|------|------|----------|-------------|----------|------------|
| 飞腾派 | ARM64 | 3018 | 3020 | `feiteng_rtos_bench` | `/apps/hzt_feiteng_rtos_bench` |
| 哪吒 | RISC-V | 3012 | 3014 | `nezha-rtos-bench` | `/apps/rtos-bench` |
| 工控机 | x86_64 | 3015 | 3017 | `gongkong_rtos_bench` | `/apps/hzt_gongkong_rtos_bench` |

**服务器 IP**: `10.134.151.3`
**默认登录**: root/root

## 编译和上传

### 编译
```bash
cd C:\Users\hzt\yihui-workspace\<项目目录>
make clean && make all
```

### 上传二进制
```bash
# 飞腾派
curl -T "Debug\strip\feiteng_rtos_bench" -u root:root "ftp://10.134.151.3:3018/apps/hzt_feiteng_rtos_bench"

# 哪吒
curl -T "Debug\strip\nezha-rtos-bench" -u root:root "ftp://10.134.151.3:3012/apps/rtos-bench"

# 工控机
curl -T "Debug\strip\gongkong_rtos_bench" -u root:root "ftp://10.134.151.3:3015/apps/hzt_gongkong_rtos_bench"
```

## 测试执行

### 方式一：自动化脚本 (推荐)

使用 `telnet-exec.ps1` 脚本自动登录并执行测试：

```powershell
# 需要设置 MSYS_NO_PATHCONV=1 防止路径被转换
MSYS_NO_PATHCONV=1 powershell -ExecutionPolicy Bypass -File "telnet-exec.ps1" `
  -HostName "10.134.151.3" `
  -Port 3020 `
  -Command "/apps/hzt_feiteng_rtos_bench test-schedule --cycles 3" `
  -Timeout 600
```

### 方式二：手动 Telnet

```bash
telnet 10.134.151.3 <端口>
# 输入用户名: root
# 输入密码: root
# 执行命令: /apps/<二进制名> test-schedule --cycles 3
```

## 常见问题和解决方案

### 1. Telnet 登录失败 (login fail)

**现象**: 脚本显示 `login fail!` 或密码被当作命令执行

**原因**: 脚本发送密码的时机不对，没有等待 `password:` 提示

**解决方案**: 确保脚本在收到 `password:` 提示后再发送密码，需要正确的状态机逻辑：
```powershell
# 1. 等待 login: 提示
# 2. 发送用户名
# 3. 等待 password: 提示
# 4. 发送密码
# 5. 等待 shell 提示符 [root@sylixos:...]#
```

### 2. "server is full of links"

**现象**: 连接时提示 `server is full of links`

**原因**: 板卡的 telnet 服务器连接数已满（通常限制为 1-2 个并发连接）

**解决方案**:
- 关闭其他占用的 telnet 会话
- 等待几分钟让超时的连接自动断开
- 如有权限，重启板卡的 telnet 服务

### 3. 路径被转换 (MSYS 环境)

**现象**: 命令 `/apps/xxx` 被转换成 `D:/Software/Git/apps/xxx`

**原因**: MSYS/Git Bash 会自动将 Unix 风格路径转换为 Windows 路径

**解决方案**: 设置环境变量禁用路径转换
```bash
MSYS_NO_PATHCONV=1 powershell -File ...
```

### 4. FTP 上传失败 (Access denied: 530)

**现象**: curl 上传时报 `Access denied: 530`

**原因**: FTP 服务器拒绝连接，可能是：
- 认证失败
- 连接数已满
- 服务未启动

**解决方案**:
- 检查用户名密码是否正确
- 等待其他 FTP 连接释放
- 通过 telnet 检查板卡状态

### 5. EPNP workload 崩溃 (Backtrace/PSTATE dump)

**现象**: 运行时出现寄存器 dump 和 Backtrace

**原因**: 栈空间不足，Eigen 库的 JacobiSVD 操作需要大量栈空间

**解决方案**:
- `RTOS-Bench/workloads/EPNP/epnp_bench.cpp` 中的 `THREAD_STACK_SIZE` 已从 5KB 增加到 4MB
- `RTOS-Bench/generator/test_schedule.c` 中的 `SCHED_POSIX_STACK_SIZE` 设为 4MB

### 6. MQTT workload 崩溃 (ptmalloc abort)

**现象**: MQTT 第二次运行时出现 `ptmalloc abort!`

**原因**: mongoose 库内部状态未正确清理，mg_mgr_free() 后再次初始化会导致内存管理器异常

**解决方案**:
- 不要对 MQTT workload 执行 teardown/init 重置
- MQTT 通常因 WCET > 2s 被自动跳过
- 如需测试 MQTT，确保只运行一次或重启进程

### 7. 二进制加载崩溃 (lib_strlen crash)

**现象**: 刚执行命令就崩溃，backtrace 显示 `lib_strlen` -> `API_ModuleLoadEx`

**原因**: 二进制文件损坏或上传不完整

**解决方案**:
- 重新编译: `make clean && make all`
- 重新上传
- 检查 strip 目录下的文件是否完整

## 验证成功标准

测试成功时会输出：
```
Final Score: XX.XX / 100
```

## 测试脚本位置

- `RTOS-Bench/utils/remote-test/telnet-exec.ps1` - Telnet 自动化脚本
- 本文档: `RTOS-Bench/utils/remote-test/boards/BOARDS.md`
