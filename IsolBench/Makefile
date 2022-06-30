CC=gcc
PGMS=latency bandwidth
BASE_SRC=$(wildcard ../rt-bench_generator/*.c)
BASE_O=$(patsubst %.c,%.o,$(BASE_SRC))
CFLAGS=-O2 -Wall -g -I../base -DGCC
CXXFLAGS=$(CFLAGS)
LDFLAGS=-Wl,--no-as-needed -lrt -lm -pthread -Wl,--wrap=malloc -Wl,--wrap=mmap


ifdef CORE
ifeq ($(CORE),CORTEX_A53)
override CFLAGS +=-DAARCH64 -DCORTEX_A53
endif
endif


all: $(PGMS)
latency:latency.o $(BASE_O)
bandwidth: bandwidth.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(BASE_SRC) -o $@
clean:
	rm *.o $(PGMS) $(BASE_O)
