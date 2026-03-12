# Docker Deployment Guide

This guide shows how to package and deploy RTOS-Bench using Docker.

## Quick Start

```bash
# Build image (~2.8GB, includes toolchain + RT-Thread)
./docker-build.sh

# Run interactive shell
./docker-run.sh

# Run QEMU with RT-Thread
./docker-run.sh -q
```

## Build Image

```bash
docker build -t rtos-bench:latest .
```

**Build time:** 5-10 minutes (depends on network/disk speed)
**Image size:** ~2.8GB (Ubuntu 22.04 + QEMU + toolchain + RT-Thread)

## Run Container

### Interactive bash shell
```bash
docker run -it --rm rtos-bench:latest
```

### Run QEMU with RT-Thread
```bash
docker run -it --rm rtos-bench:latest ./run-rtthread.sh
```

In msh shell:
```
msh /> rtbench test-realtime
msh /> rtbench test-cmd
msh /> rtbench -b busywait -p 0.5 -t 3 -q
```

Press `Ctrl+A X` to exit QEMU.

### Rebuild RT-Thread
```bash
docker run -it --rm rtos-bench:latest ./run-rtthread.sh -b
```

## Automated Testing with tmux

```bash
docker run -it --rm rtos-bench:latest bash -c '
  tmux new-session -d -s qemu "./run-rtthread.sh"
  sleep 5
  tmux send-keys -t qemu "rtbench test-realtime" Enter
  sleep 10
  tmux capture-pane -t qemu -p -S -200
  tmux kill-session -t qemu
'
```

## What's Included

- Ubuntu 22.04 base
- QEMU 6.2.0 (qemu-system-aarch64)
- xPack aarch64-none-elf-gcc 14.2.1
- RT-Thread (shallow clone)
- Python venv with SCons 4.10.1
- Pre-built binaries: `rtthread.bin`, `sd.bin`

## Environment Variables

- `RTT_EXEC_PATH`: Toolchain path
- `VIRTUAL_ENV`: Python venv path
- `PATH`: Includes toolchain and venv bins

## Verification

```bash
# Check toolchain
docker run --rm rtos-bench:latest aarch64-none-elf-gcc --version

# Check SCons
docker run --rm rtos-bench:latest scons --version

# Check QEMU
docker run --rm rtos-bench:latest qemu-system-aarch64 --version
```

## Troubleshooting

**Build fails with "Toolchain not found":**
- Ensure `extern/` directory exists with toolchain before building
- Check `.dockerignore` doesn't exclude `extern/`

**Build fails with "Python venv not found":**
- Ensure `extern/.venv/` exists before building
- Run `./install.sh` first to set up environment

**QEMU doesn't start:**
- Check if `rtthread.bin` exists in BSP directory
- Rebuild: `docker run -it --rm rtos-bench:latest ./run-rtthread.sh -b`

## Notes

- No port exposure needed (QEMU uses serial console)
- Pre-built binaries included, no rebuild required
- Symlinks preserved (BSP `rtos-bench` link works)
- Build context is ~2.5GB, be patient during `docker build`
