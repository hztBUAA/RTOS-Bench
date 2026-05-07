# Ruihua/ReWorks IDE 工程模板

这个目录提供瑞华 IDE（ReDe/ReWorks）下编译 RTOS-Bench 的工程壳模板。

RTOS-Bench 的瑞华适配源码已经在仓库内维护：

- `generator/ruihua_entry.c`
- `generator/rtbench_command.c`
- `generator/platform/ruihua/*`
- `generator/test_schedule.c`
- `generator/test_realtime.c`
- `generator/test_stress.c`
- `generator/test_cmd.c`
- `generator/result_export.c`

新同学不需要自己写 entry，也不需要手工枚举框架源码。瑞华 IDE 工程只负责设置工具链、SDK 头文件和调用 Makefile。

## 推荐使用方式

1. 克隆最新 RTOS-Bench 仓库。
2. 将 `platforms/ruihua/rtosbench-project-template` 复制到瑞华 workspace，例如：

   `C:/rtos/6.1.1-ARM/workspace/rtosbench`

3. 在复制后的工程目录里，将 `makefile.local.example` 复制为 `makefile.local`。
4. 修改 `makefile.local` 里的 `RTOSBENCH_SRC`，指向自己的 RTOS-Bench 仓库根目录，例如：

   ```makefile
   RTOSBENCH_SRC := D:/workspace/RTOS-Bench
   ```

5. 在瑞华 IDE 中导入或打开 `rtosbench` 工程。
6. 选择 `rtosbench [gnuarm]`，执行 Build Project。

默认输出：

`gnuarm/rtosbench.out`

## 命令行验证

如果当前命令行环境已经能找到瑞华 IDE 的 `gnu_make`，可以在工程目录运行：

```bat
gnu_make ARCH=arm clean all
```

如果 `gnu_make` 只在 IDE 内可见，优先在 IDE 中构建。不要依赖翼辉/RealEvo 的 `make.exe` 作为瑞华正式构建入口。

## 配置说明

- `REDE_HOME`：瑞华 IDE/SDK 安装目录，默认 `C:/rtos/6.1.1-ARM`。
- `RTOSBENCH_SRC`：RTOS-Bench 源码根目录。
- `ARCH`：默认 `arm`，也可设为 `aarch64`。
- `FULL_WORKLOADS`：默认 `0`，只构建框架验收路径：`test-all/test-schedule/test-realtime/test-stress/test-cmd/export-result`。如需尝试完整业务 workload，可设为 `1`，但需要额外处理瑞华 IPNet 与部分 workload 头文件兼容问题。

## 验收入口

加载 `rtosbench.out` 后，可从瑞华 shell 调用：

- `rtbench_help`
- `rtbench_test`
- `rtbench_test_all`
- `rtbench_test_all_quick`
- `rtbench_test_schedule`
- `rtbench_test_schedule_quick`
- `rtbench_test_realtime`
- `rtbench_test_stress`
- `rtbench_test_cmd`
- `rtbench_export_result`

