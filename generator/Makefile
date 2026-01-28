ifdef CORE
ifeq ($(CORE),CORTEX_A53)
override CFLAGS +=-DAARCH64 -DCORTEX_A53
endif
ifeq ($(CORE),CORE_I7)
override CFLAGS +=-DX86_64 -DCORE_I7
endif
endif

ifeq ($(JSON),1)
override CFLAGS+=-DJSON_SUPPORT
override LDFLAGS+=-ljson-c
endif

GENERATOR_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

PLATFORM ?= linux

# Include packaged workloads under ../workloads by default.
# Set RTOS_WORKLOADS=0 to disable.
RTOS_WORKLOADS ?= 1
WORKLOADS_DIR := $(abspath $(GENERATOR_DIR)/../workloads)
ifeq ($(RTOS_WORKLOADS),1)
HSW_WORKLOADS_SRC := $(shell find $(WORKLOADS_DIR) -name '*.c' -o -name '*.cpp')
override CFLAGS += -I$(WORKLOADS_DIR)
override LDFLAGS += -lstdc++
endif

ifeq ($(PLATFORM),sylixos)
override CFLAGS += -DSYLIXOS_PLATFORM
ifdef SYLIXOS_ROOT
override CFLAGS += -I$(SYLIXOS_ROOT)/include
endif
PLATFORM_DIR := $(GENERATOR_DIR)/platform/sylixos
# SylixOS is POSIX-compatible but doesn't need --wrap for memory_watcher
override LDFLAGS += -lrt -lm -pthread -Wl,--no-as-needed
else ifeq ($(PLATFORM),oneos)
override CFLAGS += -DONEOS_PLATFORM
PLATFORM_DIR := $(GENERATOR_DIR)/platform/posix-lite
# POSIX-lite: keep link flags minimal to ease cross toolchains
override LDFLAGS += -lm -pthread
else ifeq ($(PLATFORM),dongtu)
override CFLAGS += -DDONGTU_PLATFORM
PLATFORM_DIR := $(GENERATOR_DIR)/platform/posix-lite
override LDFLAGS += -lm -pthread
else ifeq ($(PLATFORM),ruihua)
override CFLAGS += -DRUIHUA_PLATFORM
PLATFORM_DIR := $(GENERATOR_DIR)/platform/posix-lite
override LDFLAGS += -lm -pthread
else ifeq ($(PLATFORM),rt-thread)
override CFLAGS += -DRT_THREAD_PLATFORM
ifdef RT_THREAD_ROOT
override CFLAGS += -I$(RT_THREAD_ROOT)/include
endif
PLATFORM_DIR := $(GENERATOR_DIR)/platform/rt-thread
else
override CFLAGS += -DLINUX_PLATFORM
PLATFORM_DIR := $(GENERATOR_DIR)/platform/linux
override LDFLAGS += -lrt -lm -pthread -Wl,--wrap=malloc -Wl,--wrap=mmap
endif

PLATFORM_SRC := $(PLATFORM_DIR)/timer.c \
		$(PLATFORM_DIR)/sync.c \
		$(PLATFORM_DIR)/timestamp.c \
		$(PLATFORM_DIR)/scheduler.c \
		$(PLATFORM_DIR)/signal.c

# Core source files for all platforms
CORE_SRC := $(GENERATOR_DIR)/periodic_benchmark.c \
	$(GENERATOR_DIR)/logging.c \
	$(GENERATOR_DIR)/memory_watcher.c

# Workload registry (unified for all platforms, always included)
WORKLOAD_REGISTRY_SRC := $(GENERATOR_DIR)/workload_registry.c \
	$(GENERATOR_DIR)/workload_stub.c \
	$(GENERATOR_DIR)/workload_busywait.c

# Linux/SylixOS specific sources
POSIX_ENTRY_SRC := $(GENERATOR_DIR)/main.c \
	$(GENERATOR_DIR)/get_cpu_timestamp.c \
	$(GENERATOR_DIR)/performance_counters.c \
	$(GENERATOR_DIR)/performance_sampler.c

# POSIX-lite entry (no argp/perf) for lightweight RTOS ports
POSIXLITE_ENTRY_SRC := $(GENERATOR_DIR)/posixlite_entry.c

# RT-Thread specific sources
RTTHREAD_ENTRY_SRC := $(GENERATOR_DIR)/rtthread_entry.c \
	$(GENERATOR_DIR)/rtthread_fini_stub.c

# Build mode: MULTI_WORKLOAD=1 enables workload registry for Linux/SylixOS
# RT-Thread always uses workload registry (built-in workloads selected via -b)
# Linux/SylixOS now also always include workload registry (weak symbols allow
# legacy single-benchmark overrides).
WORKLOAD_SRC := $(WORKLOAD_REGISTRY_SRC)
override CFLAGS += -DMULTI_WORKLOAD

ifeq ($(PLATFORM),rt-thread)
# RT-Thread always uses workload registry with built-in workloads
# Note: RT-Thread defines benchmark_* via workload_registry, not from individual benchmark files
GENERATOR_SRC := $(CORE_SRC) $(WORKLOAD_REGISTRY_SRC) $(RTTHREAD_ENTRY_SRC) $(PLATFORM_SRC)
else ifeq ($(PLATFORM),oneos)
# POSIX-lite entry avoids argp/perf for minimal libc toolchains
GENERATOR_SRC := $(CORE_SRC) $(WORKLOAD_SRC) $(POSIXLITE_ENTRY_SRC) $(PLATFORM_SRC)
else ifeq ($(PLATFORM),dongtu)
GENERATOR_SRC := $(CORE_SRC) $(WORKLOAD_SRC) $(POSIXLITE_ENTRY_SRC) $(PLATFORM_SRC)
else ifeq ($(PLATFORM),ruihua)
GENERATOR_SRC := $(CORE_SRC) $(WORKLOAD_SRC) $(POSIXLITE_ENTRY_SRC) $(PLATFORM_SRC)
else ifeq ($(PLATFORM),sylixos)
# SylixOS uses Linux-style entry (main.c with argp)
GENERATOR_SRC := $(CORE_SRC) $(WORKLOAD_SRC) $(POSIX_ENTRY_SRC) $(PLATFORM_SRC)
else
# Linux default
GENERATOR_SRC := $(CORE_SRC) $(WORKLOAD_SRC) $(POSIX_ENTRY_SRC) $(PLATFORM_SRC)
endif

# Add optional packaged workloads (Linux/SylixOS builds). RT-Thread pulls them via SCons.
GENERATOR_SRC += $(HSW_WORKLOADS_SRC)
