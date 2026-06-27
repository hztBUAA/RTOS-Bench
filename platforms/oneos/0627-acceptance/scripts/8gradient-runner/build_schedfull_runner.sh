#!/bin/bash
set -e
BASE=/home/hzt/oneos-benchmark-v1.6/OneOS-Benchmark-V1.6/projects
SRC=$BASE/d1h-nezha_out
DST=$BASE/schedfull_runner
CM=/home/hzt/oneos-tools/cmake-3.27.9-linux-x86_64/bin/cmake

echo "[1] clone project skeleton (no RTOS-Bench/build/out)"
rm -rf "$DST"; mkdir -p "$DST"
rsync -a --exclude 'RTOS-Bench' --exclude 'build' --exclude 'out' "$SRC/" "$DST/"
sed -i '/add_subdirectory(RTOS-Bench)/d' "$DST/CMakeLists.txt"

echo "[2] write separate full-schedule runner (finds /user/ctest.out, calls cmd_rtbench_stub full args)"
cat > "$DST/src/hello.c" <<'EOF'
/* Separate full test-schedule runner (mirrors the proven schedrun.out pattern).
 * Finds the loaded /user/ctest.out and calls its cmd_rtbench_stub with full
 * args (no --quick) => 8 gradients (U 30-100 step10), cycles=3.
 * Board workload allowlist (compiled into ctest.out) still applies. */
#include <kernel_config.h>
#include <bsp_config.h>
#include <os_util.h>
#include <os_task.h>
#include <os_module.h>
#include "hello.h"

typedef int (*cmd_func_t)(int argc, char **argv);

void module_task_hello(void *parameter)
{
    struct os_module *handle;
    os_ubase_t addr = 0;
    static char *argv[] = {"rtbench", "test-schedule"};

    os_task_msleep(1000);
    handle = os_module_find("/user/ctest.out");
    if (handle == OS_NULL) { os_kprintf("[schedfull] /user/ctest.out NOT loaded\r\n"); return; }
    if (os_module_symbol_find_by_handle(handle, "cmd_rtbench_stub", &addr) != OS_SUCCESS || addr == 0) {
        os_kprintf("[schedfull] cmd_rtbench_stub NOT found\r\n"); return; }
    os_kprintf("[schedfull] === test-schedule FULL (U30-100 step10 cycles3) ===\r\n");
    ((cmd_func_t)addr)(2, argv);
    os_kprintf("[schedfull] === DONE ===\r\n");
}

int main_init(void)
{
    os_task_id task_id;
    task_id = os_task_create(OS_NULL, OS_NULL, 262144, "schedfull", (void *)&module_task_hello, OS_NULL, 1);
    os_task_startup(task_id);
    return 0;
}
module_init(main_init);
EOF

echo "[3] build"
"$CM" -S "$DST" -B "$DST/build" 2>&1 | tail -2
make -C "$DST/build" -j 2>&1 | tail -5
echo "[4] artifact"
ls -la "$DST/out/"*.out 2>&1
sha256sum "$DST/out/"*.out 2>&1
