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

用给定的userAppInit.c替换src/userAppInit.cpp

5.在IDE里右击项目->属性->C/C++常规->路径和符号

在“包含”选项卡，往下滑能看到include的文件夹（如/rtos_bench/RTOS-Bench/generator）

在“源位置”选项卡，能看到包含的RTOS-Bench文件夹，且展开后过滤器正确排除文件

6.右键项目->构建项目，生成Debug文件夹下的Makefile


7.后续编辑Makefile说明：

建议保留自动生成Makefile选项（项目->属性->C/C++构建，默认确定）

更改编译时，修改项目->属性->C/C++常规->路径和符号（较少更改），或者项目的 `.cproject`文件（批量），并提交git

# 东土IDE运行说明
0.确认输入命令

系统启动后运行工具的命令在src/userAppInit.c中配置:
```aiignore
int userAppInit(void)
{
	VMK_Ioctl(LOG_ADDR_GET, &logAddr, &sysTimerVectorInt);
	debugEventInit((T_VOID *)logAddr, 0x8000*4, 0xffffffff);
	/* argv: argv[0] should be program name, argv[1] the subcommand */
	char *argv[] = { "rtbench", "test-schedule", NULL };
	int argc = 0;
	while (argv[argc] != NULL) {
		argc++;
	}
	rtbench_dongtu_entry(argc, argv);
    return 0;
}
```
1.编译镜像

右键rtos_bench项目根目录，点击构建项目，在Debug/Make目录下会生成rtos_bench.bin二进制

2.上传镜像

在powershell里运行如下命令：
```
ssh -L 5556:192.168.31.240:5556 rtbench@10.134.151.45 -p 1026
#输入密码rtbench   
```
该命令将192.168.31.240:5556的东土IDE转发到`127.0.0.1:5556`上，在浏览器中打开网址：
```aiignore
127.0.0.1:5556
```

在右侧点击实时虚拟机 > rtos_bench 右侧编译按钮 > 在打开的选项卡中点击“镜像”，在右侧上传步骤1中的bin镜像 > 点击“生效”选项卡，在打开的界面中点击“生效”

3.查看日志

在“系统服务配置”中“日志配置”，默认输出到ubuntu的download/log.txt文件里，采用如下方式查看:

在步骤二的cmd命令行ssh进入的rtbench终端中，进入东土工控机的终端：
```aiignore
ssh kd@192.168.31.240
#输入密码1
```
然后用cat查看输出结果
```aiignore
cat /download/log.txt
```

# 目前存在的问题
## 实时负载
userAppInit.c 中的intewell_stub函数没有在/generator/realtime_orig中具体插入，只是用了一个空实现

## 典型负载

EPNP负载报错：
```aiignore
[POSIX] Starting ePnP Benchmark...
assertion "(std::uintptr_t(m_data) % __alignof__ (Scalar) == 0) && "data is not scalar-aligned"" failed: file "A:/IntelWell-IDE/eclipse/workspace/rtos_bench/RTOS-Bench/workloads/EPNP/Eigen/src/Core/MapBase.h", line 191, function: void Eigen::MapBase<Derived, 0>::checkSanity(std::enable_if_t<(Eigen::internal::traits<OtherDerived>::Alignment == 0), void*>) const [with T = Eigen::Block<const Eigen::Matrix<double, 3, 3>, 1, 3, false>; Derived = Eigen::Block<const Eigen::Matrix<double, 3, 3>, 1, 3, false>; std::enable_if_t<(Eigen::internal::traits<OtherDerived>::Alignment == 0), void*> = void*]
```
现在已经在提交(见提交修改的文件)暂时的修复此问题，但是日志输出的结果仍然不对，待进一步核查
```aiignore
[POSIX] Starting ePnP Benchmark...
assertion "(std::uintptr_t(m_data) % __alignof__ (Scalar) == 0) && "data is not scalar-aligned"" failed: file "A:/IntelWell-IDE/eclipse/workspace/rtos_bench/RTOS-Bench/workloads/EPNP/Eigen/src/Core/MapBase.h", line 191, function: void Eigen::MapBase<Derived, 0>::checkSanity(std::enable_if_t<(Eigen::internal::traits<OtherDerived>::Alignment == 0), void*>) const [with T = Eigen::Block<const Eigen::Matrix<double, 3, 3>, 1, 3, false>; Derived = Eigen::Block<const Eigen::Matrix<double, 3, 3>, 1, 3, false>; std::enable_if_t<(Eigen::internal::traits<OtherDerived>::Alignment == 0), void*> = void*]
```

## cmd测试

暂时还不支持，见提交修改，给了一个空实现