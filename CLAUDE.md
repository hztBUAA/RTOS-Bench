# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

RT-Bench is a collection of real-time benchmarks restructured for periodic execution. It supports multiple platforms (Linux, RT-Thread, SylixOS) through a platform abstraction layer.

**Current Status**: RT-Thread support complete and tested on QEMU. SylixOS platform code complete (POSIX-based), verified via Docker compilation.

## Quick Start - One-Click Testing

```bash
# RT-Thread (qemu-virt64-aarch64)
./run-rtthread.sh        # Build and run interactively
./run-rtthread.sh -r     # Run only (skip build)
./run-rtthread.sh -b     # Build only
./run-rtthread.sh -t     # Run with auto-test

# SylixOS (qemu-x86_64)
./run-sylixos.sh -n      # Run in nographic mode
./run-sylixos.sh         # Run with GUI (if available)

# SylixOS compilation (via Docker)
docker run --rm -v "$(pwd):/work" -w /work gcc:13 bash -c "cd IsolBench && make PLATFORM=sylixos"
```

## Build Commands

```bash
# Build documentation
make docs

# Build benchmark suites
make compile-isolbench      # IsolBench (bandwidth/latency)
make compile-tacle          # TACLeBench (WCET)
make compile-vision         # SD-VBS vision benchmarks
make compile-image-filters  # Image filter benchmarks

# Setup submodules
make setup-tacle
make setup-image-filters

# Clean
make clean
```

### Building Individual Benchmarks

Benchmarks link against the generator framework. Example for IsolBench:

```bash
cd IsolBench
make              # Builds bandwidth and latency
```

### Platform-Specific Build

```bash
# Linux (default)
make PLATFORM=linux

# RT-Thread
make PLATFORM=rt-thread RT_THREAD_ROOT=/path/to/rtthread

# SylixOS
make PLATFORM=sylixos
```

### RT-Thread Build Example (qemu-virt64-aarch64)

```bash
export RTT_EXEC_PATH=$PWD/extern/toolchains/xpack-aarch64-none-elf-gcc-14.2.1-1.1/bin
export PATH=$RTT_EXEC_PATH:$PATH
source extern/.venv/bin/activate
cd extern/rt-thread/bsp/qemu-virt64-aarch64
scons -j4  # Generates rtthread.elf/bin
```

### Running on QEMU (RT-Thread)

```bash
/usr/local/bin/qemu-system-aarch64 -M virt,gic-version=2 -cpu cortex-a53 -m 128M -smp 4 -kernel rtthread.bin -nographic

# In msh shell:
rtbench -p 0.5 -b busywait -t 1
```

## Architecture

### Generator Framework (`generator/`)

Core periodic execution framework that benchmarks link against:

- `periodic_benchmark.c/.h` - Main periodic execution loop with timer-based scheduling
- `platform_abstraction.h` - Platform abstraction interface
- `benchmark_registry.c/.h` - Runtime benchmark selection for multi-workload support
- `main.c` - Linux entry point with argp CLI parsing
- `rtthread_entry.c` - RT-Thread entry point (simplified CLI)
- `logging.c/.h` - Logging utilities
- `benchmark_stub.c` / `benchmark_busywait.c` - Built-in test workloads
- `rtthread_fini_stub.c` - Linker symbol fix for RT-Thread

### Platform Abstraction (`generator/platform/`)

Each platform implements these interfaces in separate files:

| File | Provides |
|------|----------|
| `timer.c` | `rtbench_timer_create/settime/delete` |
| `sync.c` | `rtbench_sem_create/wait/post/destroy` |
| `scheduler.c` | `rtbench_set_priority/deadline/affinity` |
| `timestamp.c` | `rtbench_get_rdtsc/timestamp` |
| `signal.c` | `rtbench_signal_register` |

Platform directories: `linux/`, `rt-thread/`, `sylixos/`, `freertos/`

### Benchmark Interface

Each benchmark must implement:

```c
int benchmark_init(int parameters_num, void **parameters);
void benchmark_execution(int parameters_num, void **parameters);
void benchmark_teardown(int parameters_num, void **parameters);

// Optional (with EXTENDED_REPORT):
const char* benchmark_log_header();
float benchmark_log_data();
```

### Benchmark Suites

- `IsolBench/` - Memory interference benchmarks (bandwidth, latency)
- `vision/benchmarks/` - SD-VBS computer vision (disparity, sift, tracking, etc.)
- `rt-tacle-bench/` - WCET benchmarks (git submodule)
- `image-filters/` - Image processing (git submodule)

## Running Benchmarks

```bash
# Example: run bandwidth benchmark
./IsolBench/bandwidth -p 1.0 -d 0.5 -t 10 -c 0 -l 2

# Options:
# -p <sec>    Period
# -d <sec>    Deadline
# -t <count>  Number of tasks/iterations
# -c <cpu>    CPU affinity
# -l <level>  Log level (0=debug, 1=info, 2=warn, 3=error)
# -f <prio>   FIFO priority
# -o <path>   Output file
# -b <name>   Select benchmark (RT-Thread only, e.g., stub, busywait)
```

