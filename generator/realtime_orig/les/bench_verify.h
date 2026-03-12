#ifndef __BENCH_VERIFY_H__
#define __BENCH_VERIFY_H__

void realtime_verify_all(void);
void time_verify(void);
void freq_verify(void);
void interrupt_stub_verify(void);
void schedule_stub_verify(void);
void cpu_bind_verify(void);

#endif