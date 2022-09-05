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

override LDFLAGS += -lrt -lm -pthread -Wl,--wrap=malloc -Wl,--wrap=mmap
