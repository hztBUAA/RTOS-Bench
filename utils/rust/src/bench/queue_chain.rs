use core::cell::UnsafeCell;
use core::ffi::c_void;
use core::sync::atomic::{AtomicBool, AtomicU32, AtomicU64, Ordering};

use crate::rtos::{Duration, RtosError, Semaphore, Task, TaskPriority};
use crate::{bench_elapsed, bench_now};

const TASKS: usize = 4;
const ITERATIONS: u32 = 10_000;
const TOTAL_HOPS: u32 = TASKS as u32 * ITERATIONS;
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

struct ChainCtx {
    sems: [Semaphore; TASKS],
    ready: Semaphore,
    start_gate: Semaphore,
    done: Semaphore,
    stopped: Semaphore,
    next_hop: AtomicU32,
    start_us: AtomicU64,
    total_us: AtomicU64,
    stop: AtomicBool,
}

impl ChainCtx {
    const fn new() -> Self {
        Self {
            sems: [Semaphore::invalid(); TASKS],
            ready: Semaphore::invalid(),
            start_gate: Semaphore::invalid(),
            done: Semaphore::invalid(),
            stopped: Semaphore::invalid(),
            next_hop: AtomicU32::new(1),
            start_us: AtomicU64::new(0),
            total_us: AtomicU64::new(0),
            stop: AtomicBool::new(false),
        }
    }
}

#[repr(transparent)]
struct TaskIndex(usize);

static CHAIN: GlobalCell<ChainCtx> = GlobalCell::new(ChainCtx::new());
static TASK_INDEXES: [TaskIndex; TASKS] = [TaskIndex(0), TaskIndex(1), TaskIndex(2), TaskIndex(3)];

pub fn bench_queue_chain() -> Result<(), RtosError> {
    crate::bench_log!("{{\"benchmark\":\"queue_chain\",\"event\":\"start\"}}");

    let ctx = unsafe { &mut *CHAIN.get() };
    for sem in &mut ctx.sems {
        *sem = Semaphore::binary()?;
    }
    ctx.ready = Semaphore::counting(TASKS as u32, 0)?;
    ctx.start_gate = Semaphore::binary()?;
    ctx.done = Semaphore::binary()?;
    ctx.stopped = Semaphore::counting(TASKS as u32, 0)?;
    ctx.next_hop.store(1, Ordering::Release);
    ctx.start_us.store(0, Ordering::Release);
    ctx.total_us.store(0, Ordering::Release);
    ctx.stop.store(false, Ordering::Release);

    for index in 0..TASKS {
        Task::spawn(
            "bench_chain",
            STACK_BYTES,
            TASK_PRIORITY,
            chain_task,
            &TASK_INDEXES[index] as *const TaskIndex as *mut c_void,
        )?;
    }

    for _ in 0..TASKS {
        ctx.ready.take(Duration::infinite())?;
    }

    ctx.start_gate.give()?;
    ctx.done.take(Duration::infinite())?;

    for _ in 0..TASKS {
        ctx.stopped.take(Duration::infinite())?;
    }

    let total_us = ctx.total_us.load(Ordering::Acquire);
    crate::bench_log!(
        "{{\"benchmark\":\"queue_chain\",\"tasks\":{},\"iterations\":{},\"hops\":{},\"total_us\":{},\"avg_hop_us_num\":{},\"avg_hop_us_den\":{},\"unit\":\"us\",\"note\":\"semaphore-ring token transfer plus task wakeup plus scheduling/switch cost\"}}",
        TASKS,
        ITERATIONS,
        TOTAL_HOPS,
        total_us,
        total_us,
        TOTAL_HOPS
    );

    for sem in ctx.sems {
        sem.destroy();
    }
    ctx.ready.destroy();
    ctx.start_gate.destroy();
    ctx.done.destroy();
    ctx.stopped.destroy();
    Ok(())
}

extern "C" fn chain_task(arg: *mut c_void) {
    let index = unsafe { (*(arg as *const TaskIndex)).0 };
    let ctx = unsafe { &mut *CHAIN.get() };
    let _ = ctx.ready.give();

    if index == 0 {
        let _ = ctx.start_gate.take(Duration::infinite());
        ctx.start_us.store(bench_now(), Ordering::Release);
        let _ = ctx.sems[1].give();
    }

    loop {
        let _ = ctx.sems[index].take(Duration::infinite());
        if ctx.stop.load(Ordering::Acquire) {
            let _ = ctx.stopped.give();
            return;
        }

        let hop = ctx.next_hop.fetch_add(1, Ordering::AcqRel);
        if hop >= TOTAL_HOPS {
            let total = bench_elapsed(ctx.start_us.load(Ordering::Acquire), bench_now());
            ctx.total_us.store(total, Ordering::Release);
            ctx.stop.store(true, Ordering::Release);

            for sem in ctx.sems {
                let _ = sem.give();
            }
            let _ = ctx.done.give();
            let _ = ctx.stopped.give();
            return;
        }

        let _ = ctx.sems[(index + 1) % TASKS].give();
    }
}
