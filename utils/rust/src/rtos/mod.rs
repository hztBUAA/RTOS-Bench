use core::alloc::{GlobalAlloc, Layout};
use core::ffi::c_void;

pub type ThreadEntry = extern "C" fn(*mut c_void);

const WAIT_FOREVER_US: u64 = u64::MAX;

#[derive(Copy, Clone, Debug, Eq, PartialEq)]
pub enum RtosError {
    PortError,
}

#[derive(Copy, Clone, Debug)]
pub struct TaskPriority(pub u32);

pub struct Task;

impl Task {
    pub fn spawn(
        name: &str,
        stack_bytes: u32,
        priority: TaskPriority,
        entry: ThreadEntry,
        arg: *mut c_void,
    ) -> Result<(), RtosError> {
        let ret = unsafe {
            rtos_bench_thread_spawn(
                entry,
                arg,
                priority.0,
                stack_bytes,
                name.as_ptr(),
                name.len(),
            )
        };

        if ret == 0 {
            Ok(())
        } else {
            Err(RtosError::PortError)
        }
    }

    #[allow(dead_code)]
    pub fn exit() -> ! {
        unsafe {
            rtos_bench_thread_exit();
        }
    }
}

#[derive(Copy, Clone, Debug, Eq, PartialEq)]
pub struct Duration {
    micros: u64,
}

impl Duration {
    pub const fn infinite() -> Self {
        Self {
            micros: WAIT_FOREVER_US,
        }
    }

    pub const fn as_us(self) -> u64 {
        self.micros
    }
}

#[derive(Copy, Clone, Debug)]
pub struct Semaphore {
    raw: *mut c_void,
}

unsafe impl Send for Semaphore {}
unsafe impl Sync for Semaphore {}

impl Semaphore {
    pub const fn invalid() -> Self {
        Self {
            raw: core::ptr::null_mut(),
        }
    }

    pub fn binary() -> Result<Self, RtosError> {
        Self::create(0, 1)
    }

    pub fn counting(max: u32, initial: u32) -> Result<Self, RtosError> {
        Self::create(initial, max)
    }

    fn create(initial: u32, max: u32) -> Result<Self, RtosError> {
        let mut raw = core::ptr::null_mut();
        let ret = unsafe { rtos_bench_sem_create(initial, max, &mut raw) };
        if ret == 0 && !raw.is_null() {
            Ok(Self { raw })
        } else {
            Err(RtosError::PortError)
        }
    }

    pub fn take(self, timeout: Duration) -> Result<(), RtosError> {
        let ret = unsafe { rtos_bench_sem_take(self.raw, timeout.as_us()) };
        if ret == 0 {
            Ok(())
        } else {
            Err(RtosError::PortError)
        }
    }

    pub fn give(self) -> Result<(), RtosError> {
        let ret = unsafe { rtos_bench_sem_give(self.raw) };
        if ret == 0 {
            Ok(())
        } else {
            Err(RtosError::PortError)
        }
    }

    pub fn destroy(self) {
        if !self.raw.is_null() {
            unsafe {
                rtos_bench_sem_destroy(self.raw);
            }
        }
    }
}

pub struct RtosUtils;

impl RtosUtils {
    pub fn now_us() -> u64 {
        unsafe { rtos_bench_now_us() }
    }

    pub fn write(bytes: &[u8]) {
        unsafe {
            rtos_bench_write(bytes.as_ptr(), bytes.len());
        }
    }

    pub fn busy_hint() {
        unsafe {
            rtos_bench_busy_hint();
        }
    }
}

pub struct RtosAllocator;

unsafe impl GlobalAlloc for RtosAllocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        unsafe { rtos_bench_alloc(layout.size()) as *mut u8 }
    }

    unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
        unsafe {
            rtos_bench_free(ptr as *mut c_void);
        }
    }
}

unsafe extern "C" {
    fn rtos_bench_thread_spawn(
        entry: ThreadEntry,
        arg: *mut c_void,
        priority: u32,
        stack_bytes: u32,
        name: *const u8,
        name_len: usize,
    ) -> i32;
    fn rtos_bench_thread_exit() -> !;

    fn rtos_bench_sem_create(initial: u32, max: u32, out_sem: *mut *mut c_void) -> i32;
    fn rtos_bench_sem_take(sem: *mut c_void, timeout_us: u64) -> i32;
    fn rtos_bench_sem_give(sem: *mut c_void) -> i32;
    fn rtos_bench_sem_destroy(sem: *mut c_void);

    fn rtos_bench_now_us() -> u64;
    fn rtos_bench_write(ptr: *const u8, len: usize);
    fn rtos_bench_alloc(size: usize) -> *mut c_void;
    fn rtos_bench_free(ptr: *mut c_void);
    fn rtos_bench_busy_hint();
}
