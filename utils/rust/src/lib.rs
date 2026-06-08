#![no_std]

mod bench;
mod rtos;

use core::fmt::{self, Write};
use core::panic::PanicInfo;

use rtos::{RtosAllocator, RtosUtils};

#[global_allocator]
static GLOBAL: RtosAllocator = RtosAllocator;

pub(crate) fn bench_now() -> u64 {
    RtosUtils::now_us()
}

pub(crate) fn bench_elapsed(start: u64, end: u64) -> u64 {
    end.wrapping_sub(start)
}

#[no_mangle]
pub extern "C" fn rtos_bench_run_all() -> i32 {
    log_line(format_args!(
        "{{\"event\":\"start\",\"timer\":\"rtos_port_us\"}}"
    ));

    let result = bench::bench_thread_pingpong()
        .and_then(|_| bench::bench_priority_wakeup())
        .and_then(|_| bench::bench_queue_chain());

    match result {
        Ok(()) => {
            log_line(format_args!("{{\"event\":\"complete\"}}"));
            0
        }
        Err(_) => {
            log_line(format_args!("{{\"event\":\"error\",\"code\":1}}"));
            1
        }
    }
}

pub(crate) fn log_line(args: fmt::Arguments<'_>) {
    let mut buffer = LogBuffer::new();
    let _ = fmt::write(&mut buffer, args);
    buffer.push(b'\n');
    RtosUtils::write(buffer.as_slice());
}

#[macro_export]
macro_rules! bench_log {
    ($($arg:tt)*) => {
        $crate::log_line(format_args!($($arg)*))
    };
}

struct LogBuffer {
    bytes: [u8; 768],
    len: usize,
}

impl LogBuffer {
    const fn new() -> Self {
        Self {
            bytes: [0; 768],
            len: 0,
        }
    }

    fn push(&mut self, byte: u8) {
        if self.len < self.bytes.len() {
            self.bytes[self.len] = byte;
            self.len += 1;
        }
    }

    fn as_slice(&self) -> &[u8] {
        &self.bytes[..self.len]
    }
}

impl Write for LogBuffer {
    fn write_str(&mut self, value: &str) -> fmt::Result {
        for byte in value.as_bytes() {
            self.push(*byte);
        }
        Ok(())
    }
}

#[panic_handler]
fn panic(_info: &PanicInfo<'_>) -> ! {
    log_line(format_args!("{{\"event\":\"panic\"}}"));
    loop {
        core::hint::spin_loop();
    }
}
