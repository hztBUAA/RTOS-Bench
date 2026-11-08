#ifndef RTBENCH_KERNEL_PROBE_H__
#define RTBENCH_KERNEL_PROBE_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RTBENCH_KERNEL_INSTRUMENTATION

void rtbench_kernel_sched_probe(void);
void rtbench_kernel_irq_end_probe(void);
void rtbench_kernel_syscall_probe(void);

#endif /* RTBENCH_KERNEL_INSTRUMENTATION */

#ifdef __cplusplus
}
#endif

#endif /* RTBENCH_KERNEL_PROBE_H__ */
