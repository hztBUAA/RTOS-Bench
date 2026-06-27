#!/bin/bash
set -e
PROJ=/home/hzt/oneos-benchmark-v1.6/OneOS-Benchmark-V1.6/projects/d1h-nezha_out
H="$PROJ/src/hello.c"

echo "[1] backup + write full-schedule runner hello.c"
[ -f "$H.orig" ] || cp "$H" "$H.orig"
cat > "$H" <<'EOF'
/* Full test-schedule acceptance runner (8 gradients, cycles=3).
 * RTOS-Bench is linked into this .out; its module_init task calls
 * cmd_rtbench_stub("rtbench","test-schedule") with no --quick => full sweep
 * (U 30%-100% step 10%, cycles 3). Board workload allowlist still applies. */
#include <kernel_config.h>
#include <bsp_config.h>
#include <os_util.h>
#include <os_task.h>
#include <os_module.h>
#include "hello.h"

extern int cmd_rtbench_stub(int argc, char **argv);

void module_task_hello(void *parameter)
{
    static char *sched_argv[] = {"rtbench", "test-schedule"};
    os_task_msleep(3000); /* let workload registration settle */
    os_kprintf("[runner] === test-schedule FULL (U 30-100 step10, cycles 3) ===\r\n");
    cmd_rtbench_stub(2, sched_argv);
    os_kprintf("[runner] === test-schedule DONE ===\r\n");
}

int main_init(void)
{
    os_task_id task_id;
    task_id = os_task_create(OS_NULL, OS_NULL, 262144, "rbrunner", (void *)&module_task_hello, OS_NULL, 1);
    os_task_startup(task_id);
    return 0;
}

module_init(main_init);
EOF

echo "[2] build"
make -C "$PROJ/build" -j 2>&1 | tail -5
echo "[3] sha"
sha256sum "$PROJ/out/d1h-nezha_out.out"
