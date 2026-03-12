FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    qemu-system-arm \
    dosfstools \
    git \
    wget \
    tar \
    python3 \
    python3-pip \
    python3-venv \
    tmux \
    && rm -rf /var/lib/apt/lists/*

COPY . /rtos-bench
WORKDIR /rtos-bench

ENV RTT_EXEC_PATH=/rtos-bench/extern/toolchains/xpack-aarch64-none-elf-gcc-14.2.1-1.1/bin
ENV VIRTUAL_ENV=/rtos-bench/extern/.venv
ENV PATH="${RTT_EXEC_PATH}:${VIRTUAL_ENV}/bin:${PATH}"

RUN test -x ${RTT_EXEC_PATH}/aarch64-none-elf-gcc || \
    (echo "ERROR: Toolchain not found. Ensure extern/ directory is included in build context." && exit 1)

RUN test -f ${VIRTUAL_ENV}/bin/scons || \
    (echo "ERROR: Python venv not found. Ensure extern/.venv/ is included in build context." && exit 1)

CMD ["/bin/bash"]
