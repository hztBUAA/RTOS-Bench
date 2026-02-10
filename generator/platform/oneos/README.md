# RTOS-Bench OneOS Platform Integration

This directory contains the OneOS platform layer for RTOS-Bench.

## Build System Differences

RTOS-Bench uses **modular SConscript** (RT-Thread style) in the main repository, but **OneOS uses a different build API**.

| Feature | RT-Thread | OneOS |
|---------|-----------|-------|
| Import | `from building import *` | `from build_tools import *` |
| Code Group | `DefineGroup(...)` | `AddCodeGroup(...)` |
| Dependency Check | `GetDepend(['XXX'])` | `IsDefined('XXX')` |
| Current Dir | `GetCurrentDir()` | `PresentDir()` |

## Integration Options

### Option 1: Use Flat SConscript (Recommended)

Copy `SConscript.example` to your project's rtos-bench source directory:

```bash
cp generator/platform/oneos/SConscript.example /path/to/rtos-bench-src/SConscript
```

Then **delete** the RT-Thread style files (if they exist):
- `generator/SConscript`
- `workloads/SConscript`

### Option 2: Modify Build Scripts

If you prefer modular structure, convert all SConscript files to OneOS API.

## Workload Support Matrix

| Workload | Language | Network | C++11 | Eigen | Notes |
|----------|----------|---------|-------|-------|-------|
| CUSUM | C | - | - | - | ✅ Supported |
| EWMA | C | - | - | - | ✅ Supported |
| FAST | C | - | - | - | ✅ Supported |
| PID | C++ | - | - | - | ✅ Supported |
| MODBUS | C | ✅ | - | - | Requires LWIP/Molink |
| MQTT | C | ✅ | - | - | Requires LWIP/Molink |
| EKF | C++ | - | ✅ | - | Uses bundled matrix lib |
| EPNP | C++ | - | ✅ | ✅ | Requires Eigen library |
| ICP | C++ | - | ✅ | ✅ | Requires Eigen library |

## Hardware Requirements

### Network Workloads (MODBUS/MQTT)

Requires Ethernet or cellular network support:

| GD32 Series | Ethernet Driver | Support |
|-------------|-----------------|---------|
| F30x | ❌ | No `drv_eth.c` |
| F3x0 | ❌ | No `drv_eth.c` |
| F4xx | ✅ | Supported |
| F403 | ✅ | Supported |

**Configuration** (in menuconfig):
```
CONFIG_NET_USING_LWIP=y
CONFIG_NET_USING_BSD=y
```

### Compute Workloads (EKF/EPNP/ICP)

- Compiler with C++11 support
- Sufficient Flash/RAM (Eigen is large)
- GD32F303CC (256KB Flash) may be too small for Eigen

## Directory Structure

```
generator/platform/oneos/
├── README.md              # This file
├── SConscript.example     # Flat SConscript template for OneOS
├── include/
│   ├── time.h             # Shadow header (CLOCK_MONOTONIC fix)
│   └── posix_sched_adapter.h
├── timer.c                # Platform timer implementation
├── sync.c                 # Synchronization primitives
├── timestamp.c            # Timestamp functions
├── scheduler.c            # Scheduler interface
├── signal.c               # Signal handling
└── posix_sched_adapter.c  # pthread_attr_setinheritsched() shim
```

## POSIX Compatibility Notes

OneOS POSIX layer has some gaps that this platform layer addresses:

1. **CLOCK_MONOTONIC**: Defined but not implemented in `clock_gettime()`. Shadow `time.h` maps it to `CLOCK_REALTIME`.

2. **pthread_attr_setinheritsched()**: Missing from OneOS. Provided by `posix_sched_adapter.c`.

## Troubleshooting

### Error: Source 'drv_eth.c' not found

Your hardware doesn't support Ethernet. Disable network in `.config`:
```
# CONFIG_NET_USING_LWIP is not set
# CONFIG_NET_USING_BSD is not set
```

### Error: "Please select Molink stack with BSD socket"

BSD sockets enabled without network stack. Either:
- Enable LWIP (requires Ethernet hardware)
- Enable Molink (requires cellular modem)
- Disable BSD sockets

### Build cache issues

Clean SCons cache after config changes:
```bash
rm -rf build/
rm -f .sconsign.dblite
```
