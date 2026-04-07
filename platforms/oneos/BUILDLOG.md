D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2>d:\OS\ZHONGYI\OneOS-V2.0\OneOS_Setup_V2.0\Tools\init_temp.bat > nul && oos build-install Release
-- ONEOS_BUILDER_PATH PATH:    d:/OS/ZHONGYI/OneOS-V2.0/OneOS_Setup_V2.0/Tools
-- ONEOS_TOOLCHAIN_PATH PATH:  d:/OS/ZHONGYI/OneOS-V2.0/OneOS_Setup_V2.0/Tools/toolchain/aarch64-oos-eabi/bin
-- ONEOS_PYTHON_PATH PATH:     d:/OS/ZHONGYI/OneOS-V2.0/OneOS_Setup_V2.0/Tools/Python3
-- Windows系统
-- Toolchain Prefix: aarch64-oos-eabi
-- gcc: D:/OS/ZHONGYI/OneOS-V2.0/OneOS_Setup_V2.0/Tools/toolchain/aarch64-oos-eabi/bin/aarch64-oos-eabi-gcc.exe
-- python: d:/OS/ZHONGYI/OneOS-V2.0/OneOS_Setup_V2.0/Tools/Python3/python
-- 处理 include.txt 中的包含路径...
-- 处理 include.txt 中的包含路径...
-- 配置可执行文件项目: phytium_pi_out_2
-- Configuring done (0.2s)
-- Generating done (0.1s)
-- Build files have been written to: D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi_out_2/build
[  1%] Building C object CMakeFiles/phytium_pi_out_2.dir/RTOS-Bench/generator/periodic_benchmark.c.obj
[  1%] Building C object CMakeFiles/phytium_pi_out_2.dir/RTOS-Bench/generator/platform/oneos/posix_sched_adapter.c.obj
[  2%] Building C object CMakeFiles/phytium_pi_out_2.dir/RTOS-Bench/generator/platform/oneos/scheduler.c.obj
[  3%] Building C object CMakeFiles/phytium_pi_out_2.dir/RTOS-Bench/generator/platform/oneos/sync.c.obj
[  3%] Building C object CMakeFiles/phytium_pi_out_2.dir/RTOS-Bench/generator/platform/oneos/timer.c.obj
[  4%] Building C object CMakeFiles/phytium_pi_out_2.dir/RTOS-Bench/generator/platform/oneos/timestamp.c.obj
[  4%] Building C object CMakeFiles/phytium_pi_out_2.dir/RTOS-Bench/generator/test_schedule.c.obj
[  5%] Building C object CMakeFiles/phytium_pi_out_2.dir/RTOS-Bench/generator/test_stress.c.obj
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\posix_sched_adapter.c: In function 'pthread_attr_setinheritsched':
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\posix_sched_adapter.c:37:9: error: 'pthread_attr_t' has no member named 'inheritsched'
   37 |     attr->inheritsched = inheritsched;
      |         ^~
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\posix_sched_adapter.c: In function 'pthread_attr_getinheritsched':
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\posix_sched_adapter.c:55:25: error: 'pthread_attr_t' has no member named 'inheritsched'
   55 |     *inheritsched = attr->inheritsched;
      |                         ^~
In file included from D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/libc/include/time.h:33,
                 from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\include/time.h:17,
                 from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\periodic_benchmark.c:22:
D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/libc/arch/arm64/include/bits/alltypes.h:217:16: error: conflicting types for 'timer_t'
  217 | typedef void * timer_t;
      |                ^~~~~~~
In file included from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\periodic_benchmark.c:16:
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform_abstraction.h:39:27: note: previous declaration of 'timer_t' was here
   39 |     typedef unsigned long timer_t;
      |                           ^~~~~~~
In file included from D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/libc/include/time.h:33,
                 from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\include/time.h:17,
                 from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\periodic_benchmark.c:22:
D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/libc/arch/arm64/include/bits/alltypes.h:222:13: error: conflicting types for 'clockid_t'
  222 | typedef int clockid_t;
      |             ^~~~~~~~~
In file included from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\periodic_benchmark.c:16:
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform_abstraction.h:35:27: note: mprevious declaration of 'ake 2]: *** [clockid_tC akeFiles\phytium_p' was here
   35 |     typedef unsigned long i_ ut_2.dirclockid_t\ uild.;
      |                           mak :134: CMakeFiles/phytium_^~~~~~~~~pi out_2.dir/R
