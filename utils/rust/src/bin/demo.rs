#![no_std]
#![no_main]

use core::cell::UnsafeCell;
use core::ffi::c_void;
use core::sync::atomic::{AtomicU64, AtomicUsize, Ordering};

use rtos_rust_bench::rtos_bench_run_all;

const DEMO_SEMAPHORES: usize = 32;
const DEMO_HEAP_SIZE: usize = 64 * 1024;

#[repr(C)]
struct DemoSemaphore {
    count: AtomicU64,
    max: AtomicU64,
}

impl DemoSemaphore {
    const fn new() -> Self {
        Self {
            count: AtomicU64::new(0),
            max: AtomicU64::new(0),
        }
    }
}

struct SyncCell<T>(UnsafeCell<T>);

unsafe impl<T> Sync for SyncCell<T> {}

impl<T> SyncCell<T> {
    const fn new(value: T) -> Self {
        Self(UnsafeCell::new(value))
    }

    fn get(&self) -> *mut T {
        self.0.get()
    }
}

static NEXT_TIME_US: AtomicU64 = AtomicU64::new(1);
static NEXT_SEMAPHORE: AtomicUsize = AtomicUsize::new(0);
static SEMAPHORES: [DemoSemaphore; DEMO_SEMAPHORES] = [
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
    DemoSemaphore::new(),
];
static HEAP_OFFSET: AtomicUsize = AtomicUsize::new(0);
static HEAP: SyncCell<[u8; DEMO_HEAP_SIZE]> = SyncCell::new([0; DEMO_HEAP_SIZE]);

#[no_mangle]
pub extern "C" fn _start() -> ! {
    let _ = rtos_bench_run_all();
    loop {
        core::hint::spin_loop();
    }
}

#[no_mangle]
pub extern "C" fn rtos_bench_thread_spawn(
    entry: extern "C" fn(*mut c_void),
    arg: *mut c_void,
    _priority: u32,
    _stack_bytes: u32,
    _name: *const u8,
    _name_len: usize,
) -> i32 {
    entry(arg);
    0
}

#[no_mangle]
pub extern "C" fn rtos_bench_thread_exit() -> ! {
    loop {
        core::hint::spin_loop();
    }
}

#[no_mangle]
pub extern "C" fn rtos_bench_sem_create(initial: u32, max: u32, out_sem: *mut *mut c_void) -> i32 {
    if out_sem.is_null() {
        return -1;
    }

    let index = NEXT_SEMAPHORE.fetch_add(1, Ordering::AcqRel);
    if index >= DEMO_SEMAPHORES {
        return -1;
    }

    let sem = &SEMAPHORES[index];
    sem.count.store(initial as u64, Ordering::Release);
    sem.max.store(max as u64, Ordering::Release);

    unsafe {
        *out_sem = sem as *const DemoSemaphore as *mut c_void;
    }

    0
}

#[no_mangle]
pub extern "C" fn rtos_bench_sem_take(_sem: *mut c_void, _timeout_us: u64) -> i32 {
    0
}

#[no_mangle]
pub extern "C" fn rtos_bench_sem_give(sem: *mut c_void) -> i32 {
    if sem.is_null() {
        return -1;
    }

    let sem = unsafe { &*(sem as *const DemoSemaphore) };
    let max = sem.max.load(Ordering::Acquire);
    let mut current = sem.count.load(Ordering::Acquire);

    while current < max {
        match sem.count.compare_exchange_weak(
            current,
            current + 1,
            Ordering::AcqRel,
            Ordering::Acquire,
        ) {
            Ok(_) => return 0,
            Err(next) => current = next,
        }
    }

    0
}

#[no_mangle]
pub extern "C" fn rtos_bench_sem_destroy(_sem: *mut c_void) {}

#[no_mangle]
pub extern "C" fn rtos_bench_now_us() -> u64 {
    NEXT_TIME_US.fetch_add(1, Ordering::AcqRel)
}

#[no_mangle]
pub extern "C" fn rtos_bench_write(_ptr: *const u8, _len: usize) {}

#[no_mangle]
pub extern "C" fn rtos_bench_alloc(size: usize) -> *mut c_void {
    if size == 0 {
        return core::ptr::null_mut();
    }

    let align = core::mem::align_of::<usize>();
    let size = (size + align - 1) & !(align - 1);
    let offset = HEAP_OFFSET.fetch_add(size, Ordering::AcqRel);

    if offset + size > DEMO_HEAP_SIZE {
        return core::ptr::null_mut();
    }

    unsafe { (*HEAP.get()).as_mut_ptr().add(offset) as *mut c_void }
}

#[no_mangle]
pub extern "C" fn rtos_bench_free(_ptr: *mut c_void) {}

#[no_mangle]
pub extern "C" fn rtos_bench_yield() {}

#[no_mangle]
pub extern "C" fn rtos_bench_busy_hint() {
    core::hint::spin_loop();
}