## Key Patterns

### Platform Detection

```c
#if defined(SYLIXOS_PLATFORM)
    #define RTBENCH_PLATFORM_SYLIXOS
#elif defined(RT_THREAD_PLATFORM)
    #define RTBENCH_PLATFORM_RTTHREAD
#elif defined(LINUX_PLATFORM) || defined(__linux__)
    #define RTBENCH_PLATFORM_LINUX
#endif
```

### Adding RT-Thread Support

1. Copy `platform/rt-thread/*.c.example` to `*.c`
2. Implement platform-specific functions
3. Build with `PLATFORM=rt-thread`

### Multi-Workload Support

Use `benchmark_registry.h` to register multiple benchmarks in one binary:

```c
rtbench_select_benchmark("benchmark_name");
```

Built-in workloads: `stub`, `busywait`

## External Dependencies

- `extern/rt-thread/` - RT-Thread v5.0.2 source with BSPs
- `extern/toolchains/` - Cross-compilation toolchains
  - `xpack-arm-none-eabi-gcc-12.2.1-1.2/` - ARM32
  - `xpack-aarch64-none-elf-gcc-14.2.1-1.1/` - ARM64
- `extern/qemu/` - QEMU source/packages (mainly use system brew QEMU)
- `extern/yihui/` - SylixOS IDE and VMware image

## RT-Thread Integration Notes

Key implementation details:

- **Platform trimming**: When `RT_THREAD_PLATFORM` is defined, only core/platform layer/lightweight entry are compiled; argp/perf/sbrk are excluded
- **Entry point**: `generator/rtthread_entry.c`, exports msh command `rtbench`
- **Timer**: `platform/rt-thread/timer.c` uses thread-driven approach (avoids ISR assertions)
- **Workload switching**: `benchmark_registry` with built-in `stub`/`busywait`, use `-b <name>` parameter
- **BSP integration**: Each BSP's `applications/SConscript` imports rt-bench source and adds `rtbench_entry.c` to export the command
- **Linker fix**: `rtthread_fini_stub.c` resolves `_fini` symbol issues

## SylixOS Integration Notes

SylixOS supports full POSIX API, so the implementation reuses Linux code with minimal changes.

**Platform Code Status**: Complete. The SylixOS platform layer (`generator/platform/sylixos/`) implements POSIX-based timer, sync, scheduler, timestamp, and signal APIs identical to Linux.

**Toolchain Limitation**: The VMware image does not include gcc/make. Compilation requires either:
1. **RealEvo-IDE (Windows)**: Use the official SylixOS IDE with embedded cross-compiler
2. **Install gcc in SylixOS**: Download and install gcc toolchain into the running system
3. **Cross-compile from Linux**: Use a SylixOS-compatible x86 gcc toolchain

**Quick Start (QEMU):**
```bash
# Start SylixOS in QEMU
./run-sylixos.sh -n

# Or manually:
cd extern/yihui/SylixOS\ IDE\ 6.5.0_professional/VMware/SylixOSx86
qemu-system-x86_64 -m 512M -hda sylixos_boot.qcow2 -hdb sylixos_main.qcow2 \
  -net nic -net user,hostfwd=tcp::2222-:22 -nographic
# Exit: Ctrl+A X
```

**If gcc is available in SylixOS:**
```bash
# Transfer files via SCP (if SSH is enabled)
scp -P 2222 -r /path/to/rt-bench root@localhost:/tmp/

# In SylixOS shell
cd /tmp/rt-bench/IsolBench
make PLATFORM=sylixos
./bandwidth -p 0.5 -t 5
```

**Key differences from Linux:**
- No `SCHED_DEADLINE` support (returns -1)
- Uses POSIX `pthread_setschedparam` for priority
- CPU affinity via `pthread_setaffinity_np` (if available)
- `SylixOS.h` include is optional (only needed for SylixOS-specific extensions)

## Adding New Platform Support (e.g., FreeRTOS)

Follow the RT-Thread pattern:

1. Add `PLATFORM=<name>` support in `generator/Makefile`
2. Create `platform/<name>/` directory with:
   - `timer.c`, `sync.c`, `scheduler.c`, `timestamp.c`, `signal.c`
3. Define `<NAME>_PLATFORM` macro and add detection in `platform_abstraction.h`
4. If toolchain lacks symbols/types, use typedef guards (see `platform_abstraction.h`) and stub files (see `rtthread_fini_stub.c`)
5. Integrate in BSP build scripts, define platform macro, import rt-bench sources

Reference implementations:
- `platform/linux/*` - Linux platform (full POSIX)
- `platform/sylixos/*` - SylixOS platform (POSIX-based)
- `platform/rt-thread/*` - RT-Thread platform (native API)
