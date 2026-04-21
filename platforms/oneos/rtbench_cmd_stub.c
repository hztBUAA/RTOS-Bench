#include <os_module.h>
#include <shell.h>
#include <string.h>

extern volatile uint64_t LES_buffer[];
extern volatile uint32_t LES_offset;
extern volatile uint64_t LES_syscall_val;
extern volatile uint64_t LES_interrupt_start_val;
extern volatile uint64_t LES_interrupt_end_val;
extern volatile uint32_t LES_flag;
extern volatile uint32_t LES_syscall_flag;
extern volatile uint32_t LES_interrupt_flag;

EXPORT_SYMBOL(LES_buffer);
EXPORT_SYMBOL(LES_offset);
EXPORT_SYMBOL(LES_syscall_val);
EXPORT_SYMBOL(LES_interrupt_start_val);
EXPORT_SYMBOL(LES_interrupt_end_val);
EXPORT_SYMBOL(LES_flag);
EXPORT_SYMBOL(LES_syscall_flag);
EXPORT_SYMBOL(LES_interrupt_flag);

typedef int (*shell_cmd_func_t)(int argc, char **argv);

int rtbench_stub(int argc, char **argv)
{
    const char *module_name = "/user/phytium_pi_out.out";
    struct os_module *handle;
    os_ubase_t symbol_addr = 0;

    handle = os_module_find(module_name);
    if (handle == OS_NULL)
    {
        os_kprintf("Error: Module '%s' not loaded.\n", module_name);
        os_kprintf("Please run: ld %s first.\n", module_name);
        return -1;
    }

    int err = os_module_symbol_find_by_handle(handle, "cmd_rtbench_stub", &symbol_addr);
    if (err != OS_SUCCESS) {
        os_kprintf("Error: can't execute find_by_handle.\n");
        return -1;
    }
    
    if (symbol_addr != 0)
    {
        shell_cmd_func_t target_cmd = (shell_cmd_func_t)symbol_addr;
        return target_cmd(argc, argv);
    }
    else
    {
        os_kprintf("Error: Symbol 'cmd_rtbench_stub' not found in module.\n");
        return -2;
    }
}

SH_CMD_EXPORT(rtbench, rtbench_stub, "RTOS-Bench runner command");