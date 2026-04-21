# 东土IDE部署说明

1.在Intewell Developer里点击 文件->新建->项目->InteWell RTOS 选择应用项目

2.git进入项目命令，使用以下命令加入项目

```
cd ~/workspace/rtos-bench
git init  # 如果还不是 git 仓库
git submodule add https://github.com/hztBUAA/RTOS-Bench.git
```

3.生成模板Makefile

右键项目文件夹-> 构建项目

在Debug文件夹（注意在IDE里看不到，需要用其他IDE里看）下会生成make文件夹，包含了构建所需的文件。

**4.替换如下文件：**

1.把生成的Debug文件夹替换为给的Debug

2.把生成的src文件夹替换为给的src