TOS-BeIn file included from nch/ge eratD:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/libc/include/time.h:33o /p,
                 from la form/oneoD:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\include/time.h:17s po,
                 from si  schD:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\periodic_benchmark.c:22e _adapter.c.obj] Error 1
:
D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/libc/arch/arm64/include/bits/alltypes.h:227:14:make[ 2]  *error: *  Waiting for conflicting types for 'un inished jobclock_ts ..'
  227 | typedef long .
clock_t;
      |              ^~~~~~~
In file included from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\periodic_benchmark.c:16:
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform_abstraction.h:27:27: note: previous declaration of 'clock_t' was here
   27 |     typedef unsigned long clock_t;
      |                           ^~~~~~~
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timestamp.c: In function 'rtbench_get_rdtsc':
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timestamp.c:15:23: error: implicit declaration of function 'os_tick_get'; did you mean 'os_tick_get_value'? [-Werror=implicit-function-declaration]
   15 |     os_tick_t ticks = os_tick_get();
      |                       ^~~~~~~~~~~
      |                       os_tick_get_value
make[2]: *** [CMakeFiles\phytium_pi_out_2.dir\build.make:120: CMakeFiles/phytium_pi_out_2.dir/RTOS-Bench/generator/periodic_benchmark.c.obj] Error 1
cc1.exe: some warnings being treated as errors
In file included from D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/libc/arch/arm64/include/pthread.h:28,
                 from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\test_schedule.c:31:
D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/libc/arch/arm64/include/bits/alltypes.h:222:13: error: conflicting types for 'clockid_t'
  222 | typedef int make[clockid_t2 : ***;
      |              [CM keFiles\p^~~~~~~~~hy iu
m_pi_out_2.dir\build.make:204: CMakeFiles/phytium_pi_out_2.diIn file included from r/ TOD:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\test_schedule.c:17S Bench:
/ge  ratoD:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform_abstraction.h:35:27:r platform/o ne s/timestanote: m .c.obj] Error 1
previous declaration of 'clockid_t' was here
   35 |     typedef unsigned long clockid_t;
      |                           ^~~~~~~~~
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\scheduler.c: In function 'rtbench_set_priority':
In file included from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\scheduler.c:15:5:D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/libc/include/time.h:33 ,    
                 from        D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\include/time.h:17unknown type name ',
                 from os_task_tD:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/libc/arch/arm64/include/pthread.h:31'; did you mean ',
                 from os_task_idD:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\test_schedule.c:31'?
   15 |     :
os_task_tD:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/libc/arch/arm64/include/bits/alltypes.h:217:16: *current = os_task_self();
      |      ^~~~~~~~~error:
      |     conflicting types for 'os_task_idtimer_t
'
  217 | typedef void * timer_t;
      |                ^~~~~~~
In file included from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\test_schedule.c:17:
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform_abstraction.h:39:27: note: previous declaration of 'timer_t' was here
   39 |     typedef unsigned long D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\scheduler.c:15:26:timer_t ;
      |                           error: ^~~~~~~implicit declaration of function '
os_task_selfIn file included from                  D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/libc/include/time.h:33os_task_yield,
                 from '? [D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\include/time.h:17,
                 from -Werror=implicit-function-declarationD:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/libc/arch/arm64/include/pthread.h:31]
   15 |     os_task_t *current = ,
                 from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\test_schedule.c:31os_task_self:
();
                                 D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/libc/arch/arm64/include/bits/alltypes.h:227:14:^~~~~~~~~~~~
      |
clock_t'
  227 | typedef long D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\scheduler.c:15:26:clock_t ;
      |              warning: ^~~~~~~initialization of '
int *In file included from         D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\test_schedule.c:17int:
' makes pointer from integer without a cast [D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform_abstraction.h:27:27: -Wint-conversionnote: ]
previous declaration of 'clock_t' was here
   27 |     typedef unsigned long clock_t;
      |                           ^~~~~~~
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\scheduler.c:20:36: error: 'os_uint8_t' undeclared (first use in this function); did you mean 'uint8_t'?
   20 |     os_task_set_priority(current, (os_uint8_t)priority);
      |                                    ^~~~~~~~~~
      |                                    uint8_t
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\scheduler.c:20:36: note: each undeclared identifier is reported only once for each function it appears in
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\scheduler.c:20:47: error: expected ')' before 'priority'
   20 |     os_task_set_priority(current, (os_uint8_t)priority);
      |                                               ^~~~~~~~
      |                                               D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\sync.c:13:10:)
