#include "rtbench_kernel_probe.h"

#ifdef RTBENCH_KERNEL_INSTRUMENTATION

#include "les.h"

void rtbench_kernel_sched_probe(void)
{
    LES_stub();
}

void rtbench_kernel_irq_end_probe(void)
{
    LES_interrupt_end_stub();
}

void rtbench_kernel_syscall_probe(void)
{
    LES_syscall_stub();
}

#endif /* RTBENCH_KERNEL_INSTRUMENTATION */
