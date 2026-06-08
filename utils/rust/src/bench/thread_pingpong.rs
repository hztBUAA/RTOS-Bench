use core::cell::UnsafeCell;
use core::ffi::c_void;

use crate::rtos::{Duration, RtosError, Semaphore, Task, TaskPriority};
use crate::{bench_elapsed, bench_now};

const ITERATIONS: u32 = 10_000;
const STACK_BYTES: u32 = 4096;
const TASK_PRIORITY: TaskPriority = TaskPriority(3);

struct GlobalCell<T>(UnsafeCell<T>);

unsafe impl<T> Sync for GlobalCell<T> {}

impl<T> GlobalCell<T> {
    const fn new(value: T) -> Self {
        Self(UnsafeCell::new(value))
    }

    unsafe fn get(&self) -> *mut T {
        self.0.get()
    }
}

#[derive(Copy, Clone)]
struct PingPongCtx {
    sem_a: Semaphore,
    sem_b: Semaphore,
    ready: Semaphore,
    done: Semaphore,
    total_us: u64,
}

impl PingPongCtx {
    const fn new() -> Self {
        Self {
            sem_a: Semaphore::invalid(),
            sem_b: Semaphore::invalid(),
            ready: Semaphore::invalid(),
            done: Semaphore::invalid(),
            total_us: 0,
        }
    }
}

static PINGPONG: GlobalCell<PingPongCtx> = GlobalCell::new(PingPongCtx::new());

pub fn bench_thread_pingpong() -> Result<(), RtosError> {
    crate::bench_log!("{{\"benchmark\":\"thread_pingpong\",\"event\":\"start\"}}");

    let ctx = unsafe { &mut *PINGPONG.get() };
    ctx.sem_a = Semaphore::binary()?;
    ctx.sem_b = Semaphore::binary()?;
    ctx.ready = Semaphore::counting(2, 0)?;
    ctx.done = Semaphore::binary()?;
    ctx.total_us = 0;

    Task::spawn(
        "bench_pp_a",
        STACK_BYTES,
        TASK_PRIORITY,
        ping_task_a,
        core::ptr::null_mut(),
    )?;
    Task::spawn(
        "bench_pp_b",
        STACK_BYTES,
        TASK_PRIORITY,
        ping_task_b,
        core::ptr::null_mut(),
    )?;

    ctx.ready.take(Duration::infinite())?;
    ctx.ready.take(Duration::infinite())?;
    ctx.sem_a.give()?;
    ctx.done.take(Duration::infinite())?;

    let total_us = ctx.total_us;
    let switch_den = ITERATIONS as u64 * 2;
    crate::bench_log!(
        "{{\"benchmark\":\"thread_pingpong\",\"iterations\":{},\"total_us\":{},\"avg_round_us_num\":{},\"avg_round_us_den\":{},\"est_switch_us_num\":{},\"est_switch_us_den\":{},\"unit\":\"us\",\"note\":\"semaphore wakeup plus scheduling plus task switch, not pure context-switch cost\"}}",
        ITERATIONS,
        total_us,
        total_us,
        ITERATIONS,
        total_us,
        switch_den
    );

    ctx.sem_a.destroy();
    ctx.sem_b.destroy();
    ctx.ready.destroy();
    ctx.done.destroy();
    Ok(())
}

extern "C" fn ping_task_a(_arg: *mut c_void) {
    let ctx = unsafe { &mut *PINGPONG.get() };
    let _ = ctx.ready.give();
    let _ = ctx.sem_a.take(Duration::infinite());

    let start = bench_now();
    for _ in 0..ITERATIONS {
        let _ = ctx.sem_b.give();
        let _ = ctx.sem_a.take(Duration::infinite());
    }
    let end = bench_now();

    ctx.total_us = bench_elapsed(start, end);
    let _ = ctx.done.give();
}

extern "C" fn ping_task_b(_arg: *mut c_void) {
    let ctx = unsafe { &mut *PINGPONG.get() };
    let _ = ctx.ready.give();

    for _ in 0..ITERATIONS {
        let _ = ctx.sem_b.take(Duration::infinite());
        let _ = ctx.sem_a.give();
    }
}