fatal error: D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\scheduler.c:20:26:os_errno.h: No such file or directory
   13 | #include  <os_errno.h>
      |          passing argument 1 of '
' makes integer from pointer without a cast [ o-Wint-conversionmp]
   20 |     os_task_set_priority(i acurrentti, (os_uint8_t)priority);
      |                          o  ^~~~~~~te
      |                          rm n|at
      |                          e .int *

In file included from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\scheduler.c:11:
D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/kernel/include/os_task.h:148:49: note: expected 'os_task_id' {aka 'int'} but argument is of type 'int *'
  148 | extern os_err_t os_task_set_priority(os_task_id tid, uint8_t new_priority);
      |                                      ~~~~~~~~~~~^~~
cc1.exe: some warnings being treated as errors
make[2]: *** [CMakeFiles\phytium_pi_out_2.dir\build.make:176: CMakeFiles/phytium_pi_out_2.dir/RTOS-Bench/generator/pl tform/D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:17:5:o eo s/s nc.c.obj] Error 1
unknown type name 'os_timer_t'
   17 |     os_timer_tma e[2]: *timer;
      |      * * [CMakeFile^~~~~~~~~~s phytium_pi
_out_2.dir\build.make:260: CMakeFiles/phytium_pi_out_2.dir/RTOS-Be ch/D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:g nerator/test_schedule.c.obj] Error 1
 In function 'rtbench_timer_create':
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:36:5:m ke[2]: *** [CMakeFiles\phytium_pi_out_2.dir\build.mak e:1 8: error: CM keFiles/phytium_pi_out_2.dir/RTunknown type name 'OS- encos_uint8_th/generator'; did you mean '/pl tform/oneos/scheuint8_td ler.c.obj] Error 1
'?
   36 |     os_uint8_t flag;
      |     ^~~~~~~~~~
      |     uint8_t
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:51:32: warning: passing argument 1 of 'os_timer_create' from incompatible pointer type [-Wincompatible-pointer-types]        
   51 |     t->timer = os_timer_create("rtbench", rtbench_timer_dispatch, t, 1, flag);
      |                                ^~~~~~~~~
      |                                |
      |                                const char *
In file included from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:10:
D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/kernel/include/os_timer.h:59:54: note: expected 'os_timer_dummy_t *' {aka 'struct dummy_timer *'} but argument is of type 'const char *'
   59 | extern os_timer_id os_timer_create(os_timer_dummy_t *timer_cb,
      |                                    ~~~~~~~~~~~~~~~~~~^~~~~~~~
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:51:43: warning: passing argument 2 of 'os_timer_create' from incompatible pointer type [-Wincompatible-pointer-types]        
   51 |     t->timer = os_timer_create("rtbench", rtbench_timer_dispatch, t, 1, flag);
      |                                           ^~~~~~~~~~~~~~~~~~~~~~
      |                                           |
      |                                           void (*)(void *)
In file included from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:10:
D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/kernel/include/os_timer.h:60:54: note: expected 'const char *' but argument is of type 'void (*)(void *)'
   60 |                                    const char       *name,
      |                                    ~~~~~~~~~~~~~~~~~~^~~~
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:51:67: warning: passing argument 3 of 'os_timer_create' from incompatible pointer type [-Wincompatible-pointer-types]        
   51 |     t->timer = os_timer_create("rtbench", rtbench_timer_dispatch, t, 1, flag);
      |                                                                   ^
      |                                                                   |
      |                                                                   struct rtbench_timer_internal *
In file included from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:10:
D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/kernel/include/os_timer.h:61:43: note: expected 'void (*)(void *)' but argument is of type 'struct rtbench_timer_internal *'
   61 |                                    void (*function)(void *parameter),
      |                                    ~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:51:70: warning: passing argument 4 of 'os_timer_create' makes pointer from integer without a cast [-Wint-conversion]
   51 |     t->timer = os_timer_create("rtbench", rtbench_timer_dispatch, t, 1, flag);
      |                                                                      ^
      |                                                                      |
      |                                                                      int
In file included from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:10:
D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/kernel/include/os_timer.h:62:46: note: expected 'void *' but argument is of type 'int'
   62 |                                    void     *parameter,
      |                                    ~~~~~~~~~~^~~~~~~~~
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:51:16: error: too few arguments to function 'os_timer_create'
   51 |     t->timer = os_timer_create("rtbench", rtbench_timer_dispatch, t, 1, flag);
      |                ^~~~~~~~~~~~~~~
In file included from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:10:
D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/kernel/include/os_timer.h:59:20: note: declared here
   59 | extern os_timer_id os_timer_create(os_timer_dummy_t *timer_cb,
      |                    ^~~~~~~~~~~~~~~
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c: In function 'rtbench_timer_settime':
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:68:5: error: unknown type name 'os_uint32_t'; did you mean 'uint32_t'?
   68 |     os_uint32_t ms;
      |     ^~~~~~~~~~~
      |     uint32_t
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:75:11: error: 'os_uint32_t' undeclared (first use in this function); did you mean 'uint32_t'?
   75 |     ms = (os_uint32_t)(sec * 1000 + nsec / 1000000);
      |           ^~~~~~~~~~~
      |           uint32_t
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:75:11: note: each undeclared identifier is reported only once for each function it appears in
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:84:20: warning: passing argument 1 of 'os_timer_stop' from incompatible pointer type [-Wincompatible-pointer-types]
   84 |     os_timer_stop(t->timer);
      |                   ~^~~~~~~
      |                    |
      |                    int *
In file included from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:10:
D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/kernel/include/os_timer.h:69:43: note: expected 'os_timer_id' {aka 'struct dummy_timer *'} but argument is of type 'int *'
   69 | extern os_err_t os_timer_stop(os_timer_id timer_id);
      |                               ~~~~~~~~~~~~^~~~~~~~
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:85:33: warning: passing argument 1 of 'os_timer_set_timeout_ticks' from incompatible pointer type [-Wincompatible-pointer-types]
   85 |     os_timer_set_timeout_ticks(t->timer, ticks);
      |                                ~^~~~~~~
      |                                 |
      |                                 int *
In file included from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:10:
D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/kernel/include/os_timer.h:71:57: note: expected 'os_timer_id' {aka 'struct dummy_timer *'} but argument is of type 'int *'
   71 | extern os_err_t  os_timer_set_timeout_ticks(os_timer_id timer_id, os_tick_t timeout);
      |                                             ~~~~~~~~~~~~^~~~~~~~
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:86:21: warning: passing argument 1 of 'os_timer_start' from incompatible pointer type [-Wincompatible-pointer-types]
   86 |     os_timer_start(t->timer);
      |                    ~^~~~~~~
      |                     |
      |                     int *
In file included from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:10:
D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/kernel/include/os_timer.h:68:44: note: expected 'os_timer_id' {aka 'struct dummy_timer *'} but argument is of type 'int *'
   68 | extern os_err_t os_timer_start(os_timer_id timer_id);
      |                                ~~~~~~~~~~~~^~~~~~~~
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c: In function 'rtbench_timer_delete':
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:100:24: warning: passing argument 1 of 'os_timer_stop' from incompatible pointer type [-Wincompatible-pointer-types]
  100 |         os_timer_stop(t->timer);
      |                       ~^~~~~~~
      |                        |
      |                        int *
In file included from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:10:
D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/kernel/include/os_timer.h:69:43: note: expected 'os_timer_id' {aka 'struct dummy_timer *'} but argument is of type 'int *'
   69 | extern os_err_t os_timer_stop(os_timer_id timer_id);
      |                               ~~~~~~~~~~~~^~~~~~~~
D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:101:27: warning: passing argument 1 of 'os_timer_destroy' from incompatible pointer type [-Wincompatible-pointer-types]      
  101 |         os_timer_destroy(t->timer);
      |                          ~^~~~~~~
      |                           |
      |                           int *
In file included from D:\OS\ZHONGYI\OneOS-V2.0\workspaces\armv8-out\phytium_pi_out_2\RTOS-Bench\generator\platform\oneos\timer.c:10:
D:/OS/ZHONGYI/OneOS-V2.0/workspaces/armv8-out/phytium_pi/kernel/h/kernel/include/os_timer.h:66:46: note: expected 'os_timer_id' {aka 'struct dummy_timer *'} but argument is of type 'int *'
   66 | extern os_err_t os_timer_destroy(os_timer_id timer_id);
      |                                  ~~~~~~~~~~~~^~~~~~~~
make[2]: *** [CMakeFiles\phytium_pi_out_2.dir\build.make:190: CMakeFiles/phytium_pi_out_2.dir/RTOS-Bench/generator/platform/oneos/timer.c.obj] Error 1
make[1]: *** [CMakeFiles\Makefile2:131: CMakeFiles/phytium_pi_out_2.dir/all] Error 2
make: *** [Makefile:90: all] Error 2
Error: Build failed