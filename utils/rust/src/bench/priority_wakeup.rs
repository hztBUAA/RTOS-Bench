use core::cell::UnsafeCell;
use core::ffi::c_void;
use core::sync::atomic::{AtomicU64, Ordering};

use crate::rtos::{Duration, RtosError, RtosUtils, Semaphore, Task, TaskPriority};
use crate::{bench_elapsed, bench_now};

const ITERATIONS: u32 = 10_000;
const STACK_BYTES: u32 = 4096;

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

struct WakeupCtx {
    wake: Semaphore,
    ack: Semaphore,
    low_start: Semaphore,
    ready: Semaphore,
    done: Semaphore,
    timestamp_us: AtomicU64,
    total_us: AtomicU64,
    min_us: AtomicU64,
    max_us: AtomicU64,
}

impl WakeupCtx {
    const fn new() -> Self {
        Self {
            wake: Semaphore::invalid(),
            ack: Semaphore::invalid(),
            low_start: Semaphore::invalid(),
            ready: Semaphore::invalid(),
            done: Semaphore::invalid(),
            timestamp_us: AtomicU64::new(0),
            total_us: AtomicU64::new(0),
            min_us: AtomicU64::new(u64::MAX),
            max_us: AtomicU64::new(0),
        }
    }
}

static WAKEUP: GlobalCell<WakeupCtx> = GlobalCell::new(WakeupCtx::new());

pub fn bench_priority_wakeup() -> Result<(), RtosError> {
    crate::bench_log!("{{\"benchmark\":\"priority_wakeup\",\"event\":\"start\"}}");

    let ctx = unsafe { &mut *WAKEUP.get() };
    ctx.wake = Semaphore::binary()?;
    ctx.ack = Semaphore::binary()?;
    ctx.low_start = Semaphore::binary()?;
    ctx.ready = Semaphore::counting(2, 0)?;
    ctx.done = Semaphore::binary()?;
    ctx.timestamp_us.store(0, Ordering::Release);
    ctx.total_us.store(0, Ordering::Release);
    ctx.min_us.store(u64::MAX, Ordering::Release);
    ctx.max_us.store(0, Ordering::Release);

    Task::spawn(
        "bench_high",
        STACK_BYTES,
        TaskPriority(4),
        high_task,
        core::ptr::null_mut(),
    )?;
    Task::spawn(
        "bench_low",
        STACK_BYTES,
        TaskPriority(2),
        low_task,
        core::ptr::null_mut(),
    )?;

    ctx.ready.take(Duration::infinite())?;
    ctx.ready.take(Duration::infinite())?;
    ctx.low_start.give()?;
    ctx.done.take(Duration::infinite())?;

    let total_us = ctx.total_us.load(Ordering::Acquire);
    let min_us = ctx.min_us.load(Ordering::Acquire);
    let max_us = ctx.max_us.load(Ordering::Acquire);
    crate::bench_log!(
        "{{\"benchmark\":\"priority_wakeup\",\"iterations\":{},\"total_latency_us\":{},\"avg_latency_us_num\":{},\"avg_latency_us_den\":{},\"min_latency_us\":{},\"max_latency_us\":{},\"unit\":\"us\",\"note\":\"sync-object release plus ready-list update plus scheduling/preemption plus switch\"}}",
        ITERATIONS,
        total_us,
        total_us,
        ITERATIONS,
        min_us,
        max_us
    );

    ctx.wake.destroy();
    ctx.ack.destroy();
    ctx.low_start.destroy();
    ctx.ready.destroy();
    ctx.done.destroy();
    Ok(())
}

extern "C" fn high_task(_arg: *mut c_void) {
    let ctx = unsafe { &mut *WAKEUP.get() };
    let mut total = 0_u64;
    let mut min = u64::MAX;
    let mut max = 0_u64;

    let _ = ctx.ready.give();
    for _ in 0..ITERATIONS {
        let _ = ctx.wake.take(Duration::infinite());
        let t1 = bench_now();
        let t0 = ctx.timestamp_us.load(Ordering::Acquire);
        let latency = bench_elapsed(t0, t1);

        total = total.wrapping_add(latency);
        if latency < min {
            min = latency;
        }
        if latency > max {
            max = latency;
        }

        let _ = ctx.ack.give();
    }

    ctx.total_us.store(total, Ordering::Release);
    ctx.min_us.store(min, Ordering::Release);
    ctx.max_us.store(max, Ordering::Release);
    let _ = ctx.done.give();
}

extern "C" fn low_task(_arg: *mut c_void) {
    let ctx = unsafe { &mut *WAKEUP.get() };
    let _ = ctx.ready.give();
    let _ = ctx.low_start.take(Duration::infinite());

    for _ in 0..ITERATIONS {
        ctx.timestamp_us.store(bench_now(), Ordering::Release);
        let _ = ctx.wake.give();

        for _ in 0..128 {
            RtosUtils::busy_hint();
        }

        let _ = ctx.ack.take(Duration::infinite());
    }
}
