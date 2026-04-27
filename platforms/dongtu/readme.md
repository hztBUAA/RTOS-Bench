# 东土IDE部署说明

1.在Intewell Developer里点击 文件->新建->项目->InteWell RTOS 选择应用项目

项目名称：rtos_bench（注意不能改）

项目语言：C++

2.git进入项目命令，使用以下命令加入项目

```
cd ~/workspace/rtos-bench
git init  # 如果还不是 git 仓库
git submodule add https://github.com/hztBUAA/RTOS-Bench.git
```

3.生成模板Makefile

右键项目文件夹-> 构建项目

在Debug文件夹（注意在IDE里看不到，需要用其他IDE里看）下会生成make文件夹，包含了构建所需的文件。

4.替换如下文件：

在文件资源管理器中打开项目rtos_bench，用给的 `.cproject`文件替换项目根目录的同名文件夹

5.在IDE里右击项目->属性->C/C++常规->路径和符号

在“包含”选项卡，往下滑能看到include的文件夹（如/rtos_bench/RTOS-Bench/generator）

在“源位置”选项卡，能看到包含的RTOS-Bench文件夹，且展开后过滤器正确排除文件

6.右键项目->构建项目，生成Debug文件夹下的Makefile


7.后续编辑Makefile说明：

建议保留自动生成Makefile选项（项目->属性->C/C++构建，默认确定）

更改编译时，修改项目->属性->C/C++常规->路径和符号（较少更改），或者项目的 `.cproject`文件（批量），并提交git
