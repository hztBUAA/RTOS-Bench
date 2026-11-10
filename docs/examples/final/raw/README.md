# 原始结果上传目录

各板卡的原始 `result.json` 放在本目录下，一块板一个子目录：

```
docs/examples/final/raw/<板卡>/result.json
```

示例：

```
docs/examples/final/raw/Phytium-Pi_OneOS/result.json
docs/examples/final/raw/Orange-Pi5_ReWorks/result.json
```

## 文件要求

- 字段按 `../template_result.json` 填写。
- 必须包含 `env`、`test-realtime`、`test-schedule`、`typical-workload`、`test-stress.power`。
- `env.board` 用官方名：`MIC-7700`、`Orange-Pi5`、`DH-1`、`LS2K1000LA`、`Phytium-Pi`。
- `env.os_name` 用官方名：`SylixOS`、`Intewell`、`ReWorks`、`OneOS`、`RT-Thread`、`openEuler`。
- 功耗缺失时总分按 partial 权重计算，请尽量补齐。

## 提交方式

提 PR 到 main 分支，标题写「data: 新增 <板卡> 结果」。或在群里发给振庭。
