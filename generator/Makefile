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

ifeq ($(PLATFORM),rt-thread)
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

COMMON_SRC := $(GENERATOR_DIR)/periodic_benchmark.c \
	$(GENERATOR_DIR)/logging.c \
	$(GENERATOR_DIR)/memory_watcher.c

LINUX_ONLY_SRC := $(GENERATOR_DIR)/main.c \
	$(GENERATOR_DIR)/get_cpu_timestamp.c \
	$(GENERATOR_DIR)/performance_counters.c \
	$(GENERATOR_DIR)/performance_sampler.c

RTTHREAD_ONLY_SRC := $(GENERATOR_DIR)/rtthread_entry.c \
	$(GENERATOR_DIR)/benchmark_stub.c

ifeq ($(PLATFORM),rt-thread)
GENERATOR_SRC := $(COMMON_SRC) $(RTTHREAD_ONLY_SRC) $(PLATFORM_SRC)
else
GENERATOR_SRC := $(COMMON_SRC) $(LINUX_ONLY_SRC) $(PLATFORM_SRC)
endif
