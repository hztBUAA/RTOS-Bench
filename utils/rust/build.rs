use std::env;

const TARGET: &str = "aarch64-unknown-none";

fn main() {
    let target = env::var("TARGET").unwrap_or_default();
    if target != TARGET {
        panic!(
            "rtos-rust-bench staticlib only supports target {}, got {}",
            TARGET, target
        );
    }

    println!("cargo:rerun-if-changed=build.rs");
}
